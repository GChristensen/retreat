# Enso Retreat

This is a development page. Please visit the main site at: https://gchristensen.github.io/retreat

## Building

Prerequisites: CMake ≥ 3.28, Ninja, [vcpkg](https://vcpkg.io) (`VCPKG_ROOT` set), and a C++23 compiler
with modules support. On Windows that is Visual Studio 2022 17.10+ with the *C++ ATL* component; a
VS Developer PowerShell already provides CMake, Ninja and vcpkg.

```sh
cmake --preset win-msvc-release        # or win-msvc-debug, win-clang-cl-release, win-x86-release
cmake --build --preset win-msvc-release
ctest --preset win-msvc-release
```

The executable ends up in `build/<preset>/src/win32/retreat.exe`. Dependencies (Boost.PropertyTree, WTL)
come from `vcpkg.json`.

Enso Python module (`retreatlib.pyd`):

```sh
cmake --preset win-python -DPython3_ROOT_DIR=<enso>/python -DRETREAT_PYD_OUTPUT_DIR=<enso>/enso/contrib
cmake --build --preset win-python
```

The portable core (scheduler, state machine, settings) also builds on Linux and macOS (GCC 14+ / Clang 18+):

```sh
cmake --preset unix-core && cmake --build --preset unix-core && ctest --preset unix-core
```

Installer: build `win-msvc-release`, then run `makensis installer/setup.nsi`.

## Source layout

| Path | Contents |
|------|----------|
| `src/core` | Portable library `retreat_core`: scheduler, state machine, settings, cron parser. The `system` module and the window/monitor factories are declared here and implemented per platform. |
| `src/win32` | Windows layer: WTL UI (`ui/`), Win32 implementations of `system` and the factories, resources, entry points for `retreat.exe` and `retreatlib.pyd`. |
| `tests` | Core smoke test with a headless stub platform. |
| `installer` | NSIS installer script. |
| `help`, `media`, `landing`, `index.html` | Website (GitHub Pages) and installer assets. |
