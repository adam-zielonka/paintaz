# paintAZ #

[![Build and release](https://github.com/adam-zielonka/paintaz/actions/workflows/build.yml/badge.svg)](https://github.com/adam-zielonka/paintaz/actions/workflows/build.yml)

The project was created for the study of cross-platform programming.

All icons used in this app come from [icons8.com](https://icons8.com/).

![paintAZ.png](./img/paintAZ.png)

## Browser version

[Open Paint AZ in your browser](https://adam-zielonka.github.io/paintaz/).
The web build uses the same C++/Qt drawing code. Open imports PNG/JPEG/BMP;
Save downloads a new PNG. Download before reloading: drawings stay in memory.
A browser with WebAssembly and WebGL is required. TIFF is not included in the
web build. This is the desktop Qt interface, with limited mobile ergonomics.

[Build instructions, tests, limits and licensing](docs/webassembly.md).
The web site includes complete corresponding source and third-party notices;
the application's own source remains MIT licensed.

## Help and first launch

The **Help** menu contains **Quick start** (F1), **Licenses, source & help**,
and **About Paint AZ**, including author and Icons8 credits.
The first launch shows a short welcome dialog with **Don't show this again**
checked by default. Closing it remembers that choice on this computer or in
this browser. Open **Help → Quick start** to see the tips again; uncheck the
box there to show them on subsequent launches. Clearing browser site data also
resets the preference.

## Downloads

Download the latest version from [GitHub Releases](https://github.com/adam-zielonka/paintaz/releases/latest):

- **Linux x86_64 / ARM64:** choose the matching AppImage, run `chmod +x paintAZ-*.AppImage`, then launch it. Built on Ubuntu 24.04; requires glibc 2.39 or newer. If FUSE is unavailable, launch with `--appimage-extract-and-run`.
- **Windows x86_64 / ARM64:** choose the matching ZIP, extract it and run `paintAZ.exe` inside the `paintAZ` folder. Keep the bundled DLLs and plugin folders beside the executable. x86_64 requires Windows 10 1809 or later; ARM64 requires Windows 11 on Arm.
- **macOS x86_64 / ARM64:** choose Intel (`x86_64`) or Apple Silicon (`arm64`), open the DMG and drag `paintAZ.app` into Applications. Requires macOS 14.4 or newer. The app is ad-hoc signed, without Apple notarization; macOS may require allowing it in System Settings → Privacy & Security after the first launch attempt.

Qt 6.12.0 libraries and plugins are bundled with each package; installing Qt separately is unnecessary. Each release includes `SHA256SUMS.txt` to verify the downloads.

## Build from source

Release builds use **Qt 6.12.0**, C++17, CMake 3.21+ and Ninja. Install the matching native Qt desktop kit (GCC on Linux, MSVC 2022 on Windows, clang on macOS) and its system dependencies. See the [verified toolchain and native ARM64 notes](docs/toolchain.md).

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.12.0/gcc_64 -DBUILD_TESTING=ON
cmake --build build --parallel 3
ctest --test-dir build --output-on-failure
```

Use your kit path instead of `gcc_64`: `gcc_arm64`, `msvc2022_64`, `msvc2022_arm64` or `macos`. On Windows run from a native x64/ARM64 Visual Studio developer shell with MSVC v143 (14.44). On macOS add `-DCMAKE_OSX_ARCHITECTURES=x86_64` or `-DCMAKE_OSX_ARCHITECTURES=arm64`. The executable is `build/paintAZ`, `build/paintAZ.exe` or `build/paintAZ.app`.

The legacy `paintAZ.pro` and `tests/desktop/tests.pro` remain available for qmake/Qt 5.15 compatibility; official release packages and CTest use Qt 6.12.0. Use separate build directories when switching Qt/toolchains.

## CI and releases

GitHub Actions builds, bundles dependencies and smoke tests the deployed application natively on Linux, Windows and macOS, each for x86_64 and ARM64, for pushes to `main`, pull requests, and manual runs. Download development packages from each run's **Artifacts** section (retained for 14 days).

To publish a release, tag the desired commit and push the tag:

```sh
git tag v1.1.0
git push origin v1.1.0
```

Tags starting with `v` publish a GitHub Release only after all six native builds, workflow tests and package smoke tests pass. Tags containing a hyphen (for example `v1.1.0-rc.1`) publish prereleases. Publication uses the built-in `GITHUB_TOKEN`; no additional repository secrets are needed. A failed run can be rerun for the same tag.
