#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

namespace {

int fail(const char* message) {
    std::cerr << message << '\n';
    return EXIT_FAILURE;
}

}  // namespace

int main() {
    using pyfmu_csv::runtime::FmuRuntime;
    using pyfmu_csv::runtime::SignalBinding;
    using pyfmu_csv::runtime::ValueType;

    FmuRuntime runtime;
    std::vector<SignalBinding> bindings {
        {"temperature", 0, ValueType::real},
        {"pressure", 1, ValueType::real},
    };

    if (!runtime.initialize("samples/signals.csv", bindings)) {
        return fail("expected runtime initialization to succeed");
    }

    if (!runtime.initialized()) {
        return fail("runtime should report initialized state");
    }

    if (runtime.csv_path() != "samples/signals.csv") {
        return fail("runtime should retain the configured csv path");
    }

    if (runtime.binding_count() != 2) {
        return fail("runtime should retain binding metadata");
    }

    return EXIT_SUCCESS;
}
