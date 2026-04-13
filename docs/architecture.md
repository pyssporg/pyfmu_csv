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

### Per-model package content

Generated per FMU:

- `modelDescription.xml`
- `resources/model.json`
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

The C++ runtime target exists as a reusable generic artifact and is covered by a smoke test. It is not yet an FMI-complete shared library and is not yet injected into the generated FMU.
