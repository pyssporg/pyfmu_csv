# Usage

## Prerequisites

- Python 3.10+
- `pytest` for Python tests
- `fmpy` for structural FMU inspection
- `cmake` plus a C++ compiler for the reusable runtime artifact

## Install the CLI

Install the package so setuptools exposes the `pyfmu-csv` command declared in `pyproject.toml`:

```toml
[project.scripts]
pyfmu-csv = "pyfmu_csv.cli:main"
```

From the repository root:

```bash
pip install -e .
```

After that, the CLI is available as:

```bash
pyfmu-csv --help
```

If you do not want to install the package, you can still run the module directly with `PYTHONPATH=python ./venv/bin/python -m pyfmu_csv ...`.

## Generate an FMU

```bash
pyfmu-csv generate-fmu \
  --input-csv samples/signals.csv \
  --output build/CsvSignals.fmu \
  --model-name CsvSignals
```

This command:

- reads the CSV header
- treats the first column as `time`
- maps all remaining columns to output variables
- assigns sequential value references starting at `1`
- copies the compiled generic FMI runtime into the FMU
- writes a zipped `.fmu` archive

Header syntax:

```text
time,temperature,count:Integer,enabled:Boolean,mode:String
```

Type annotations are optional. Columns without an annotation default to `Real`.

By default, the generator looks for the compiled runtime at:

```text
build/runtime/libpyfmu_csv_fmi2_cs.so
```

You can override that with:

```bash
--runtime-library /path/to/shared/library
```

## Inspect a CSV Contract

```bash
pyfmu-csv inspect-csv \
  --input-csv samples/signals.csv \
  --model-name CsvSignals
```

This prints:

- model name
- sanitized FMI model identifier
- CSV path parameter name
- exported signals, value references, and scalar types

## Create an Unzipped Package Skeleton

```bash
pyfmu-csv create-fmu-skeleton \
  --input-csv samples/signals.csv \
  --output build/CsvSignals.dir \
  --model-name CsvSignals
```

Use this when you want to inspect the intermediate package layout before it is zipped into a final `.fmu`.

## Run the Full Verification Chain

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
pytest tests
```
