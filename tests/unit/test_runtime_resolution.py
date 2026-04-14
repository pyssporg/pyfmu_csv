from __future__ import annotations

from pathlib import Path

import pytest

from pyfmu_csv.csv_model import load_csv_model
from pyfmu_csv.packaging import create_fmu_skeleton, host_platform_tuple, resolve_runtime_library_path


def test_resolve_runtime_library_path_prefers_explicit_argument(tmp_path, monkeypatch) -> None:
    explicit = tmp_path / "explicit.so"
    explicit.write_bytes(b"explicit")
    bundled = tmp_path / "bundled.so"
    bundled.write_bytes(b"bundled")

    monkeypatch.setattr("pyfmu_csv.packaging.bundled_runtime_library", lambda: bundled)

    resolved = resolve_runtime_library_path(explicit)

    assert resolved == explicit


def test_create_fmu_skeleton_prefers_bundled_runtime_over_local_build(tmp_path, monkeypatch) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature\n0,1\n", encoding="utf-8")
    model = load_csv_model(csv_path, "BundledWins")
    output_dir = tmp_path / "BundledWins.fmu"
    bundled = tmp_path / "bundled.so"
    local = tmp_path / "local.so"
    bundled.write_bytes(b"bundled")
    local.write_bytes(b"local")

    monkeypatch.setattr("pyfmu_csv.packaging.bundled_runtime_library", lambda: bundled)
    monkeypatch.setattr("pyfmu_csv.packaging.default_runtime_library_path", lambda project_root=None: local)

    create_fmu_skeleton(output_dir, model)

    platform_dir, extension = host_platform_tuple()
    runtime_copy = output_dir / "binaries" / platform_dir / f"{model.model_identifier}{extension}"
    assert runtime_copy.read_bytes() == b"bundled"


def test_create_fmu_skeleton_falls_back_to_local_runtime_build(tmp_path, monkeypatch) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature\n0,1\n", encoding="utf-8")
    model = load_csv_model(csv_path, "LocalFallback")
    output_dir = tmp_path / "LocalFallback.fmu"
    local = tmp_path / "local.so"
    local.write_bytes(b"local")

    monkeypatch.setattr("pyfmu_csv.packaging.bundled_runtime_library", lambda: None)
    monkeypatch.setattr("pyfmu_csv.packaging.default_runtime_library_path", lambda project_root=None: local)

    create_fmu_skeleton(output_dir, model)

    platform_dir, extension = host_platform_tuple()
    runtime_copy = output_dir / "binaries" / platform_dir / f"{model.model_identifier}{extension}"
    assert runtime_copy.read_bytes() == b"local"


def test_create_fmu_skeleton_raises_clear_error_when_runtime_missing(tmp_path, monkeypatch) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature\n0,1\n", encoding="utf-8")
    model = load_csv_model(csv_path, "MissingRuntime")

    monkeypatch.setattr("pyfmu_csv.packaging.bundled_runtime_library", lambda: None)
    monkeypatch.setattr(
        "pyfmu_csv.packaging.default_runtime_library_path",
        lambda project_root=None: Path(tmp_path / "missing.so"),
    )

    with pytest.raises(FileNotFoundError, match="runtime library not found"):
        create_fmu_skeleton(tmp_path / "MissingRuntime.fmu", model)
