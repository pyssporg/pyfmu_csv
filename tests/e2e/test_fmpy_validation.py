from __future__ import annotations

from pathlib import Path

import pytest
fmpy = pytest.importorskip("fmpy")
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
    csv_path.write_text(
        "time,temperature,count:Integer,enabled:Boolean,mode:String\n0,1,2,true,auto\n",
        encoding="utf-8",
    )
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
    assert "count" in variable_names
    assert "enabled" in variable_names
    assert "mode" in variable_names
    csv_parameter = next(variable for variable in model_description.modelVariables if variable.name == "csv_path")
    assert csv_parameter.start == "data/signals.csv"


@pytest.mark.xfail(
    reason="Current FMPy simulate_fmu path does not apply the String start value from modelDescription.xml",
    strict=False,
)
def test_generated_fmu_runs_through_fmpy_with_packaged_csv_fallback(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text(
        "time,temperature,count:Integer,enabled:Boolean,mode:String\n"
        "0.0,10.0,2,true,auto\n"
        "1.0,12.0,4,false,manual\n",
        encoding="utf-8",
    )
    output_fmu = tmp_path / "PlantOutputs.fmu"

    package_fmu_from_csv(csv_path, output_fmu, "PlantOutputs", runtime_library=built_runtime_library())

    result = simulate_fmu(
        output_fmu,
        validate=False,
        start_time=0.0,
        stop_time=1.0,
        output_interval=0.5,
        apply_default_start_values=True,
        output=["temperature", "count", "enabled"],
    )

    assert result["temperature"].tolist() == pytest.approx([10.0, 11.0, 12.0])
    assert result["count"].tolist() == [2, 2, 4]
    assert result["enabled"].tolist() == [True, True, False]


def test_generated_fmu_explicit_csv_path_override_still_works(tmp_path) -> None:
    packaged_csv_path = tmp_path / "signals.csv"
    packaged_csv_path.write_text(
        "time,temperature,count:Integer,enabled:Boolean,mode:String\n"
        "0.0,10.0,2,true,auto\n"
        "1.0,12.0,4,false,manual\n",
        encoding="utf-8",
    )
    override_csv_path = tmp_path / "override-signals.csv"
    override_csv_path.write_text(
        "time,temperature,count:Integer,enabled:Boolean,mode:String\n"
        "0.0,20.0,3,false,override\n"
        "1.0,22.0,5,true,override-next\n",
        encoding="utf-8",
    )
    output_fmu = tmp_path / "PlantOutputs.fmu"

    package_fmu_from_csv(packaged_csv_path, output_fmu, "PlantOutputs", runtime_library=built_runtime_library())

    result = simulate_fmu(
        output_fmu,
        validate=False,
        start_time=0.0,
        stop_time=1.0,
        output_interval=0.5,
        start_values={"csv_path": str(override_csv_path)},
        output=["temperature", "count", "enabled"],
    )

    assert result["temperature"].tolist() == pytest.approx([20.0, 21.0, 22.0])
    assert result["count"].tolist() == [3, 3, 5]
    assert result["enabled"].tolist() == [False, False, True]
