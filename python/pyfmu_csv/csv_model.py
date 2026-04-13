from __future__ import annotations

import csv
from pathlib import Path

from .model import CsvModelDescription, SignalDefinition, parse_signal_type


def parse_signal_header(column: str) -> SignalDefinition:
    name_part, separator, type_part = column.partition(":")
    name = name_part.strip()
    if not name:
        raise ValueError("signal names must not be empty")
    signal_type = parse_signal_type(type_part if separator else None)
    return SignalDefinition(name=name, value_reference=0, signal_type=signal_type)


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

    parsed_outputs = [parse_signal_header(column) for column in normalized[1:]]
    outputs = tuple(
        SignalDefinition(
            name=signal.name,
            value_reference=index,
            signal_type=signal.signal_type,
        )
        for index, signal in enumerate(parsed_outputs, start=1)
    )

    if len({signal.name for signal in outputs}) != len(outputs):
        raise ValueError("Signal names must be unique")

    return CsvModelDescription.create(
        model_name=model_name,
        source_csv=source_csv,
        outputs=outputs,
        csv_path_parameter=csv_path_parameter,
    )
