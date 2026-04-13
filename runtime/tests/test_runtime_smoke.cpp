#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

int fail(const char* message) {
    std::cerr << message << '\n';
    return EXIT_FAILURE;
}

std::filesystem::path create_fixture_root(const std::string& test_name) {
    const auto root = std::filesystem::temp_directory_path() / test_name;
    std::filesystem::create_directories(root / "resources");
    return root;
}

std::string to_file_uri(const std::filesystem::path& path) {
    return "file://" + path.string();
}

}  // namespace

int main() {
    namespace fs = std::filesystem;
    using pyfmu_csv::runtime::FmuRuntime;

    const fs::path root = create_fixture_root("pyfmu_csv_runtime_smoke");
    const fs::path csv_path = root / "signals.csv";
    const fs::path model_description_path = root / "modelDescription.xml";

    std::ofstream(csv_path) << "time,temperature,count,enabled,mode\n0.0,10.0,2,true,auto\n1.0,12.0,4,false,manual\n";
    std::ofstream(model_description_path)
        << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        << "<fmiModelDescription modelName=\"Demo\" fmiVersion=\"2.0\" guid=\"g\" generationTool=\"t\" variableNamingConvention=\"structured\">"
        << "<CoSimulation modelIdentifier=\"Demo\"/>"
        << "<ModelVariables>"
        << "<ScalarVariable name=\"csv_path\" valueReference=\"0\" variability=\"tunable\" causality=\"parameter\"><String start=\"\"/></ScalarVariable>"
        << "<ScalarVariable name=\"temperature\" valueReference=\"1\" variability=\"continuous\" causality=\"output\"><Real/></ScalarVariable>"
        << "<ScalarVariable name=\"count\" valueReference=\"2\" variability=\"discrete\" causality=\"output\"><Integer/></ScalarVariable>"
        << "<ScalarVariable name=\"enabled\" valueReference=\"3\" variability=\"discrete\" causality=\"output\"><Boolean/></ScalarVariable>"
        << "<ScalarVariable name=\"mode\" valueReference=\"4\" variability=\"discrete\" causality=\"output\"><String/></ScalarVariable>"
        << "</ModelVariables><ModelStructure/></fmiModelDescription>";

    FmuRuntime runtime;
    if (!runtime.load_model_description(to_file_uri(root / "resources"))) {
        return fail("expected model description loading to succeed");
    }
    if (!runtime.set_csv_path(csv_path.string())) {
        return fail("expected csv path assignment to succeed");
    }
    if (!runtime.initialize()) {
        return fail(runtime.last_error().c_str());
    }
    if (!runtime.initialized()) {
        return fail("runtime should report initialized state");
    }
    if (runtime.binding_count() != 4) {
        return fail("runtime should expose output bindings from the xml");
    }

    runtime.set_time(0.5);
    double real_value = 0.0;
    if (!runtime.try_get_real(1, real_value)) {
        return fail("expected real output lookup to succeed");
    }
    if (real_value != 11.0) {
        return fail("expected interpolated temperature value");
    }
    std::int64_t integer_value = 0;
    if (!runtime.try_get_integer(2, integer_value) || integer_value != 2) {
        return fail("expected piecewise constant integer value");
    }
    bool boolean_value = false;
    if (!runtime.try_get_boolean(3, boolean_value) || !boolean_value) {
        return fail("expected piecewise constant boolean value");
    }
    const std::string* string_value = nullptr;
    if (!runtime.try_get_string(4, string_value) || string_value == nullptr || *string_value != "auto") {
        return fail("expected piecewise constant string value");
    }

    return EXIT_SUCCESS;
}
