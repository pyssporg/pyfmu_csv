from __future__ import annotations

from zipfile import ZipFile

from pyfmu_csv.cli import main


def test_cli_creates_skeleton(tmp_path, capsys) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,count:Integer\n0,1,2\n", encoding="utf-8")
    output_dir = tmp_path / "Generated"
    runtime_library = tmp_path / "libpyfmu_csv_fmi2_cs.so"
    runtime_library.write_bytes(b"fake runtime")

    exit_code = main(
        [
            "create-fmu-skeleton",
            "--input-csv",
            str(csv_path),
            "--output",
            str(output_dir),
            "--model-name",
            "Generated",
            "--runtime-library",
            str(runtime_library),
        ]
    )

    captured = capsys.readouterr()

    assert exit_code == 0
    assert "Created FMU skeleton" in captured.out
    assert (output_dir / "modelDescription.xml").is_file()


def test_cli_can_create_skeleton_without_packaged_csv(tmp_path, capsys) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,count:Integer\n0,1,2\n", encoding="utf-8")
    output_dir = tmp_path / "Generated"
    runtime_library = tmp_path / "libpyfmu_csv_fmi2_cs.so"
    runtime_library.write_bytes(b"fake runtime")

    exit_code = main(
        [
            "create-fmu-skeleton",
            "--input-csv",
            str(csv_path),
            "--output",
            str(output_dir),
            "--model-name",
            "Generated",
            "--runtime-library",
            str(runtime_library),
            "--no-copy-csv",
        ]
    )

    captured = capsys.readouterr()

    assert exit_code == 0
    assert "Created FMU skeleton" in captured.out
    assert (output_dir / "modelDescription.xml").is_file()
    assert not (output_dir / "resources" / "data" / "signals.csv").exists()


def test_cli_generates_fmu_archive(tmp_path, capsys) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,count:Integer\n0,1,2\n", encoding="utf-8")
    output_fmu = tmp_path / "Generated.fmu"
    runtime_library = tmp_path / "libpyfmu_csv_fmi2_cs.so"
    runtime_library.write_bytes(b"fake runtime")

    exit_code = main(
        [
            "generate-fmu",
            "--input-csv",
            str(csv_path),
            "--output",
            str(output_fmu),
            "--model-name",
            "Generated",
            "--runtime-library",
            str(runtime_library),
        ]
    )

    captured = capsys.readouterr()

    assert exit_code == 0
    assert "Generated FMU" in captured.out
    assert output_fmu.is_file()
    with ZipFile(output_fmu) as archive:
        assert "resources/data/signals.csv" in archive.namelist()


def test_cli_can_generate_fmu_without_packaged_csv(tmp_path, capsys) -> None:
    csv_path = tmp_path / "signals.csv"
    csv_path.write_text("time,temperature,count:Integer\n0,1,2\n", encoding="utf-8")
    output_fmu = tmp_path / "Generated.fmu"
    runtime_library = tmp_path / "libpyfmu_csv_fmi2_cs.so"
    runtime_library.write_bytes(b"fake runtime")

    exit_code = main(
        [
            "generate-fmu",
            "--input-csv",
            str(csv_path),
            "--output",
            str(output_fmu),
            "--model-name",
            "Generated",
            "--runtime-library",
            str(runtime_library),
            "--no-copy-csv",
        ]
    )

    captured = capsys.readouterr()

    assert exit_code == 0
    assert "Generated FMU" in captured.out
    assert output_fmu.is_file()
    with ZipFile(output_fmu) as archive:
        assert "resources/data/signals.csv" not in archive.namelist()
