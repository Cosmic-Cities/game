@echo off
setlocal

set BUILD_DIR=build

if not exist "%BUILD_DIR%\build.ninja" (
    cmake -S . -B %BUILD_DIR% -G Ninja
)

cmake --build %BUILD_DIR%

endlocal