@echo off
rem Pilnas atkartojimas Windows'e (MSVC): kompiliavimas, patikros, visi eksperimentai, grafikai.
rem Reikia: Visual Studio 2022 (cl), Python 3, Git Bash (sh) CLI patikroms.
setlocal
cd /d "%~dp0"
call "%~dp0build.bat" || exit /b 1
"%~dp0build\sanity.exe" || exit /b 1
if not exist data\exp1 mkdir data\exp1
"%~dp0build\experiments.exe" all || exit /b 1
set SH=sh
where sh >nul 2>&1 || set "SH=C:\Program Files\Git\bin\sh.exe"
"%SH%" tests/cli_checks.sh build || echo CLI patikroms reikia Git Bash (sh).
python tools\plot.py
echo Viskas baigta. Rezultatai: results\
