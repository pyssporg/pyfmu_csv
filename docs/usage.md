# Usage

## Prerequisites

- Python 3.11+
- `pytest` for Python tests
- `fmpy` for structural FMU inspection
- `cmake` plus a C++ compiler for the reusable runtime artifact

## Generate an FMU

```bash
PYTHONPATH=python ./venv/bin/python -m pyfmu_csv generate-fmu \
  --input-csv samples/signals.csv \
  --output build/CsvSignals.fmu \
  --model-name CsvSignals
```

This command:

- reads the CSV header
- treats the first column as `time`
- maps all remaining columns to output variables
- assigns sequential value references starting at `1`
- writes a zipped `.fmu` archive

## Inspect a CSV Contract

```bash
PYTHONPATH=python ./venv/bin/python -m pyfmu_csv inspect-csv \
  --input-csv samples/signals.csv \
  --model-name CsvSignals
```

This prints:

- model name
- sanitized FMI model identifier
- CSV path parameter name
- exported signals and value references

## Create an Unzipped Package Skeleton

```bash
PYTHONPATH=python ./venv/bin/python -m pyfmu_csv create-fmu-skeleton \
  --input-csv samples/signals.csv \
  --output build/CsvSignals.dir \
  --model-name CsvSignals
```

Use this when you want to inspect the intermediate package layout before it is zipped into a final `.fmu`.
