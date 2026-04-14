#!/usr/bin/env bash

container_runtime_args() {
  local repo_root="$1"
  local args=(
    --rm
    --userns=keep-id
    --security-opt label=disable
    --volume "${repo_root}:/workspace"
    --workdir /workspace
  )

  local name
  for name in \
    http_proxy https_proxy HTTP_PROXY HTTPS_PROXY no_proxy NO_PROXY \
    PIP_CERT PIP_INDEX_URL PIP_EXTRA_INDEX_URL PIP_TRUSTED_HOST \
    SSL_CERT_FILE REQUESTS_CA_BUNDLE
  do
    if [[ -n "${!name-}" ]]; then
      args+=(--env "${name}=${!name}")
    fi
  done

  # Reuse host trust anchors so Podman runs can reach package indexes behind
  # enterprise TLS interception without baking local certificates into the image.
  if [[ -d /etc/ssl/certs ]]; then
    args+=(--volume /etc/ssl/certs:/etc/ssl/certs:ro)
  fi
  if [[ -f /etc/ssl/cert.pem ]]; then
    args+=(--volume /etc/ssl/cert.pem:/etc/ssl/cert.pem:ro)
  fi
  if [[ -d /usr/local/share/ca-certificates ]]; then
    args+=(--volume /usr/local/share/ca-certificates:/usr/local/share/ca-certificates:ro)
  fi
  if [[ -z "${SSL_CERT_FILE-}" && -f /usr/local/share/ca-certificates/cacerts.pem ]]; then
    args+=(--env SSL_CERT_FILE=/usr/local/share/ca-certificates/cacerts.pem)
  fi
  if [[ -z "${REQUESTS_CA_BUNDLE-}" && -f /usr/local/share/ca-certificates/cacerts.pem ]]; then
    args+=(--env REQUESTS_CA_BUNDLE=/usr/local/share/ca-certificates/cacerts.pem)
  fi
  if [[ -z "${PIP_CERT-}" && -f /usr/local/share/ca-certificates/cacerts.pem ]]; then
    args+=(--env PIP_CERT=/usr/local/share/ca-certificates/cacerts.pem)
  fi

  printf '%s\n' "${args[@]}"
}
