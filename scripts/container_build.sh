#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
IMAGE_TAG="${PYFMU_CSV_CONTAINER_IMAGE:-pyfmu-csv-build:ubuntu22-gcc13}"
source "${SCRIPT_DIR}/container_common.sh"

if ! podman image exists "${IMAGE_TAG}"; then
  "${SCRIPT_DIR}/build_container.sh"
fi

mapfile -t RUNTIME_ARGS < <(container_runtime_args "${REPO_ROOT}")

exec podman run \
  "${RUNTIME_ARGS[@]}" \
  "${IMAGE_TAG}" \
  /bin/bash -lc '
    set -euo pipefail
    rm -rf build/CMakeCache.txt build/CMakeFiles build/CTestTestfile.cmake build/Makefile build/Testing build/cmake_install.cmake build/runtime
    python3 -m venv /tmp/pyfmu-csv-container-venv
    source /tmp/pyfmu-csv-container-venv/bin/activate
    pip_args=()
    if [[ -n "${PIP_CERT-}" ]]; then
      pip_args+=(--cert "$PIP_CERT")
    fi
    if [[ -n "${https_proxy-}${HTTPS_PROXY-}" ]]; then
      pip_args+=(--trusted-host pypi.org --trusted-host files.pythonhosted.org)
    fi
    python -m pip install "${pip_args[@]}" --upgrade pip setuptools wheel
    python -m pip install "${pip_args[@]}" --no-build-isolation -e ".[dev]"
    cmake -S . -B build -G Ninja
    cmake --build build
    ctest --test-dir build --output-on-failure
    python -m pytest tests
  '
