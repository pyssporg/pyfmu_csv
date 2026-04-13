from __future__ import annotations

from fmpy import read_model_description

from pyfmu_csv.packaging import package_fmu_from_csv


def test_generated_fmu_is_parseable_by_fmpy(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0,1,2\n", encoding="utf-8")
    output_fmu = tmp_path / "PlantOutputs.fmu"

    package_fmu_from_csv(csv_path, output_fmu, "PlantOutputs")

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
