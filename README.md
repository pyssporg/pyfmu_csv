# pyfmu_csv

`pyfmu_csv` generates FMI 2.0 Co-Simulation FMU packages from CSV-described signals using a hybrid toolchain:

- Python handles CSV inspection, FMU metadata generation, and packaging.
- C++ provides the reusable runtime artifact that will eventually execute inside the FMU host.

The current product can:

- inspect a CSV header and assign FMI value references
- read header type annotations with `Real` fallback
- generate `modelDescription.xml` plus package metadata
- assemble a zipped `.fmu` artifact
- build a reusable FMI 2.0 Co-Simulation shared library
- expose the FMI 2.0 C API for initialization, stepping, and value access
- embed the compiled runtime into `binaries/<platform>/<modelIdentifier>`
- run the generated FMU through FMPy using the external `csv_path` parameter
- validate the generated artifact structurally with FMPy

The current chain works end to end after the native runtime has been built.

Supported output types today:

- `Real`
- `Integer`
- `Boolean`
- `String`

## Quick Start

Build the reusable C++ artifact:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the Python tests:

```bash
python -m pytest tests
```

Use the Podman-based build container for a reproducible Ubuntu 22.04 / GCC 13 environment:

```bash
./scripts/build_container.sh
./scripts/container_build.sh
```

Open an interactive shell inside the same container:

```bash
./scripts/container_shell.sh
```

Install the Python package to register the `pyfmu-csv` CLI from `[project.scripts]`:

```bash
pip install -e .
```

Built wheels can bundle the native runtime when `build/runtime/libpyfmu_csv_fmi2_cs*`
already exists. Editable and source installs remain Python-only and will use either a
bundled wheel runtime, an explicit `--runtime-library`, or a local `build/runtime`
artifact at FMU generation time.

Generate an FMU from a CSV file:

```bash
pyfmu-csv generate-fmu \
  --input-csv samples/signals.csv \
  --output build/CsvSignals.fmu \
  --model-name CsvSignals
```

This console command is provided by:

```toml
[project.scripts]
pyfmu-csv = "pyfmu_csv.cli:main"
```

If you prefer not to install the package, the module form remains available:

```bash
python -m pyfmu_csv generate-fmu \
  --input-csv samples/signals.csv \
  --output build/CsvSignals.fmu \
  --model-name CsvSignals
```

Inspect the CSV-to-VR mapping:

```bash
pyfmu-csv inspect-csv \
  --input-csv samples/signals.csv \
  --model-name CsvSignals
```

## Documentation

- [Overview](docs/overview.md)
- [Architecture](docs/architecture.md)
- [Usage](docs/usage.md)
- [Testing](docs/testing.md)
- [Limitations](docs/limitations.md)
- [Localization Notes](docs/localization.md)
