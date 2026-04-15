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

## Install And Use

This is the main usage path. Install the published wheel and use the bundled runtime
without cloning the repo or building the source tree.

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install https://github.com/pyssporg/pyfmu_csv/releases/latest/download/pyfmu_csv-0-cp310-cp310-linux_x86_64.whl
```

After installation, the CLI and bundled runtime are ready to use:

```bash
pyfmu-csv --help
pyfmu-csv generate-fmu \
  --input-csv samples/signals.csv \
  --output CsvSignals.fmu \
  --model-name CsvSignals
```

This published wheel path is currently for Linux `cp310`.

## Generate An FMU

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

## Build From Source

Everything below this point requires access to the source tree.

### Prerequisites

For a native host setup, install:

- Git
- Python 3.10+
- `venv`
- CMake 3.21+
- a C++17 compiler

For the containerized path, install Podman instead of the native toolchain.

### Clone And Initialize Submodules

The FMI reference material is provided through the `3rd_party/fmi_2` submodule.

```bash
git clone <repo-url>
cd pyfmu_csv
git submodule update --init --recursive
```

If the repository is already cloned, still run:

```bash
git submodule update --init --recursive
```

### Create And Activate A Virtual Environment

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
python -m pip install -e .
```

Editable and source installs remain Python-only and will use either a bundled wheel
runtime, an explicit `--runtime-library`, or a local `build/runtime` artifact at FMU
generation time.

### Native Build And Test

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
python -m pytest
```

After the native build, the runtime is available under `build/runtime/` and FMU
generation can find it automatically.

### Containerized Build And Test

```bash
./scripts/build_container.sh
./scripts/container_build.sh
```

For an interactive shell inside the same container:

```bash
./scripts/container_shell.sh
```

The container path builds the runtime, runs the tests, and packages a wheel in `dist/`.

## Documentation

- [Overview](docs/overview.md)
- [Architecture](docs/architecture.md)
- [Usage](docs/usage.md)
- [Testing](docs/testing.md)
- [Limitations](docs/limitations.md)
- [Localization Notes](docs/localization.md)
