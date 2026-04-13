from __future__ import annotations

import csv
from pathlib import Path

from .model import CsvModelDescription, SignalDefinition


def load_csv_model(
    csv_path: str | Path,
    model_name: str,
    csv_path_parameter: str = "csv_path",
) -> CsvModelDescription:
    source_csv = Path(csv_path)
    if not source_csv.is_file():
        raise FileNotFoundError(f"CSV file does not exist: {source_csv}")

    with source_csv.open(newline="", encoding="utf-8") as handle:
        reader = csv.reader(handle)
        try:
            header = next(reader)
        except StopIteration as exc:
            raise ValueError("CSV file is empty") from exc

    normalized = [column.strip() for column in header]
    if len(normalized) < 2:
        raise ValueError("CSV must contain a time column and at least one signal column")
    if normalized[0] != "time":
        raise ValueError("The first CSV column must be named 'time'")

    outputs = tuple(
        SignalDefinition(name=column, value_reference=index)
        for index, column in enumerate(normalized[1:], start=1)
    )

    if len({signal.name for signal in outputs}) != len(outputs):
        raise ValueError("Signal names must be unique")

    return CsvModelDescription.create(
        model_name=model_name,
        source_csv=source_csv,
        outputs=outputs,
        csv_path_parameter=csv_path_parameter,
    )
