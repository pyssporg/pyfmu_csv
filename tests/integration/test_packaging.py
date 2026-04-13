from __future__ import annotations

import json
from zipfile import ZipFile

from pyfmu_csv.packaging import create_fmu_skeleton, package_fmu_from_csv
from pyfmu_csv.csv_model import load_csv_model


def test_create_fmu_skeleton_creates_expected_layout(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0,1,2\n", encoding="utf-8")
    model = load_csv_model(csv_path, "DemoModel")
    output_dir = tmp_path / "DemoModel.fmu"

    create_fmu_skeleton(output_dir, model)

    assert (output_dir / "modelDescription.xml").is_file()
    assert (output_dir / "binaries" / "README.txt").is_file()
    assert (output_dir / "resources" / "model.json").is_file()
    manifest = json.loads((output_dir / "resources" / "model.json").read_text(encoding="utf-8"))
    assert manifest["outputs"][0]["name"] == "temperature"


def test_package_fmu_from_csv_writes_archive(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature\n0,1\n", encoding="utf-8")
    output_fmu = tmp_path / "DemoModel.fmu"

    package_fmu_from_csv(csv_path, output_fmu, "DemoModel")

    assert output_fmu.is_file()
    with ZipFile(output_fmu) as archive:
        names = set(archive.namelist())
    assert "modelDescription.xml" in names
    assert "resources/model.json" in names
