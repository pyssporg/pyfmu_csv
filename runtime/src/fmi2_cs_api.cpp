#include "pyfmu_csv/runtime/fmu_runtime.hpp"

#include "../../3rd_party/fmi_2/headers/fmi2Functions.h"

#include <cstdint>
#include <new>
#include <string>

namespace {

using pyfmu_csv::runtime::FmuRuntime;

struct InstanceState {
    FmuRuntime runtime;
    fmi2CallbackFunctions callbacks {};
    std::string instance_name;
    bool logging_on {false};
    bool in_initialization_mode {false};
    bool terminated {false};
    fmi2Real start_time {0.0};
    fmi2Real current_time {0.0};
};

InstanceState* as_instance(fmi2Component component) {
    return static_cast<InstanceState*>(component);
}

void log_message(InstanceState* instance, fmi2Status status, const char* category, const std::string& message) {
    if (instance == nullptr || instance->callbacks.logger == nullptr) {
        return;
    }
    instance->callbacks.logger(
        instance->callbacks.componentEnvironment,
        instance->instance_name.c_str(),
        status,
        category,
        message.c_str());
}

fmi2Status report_error(InstanceState* instance, const std::string& message) {
    log_message(instance, fmi2Error, "error", message);
    return fmi2Error;
}

}  // namespace

extern "C" {

const char* fmi2GetTypesPlatform(void) {
    return fmi2TypesPlatform;
}

const char* fmi2GetVersion(void) {
    return fmi2Version;
}

fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t, const fmi2String[]) {
    auto* instance = as_instance(c);
    if (instance == nullptr) {
        return fmi2Error;
    }
    instance->logging_on = loggingOn == fmi2True;
    return fmi2OK;
}

fmi2Component fmi2Instantiate(
    fmi2String instanceName,
    fmi2Type fmuType,
    fmi2String,
    fmi2String fmuResourceLocation,
    const fmi2CallbackFunctions* functions,
    fmi2Boolean,
    fmi2Boolean loggingOn) {
    if (fmuType != fmi2CoSimulation || functions == nullptr) {
        return nullptr;
    }

    auto* instance = new (std::nothrow) InstanceState {};
    if (instance == nullptr) {
        return nullptr;
    }

    instance->callbacks = *functions;
    instance->instance_name = instanceName != nullptr ? instanceName : "pyfmu_csv";
    instance->logging_on = loggingOn == fmi2True;

    if (fmuResourceLocation != nullptr && !instance->runtime.load_resource_location(fmuResourceLocation)) {
        report_error(instance, instance->runtime.last_error().empty() ? "unable to load FMU resources" : instance->runtime.last_error());
        delete instance;
        return nullptr;
    }

    return instance;
}

void fmi2FreeInstance(fmi2Component c) {
    delete as_instance(c);
}

fmi2Status fmi2SetupExperiment(
    fmi2Component c,
    fmi2Boolean,
    fmi2Real,
    fmi2Real startTime,
    fmi2Boolean,
    fmi2Real) {
    auto* instance = as_instance(c);
    if (instance == nullptr) {
        return fmi2Error;
    }
    instance->start_time = startTime;
    instance->current_time = startTime;
    instance->runtime.set_time(startTime);
    return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c) {
    auto* instance = as_instance(c);
    if (instance == nullptr) {
        return fmi2Error;
    }
    instance->in_initialization_mode = true;
    instance->runtime.set_time(instance->start_time);
    return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c) {
    auto* instance = as_instance(c);
    if (instance == nullptr) {
        return fmi2Error;
    }
    if (!instance->runtime.initialize()) {
        return report_error(instance, instance->runtime.last_error());
    }
    instance->in_initialization_mode = false;
    return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component c) {
    auto* instance = as_instance(c);
    if (instance == nullptr) {
        return fmi2Error;
    }
    instance->terminated = true;
    return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component c) {
    auto* instance = as_instance(c);
    if (instance == nullptr) {
        return fmi2Error;
    }
    instance->runtime.reset();
    instance->current_time = 0.0;
    instance->start_time = 0.0;
    instance->in_initialization_mode = false;
    instance->terminated = false;
    return fmi2OK;
}

fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]) {
    auto* instance = as_instance(c);
    if (instance == nullptr || vr == nullptr || value == nullptr) {
        return fmi2Error;
    }

    for (size_t index = 0; index < nvr; ++index) {
        double real_value = 0.0;
        if (!instance->runtime.try_get_real(vr[index], real_value)) {
            return report_error(instance, "unsupported or unavailable real value reference");
        }
        value[index] = real_value;
    }
    return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]) {
    auto* instance = as_instance(c);
    if (instance == nullptr || vr == nullptr || value == nullptr) {
        return fmi2Error;
    }

    for (size_t index = 0; index < nvr; ++index) {
        std::int64_t integer_value = 0;
        if (!instance->runtime.try_get_integer(vr[index], integer_value)) {
            return report_error(instance, "unsupported or unavailable integer value reference");
        }
        value[index] = static_cast<fmi2Integer>(integer_value);
    }
    return fmi2OK;
}

fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]) {
    auto* instance = as_instance(c);
    if (instance == nullptr || vr == nullptr || value == nullptr) {
        return fmi2Error;
    }

    for (size_t index = 0; index < nvr; ++index) {
        bool boolean_value = false;
        if (!instance->runtime.try_get_boolean(vr[index], boolean_value)) {
            return report_error(instance, "unsupported or unavailable boolean value reference");
        }
        value[index] = boolean_value ? fmi2True : fmi2False;
    }
    return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String value[]) {
    auto* instance = as_instance(c);
    if (instance == nullptr || vr == nullptr || value == nullptr) {
        return fmi2Error;
    }

    for (size_t index = 0; index < nvr; ++index) {
        if (instance->runtime.is_csv_path_reference(vr[index])) {
            value[index] = instance->runtime.csv_path().c_str();
            continue;
        }

        const std::string* string_value = nullptr;
        if (!instance->runtime.try_get_string(vr[index], string_value) || string_value == nullptr) {
            return report_error(instance, "unsupported or unavailable string value reference");
        }
        value[index] = string_value->c_str();
    }
    return fmi2OK;
}

fmi2Status fmi2SetReal(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Real[]) {
    return fmi2Error;
}

fmi2Status fmi2SetInteger(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Integer[]) {
    return fmi2Error;
}

fmi2Status fmi2SetBoolean(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Boolean[]) {
    return fmi2Error;
}

fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[]) {
    auto* instance = as_instance(c);
    if (instance == nullptr || vr == nullptr || value == nullptr) {
        return fmi2Error;
    }

    for (size_t index = 0; index < nvr; ++index) {
        if (!instance->runtime.is_csv_path_reference(vr[index]) || value[index] == nullptr) {
            return report_error(instance, "unsupported string parameter");
        }
        if (!instance->runtime.set_csv_path(value[index])) {
            return report_error(instance, "csv_path must not be empty");
        }
    }
    return fmi2OK;
}

fmi2Status fmi2GetFMUstate(fmi2Component, fmi2FMUstate*) { return fmi2Error; }
fmi2Status fmi2SetFMUstate(fmi2Component, fmi2FMUstate) { return fmi2Error; }
fmi2Status fmi2FreeFMUstate(fmi2Component, fmi2FMUstate*) { return fmi2Error; }
fmi2Status fmi2SerializedFMUstateSize(fmi2Component, fmi2FMUstate, size_t*) { return fmi2Error; }
fmi2Status fmi2SerializeFMUstate(fmi2Component, fmi2FMUstate, fmi2Byte[], size_t) { return fmi2Error; }
fmi2Status fmi2DeSerializeFMUstate(fmi2Component, const fmi2Byte[], size_t, fmi2FMUstate*) { return fmi2Error; }
fmi2Status fmi2GetDirectionalDerivative(fmi2Component, const fmi2ValueReference[], size_t, const fmi2ValueReference[], size_t, const fmi2Real[], fmi2Real[]) { return fmi2Error; }

fmi2Status fmi2EnterEventMode(fmi2Component) { return fmi2Error; }
fmi2Status fmi2NewDiscreteStates(fmi2Component, fmi2EventInfo*) { return fmi2Error; }
fmi2Status fmi2EnterContinuousTimeMode(fmi2Component) { return fmi2Error; }
fmi2Status fmi2CompletedIntegratorStep(fmi2Component, fmi2Boolean, fmi2Boolean*, fmi2Boolean*) { return fmi2Error; }
fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time) {
    auto* instance = as_instance(c);
    if (instance == nullptr) {
        return fmi2Error;
    }
    instance->current_time = time;
    instance->runtime.set_time(time);
    return fmi2OK;
}
fmi2Status fmi2SetContinuousStates(fmi2Component, const fmi2Real[], size_t) { return fmi2Error; }
fmi2Status fmi2GetDerivatives(fmi2Component, fmi2Real[], size_t) { return fmi2Error; }
fmi2Status fmi2GetEventIndicators(fmi2Component, fmi2Real[], size_t) { return fmi2Error; }
fmi2Status fmi2GetContinuousStates(fmi2Component, fmi2Real[], size_t) { return fmi2Error; }
fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component, fmi2Real[], size_t) { return fmi2Error; }

fmi2Status fmi2SetRealInputDerivatives(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Integer[], const fmi2Real[]) { return fmi2Error; }
fmi2Status fmi2GetRealOutputDerivatives(fmi2Component, const fmi2ValueReference[], size_t, const fmi2Integer[], fmi2Real[]) { return fmi2Error; }

fmi2Status fmi2DoStep(
    fmi2Component c,
    fmi2Real currentCommunicationPoint,
    fmi2Real communicationStepSize,
    fmi2Boolean) {
    auto* instance = as_instance(c);
    if (instance == nullptr || !instance->runtime.initialized()) {
        return fmi2Error;
    }
    instance->current_time = currentCommunicationPoint + communicationStepSize;
    instance->runtime.set_time(instance->current_time);
    return fmi2OK;
}

fmi2Status fmi2CancelStep(fmi2Component) { return fmi2Error; }

fmi2Status fmi2GetStatus(fmi2Component, const fmi2StatusKind, fmi2Status*) { return fmi2Error; }
fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real* value) {
    auto* instance = as_instance(c);
    if (instance == nullptr || value == nullptr) {
        return fmi2Error;
    }
    if (s == fmi2LastSuccessfulTime) {
        *value = instance->current_time;
        return fmi2OK;
    }
    return fmi2Error;
}
fmi2Status fmi2GetIntegerStatus(fmi2Component, const fmi2StatusKind, fmi2Integer*) { return fmi2Error; }
fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value) {
    auto* instance = as_instance(c);
    if (instance == nullptr || value == nullptr) {
        return fmi2Error;
    }
    if (s == fmi2Terminated) {
        *value = instance->terminated ? fmi2True : fmi2False;
        return fmi2OK;
    }
    return fmi2Error;
}
fmi2Status fmi2GetStringStatus(fmi2Component, const fmi2StatusKind, fmi2String*) { return fmi2Error; }

}  // extern "C"
