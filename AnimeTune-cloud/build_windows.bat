@echo off
setlocal
cd /d "%~dp0"
echo === AnimeTune local build ===
where cmake >nul 2>nul
if errorlevel 1 (
  echo ERROR: CMake was not found. Use the GitHub Actions cloud build instead,
  echo or install CMake and Visual Studio 2022 with Desktop C++ workload.
  pause
  exit /b 1
)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto failed
cmake --build build --config Release --parallel
if errorlevel 1 goto failed
echo.
echo Build completed. Look under build\AnimeTune_artefacts\Release\VST3\
pause
exit /b 0
:failed
echo.
echo BUILD FAILED. Read the error messages above.
pause
exit /b 1
