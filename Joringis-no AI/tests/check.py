import subprocess
import sys
from pathlib import Path

from ratas_ref import ratas256

HERE = Path(__file__).resolve().parent.parent
BUILD = HERE / "build"


def cli_hash(path):
    out = subprocess.run([str(BUILD / "ratas.exe"), str(path)], capture_output=True,
                         stdin=subprocess.DEVNULL).stdout.decode("utf-8", "replace")
    for line in out.splitlines():
        if line.startswith("Ratas-256: "):
            return line.split(": ")[1].strip()
    return ""


def main():
    ok = total = 0
    files = sorted((HERE / "data" / "exp1").iterdir()) + [HERE / "pavyzdys.txt", HERE / "data" / "konstitucija.txt"]
    for f in files:
        total += 1
        if cli_hash(f) == ratas256(f.read_bytes()):
            ok += 1
        else:
            print("NESUTAMPA failas:", f.name)
    print(f"programa ir Python realizacija, failai: {ok}/{total}")

    run = subprocess.run([str(BUILD / "stream_test.exe")], capture_output=True, text=True)
    lines = run.stdout.split()
    ok2 = total2 = 0
    for n, h in zip(lines[0:-2:2], lines[1:-2:2]):
        n = int(n)
        data = bytes((i * 131 + n) & 0xFF for i in range(n))
        total2 += 1
        if ratas256(data) == h:
            ok2 += 1
        else:
            print("NESUTAMPA ilgis:", n)
    print(f"C++ ir Python realizacija, ilgiai 0-600 ir dideli: {ok2}/{total2}")
    print(f"dalimis skaiciuota maisa nesutapo: {lines[-1]} kartu is {total2 * 5}")
    return 0 if ok == total and ok2 == total2 and lines[-1] == "0" and run.returncode == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
