# Usage

## Install the CLI

The main usage path is to install the published wheel so you do not need a local native
build.

Install the published wheel:

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install https://github.com/pyssporg/pyfmu_csv/releases/latest/download/pyfmu_csv-0-py3-none-linux_x86_64.whl
pyfmu-csv --help
```

This published wheel path is for Linux and uses the `py3-none-linux_x86_64` wheel tag,
so it is not tied to a single CPython minor version.

If the published wheel does not match your platform or Python version, use the
[Build From Source](build-from-source.md) guide.

## Generate an FMU

```bash
pyfmu-csv generate-fmu \
  --input-csv samples/signals.csv \
  --output CsvSignals.fmu \
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

Runtime lookup order is:

1. `--runtime-library`
2. a runtime bundled into an installed wheel
3. `build/runtime/libpyfmu_csv_fmi2_cs.*`

You can override that with:

```bash
--runtime-library /path/to/shared/library
```

Wheel builds can bundle the native runtime if it has already been compiled into
`build/runtime/`. Editable and source installs do not build the runtime during
`pip install`.

## Build From Source

For checkout, submodules, editable install, native build, Podman, and wheel packaging,
use the canonical guide:

- [Build From Source](build-from-source.md)

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

See [Testing](testing.md).
