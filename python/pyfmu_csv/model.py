from __future__ import annotations

from dataclasses import dataclass
from enum import Enum
from pathlib import Path
from uuid import NAMESPACE_URL, uuid5


class SignalType(str, Enum):
    REAL = "Real"


@dataclass(frozen=True)
class SignalDefinition:
    name: str
    value_reference: int
    signal_type: SignalType = SignalType.REAL


@dataclass(frozen=True)
class CsvModelDescription:
    model_name: str
    model_identifier: str
    guid: str
    csv_path_parameter: str
    source_csv: Path
    outputs: tuple[SignalDefinition, ...]

    @classmethod
    def create(
        cls,
        model_name: str,
        source_csv: Path,
        outputs: tuple[SignalDefinition, ...],
        csv_path_parameter: str = "csv_path",
    ) -> "CsvModelDescription":
        identifier = sanitize_identifier(model_name)
        guid_seed = f"{model_name}:{source_csv.name}:{','.join(signal.name for signal in outputs)}"
        return cls(
            model_name=model_name,
            model_identifier=identifier,
            guid=str(uuid5(NAMESPACE_URL, guid_seed)),
            csv_path_parameter=csv_path_parameter,
            source_csv=source_csv,
            outputs=outputs,
        )


def sanitize_identifier(name: str) -> str:
    cleaned = "".join(character if character.isalnum() else "_" for character in name)
    cleaned = cleaned.strip("_")
    if not cleaned:
        return "CsvModel"
    if cleaned[0].isdigit():
        cleaned = f"model_{cleaned}"
    return cleaned
