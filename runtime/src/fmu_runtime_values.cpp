#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <algorithm>

namespace pyfmu_csv::runtime {

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

const OutputBinding* FmuRuntime::binding_for(std::size_t value_reference) const noexcept {
    for (const auto& binding : bindings_) {
        if (binding.value_reference == value_reference) {
            return &binding;
        }
    }
    return nullptr;
}

bool FmuRuntime::has_valid_access(std::size_t value_reference, ValueType expected_type) const noexcept {
    if (!initialized_) {
        return false;
    }
    const auto* binding = binding_for(value_reference);
    return binding != nullptr && binding->value_type == expected_type;
}

const std::vector<OutputValue>* FmuRuntime::values_at(std::size_t value_reference) const noexcept {
    if (value_reference >= output_samples_.size()) {
        return nullptr;
    }
    return &output_samples_[value_reference];
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
        return std::get<double>(samples->front());
    }
    if (query_time <= time_points_.front()) {
        return std::get<double>(samples->front());
    }
    if (query_time >= time_points_.back()) {
        return std::get<double>(samples->back());
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

}  // namespace pyfmu_csv::runtime
