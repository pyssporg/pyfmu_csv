from __future__ import annotations

from pyfmu_csv.cli import main


def test_cli_creates_skeleton(tmp_path, capsys) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature\n0,1\n", encoding="utf-8")
    output_dir = tmp_path / "Generated"

    exit_code = main(
        [
            "create-fmu-skeleton",
            "--input-csv",
            str(csv_path),
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


def test_cli_generates_fmu_archive(tmp_path, capsys) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature\n0,1\n", encoding="utf-8")
    output_fmu = tmp_path / "Generated.fmu"

    exit_code = main(
        [
            "generate-fmu",
            "--input-csv",
            str(csv_path),
            "--output",
            str(output_fmu),
            "--model-name",
            "Generated",
        ]
    )

    captured = capsys.readouterr()

    assert exit_code == 0
    assert "Generated FMU" in captured.out
    assert output_fmu.is_file()
