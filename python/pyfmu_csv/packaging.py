from __future__ import annotations

import json
from pathlib import Path
from tempfile import TemporaryDirectory
from textwrap import dedent
from zipfile import ZIP_DEFLATED, ZipFile

from .csv_model import load_csv_model
from .model import CsvModelDescription
from .model_description import build_model_description_xml


def create_fmu_skeleton(output_dir: str | Path, model: CsvModelDescription) -> Path:
    root = Path(output_dir)
    binaries_dir = root / "binaries"
    resources_dir = root / "resources"
    sources_dir = root / "sources"

    binaries_dir.mkdir(parents=True, exist_ok=True)
    resources_dir.mkdir(parents=True, exist_ok=True)
    sources_dir.mkdir(parents=True, exist_ok=True)

    (root / "modelDescription.xml").write_text(
        build_model_description_xml(model),
        encoding="utf-8",
    )
    (resources_dir / "model.json").write_text(
        json.dumps(
            {
                "modelName": model.model_name,
                "csvPathParameter": model.csv_path_parameter,
                "sourceCsvName": model.source_csv.name,
                "outputs": [
                    {
                        "name": signal.name,
                        "valueReference": signal.value_reference,
                        "type": signal.signal_type.value,
                    }
                    for signal in model.outputs
                ],
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
    (resources_dir / "README.txt").write_text(
        dedent(
            """\
            This FMU expects the CSV file to remain external.
            Configure the csv_path model parameter before initialization.
            The runtime is expected to load the file once and serve signal values
            from memory during execution.
            """
        ),
        encoding="utf-8",
    )
    (binaries_dir / "README.txt").write_text(
        dedent(
            """\
            The reusable runtime binary is not packaged by this bootstrap version.
            Future versions will place the compiled generic FMI runtime here using
            the platform-specific FMI binaries layout.
            """
        ),
        encoding="utf-8",
    )
    (sources_dir / "README.txt").write_text(
        dedent(
            """\
            Source distribution is intentionally omitted from the generated package.
            The project direction is to reuse one compiled runtime across models.
            """
        ),
        encoding="utf-8",
    )

    return root


def package_fmu_from_csv(
    csv_path: str | Path,
    output_fmu: str | Path,
    model_name: str,
    csv_path_parameter: str = "csv_path",
) -> Path:
    model = load_csv_model(
        csv_path=csv_path,
        model_name=model_name,
        csv_path_parameter=csv_path_parameter,
    )
    destination = Path(output_fmu)
    destination.parent.mkdir(parents=True, exist_ok=True)

    with TemporaryDirectory(prefix="pyfmu_csv_") as temporary_root:
        stage_dir = Path(temporary_root) / destination.stem
        create_fmu_skeleton(stage_dir, model)
        with ZipFile(destination, "w", compression=ZIP_DEFLATED) as archive:
            for path in sorted(stage_dir.rglob("*")):
                if path.is_file():
                    archive.write(path, arcname=path.relative_to(stage_dir))

    return destination
