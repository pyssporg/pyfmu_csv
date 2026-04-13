#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace pyfmu_csv::runtime {

struct OutputBinding {
    std::string name;
    std::size_t value_reference {};
    std::size_t csv_column_index {};
};

struct ModelDescription {
    std::string model_name;
    std::string csv_path_parameter_name {"csv_path"};
    std::vector<OutputBinding> outputs;
};

class FmuRuntime {
public:
    bool load_model_description(std::string_view resource_location);
    bool set_csv_path(std::string csv_path);
    bool initialize();
    void reset();

    bool initialized() const noexcept;
    const std::string& csv_path() const noexcept;
    std::size_t binding_count() const noexcept;
    const std::vector<OutputBinding>& bindings() const noexcept;

    void set_time(double time) noexcept;
    double time() const noexcept;
    bool try_get_real(std::size_t value_reference, double& value) const noexcept;
    bool is_csv_path_reference(std::size_t value_reference) const noexcept;
    const std::string& last_error() const noexcept;

private:
    bool parse_model_description();
    bool load_csv_data();
    double interpolate_value(std::size_t output_index, double query_time) const noexcept;
    void set_error(std::string message);

    std::string model_root_;
    std::string csv_path_;
    ModelDescription model_description_;
    std::unordered_map<std::size_t, std::size_t> output_index_by_vr_;
    std::vector<double> time_points_;
    std::vector<std::vector<double>> output_samples_;
    double current_time_ {0.0};
    bool model_loaded_ {false};
    bool initialized_ {false};
    std::string last_error_;
};

}  // namespace pyfmu_csv::runtime
