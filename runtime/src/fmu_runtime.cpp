#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>

namespace pyfmu_csv::runtime {
namespace {

std::string trim(std::string value) {
    const auto first = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) { return std::isspace(ch) != 0; });
    const auto last = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) { return std::isspace(ch) != 0; }).base();
    if (first >= last) {
        return {};
    }
    return std::string(first, last);
}

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> values;
    std::stringstream stream(line);
    std::string item;
    while (std::getline(stream, item, ',')) {
        values.push_back(trim(item));
    }
    return values;
}

std::string logical_signal_name(const std::string& header) {
    const auto separator = header.find(':');
    if (separator == std::string::npos) {
        return trim(header);
    }
    return trim(header.substr(0, separator));
}

ValueType parse_header_value_type(const std::string& header) {
    const auto separator = header.find(':');
    if (separator == std::string::npos) {
        return ValueType::real;
    }

    std::string type_name = trim(header.substr(separator + 1));
    std::transform(type_name.begin(), type_name.end(), type_name.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    if (type_name.empty() || type_name == "real") {
        return ValueType::real;
    }
    if (type_name == "integer") {
        return ValueType::integer;
    }
    if (type_name == "boolean") {
        return ValueType::boolean;
    }
    if (type_name == "string") {
        return ValueType::string;
    }
    return ValueType::none;
}

bool parse_double(const std::string& text, double& value) {
    char* parse_end = nullptr;
    value = std::strtod(text.c_str(), &parse_end);
    return parse_end != text.c_str() && *parse_end == '\0';
}

bool parse_integer(const std::string& text, std::int64_t& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc {} && result.ptr == end;
}

bool parse_boolean(const std::string& text, bool& value) {
    std::string lowered;
    lowered.reserve(text.size());
    for (unsigned char ch : text) {
        lowered.push_back(static_cast<char>(std::tolower(ch)));
    }

    if (lowered == "true" || lowered == "1") {
        value = true;
        return true;
    }
    if (lowered == "false" || lowered == "0") {
        value = false;
        return true;
    }
    return false;
}

std::string decode_file_uri(std::string_view location) {
    std::string uri(location);
    const std::string prefix = "file://";
    if (uri.rfind(prefix, 0) != 0) {
        return uri;
    }

    std::string path = uri.substr(prefix.size());
    if (path.size() >= 2 && path[0] == '/' && std::isalpha(static_cast<unsigned char>(path[1])) != 0 && path[2] == ':') {
        path.erase(path.begin());
    }

    std::string decoded;
    decoded.reserve(path.size());
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '%' && i + 2 < path.size()) {
            const std::string hex = path.substr(i + 1, 2);
            char value = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            decoded.push_back(value);
            i += 2;
            continue;
        }
        decoded.push_back(path[i]);
    }
    return decoded;
}

}  // namespace

bool FmuRuntime::load_resource_location(std::string_view resource_location) {
    std::filesystem::path resources_path(decode_file_uri(resource_location));
    resources_path = resources_path.lexically_normal();
    if (!resources_path.empty()) {
        resource_root_ = resources_path.string();
    } else {
        resource_root_.clear();
    }

    initialized_ = false;
    clear_loaded_data();
    last_error_.clear();
    return true;
}

bool FmuRuntime::set_csv_path(std::string csv_path) {
    csv_path_ = std::move(csv_path);
    return !csv_path_.empty();
}

bool FmuRuntime::initialize() {
    if (csv_path_.empty()) {
        set_error("csv_path parameter must be set before initialization");
        initialized_ = false;
        return false;
    }
    initialized_ = load_csv_data();
    return initialized_;
}

void FmuRuntime::reset() {
    csv_path_.clear();
    clear_loaded_data();
    current_time_ = 0.0;
    initialized_ = false;
    last_error_.clear();
}

bool FmuRuntime::initialized() const noexcept {
    return initialized_;
}

const std::string& FmuRuntime::csv_path() const noexcept {
    return csv_path_;
}

std::size_t FmuRuntime::binding_count() const noexcept {
    return bindings_.size();
}

const std::vector<OutputBinding>& FmuRuntime::bindings() const noexcept {
    return bindings_;
}

void FmuRuntime::set_time(double time) noexcept {
    current_time_ = time;
}

double FmuRuntime::time() const noexcept {
    return current_time_;
}

bool FmuRuntime::try_get_real(std::size_t value_reference, double& value) const noexcept {
    if (!has_valid_access(value_reference, ValueType::real)) {
        return false;
    }
    value = interpolate_real_value(value_reference, current_time_);
    return true;
}

bool FmuRuntime::try_get_integer(std::size_t value_reference, std::int64_t& value) const noexcept {
    if (!has_valid_access(value_reference, ValueType::integer)) {
        return false;
    }
    const auto* samples = values_at(value_reference);
    if (samples == nullptr || samples->empty()) {
        return false;
    }
    value = std::get<std::int64_t>((*samples)[sample_index_at(current_time_)]);
    return true;
}

bool FmuRuntime::try_get_boolean(std::size_t value_reference, bool& value) const noexcept {
    if (!has_valid_access(value_reference, ValueType::boolean)) {
        return false;
    }
    const auto* samples = values_at(value_reference);
    if (samples == nullptr || samples->empty()) {
        return false;
    }
    value = std::get<bool>((*samples)[sample_index_at(current_time_)]);
    return true;
}

bool FmuRuntime::try_get_string(std::size_t value_reference, const std::string*& value) const noexcept {
    if (!has_valid_access(value_reference, ValueType::string)) {
        return false;
    }
    const auto* samples = values_at(value_reference);
    if (samples == nullptr || samples->empty()) {
        return false;
    }
    value = &std::get<std::string>((*samples)[sample_index_at(current_time_)]);
    return true;
}

