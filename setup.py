from __future__ import annotations

import os
import shutil
import sys
from pathlib import Path

from setuptools import setup
from setuptools.command.build_py import build_py as _build_py

try:
    from wheel.bdist_wheel import bdist_wheel as _bdist_wheel
except ImportError:  # pragma: no cover
    _bdist_wheel = None


ROOT = Path(__file__).resolve().parent
sys.path.insert(0, str(ROOT / "python"))

from pyfmu_csv.packaging import default_runtime_library_path, runtime_library_filename  # noqa: E402


class build_py(_build_py):
    def run(self) -> None:
        super().run()
        if getattr(self.distribution, "pyfmu_csv_bundle_runtime", False):
            self._bundle_runtime_if_available()

    def _bundle_runtime_if_available(self) -> None:
        source = default_runtime_library_path(ROOT)
        if not source.is_file():
            if os.environ.get("PYFMU_CSV_REQUIRE_BUNDLED_RUNTIME") == "1":
                raise FileNotFoundError(
                    f"runtime library does not exist: {source}. Build the native runtime before building the wheel."
                )
            self.warn(f"skipping bundled runtime because {source} does not exist")
            return

        destination_dir = Path(self.build_lib) / "pyfmu_csv" / "_runtime"
        destination_dir.mkdir(parents=True, exist_ok=True)
        destination = destination_dir / runtime_library_filename()
        for existing in destination_dir.glob("libpyfmu_csv_fmi2_cs.*"):
            existing.unlink()
        shutil.copy2(source, destination)


if _bdist_wheel is not None:

    class bdist_wheel(_bdist_wheel):
        def run(self) -> None:
            self.distribution.pyfmu_csv_bundle_runtime = True
            super().run()

        def finalize_options(self) -> None:
            super().finalize_options()
            self.root_is_pure = False


cmdclass = {"build_py": build_py}
if _bdist_wheel is not None:
    cmdclass["bdist_wheel"] = bdist_wheel


setup(cmdclass=cmdclass)
