from __future__ import annotations

import argparse
from pathlib import Path

from .csv_model import load_csv_model
from .packaging import create_fmu_skeleton, package_fmu_from_csv


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog="pyfmu-csv",
        description="Build and package CSV-backed FMI artifacts.",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    create_parser = subparsers.add_parser(
        "create-fmu-skeleton",
        help="Create a minimal FMI 2.0 Co-Simulation package layout.",
    )
    create_parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Directory where the FMU package skeleton should be created.",
    )
    create_parser.add_argument(
        "--model-name",
        default="CsvSignals",
        help="Model name to embed in modelDescription.xml.",
    )
    create_parser.add_argument(
        "--runtime-library",
        type=Path,
        help="Path to the compiled reusable FMI shared library to embed in the FMU skeleton.",
    )
    create_parser.add_argument(
        "--input-csv",
        type=Path,
        required=True,
        help="CSV file used to define exported output signals.",
    )

    generate_parser = subparsers.add_parser(
        "generate-fmu",
        help="Generate a zipped FMI 2.0 Co-Simulation FMU artifact from a CSV file.",
    )
    generate_parser.add_argument(
        "--input-csv",
        type=Path,
        required=True,
        help="CSV file used to define exported output signals.",
    )
    generate_parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Path to the generated .fmu archive.",
    )
    generate_parser.add_argument(
        "--model-name",
        default="CsvSignals",
        help="Model name to embed in modelDescription.xml.",
    )
    generate_parser.add_argument(
        "--runtime-library",
        type=Path,
        help="Path to the compiled reusable FMI shared library to embed in the FMU.",
    )

    inspect_parser = subparsers.add_parser(
        "inspect-csv",
        help="Print the signal mapping inferred from a CSV file.",
    )
    inspect_parser.add_argument(
        "--input-csv",
        type=Path,
        required=True,
        help="CSV file to inspect.",
    )
    inspect_parser.add_argument(
        "--model-name",
        default="CsvSignals",
        help="Model name used for identifier normalization.",
    )

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    if args.command == "create-fmu-skeleton":
        model = load_csv_model(args.input_csv, args.model_name)
        output_path = create_fmu_skeleton(args.output, model, runtime_library=args.runtime_library)
        print(f"Created FMU skeleton at {output_path}")
        return 0
    if args.command == "generate-fmu":
        output_path = package_fmu_from_csv(
            args.input_csv,
            args.output,
            args.model_name,
            runtime_library=args.runtime_library,
        )
        print(f"Generated FMU at {output_path}")
        return 0
    if args.command == "inspect-csv":
        model = load_csv_model(args.input_csv, args.model_name)
        print(f"model_name={model.model_name}")
        print(f"model_identifier={model.model_identifier}")
        print(f"csv_parameter={model.csv_path_parameter}")
        for signal in model.outputs:
            print(f"{signal.value_reference}:{signal.name}:{signal.signal_type.value}")
        return 0

    parser.error(f"unsupported command: {args.command}")
    return 2
