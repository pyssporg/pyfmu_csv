#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

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

}  // namespace

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

bool FmuRuntime::load_csv_data(std::string_view csv_path) {
    std::ifstream stream {std::string(csv_path)};
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

}  // namespace pyfmu_csv::runtime
