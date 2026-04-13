# Overview

## Goal

`pyfmu_csv` is intended to convert CSV-described output signals into FMI 2.0 Co-Simulation FMU packages that can be consumed by generic simulation tooling such as OMSimulator and FMPy.

## Product Boundary

The project has three main parts:

- a reusable C++ runtime artifact
- a Python generator and packaging layer
- a final `.fmu` package assembled per model

## Current State

The current implementation provides an initial end-to-end product for packaging:

- CSV header parsing
- header type annotation parsing with `Real` fallback
- output signal discovery
- FMI value-reference assignment
- `modelDescription.xml` generation
- package metadata generation
- `.fmu` archive assembly
- reusable FMI 2.0 Co-Simulation runtime implementation
- FMI 2.0 C API exports for instantiate/init/step/get/set
- runtime binary packaging into the generated FMU
- end-to-end execution through FMPy
- FMPy-based structural validation

The current generated FMU is executable when:

- the native runtime has been built first
- the simulator supports the packaged host platform
- the external `csv_path` start value is provided before initialization

## Runtime Contract

- The CSV file remains external to the FMU package.
- The FMU exposes a `csv_path` parameter.
- The runtime is expected to read the file during initialization only.
- Execution uses in-memory signal data only.
- `Real` outputs are linearly interpolated.
- `Integer`, `Boolean`, and `String` outputs are piecewise constant between timestamps.
