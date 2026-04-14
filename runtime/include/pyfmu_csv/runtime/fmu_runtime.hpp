#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace pyfmu_csv::runtime {

enum class ValueType {
    none,
    real,
    integer,
    boolean,
    string
};

using OutputValue = std::variant<double, std::int64_t, bool, std::string>;

struct OutputBinding {
    std::string name;
    std::size_t value_reference {};
    ValueType value_type {ValueType::none};
};

class FmuRuntime {
public:
    bool load_resource_location(std::string_view resource_location);
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
    bool try_get_integer(std::size_t value_reference, std::int64_t& value) const noexcept;
    bool try_get_boolean(std::size_t value_reference, bool& value) const noexcept;
    bool try_get_string(std::size_t value_reference, const std::string*& value) const noexcept;
    bool is_csv_path_reference(std::size_t value_reference) const noexcept;
    const std::string& last_error() const noexcept;

private:
    void clear_loaded_data();
    bool load_csv_data();
    bool parse_header(const std::vector<std::string>& header);
    const OutputBinding* binding_for(std::size_t value_reference) const noexcept;
    bool has_valid_access(std::size_t value_reference, ValueType expected_type) const noexcept;
    const std::vector<OutputValue>* values_at(std::size_t value_reference) const noexcept;
    std::size_t sample_index_at(double query_time) const noexcept;
    double interpolate_real_value(std::size_t value_reference, double query_time) const noexcept;
    void set_error(std::string message);

    std::string resource_root_;
    std::string csv_path_;
    std::vector<OutputBinding> bindings_;
    std::vector<std::vector<OutputValue>> output_samples_;
    std::vector<double> time_points_;
    double current_time_ {0.0};
    bool initialized_ {false};
    std::string last_error_;
};

}  // namespace pyfmu_csv::runtime
