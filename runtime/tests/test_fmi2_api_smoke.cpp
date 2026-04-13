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
    const fs::path csv_path = root / "signals.csv";
    const fs::path model_description_path = root / "modelDescription.xml";

    std::ofstream(csv_path) << "time,temperature,pressure\n0.0,10.0,20.0\n1.0,12.0,24.0\n";
    std::ofstream(model_description_path)
        << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<fmiModelDescription modelName=\"Demo\" fmiVersion=\"2.0\" guid=\"g\" generationTool=\"t\" variableNamingConvention=\"structured\">"
        << "<CoSimulation modelIdentifier=\"Demo\" canHandleVariableCommunicationStepSize=\"true\"/>"
        << "<ModelVariables>"
        << "<ScalarVariable name=\"csv_path\" valueReference=\"0\" variability=\"tunable\" causality=\"parameter\"><String start=\"\"/></ScalarVariable>"
        << "<ScalarVariable name=\"temperature\" valueReference=\"1\" variability=\"continuous\" causality=\"output\"><Real/></ScalarVariable>"
        << "<ScalarVariable name=\"pressure\" valueReference=\"2\" variability=\"continuous\" causality=\"output\"><Real/></ScalarVariable>"
        << "</ModelVariables><ModelStructure><Outputs><Unknown index=\"2\"/><Unknown index=\"3\"/></Outputs></ModelStructure></fmiModelDescription>";

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

    const fmi2ValueReference csv_vr[] = {0};
    const fmi2String csv_value[] = {csv_path.c_str()};
    if (fmi2SetString(component, csv_vr, 1, csv_value) != fmi2OK) {
        return fail("expected setString to succeed");
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

    const fmi2ValueReference output_vrs[] = {1, 2};
    fmi2Real outputs[] = {0.0, 0.0};
    if (fmi2GetReal(component, output_vrs, 2, outputs) != fmi2OK) {
        return fail("expected getReal to succeed after initialization");
    }
    if (outputs[0] != 10.0 || outputs[1] != 20.0) {
        return fail("expected first csv row values at start time");
    }

    if (fmi2DoStep(component, 0.0, 0.5, fmi2True) != fmi2OK) {
        return fail("expected doStep to succeed");
    }
    if (fmi2GetReal(component, output_vrs, 2, outputs) != fmi2OK) {
        return fail("expected getReal after doStep to succeed");
    }
    if (outputs[0] != 11.0 || outputs[1] != 22.0) {
        return fail("expected interpolated values after doStep");
    }

    fmi2FreeInstance(component);
    return EXIT_SUCCESS;
}
