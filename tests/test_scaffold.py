from __future__ import annotations

from pyfmu_csv.scaffold import build_model_description, create_fmu_skeleton


def test_build_model_description_declares_csv_path_parameter() -> None:
    xml = build_model_description("DemoModel")

    assert 'modelName="DemoModel"' in xml
    assert 'name="csv_path"' in xml
    assert 'causality="parameter"' in xml
    assert "<CoSimulation" in xml


def test_create_fmu_skeleton_creates_expected_layout(tmp_path) -> None:
    output_dir = tmp_path / "DemoModel.fmu"

    create_fmu_skeleton(output_dir, "DemoModel")

    assert (output_dir / "modelDescription.xml").is_file()
    assert (output_dir / "binaries").is_dir()
    assert (output_dir / "resources").is_dir()
    assert (output_dir / "sources").is_dir()
