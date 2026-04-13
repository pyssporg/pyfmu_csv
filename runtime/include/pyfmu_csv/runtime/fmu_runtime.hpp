#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace pyfmu_csv::runtime {

enum class ValueType {
    real,
    integer,
    boolean,
    string
};

struct SignalBinding {
    std::string name;
    std::size_t value_reference {};
    ValueType value_type {ValueType::real};
};

class FmuRuntime {
public:
    bool initialize(std::string csv_path, std::vector<SignalBinding> bindings);

    bool initialized() const noexcept;
    const std::string& csv_path() const noexcept;
    std::size_t binding_count() const noexcept;
    const std::vector<SignalBinding>& bindings() const noexcept;

private:
    std::string csv_path_;
    std::vector<SignalBinding> bindings_;
    bool initialized_ {false};
};

}  // namespace pyfmu_csv::runtime
