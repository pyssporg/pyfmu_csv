from __future__ import annotations

from pathlib import Path
from textwrap import dedent


def build_model_description(model_name: str) -> str:
    return dedent(
        f"""\
        <?xml version="1.0" encoding="UTF-8"?>
        <fmiModelDescription
            fmiVersion="2.0"
            modelName="{model_name}"
            guid="TODO-GENERATE-GUID"
            generationTool="pyfmu-csv"
            variableNamingConvention="structured">
          <CoSimulation modelIdentifier="{model_name}"/>
          <ModelVariables>
            <ScalarVariable
                name="csv_path"
                valueReference="0"
                variability="tunable"
                causality="parameter">
              <String start=""/>
            </ScalarVariable>
          </ModelVariables>
          <ModelStructure/>
        </fmiModelDescription>
        """
    )


def create_fmu_skeleton(output_dir: str | Path, model_name: str) -> Path:
    root = Path(output_dir)
    binaries_dir = root / "binaries"
    resources_dir = root / "resources"
    sources_dir = root / "sources"

    binaries_dir.mkdir(parents=True, exist_ok=True)
    resources_dir.mkdir(parents=True, exist_ok=True)
    sources_dir.mkdir(parents=True, exist_ok=True)

    (root / "modelDescription.xml").write_text(
        build_model_description(model_name),
        encoding="utf-8",
    )
    (resources_dir / "README.txt").write_text(
        dedent(
            """\
            Place external CSV resources or sample data references here if the
            packaging workflow needs them during test assembly. The production
            runtime is expected to receive a csv_path parameter at initialization.
            """
        ),
        encoding="utf-8",
    )
    (sources_dir / "README.txt").write_text(
        dedent(
            """\
            This directory is reserved for FMI source distribution needs.
            The project direction is to package a reusable compiled runtime
            rather than rebuilding custom C++ sources per model.
            """
        ),
        encoding="utf-8",
    )

    return root
