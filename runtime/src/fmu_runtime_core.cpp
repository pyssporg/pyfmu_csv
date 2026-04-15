#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <string_view>

namespace pyfmu_csv::runtime {
namespace {

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

// TODO: 
// Look over how the default path is setup in the model description and compare how it should be read
// hard to access the ssp resources from the fmu at the moment

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

    const auto data_dir = resources_path / "data";
    if (std::filesystem::is_directory(data_dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(data_dir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".csv") {
                continue;
            }
            csv_path_ = (std::filesystem::path("data") / entry.path().filename()).string();
            initialized_ = load_csv_data(resolve_csv_path(csv_path_));
            break;
        }
    }
    return true;
}

bool FmuRuntime::set_csv_path(std::string csv_path) {
    csv_path_ = std::move(csv_path);
    clear_loaded_data();
    initialized_ = false;
    last_error_.clear();

    if (csv_path_.empty()) {
        return false;
    }

    initialized_ = load_csv_data(resolve_csv_path(csv_path_));
    return initialized_;
}

bool FmuRuntime::initialize() {
    if (csv_path_.empty()) {
        set_error("csv_path parameter must be set before initialization");
        initialized_ = false;
        return false;
    }
    initialized_ = load_csv_data(resolve_csv_path(csv_path_));
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

void FmuRuntime::set_error(std::string message) {
    last_error_ = std::move(message);
}

std::string FmuRuntime::resolve_csv_path(std::string_view csv_path) const {
    std::filesystem::path path(csv_path);
    if (path.is_absolute() || resource_root_.empty()) {
        return path.string();
    }
    return (std::filesystem::path(resource_root_) / path).lexically_normal().string();
}

}  // namespace pyfmu_csv::runtime
