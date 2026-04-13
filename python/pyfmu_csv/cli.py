from __future__ import annotations

import argparse
from pathlib import Path

from .scaffold import create_fmu_skeleton


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

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_parser()
    args = parser.parse_args(argv)

    if args.command == "create-fmu-skeleton":
        output_path = create_fmu_skeleton(args.output, args.model_name)
        print(f"Created FMU skeleton at {output_path}")
        return 0

    parser.error(f"unsupported command: {args.command}")
    return 2
