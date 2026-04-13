from __future__ import annotations

import json
import platform
import shutil
from pathlib import Path
from tempfile import TemporaryDirectory
from textwrap import dedent
from zipfile import ZIP_DEFLATED, ZipFile

from .csv_model import load_csv_model
from .model import CsvModelDescription
from .model_description import build_model_description_xml


def host_platform_tuple() -> tuple[str, str]:
    system = platform.system().lower()
    machine = platform.machine().lower()

    if system == "linux":
        return "linux64", ".so"
    if system == "darwin":
        return "darwin64", ".dylib"
    if system == "windows":
        return "win64", ".dll"

    raise RuntimeError(f"unsupported host platform: {system}/{machine}")


def default_runtime_library_path(project_root: str | Path | None = None) -> Path:
    root = Path(project_root) if project_root is not None else Path(__file__).resolve().parents[2]
    _, extension = host_platform_tuple()
    return root / "build" / "runtime" / f"libpyfmu_csv_fmi2_cs{extension}"


def install_runtime_binary(
    root: Path,
    model: CsvModelDescription,
    runtime_library: str | Path | None,
) -> Path:
    platform_dir, extension = host_platform_tuple()
    source = Path(runtime_library) if runtime_library is not None else default_runtime_library_path()
    if not source.is_file():
        raise FileNotFoundError(
            f"runtime library does not exist: {source}. Build the native runtime before packaging the FMU."
        )

    binary_dir = root / "binaries" / platform_dir
    binary_dir.mkdir(parents=True, exist_ok=True)
    destination = binary_dir / f"{model.model_identifier}{extension}"
    shutil.copy2(source, destination)
    return destination


def create_fmu_skeleton(
    output_dir: str | Path,
    model: CsvModelDescription,
    runtime_library: str | Path | None = None,
) -> Path:
    root = Path(output_dir)
    resources_dir = root / "resources"
    sources_dir = root / "sources"

    resources_dir.mkdir(parents=True, exist_ok=True)
    sources_dir.mkdir(parents=True, exist_ok=True)

    model_description_xml = build_model_description_xml(model)
    (root / "modelDescription.xml").write_text(
        model_description_xml,
        encoding="utf-8",
    )
    (resources_dir / "modelDescription.xml").write_text(
        model_description_xml,
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
    runtime_destination = install_runtime_binary(root, model, runtime_library)
    (sources_dir / "README.txt").write_text(
        dedent(
            """\
            Source distribution is intentionally omitted from the generated package.
            The project direction is to reuse one compiled runtime across models.
            """
        ),
        encoding="utf-8",
    )
    (root / "resources" / "runtime.json").write_text(
        json.dumps(
            {
                "binary": str(runtime_destination.relative_to(root)),
                "platform": runtime_destination.parent.name,
                "modelIdentifier": model.model_identifier,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )

    return root


def package_fmu_from_csv(
    csv_path: str | Path,
    output_fmu: str | Path,
    model_name: str,
    csv_path_parameter: str = "csv_path",
    runtime_library: str | Path | None = None,
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
        create_fmu_skeleton(stage_dir, model, runtime_library=runtime_library)
        with ZipFile(destination, "w", compression=ZIP_DEFLATED) as archive:
            for path in sorted(stage_dir.rglob("*")):
                if path.is_file():
                    archive.write(path, arcname=path.relative_to(stage_dir))

    return destination
