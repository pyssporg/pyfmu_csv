# pyfmu_csv

`pyfmu_csv` generates FMI 2.0 Co-Simulation FMU packages from CSV-described signals using a hybrid toolchain:

- Python handles CSV inspection, FMU metadata generation, and packaging.
- C++ provides the reusable runtime artifact that will eventually execute inside the FMU host.

The current product can:

- inspect a CSV header and assign FMI value references
- generate `modelDescription.xml` plus package metadata
- assemble a zipped `.fmu` artifact
- build a reusable FMI 2.0 Co-Simulation shared library
- expose the FMI 2.0 C API for initialization, stepping, and value access
- validate the generated artifact structurally with FMPy

The reusable runtime is implemented and buildable, but the Python packaging flow does not yet inject the compiled shared library into the generated `.fmu` automatically.

## Quick Start

Build the reusable C++ artifact:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the Python tests:

```bash
./venv/bin/python -m pytest tests
```

Generate an FMU from a CSV file:

```bash
PYTHONPATH=python ./venv/bin/python -m pyfmu_csv generate-fmu \
  --input-csv samples/signals.csv \
  --output build/CsvSignals.fmu \
  --model-name CsvSignals
```

Inspect the CSV-to-VR mapping:

```bash
PYTHONPATH=python ./venv/bin/python -m pyfmu_csv inspect-csv \
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
