from __future__ import annotations

import runpy
from pathlib import Path
from unittest.mock import patch

import pytest
from setuptools.dist import Distribution


def load_setup_namespace() -> dict[str, object]:
    setup_path = Path(__file__).resolve().parents[2] / "setup.py"
    with patch("setuptools.setup"):
        return runpy.run_path(setup_path)


def test_bundle_runtime_copies_binary_into_wheel_build_dir(tmp_path, monkeypatch) -> None:
    namespace = load_setup_namespace()
    build_py = namespace["build_py"]
    runtime_source = tmp_path / "libpyfmu_csv_fmi2_cs.so"
    runtime_source.write_bytes(b"fake runtime")
    monkeypatch.setitem(
        build_py._bundle_runtime.__globals__,
        "default_runtime_library_path",
        lambda project_root=None: runtime_source,
    )

    command = build_py(Distribution())
    command.build_lib = str(tmp_path / "wheel-build")
    command._bundle_runtime()

    runtime_filename = namespace["runtime_library_filename"]()
    destination = tmp_path / "wheel-build" / "pyfmu_csv" / "_runtime" / runtime_filename
    assert destination.is_file()
    assert destination.read_bytes() == b"fake runtime"


def test_bundle_runtime_raises_when_binary_is_missing(tmp_path, monkeypatch) -> None:
    namespace = load_setup_namespace()
    build_py = namespace["build_py"]
    missing_runtime = tmp_path / "build" / "runtime" / "libpyfmu_csv_fmi2_cs.so"
    monkeypatch.setitem(
        build_py._bundle_runtime.__globals__,
        "default_runtime_library_path",
        lambda project_root=None: missing_runtime,
    )

    command = build_py(Distribution())
    command.build_lib = str(tmp_path / "wheel-build")

    with pytest.raises(FileNotFoundError, match="Build the native runtime"):
        command._bundle_runtime()
