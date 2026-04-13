from __future__ import annotations

from pyfmu_csv.cli import main


def test_cli_creates_skeleton(tmp_path, capsys) -> None:
    output_dir = tmp_path / "Generated.fmu"

    exit_code = main(
        [
            "create-fmu-skeleton",
            "--output",
            str(output_dir),
            "--model-name",
            "Generated",
        ]
    )

    captured = capsys.readouterr()

    assert exit_code == 0
    assert "Created FMU skeleton" in captured.out
    assert (output_dir / "modelDescription.xml").is_file()
