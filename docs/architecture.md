# Architecture

## High-Level Split

### Python layer

Responsible for:

- reading the CSV header
- building the per-model description
- generating `modelDescription.xml`
- assembling the FMU package layout
- creating the final `.fmu` zip archive

### Reusable C++ runtime

Responsible for:

- receiving the `csv_path` parameter
- loading and validating CSV data during initialization
- resolving variable-reference bindings once
- serving output values from memory during execution
- interpolating `Real` outputs
- applying piecewise-constant semantics for `Integer`, `Boolean`, and `String` outputs

### Per-model package content

Generated per FMU:

- `modelDescription.xml`
- `resources/modelDescription.xml`
- `resources/model.json`
- platform-specific runtime binary under `binaries/<platform>/`
- package scaffolding under `binaries/`, `resources/`, and `sources/`

## Current Repository Layout

```text
.
├── CMakeLists.txt
├── docs/
├── pyproject.toml
├── python/pyfmu_csv/
├── runtime/
├── samples/
├── templates/fmi2/
└── tests/
```

## Current Python Modules

- `csv_model.py`: CSV header parsing and model extraction
- `model.py`: shared data structures
- `model_description.py`: FMI XML generation
- `packaging.py`: package assembly and `.fmu` creation
- `cli.py`: user-facing commands

## Current Runtime Status

The C++ runtime now provides:

- a reusable runtime core for CSV loading and interpolation
- FMI 2.0 Co-Simulation C API exports
- native smoke coverage for both the runtime core and the FMI entrypoints

The runtime is packaged into the generated FMU under the required `binaries/<platform>/<modelIdentifier>` path. The remaining work is to broaden FMI coverage beyond the currently implemented `csv_path` parameter plus typed-output Co-Simulation flow.
