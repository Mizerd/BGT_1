import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
SHARED = REPO / "Rokas - AI" / "experiments"
VCVARS = r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
BUILD = HERE / "build"
RAW = HERE / "results" / "raw"
DATA = HERE / "data"


def cl(args):
    cmd = f'"{VCVARS}" >nul && cl /nologo /std:c++20 /O2 /EHsc /utf-8 /DNDEBUG {args}'
    subprocess.run(cmd, shell=True, check=True, cwd=HERE)


def ratas(args, stdin=b""):
    out = subprocess.run([str(BUILD / "ratas.exe"), *args], input=stdin,
                         capture_output=True, cwd=HERE).stdout.decode("utf-8", "replace")
    for line in out.splitlines():
        if "Ratas-256: " in line:
            return line.split("Ratas-256: ")[1].strip()
    return ""


def main():
    BUILD.mkdir(exist_ok=True)
    RAW.mkdir(parents=True, exist_ok=True)
    cl(f'/I"experiments" /I"{SHARED}" /Fo:build\\ experiments\\impl_ratas.cpp '
       f'"{SHARED}\\experiments.cpp" /Fe:build\\experiments.exe')
    cl("/Fo:build\\ ratas.cpp /Fe:build\\ratas.exe")

    exp = str(BUILD / "experiments.exe")
    for name, args in [("inputs", ["inputs", str(DATA / "exp1")]),
                       ("speed", ["speed", str(DATA / "konstitucija.txt")]),
                       ("collisions", ["collisions"]), ("structured", ["structured"]),
                       ("avalanche", ["avalanche"]), ("guess", ["guess"])]:
        print("eksperimentas:", name, flush=True)
        with open(RAW / f"{name}.csv", "wb") as f:
            subprocess.run([exp, *args], stdout=f, check=True)

    rows = []
    for f in sorted((DATA / "exp1").iterdir()):
        rel = f"data/exp1/{f.name}"
        first, second = ratas([rel]), ratas([rel])
        rows.append(f"runs,{f.name},{first},{int(first == second and first != '')}")
        body = f.read_bytes()
        if not any(b in body for b in b"\r\n\0"):
            rows.append(f"typed,{f.name},{int(ratas([], b'1\n' + body + b'\n') == first)}")
            rows.append(f"text,{f.name},{int(ratas([], b'2\n' + rel.encode() + b'\n') == first)}")
    (RAW / "cli.csv").write_text("\n".join(rows) + "\n", encoding="utf-8")

    subprocess.run([sys.executable, "-X", "utf8", str(SHARED / "report.py"), str(RAW),
                    str(HERE / "results")], check=True)

    md = HERE / "results" / "exp1_3_teisingumas.md"
    text = md.read_text(encoding="utf-8")
    text = (text.replace("su `--file`", "su failo argumentu")
                .replace("`--file` maiša", "Failo argumento maiša")
                .replace("`--text` sutampa su failo maiša", "Meniu 2 (kelias įvestas ranka) sutampa su failo maiša")
                .replace("Ranka ir `--text` tikrinami", "Ranka tikrinami"))
    md.write_text(text, encoding="utf-8")


if __name__ == "__main__":
    main()
