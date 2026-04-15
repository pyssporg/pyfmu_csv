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

## Quick Install

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

If the published wheel does not match your platform or Python version, use the
[Build From Source](docs/build-from-source.md) guide instead.

## Generate An FMU

Generate an FMU from a CSV file:

```bash
pyfmu-csv generate-fmu \
  --input-csv samples/signals.csv \
  --output CsvSignals.fmu \
  --model-name CsvSignals
```

Inspect the CSV-to-VR mapping:

```bash
pyfmu-csv inspect-csv \
  --input-csv samples/signals.csv \
  --model-name CsvSignals
```

## Build From Source

Everything in the source workflow lives in the dedicated guide:

- [Build From Source](docs/build-from-source.md)
- [Testing](docs/testing.md)

## Documentation

- [Overview](docs/overview.md)
- [Architecture](docs/architecture.md)
- [Build From Source](docs/build-from-source.md)
- [Usage](docs/usage.md)
- [Testing](docs/testing.md)
- [Limitations](docs/limitations.md)
- [Localization Notes](docs/localization.md)
