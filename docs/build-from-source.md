# Build From Source

Everything in this guide requires a source checkout.

## Prerequisites

For a native host setup, install:

- Git
- Python 3.10+
- `venv`
- `pytest`
- `fmpy`
- CMake 3.21+
- a C++17 compiler

For the containerized path, install Podman instead of the native toolchain.

## Clone And Initialize Submodules

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

## Create A Python Environment

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
python -m pip install -e .
```

Editable and source installs remain Python-only. FMU generation will use either an
explicit `--runtime-library`, a bundled wheel runtime, or a local
`build/runtime/libpyfmu_csv_fmi2_cs.*` artifact.

If you do not want to install the package, you can still run the module directly with:

```bash
PYTHONPATH=python python -m pyfmu_csv --help
```

## Native Build

Build the reusable runtime:

```bash
cmake -S . -B build
cmake --build build
```

After the native build, the runtime is available under `build/runtime/`.

## Native Test Flow

```bash
ctest --test-dir build --output-on-failure
python -m pytest
```

## Podman Build Container

Build the local image:

```bash
./scripts/build_container.sh
```

Run the full build, test, and wheel-packaging pipeline inside the container:

```bash
./scripts/container_build.sh
```

Open an interactive shell in the same environment:

```bash
./scripts/container_shell.sh
```

The helper scripts mount the repository into `/workspace` and use
`containers/build/Containerfile` as the single source of truth for the build environment.

## Packaging A Wheel

After the runtime has been built, package a wheel with the bundled native runtime:

```bash
python -m build --wheel
```

The resulting wheel is written to `dist/`.

Because the wheel bundles a platform-specific FMU runtime but does not expose a Python
C extension module, it is tagged as `py3-none-<platform>` rather than `cp310-cp310`
or `abi3`.
