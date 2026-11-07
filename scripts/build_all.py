import subprocess
import pathlib

BUILD_VERSIONS = [
    "1.40.8",
    "1.45.3"
]

for ver in BUILD_VERSIONS:
    try:
        print(f"::group::building {ver}")
        if 0 != subprocess.call(["python3", str(pathlib.Path(".") / "scripts" / "multibuild" / "apply.py"), ver]):
            raise
        if 0 != subprocess.call(["qpm","restore"]):
            raise
        if 0 != subprocess.call(["qpm","s","build"]):
            raise
        if 0 != subprocess.call(["qpm","qmod","zip"]):
            raise
        libname = pathlib.Path("build") / "libLoqolizer.so"
        libname_debug = pathlib.Path("build") / "debug" / "libLoqolizer.so"

        if libname.exists():
            libname.copy(pathlib.Path("build") / f"libLoqolizer.{ver}.so")
        if libname_debug.exists():
            libname_debug.copy(pathlib.Path("build") / f"debug_libLoqolizer.{ver}.so")

    finally:
        print("::endgroup::")