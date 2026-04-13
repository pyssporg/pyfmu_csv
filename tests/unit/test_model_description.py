from __future__ import annotations

from pyfmu_csv.csv_model import load_csv_model
from pyfmu_csv.model_description import build_model_description_xml


def test_model_description_declares_csv_path_parameter_and_outputs(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0,1,2\n", encoding="utf-8")

    model = load_csv_model(csv_path, "DemoModel")
    xml = build_model_description_xml(model)

    assert 'modelName="DemoModel"' in xml
    assert 'name="csv_path"' in xml
    assert 'name="temperature"' in xml
    assert 'valueReference="1"' in xml
    assert "<Outputs>" in xml