bool FmuRuntime::is_csv_path_reference(std::size_t value_reference) const noexcept {
    return value_reference == 0;
}

const std::string& FmuRuntime::last_error() const noexcept {
    return last_error_;
}

void FmuRuntime::clear_loaded_data() {
    bindings_.clear();
    output_samples_.clear();
    time_points_.clear();
}

bool FmuRuntime::parse_header(const std::vector<std::string>& header) {
    if (header.size() < 2 || logical_signal_name(header.front()) != "time") {
        set_error("csv header must start with a time column");
        return false;
    }

    clear_loaded_data();
    output_samples_.resize(header.size());
    bindings_.reserve(header.size() - 1);

    for (std::size_t index = 1; index < header.size(); ++index) {
        const std::size_t vr = index;
        const std::string name = logical_signal_name(header[index]);
        if (name.empty()) {
            set_error("signal names must not be empty");
            return false;
        }

        const ValueType value_type = parse_header_value_type(header[index]);
        if (value_type == ValueType::none) {
            set_error("unsupported signal type in csv header");
            return false;
        }

        bindings_.push_back(OutputBinding {name, vr, value_type});
    }

    return true;
}

bool FmuRuntime::has_valid_access(std::size_t value_reference, ValueType expected_type) const noexcept {
    if (!initialized_) {
        return false;
    }
    // TODO: Non loop solution
    for (const auto& binding : bindings_) {
        if (binding.value_reference == value_reference) {
            return binding.value_type == expected_type;
        }
    }
    return false;
}

const std::vector<OutputValue>* FmuRuntime::values_at(std::size_t value_reference) const noexcept {
    if (value_reference >= output_samples_.size()) {
        return nullptr;
    }
    return &output_samples_[value_reference];
}

bool FmuRuntime::load_csv_data() {
    std::ifstream stream(csv_path_);
    if (!stream) {
        set_error("unable to open csv file");
        return false;
    }

    std::string line;
    if (!std::getline(stream, line)) {
        set_error("csv file is empty");
        return false;
    }

    const std::vector<std::string> header = split_csv_line(line);
    if (!parse_header(header)) {
        return false;
    }

    while (std::getline(stream, line)) {
        if (trim(line).empty()) {
            continue;
        }
        const std::vector<std::string> values = split_csv_line(line);
        if (values.size() != header.size()) {
            set_error("csv row width does not match header width");
            return false;
        }

        double time_value = 0.0;
        if (!parse_double(values[0], time_value)) {
            set_error("unable to parse time value from csv");
            return false;
        }
        time_points_.push_back(time_value);

        for (const auto& binding : bindings_) {
            const std::string& cell = values[binding.value_reference];
            switch (binding.value_type) {
            case ValueType::real: {
                double sample = 0.0;
                if (!parse_double(cell, sample)) {
                    set_error("unable to parse real output value from csv");
                    return false;
                }
                output_samples_[binding.value_reference].emplace_back(sample);
                break;
            }
            case ValueType::integer: {
                std::int64_t sample = 0;
                if (!parse_integer(cell, sample)) {
                    set_error("unable to parse integer output value from csv");
                    return false;
                }
                output_samples_[binding.value_reference].emplace_back(sample);
                break;
            }
            case ValueType::boolean: {
                bool sample = false;
                if (!parse_boolean(cell, sample)) {
                    set_error("unable to parse boolean output value from csv");
                    return false;
                }
                output_samples_[binding.value_reference].emplace_back(sample);
                break;
            }
            case ValueType::string:
                output_samples_[binding.value_reference].emplace_back(cell);
                break;
            case ValueType::none:
                set_error("unconfigured value reference encountered while loading csv");
                return false;
            }
        }
    }

    if (time_points_.empty()) {
        set_error("csv file contains no data rows");
        return false;
    }

    current_time_ = time_points_.front();
    last_error_.clear();
    return true;
}

std::size_t FmuRuntime::sample_index_at(double query_time) const noexcept {
    if (time_points_.size() == 1 || query_time <= time_points_.front()) {
        return 0;
    }
    if (query_time >= time_points_.back()) {
        return time_points_.size() - 1;
    }

    const auto upper = std::upper_bound(time_points_.begin(), time_points_.end(), query_time);
    return static_cast<std::size_t>(upper - time_points_.begin()) - 1;
}

double FmuRuntime::interpolate_real_value(std::size_t value_reference, double query_time) const noexcept {
    const auto* samples = values_at(value_reference);
    if (samples == nullptr || samples->empty()) {
        return 0.0;
    }
    if (time_points_.size() == 1) {
        return std::get<double>((*samples).front());
    }
    if (query_time <= time_points_.front()) {
        return std::get<double>((*samples).front());
    }
    if (query_time >= time_points_.back()) {
        return std::get<double>((*samples).back());
    }

    const auto upper = std::upper_bound(time_points_.begin(), time_points_.end(), query_time);
    const std::size_t upper_index = static_cast<std::size_t>(upper - time_points_.begin());
    const std::size_t lower_index = upper_index - 1;

    const double lower_time = time_points_[lower_index];
    const double upper_time = time_points_[upper_index];
    const double lower_value = std::get<double>((*samples)[lower_index]);
    const double upper_value = std::get<double>((*samples)[upper_index]);

    if (upper_time == lower_time) {
        return upper_value;
    }

    const double ratio = (query_time - lower_time) / (upper_time - lower_time);
    return lower_value + (upper_value - lower_value) * ratio;
}

void FmuRuntime::set_error(std::string message) {
    last_error_ = std::move(message);
}

}  // namespace pyfmu_csv::runtime
