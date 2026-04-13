#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <utility>

namespace pyfmu_csv::runtime {

bool FmuRuntime::initialize(std::string csv_path, std::vector<SignalBinding> bindings) {
    if (csv_path.empty()) {
        initialized_ = false;
        return false;
    }

    csv_path_ = std::move(csv_path);
    bindings_ = std::move(bindings);
    initialized_ = true;
    return true;
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

const std::vector<SignalBinding>& FmuRuntime::bindings() const noexcept {
    return bindings_;
}

}  // namespace pyfmu_csv::runtime
