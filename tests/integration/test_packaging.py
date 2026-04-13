from __future__ import annotations

import json
from pathlib import Path
from zipfile import ZipFile

from pyfmu_csv.packaging import create_fmu_skeleton, host_platform_tuple, package_fmu_from_csv
from pyfmu_csv.csv_model import load_csv_model


def test_create_fmu_skeleton_creates_expected_layout(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0,1,2\n", encoding="utf-8")
    model = load_csv_model(csv_path, "DemoModel")
    output_dir = tmp_path / "DemoModel.fmu"
    runtime_library = tmp_path / "libpyfmu_csv_fmi2_cs.so"
    runtime_library.write_bytes(b"fake runtime")

    create_fmu_skeleton(output_dir, model, runtime_library=runtime_library)

    assert (output_dir / "modelDescription.xml").is_file()
    assert (output_dir / "resources" / "model.json").is_file()
    manifest = json.loads((output_dir / "resources" / "model.json").read_text(encoding="utf-8"))
    assert manifest["outputs"][0]["name"] == "temperature"
    platform_dir, extension = host_platform_tuple()
    runtime_copy = output_dir / "binaries" / platform_dir / f"{model.model_identifier}{extension}"
    assert runtime_copy.is_file()
    assert runtime_copy.read_bytes() == b"fake runtime"


def test_package_fmu_from_csv_writes_archive(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature\n0,1\n", encoding="utf-8")
    output_fmu = tmp_path / "DemoModel.fmu"
    runtime_library = tmp_path / "libpyfmu_csv_fmi2_cs.so"
    runtime_library.write_bytes(b"fake runtime")

    package_fmu_from_csv(csv_path, output_fmu, "DemoModel", runtime_library=runtime_library)

    assert output_fmu.is_file()
    with ZipFile(output_fmu) as archive:
        names = set(archive.namelist())
    assert "modelDescription.xml" in names
    assert "resources/model.json" in names
    platform_dir, extension = host_platform_tuple()
    assert f"binaries/{platform_dir}/DemoModel{extension}" in names
