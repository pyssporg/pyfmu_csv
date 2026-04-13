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
- output signal discovery
- FMI value-reference assignment
- `modelDescription.xml` generation
- package metadata generation
- `.fmu` archive assembly
- FMPy-based structural validation

The generated FMU is structurally valid for inspection but not yet runnable because the FMI runtime binary is not packaged into the FMU.

## Runtime Contract

- The CSV file remains external to the FMU package.
- The FMU exposes a `csv_path` parameter.
- The runtime is expected to read the file during initialization only.
- Execution should use in-memory signal data only.
