from __future__ import annotations

import pytest

from pyfmu_csv.csv_model import load_csv_model


def test_load_csv_model_reads_outputs_from_header(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0,1,2\n", encoding="utf-8")

    model = load_csv_model(csv_path, "Plant Outputs")

    assert model.model_name == "Plant Outputs"
    assert model.model_identifier == "Plant_Outputs"
    assert [signal.name for signal in model.outputs] == ["temperature", "pressure"]
    assert [signal.value_reference for signal in model.outputs] == [1, 2]


def test_load_csv_model_requires_time_column(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("timestamp,temperature\n0,1\n", encoding="utf-8")

    with pytest.raises(ValueError, match="first CSV column must be named 'time'"):
        load_csv_model(csv_path, "Broken")
