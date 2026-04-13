#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
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

bool parse_size_t(const std::string& text, std::size_t& value) {
    const char* begin = text.data();
    const char* end = text.data() + text.size();
    auto result = std::from_chars(begin, end, value);
    return result.ec == std::errc {} && result.ptr == end;
}

bool parse_double(const std::string& text, double& value) {
    char* parse_end = nullptr;
    value = std::strtod(text.c_str(), &parse_end);
    return parse_end != text.c_str() && *parse_end == '\0';
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

std::string read_text_file(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream) {
        return {};
    }
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

std::string extract_attribute(const std::string& tag, const std::string& name) {
    const std::string key = name + "=\"";
    const auto start = tag.find(key);
    if (start == std::string::npos) {
        return {};
    }
    const auto value_start = start + key.size();
    const auto value_end = tag.find('"', value_start);
    if (value_end == std::string::npos) {
        return {};
    }
    return tag.substr(value_start, value_end - value_start);
}

}  // namespace

bool FmuRuntime::load_model_description(std::string_view resource_location) {
    const std::filesystem::path resources_path(decode_file_uri(resource_location));
    model_root_ = resources_path.parent_path().string();
    model_loaded_ = parse_model_description();
    initialized_ = false;
    output_samples_.clear();
    time_points_.clear();
    output_index_by_vr_.clear();

    if (!model_loaded_) {
        return false;
    }

    for (std::size_t index = 0; index < model_description_.outputs.size(); ++index) {
        output_index_by_vr_.emplace(model_description_.outputs[index].value_reference, index);
    }
    return true;
}

bool FmuRuntime::set_csv_path(std::string csv_path) {
    csv_path_ = std::move(csv_path);
    return !csv_path_.empty();
}

bool FmuRuntime::initialize() {
    if (!model_loaded_) {
        set_error("modelDescription.xml has not been loaded");
        initialized_ = false;
        return false;
    }
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
    time_points_.clear();
    output_samples_.clear();
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
    return model_description_.outputs.size();
}

const std::vector<OutputBinding>& FmuRuntime::bindings() const noexcept {
    return model_description_.outputs;
}

void FmuRuntime::set_time(double time) noexcept {
    current_time_ = time;
}

double FmuRuntime::time() const noexcept {
    return current_time_;
}

bool FmuRuntime::try_get_real(std::size_t value_reference, double& value) const noexcept {
    const auto match = output_index_by_vr_.find(value_reference);
    if (!initialized_ || match == output_index_by_vr_.end()) {
        return false;
    }
    value = interpolate_value(match->second, current_time_);
    return true;
}

bool FmuRuntime::is_csv_path_reference(std::size_t value_reference) const noexcept {
    return value_reference == 0;
}

const std::string& FmuRuntime::last_error() const noexcept {
    return last_error_;
}

bool FmuRuntime::parse_model_description() {
    const auto model_description_path = std::filesystem::path(model_root_) / "modelDescription.xml";
    const std::string xml = read_text_file(model_description_path);
    if (xml.empty()) {
        set_error("unable to read modelDescription.xml");
        return false;
    }

    model_description_ = {};

    const auto root_pos = xml.find("<fmiModelDescription");
    if (root_pos == std::string::npos) {
        set_error("missing fmiModelDescription root element");
        return false;
    }
    const auto root_end = xml.find('>', root_pos);
    if (root_end == std::string::npos) {
        set_error("malformed fmiModelDescription root element");
        return false;
    }
    model_description_.model_name = extract_attribute(xml.substr(root_pos, root_end - root_pos + 1), "modelName");

    std::size_t search_from = 0;
    while (true) {
        const auto scalar_start = xml.find("<ScalarVariable", search_from);
        if (scalar_start == std::string::npos) {
            break;
        }
        const auto scalar_end = xml.find('>', scalar_start);
        const auto close_tag = xml.find("</ScalarVariable>", scalar_end);
        if (scalar_end == std::string::npos || close_tag == std::string::npos) {
            set_error("malformed ScalarVariable element");
            return false;
        }

        const std::string tag = xml.substr(scalar_start, scalar_end - scalar_start + 1);
        const std::string body = xml.substr(scalar_end + 1, close_tag - scalar_end - 1);
        const std::string causality = extract_attribute(tag, "causality");
        const std::string name = extract_attribute(tag, "name");
        const std::string value_reference_text = extract_attribute(tag, "valueReference");

        std::size_t value_reference = 0;
        if (!value_reference_text.empty() && !parse_size_t(value_reference_text, value_reference)) {
            set_error("invalid valueReference in modelDescription.xml");
            return false;
        }

        if (causality == "parameter" && body.find("<String") != std::string::npos) {
            model_description_.csv_path_parameter_name = name;
        } else if (causality == "output" && body.find("<Real") != std::string::npos) {
            model_description_.outputs.push_back(OutputBinding {name, value_reference, 0});
        }

        search_from = close_tag + std::string("</ScalarVariable>").size();
    }

    if (model_description_.outputs.empty()) {
        set_error("no real output variables found in modelDescription.xml");
        return false;
    }

    return true;
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
    if (header.size() < 2 || header.front() != "time") {
        set_error("csv header must start with a time column");
        return false;
    }

    std::unordered_map<std::string, std::size_t> column_indices;
    for (std::size_t index = 0; index < header.size(); ++index) {
        column_indices.emplace(header[index], index);
    }

    for (auto& output : model_description_.outputs) {
        const auto match = column_indices.find(output.name);
        if (match == column_indices.end()) {
            set_error("csv file is missing an output column from modelDescription.xml");
            return false;
        }
        output.csv_column_index = match->second;
    }

    time_points_.clear();
    output_samples_.assign(model_description_.outputs.size(), {});

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

        for (std::size_t output_index = 0; output_index < model_description_.outputs.size(); ++output_index) {
            const auto& output = model_description_.outputs[output_index];
            double sample = 0.0;
            if (!parse_double(values[output.csv_column_index], sample)) {
                set_error("unable to parse output value from csv");
                return false;
            }
            output_samples_[output_index].push_back(sample);
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

double FmuRuntime::interpolate_value(std::size_t output_index, double query_time) const noexcept {
    const auto& samples = output_samples_[output_index];
    if (time_points_.size() == 1) {
        return samples.front();
    }
    if (query_time <= time_points_.front()) {
        return samples.front();
    }
    if (query_time >= time_points_.back()) {
        return samples.back();
    }

    const auto upper = std::upper_bound(time_points_.begin(), time_points_.end(), query_time);
    const std::size_t upper_index = static_cast<std::size_t>(upper - time_points_.begin());
    const std::size_t lower_index = upper_index - 1;

    const double lower_time = time_points_[lower_index];
    const double upper_time = time_points_[upper_index];
    const double lower_value = samples[lower_index];
    const double upper_value = samples[upper_index];

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
