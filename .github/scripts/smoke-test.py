"""Check that the deployed GUI starts without relying on the Qt SDK environment."""

import os
from pathlib import Path
import subprocess
import sys

executable = Path(sys.argv[1]).resolve()
environment = os.environ.copy()
for variable in ("QT_PLUGIN_PATH", "QML2_IMPORT_PATH", "LD_LIBRARY_PATH", "DYLD_LIBRARY_PATH"):
    environment.pop(variable, None)
# Windows otherwise finds missing DLLs in the development Qt installation.
qt_root = environment.get("QT_ROOT_DIR", "").lower()
if qt_root:
    environment["PATH"] = os.pathsep.join(
        part for part in environment["PATH"].split(os.pathsep)
        if qt_root not in part.lower()
    )
environment["QT_QPA_PLATFORM"] = "offscreen"

with subprocess.Popen([str(executable)], env=environment) as process:
    try:
        code = process.wait(timeout=10)
    except subprocess.TimeoutExpired:
        process.terminate()
        try:
            process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            process.kill()
            process.wait()
        print("Deployed application started and remained running for 10 seconds.")
    else:
        sys.exit(f"Application exited during startup (code {code}).")
