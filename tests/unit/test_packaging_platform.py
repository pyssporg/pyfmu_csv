from __future__ import annotations

from pyfmu_csv.packaging import host_platform_tuple


def test_host_platform_tuple_returns_fmi_binary_layout() -> None:
    platform_dir, extension = host_platform_tuple()

    assert platform_dir in {"linux64", "darwin64", "win64"}
    assert extension in {".so", ".dylib", ".dll"}
