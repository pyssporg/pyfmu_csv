#include "../../3rd_party/fmi_2/headers/fmi2Functions.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

void logger(fmi2ComponentEnvironment, fmi2String, fmi2Status, fmi2String, fmi2String, ...) {
}

int fail(const char* message) {
    std::cerr << message << '\n';
    return EXIT_FAILURE;
}

std::filesystem::path create_fixture_root() {
    const auto root = std::filesystem::temp_directory_path() / "pyfmu_csv_fmi2_api_smoke";
    std::filesystem::create_directories(root / "resources");
    return root;
}

std::string to_file_uri(const std::filesystem::path& path) {
    return "file://" + path.string();
}

}  // namespace

int main() {
    namespace fs = std::filesystem;
    const fs::path root = create_fixture_root();
    const fs::path packaged_csv_path = root / "resources" / "data" / "signals.csv";
    const fs::path override_csv_path = root / "override-signals.csv";

    std::filesystem::create_directories(packaged_csv_path.parent_path());
    std::ofstream(packaged_csv_path) << "time,temperature,count:Integer,enabled:Boolean,mode:String\n0.0,10.0,2,true,auto\n1.0,12.0,4,false,manual\n";
    std::ofstream(override_csv_path) << "time,temperature,count:Integer,enabled:Boolean,mode:String\n0.0,20.0,3,false,override\n1.0,22.0,5,true,override-next\n";

    fmi2CallbackFunctions callbacks {};
    callbacks.logger = logger;
    callbacks.allocateMemory = calloc;
    callbacks.freeMemory = free;

    fmi2Component component = fmi2Instantiate(
        "demo",
        fmi2CoSimulation,
        "g",
        to_file_uri(root / "resources").c_str(),
        &callbacks,
        fmi2False,
        fmi2False);

    if (component == nullptr) {
        return fail("expected instantiate to succeed");
    }

    const fmi2ValueReference pre_init_real_vr[] = {1};
    fmi2Real pre_init_real_output[] = {0.0};
    if (fmi2GetReal(component, pre_init_real_vr, 1, pre_init_real_output) == fmi2OK) {
        return fail("expected getReal before initialization to fail");
    }

    const fmi2ValueReference csv_vr[] = {0};
    const fmi2String packaged_csv_value[] = {"data/signals.csv"};
    if (fmi2SetString(component, csv_vr, 1, packaged_csv_value) != fmi2OK) {
        return fail("expected importer-style default setString to succeed");
    }
    if (fmi2SetupExperiment(component, fmi2False, 0.0, 0.0, fmi2False, 0.0) != fmi2OK) {
        return fail("expected setupExperiment to succeed");
    }
    if (fmi2EnterInitializationMode(component) != fmi2OK) {
        return fail("expected enterInitializationMode to succeed");
    }
    if (fmi2ExitInitializationMode(component) != fmi2OK) {
        return fail("expected exitInitializationMode to succeed");
    }

    const fmi2ValueReference real_vr[] = {1};
    fmi2Real real_output[] = {0.0};
    if (fmi2GetReal(component, real_vr, 1, real_output) != fmi2OK) {
        return fail("expected getReal to succeed after initialization");
    }
    if (real_output[0] != 10.0) {
        return fail("expected first real csv row value at start time");
    }

    fmi2String configured_csv_value[] = {nullptr};
    if (fmi2GetString(component, csv_vr, 1, configured_csv_value) != fmi2OK || std::string(configured_csv_value[0]) != "data/signals.csv") {
        return fail("expected csv parameter getter to expose configured path");
    }

    const fmi2ValueReference integer_vr[] = {2};
    fmi2Integer integer_output[] = {0};
    if (fmi2GetInteger(component, integer_vr, 1, integer_output) != fmi2OK || integer_output[0] != 2) {
        return fail("expected integer getter to succeed");
    }

    const fmi2ValueReference boolean_vr[] = {3};
    fmi2Boolean boolean_output[] = {fmi2False};
    if (fmi2GetBoolean(component, boolean_vr, 1, boolean_output) != fmi2OK || boolean_output[0] != fmi2True) {
        return fail("expected boolean getter to succeed");
    }

    const fmi2ValueReference string_vr[] = {4};
    fmi2String string_output[] = {nullptr};
    if (fmi2GetString(component, string_vr, 1, string_output) != fmi2OK || std::string(string_output[0]) != "auto") {
        return fail("expected string getter to succeed");
    }

    const fmi2ValueReference wrong_type_vr[] = {2};
    if (fmi2GetReal(component, wrong_type_vr, 1, real_output) == fmi2OK) {
        return fail("expected wrong-type getReal to fail");
    }

    const fmi2ValueReference out_of_range_vr[] = {1000};
    if (fmi2GetReal(component, out_of_range_vr, 1, real_output) == fmi2OK) {
        return fail("expected out-of-range getReal to fail");
    }

    if (fmi2DoStep(component, 0.0, 0.5, fmi2True) != fmi2OK) {
        return fail("expected doStep to succeed");
    }
    if (fmi2GetReal(component, real_vr, 1, real_output) != fmi2OK) {
        return fail("expected getReal after doStep to succeed");
    }
    if (real_output[0] != 11.0) {
        return fail("expected interpolated real value after doStep");
    }
    if (fmi2GetInteger(component, integer_vr, 1, integer_output) != fmi2OK || integer_output[0] != 2) {
        return fail("expected piecewise constant integer value after doStep");
    }
    if (fmi2GetBoolean(component, boolean_vr, 1, boolean_output) != fmi2OK || boolean_output[0] != fmi2True) {
        return fail("expected piecewise constant boolean value after doStep");
    }
    if (fmi2GetString(component, string_vr, 1, string_output) != fmi2OK || std::string(string_output[0]) != "auto") {
        return fail("expected piecewise constant string value after doStep");
    }

    fmi2FreeInstance(component);

    component = fmi2Instantiate(
        "override",
        fmi2CoSimulation,
        "g",
        to_file_uri(root / "resources").c_str(),
        &callbacks,
        fmi2False,
        fmi2False);
    if (component == nullptr) {
        return fail("expected override instantiate to succeed");
    }

    const fmi2String csv_override_value[] = {override_csv_path.c_str()};
    if (fmi2SetString(component, csv_vr, 1, csv_override_value) != fmi2OK) {
        return fail("expected setString override to succeed");
    }
    if (fmi2SetupExperiment(component, fmi2False, 0.0, 0.0, fmi2False, 0.0) != fmi2OK) {
        return fail("expected override setupExperiment to succeed");
    }
    if (fmi2EnterInitializationMode(component) != fmi2OK || fmi2ExitInitializationMode(component) != fmi2OK) {
        return fail("expected override initialization to succeed");
    }
    if (fmi2GetReal(component, real_vr, 1, real_output) != fmi2OK || real_output[0] != 20.0) {
        return fail("expected explicit csv_path override to take precedence");
    }
    if (fmi2GetString(component, csv_vr, 1, configured_csv_value) != fmi2OK || std::string(configured_csv_value[0]) != override_csv_path.string()) {
        return fail("expected csv parameter getter to expose explicit override path");
    }

    fmi2FreeInstance(component);
    return EXIT_SUCCESS;
}
