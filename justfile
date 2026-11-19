# Works from any PowerShell: tools/vsdev.ps1 enters the Visual Studio developer
# environment (cl, cmake, ninja, vcpkg) when it is not already active. See README.md.

set windows-shell := ["powershell.exe", "-NoLogo", "-NoProfile", "-Command"]
set dotenv-load

# Enso installation directory for the python module: ENSO_DIR from the environment or .env
enso_dir := env_var_or_default("ENSO_DIR", "")

vs := "powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File tools/vsdev.ps1"

default: build

# debug build of retreat.exe + sentinel.exe
build:
    {{vs}} cmake --preset win-msvc-debug
    {{vs}} cmake --build --preset win-msvc-debug

# release build of retreat.exe + sentinel.exe, then tests
release:
    {{vs}} cmake --preset win-msvc-release
    {{vs}} cmake --build --preset win-msvc-release
    {{vs}} ctest --preset win-msvc-release

# retreatlib.pyd + sentinel.exe; with an Enso dir both go to <enso>/enso/contrib
python enso=enso_dir:
    {{vs}} cmake --preset win-python {{ if enso == "" { "" } else { "'-DPython3_ROOT_DIR=" + replace(enso, "\\", "/") + "/python' '-DRETREAT_PYD_OUTPUT_DIR=" + replace(enso, "\\", "/") + "/enso/contrib'" } }}
    {{vs}} cmake --build --preset win-python

# NSIS installer (installer/enso-retreat-<version>.exe) from the release build
installer: release
    makensis installer/setup.nsi

# landing page -> repo-root index.html (minified); pass --plain or --out <dir> through to build.mjs
landing *args:
    if (-not (Test-Path landing/node_modules)) { npm --prefix landing install }
    node landing/build.mjs {{args}}

clean:
    cmake -E rm -rf build
