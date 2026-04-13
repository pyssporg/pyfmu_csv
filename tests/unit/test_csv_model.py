from __future__ import annotations

import pytest

from pyfmu_csv.csv_model import load_csv_model
from pyfmu_csv.model import SignalType


def test_load_csv_model_reads_outputs_from_header(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,pressure\n0,1,2\n", encoding="utf-8")

    model = load_csv_model(csv_path, "Plant Outputs")

    assert model.model_name == "Plant Outputs"
    assert model.model_identifier == "Plant_Outputs"
    assert [signal.name for signal in model.outputs] == ["temperature", "pressure"]
    assert [signal.value_reference for signal in model.outputs] == [1, 2]
    assert [signal.signal_type for signal in model.outputs] == [SignalType.REAL, SignalType.REAL]


def test_load_csv_model_reads_typed_output_annotations(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text(
        "time,temperature,count:Integer,enabled:Boolean,mode:String\n0,1.0,2,true,auto\n",
        encoding="utf-8",
    )

    model = load_csv_model(csv_path, "Typed Outputs")

    assert [signal.name for signal in model.outputs] == ["temperature", "count", "enabled", "mode"]
    assert [signal.signal_type for signal in model.outputs] == [
        SignalType.REAL,
        SignalType.INTEGER,
        SignalType.BOOLEAN,
        SignalType.STRING,
    ]


def test_load_csv_model_requires_time_column(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("timestamp,temperature\n0,1\n", encoding="utf-8")

    with pytest.raises(ValueError, match="first CSV column must be named 'time'"):
        load_csv_model(csv_path, "Broken")


def test_load_csv_model_rejects_unsupported_signal_type(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,mode:Enum\n0,auto\n", encoding="utf-8")

    with pytest.raises(ValueError, match="unsupported signal type"):
        load_csv_model(csv_path, "Broken")


def test_load_csv_model_rejects_duplicate_names_after_annotation_stripping(tmp_path) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,count:Integer,count:String\n0,1,one\n", encoding="utf-8")

    with pytest.raises(ValueError, match="Signal names must be unique"):
        load_csv_model(csv_path, "Broken")
