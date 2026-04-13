from __future__ import annotations

from pathlib import Path

import pytest
from fmpy import read_model_description, simulate_fmu

from pyfmu_csv.packaging import package_fmu_from_csv


def built_runtime_library() -> Path:
    platform_candidates = [
        Path("build/runtime/libpyfmu_csv_fmi2_cs.so"),
        Path("build/runtime/libpyfmu_csv_fmi2_cs.dylib"),
        Path("build/runtime/libpyfmu_csv_fmi2_cs.dll"),
    ]
    for candidate in platform_candidates:
        if candidate.is_file():
            return candidate
    raise FileNotFoundError("built FMI runtime library not found in build/runtime/")


def test_generated_fmu_is_parseable_by_fmpy(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0,1,2\n", encoding="utf-8")
    output_fmu = tmp_path / "PlantOutputs.fmu"

    package_fmu_from_csv(csv_path, output_fmu, "PlantOutputs", runtime_library=built_runtime_library())

    model_description = read_model_description(
        output_fmu,
        validate=False,
        validate_model_structure=False,
    )

    assert model_description.fmiVersion == "2.0"
    assert model_description.modelName == "PlantOutputs"
    variable_names = [variable.name for variable in model_description.modelVariables]
    assert "csv_path" in variable_names
    assert "temperature" in variable_names
    assert "pressure" in variable_names


def test_generated_fmu_runs_through_fmpy(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0.0,10.0,20.0\n1.0,12.0,24.0\n", encoding="utf-8")
    output_fmu = tmp_path / "PlantOutputs.fmu"

    package_fmu_from_csv(csv_path, output_fmu, "PlantOutputs", runtime_library=built_runtime_library())

    result = simulate_fmu(
        output_fmu,
        validate=False,
        start_time=0.0,
        stop_time=1.0,
        output_interval=0.5,
        start_values={"csv_path": str(csv_path)},
        output=["temperature", "pressure"],
    )

    assert result["temperature"].tolist() == pytest.approx([10.0, 11.0, 12.0])
    assert result["pressure"].tolist() == pytest.approx([20.0, 22.0, 24.0])
