#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
IMAGE_TAG="${PYFMU_CSV_CONTAINER_IMAGE:-pyfmu-csv-build:ubuntu22-gcc13}"
CONTAINERFILE="${REPO_ROOT}/containers/build/Containerfile"
BUILD_ARGS=()

for name in http_proxy https_proxy HTTP_PROXY HTTPS_PROXY no_proxy NO_PROXY; do
  if [[ -n "${!name-}" ]]; then
    BUILD_ARGS+=(--build-arg "${name}=${!name}")
  fi
done

podman build \
  "${BUILD_ARGS[@]}" \
  --tag "${IMAGE_TAG}" \
  --file "${CONTAINERFILE}" \
  "${REPO_ROOT}"
