@echo off
rem Kompiliavimas su MSVC (Visual Studio 2022). Paleisti is projekto katalogo.
rem Sukuria build\ratas.exe, build\sanity.exe, build\experiments.exe
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if not exist build mkdir build
set FLAGS=/nologo /std:c++17 /O2 /EHsc /W4 /utf-8 /Iinclude /Fo:build\
cl %FLAGS% src\ratas.cpp src\main.cpp /Fe:build\ratas.exe || exit /b 1
cl %FLAGS% src\ratas.cpp tests\sanity.cpp /Fe:build\sanity.exe || exit /b 1
cl %FLAGS% src\ratas.cpp src\std_hashes.cpp src\experiments.cpp /Fe:build\experiments.exe bcrypt.lib || exit /b 1
echo OK
