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
  --interactive \
  --tty \
  "${RUNTIME_ARGS[@]}" \
  "${IMAGE_TAG}" \
  /bin/bash
