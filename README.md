# pyfmu_csv

`pyfmu_csv` is a planned toolchain for turning CSV-described signals into a packaged FMU that can run in generic FMI-compatible simulation environments such as OMSimulator and FMPy.

The project uses a hybrid architecture:

- Python is responsible for user-facing packaging and orchestration so the tool can be installed and used easily from `pip`.
- C++ is responsible for the reusable compiled FMU runtime for predictable performance inside the simulation engine.

## Status

This repository is in the bootstrap phase.

Current work is focused on documenting the intended product, constraints, and workflow before implementing CSV parsing, reusable runtime packaging, and FMU packaging.

The repository now includes an initial scaffold for:

- building and testing the reusable C++ runtime artifact
- building and testing the Python packaging layer
- generating a minimal FMU package skeleton for end-to-end assembly work

## Project Goal

The long-term goal is to let a user describe time-varying outputs in a CSV file, run a Python-based packaging command, and produce a `.fmu` artifact that exposes the relevant outputs to an FMI 2.0 Co-Simulation host.

The CSV data is intended to remain external to the FMU package. The FMU should receive the CSV path as a parameter, load and validate the file during initialization, and then serve values from memory during simulation execution.

At a high level, the workflow is intended to be:

1. Provide a CSV file containing signal data.
2. Provide generation options such as model metadata and output mapping.
3. Generate per-model FMU metadata and package contents around a reusable compiled C++ runtime.
4. Package the result as an FMU that can be loaded by tools such as OMSimulator or FMPy.

## Intended Architecture

The intended split of responsibilities is:

- Python package/CLI
  - Validates user inputs
  - Interprets CSV structure and generation options
  - Generates per-model `modelDescription.xml` and package metadata
  - Creates the FMU package layout
  - Packages the reusable runtime with model-specific metadata
- Reusable compiled C++ FMU runtime
  - Accepts the CSV path as model configuration
  - Loads and validates CSV contents during initialization
  - Reads variable-reference mapping from `modelDescription.xml`
  - Resolves variable bindings once during initialization
  - Implements the FMI-facing runtime behavior
  - Produces output values during simulation from in-memory data only
  - Avoids per-call file access and should minimize hot-path type checking in `get`/`set`
  - Prioritizes performance and portability inside the target engine

This means the project is not "Python instead of C++". Python is the tool users run; C++ is the language used for the reusable FMU runtime binary packaged into each generated model.

## Intended Usage

The exact interface is still under design, but the expected user experience is:

1. Install the packaging tool from `pip`.
2. Provide a CSV file and model configuration.
3. Run a Python command that generates and packages the FMU.
4. Load the resulting `.fmu` in a simulator that supports FMI 2.0 Co-Simulation.
5. Provide the CSV path parameter so the FMU can load the file during initialization.

Conceptually, usage is expected to look like:

```text
pyfmu-csv generate --input signals.csv --output CsvSignals.fmu [options]
```

At simulation time, the intended model configuration is conceptually:

```text
csv_path=/path/to/signals.csv
```

The exact parameter name and simulator-specific configuration mechanism are not fixed yet, but the intended behavior is fixed: file access happens during initialization, not during time-step execution.

The final CLI flags, configuration format, and Python API are not fixed yet. They should be treated as planned behavior, not current functionality.

The preferred design is to avoid rebuilding custom C++ sources for each generated FMU. Model-specific structure should be expressed through generated FMI metadata and package contents, while the compiled runtime remains reusable across models.

## Repository Layout

```text
.
├── CMakeLists.txt
├── pyproject.toml
├── python/pyfmu_csv/
├── runtime/
│   ├── include/pyfmu_csv/runtime/
│   ├── src/
│   └── tests/
├── samples/
├── templates/fmi2/
└── tests/
```

- `runtime/` contains the reusable C++ FMU runtime artifact and its C++ smoke tests
- `python/pyfmu_csv/` contains the Python package, CLI, and FMU packaging helpers
- `tests/` contains Python tests for the packaging layer
- `samples/` contains sample CSV inputs for scaffolding and future tests
- `templates/fmi2/` is reserved for reusable FMI packaging assets

## Build And Test

Build the reusable runtime:

```bash
cmake -S . -B build
cmake --build build
```

Run the C++ smoke tests:

```bash
ctest --test-dir build --output-on-failure
```

Run the Python tests:

```bash
python3 -m pytest tests
```

Generate a minimal FMU package skeleton:

```bash
PYTHONPATH=python python3 -m pyfmu_csv.cli create-fmu-skeleton --output build/CsvSignals.fmu --model-name CsvSignals
```

The generated package skeleton is not yet a runnable FMU. It exists to validate the repository structure, packaging flow, and future assembly steps around `modelDescription.xml`, resources, and runtime binaries.

## Initial Scope

The first implementation target is:

- FMI 2.0
- Co-Simulation
- CSV-path-parameter-driven output generation
- Reusable compiled runtime with per-model XML/package generation
- Execution in generic simulation engines, with OMSimulator and FMPy as primary validation targets

The first versions are expected to focus on emitting outputs from CSV-defined data rather than solving arbitrary dynamic systems. The FMU runtime should preload the referenced CSV data during initialization, use `modelDescription.xml` for variable-reference mapping, and avoid file I/O in the execution loop.

## Current Limitations

The following areas are intentionally unresolved or out of scope for the current bootstrap phase:

- CSV schema is not finalized
- The exact FMU parameter naming and configuration interface for the CSV path are not finalized
- The exact internal binding strategy from variable references to in-memory signal data is not finalized
- Output variable typing and mapping rules are not finalized
- Time interpolation/extrapolation behavior is not finalized
- Event handling behavior is not finalized
- Error handling and validation policy are not finalized
- FMI 2.0 Model Exchange is not in the initial target scope
- FMI 3.0 is not in the initial target scope
- Broad engine compatibility is not guaranteed yet; support will begin with tested engines rather than every FMI host

## What Is Not Implemented Yet

This repository does not yet provide:

- CSV parsing logic
- A full Python packaging workflow
- A production-ready reusable C++ runtime implementation
- A production FMI runtime implementation
- Complete per-model `modelDescription.xml` generation
- FMU packaging logic
- Runtime binary insertion into the generated FMU package
- Runnable FMUs
- Automated compatibility tests against OMSimulator or FMPy

## Development Direction

The next phases of work are expected to cover:

1. Define the CSV contract and model metadata required for generation.
2. Establish the Python packaging workflow and project structure.
3. Implement a reusable C++ FMU runtime for FMI 2.0 Co-Simulation.
4. Generate per-model `modelDescription.xml` and package valid `.fmu` artifacts without rebuilding the runtime for each model.
5. Verify generated FMUs in OMSimulator and FMPy.

## Repository Expectations

When implementation begins, this repository should remain explicit about the boundary between:

- tooling code that users run directly
- reusable FMU runtime code that runs inside the simulator
- per-model XML and package contents generated by the tooling

That split should stay visible in the documentation, build system, and packaging workflow.
