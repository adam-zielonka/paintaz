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

## Downloads

Download the latest version from [GitHub Releases](https://github.com/adam-zielonka/paintaz/releases/latest):

- **Linux x86_64:** download the AppImage, run `chmod +x paintAZ-*.AppImage`, then launch it. Built on Ubuntu 22.04; requires glibc 2.35 or newer. If FUSE is unavailable, launch with `--appimage-extract-and-run`.
- **Windows x86_64:** extract the ZIP and run `paintAZ.exe` inside the `paintAZ` folder. Keep the bundled DLLs and plugin folders beside the executable.
- **macOS x86_64:** open the DMG and drag `paintAZ.app` into Applications. Apple Silicon Macs require Rosetta 2. The app is ad-hoc signed, without Apple notarization; macOS may require allowing it in System Settings → Privacy & Security after the first launch attempt.

Qt libraries and plugins are bundled with each package; installing Qt separately is unnecessary. Each release includes `SHA256SUMS.txt` to verify the downloads.

## Build from source

Install Qt 5.15.2 with its desktop compiler toolchain. On Linux/macOS:

```sh
mkdir build
cd build
qmake ../paintAZ.pro CONFIG+=release CONFIG-=debug CONFIG-=debug_and_release
make -j3
```

On Windows, use an x64 Visual Studio developer command prompt with the MSVC Qt 5.15.2 `bin` directory on `PATH`, and run `nmake` instead of `make`.

## CI and releases

GitHub Actions builds, bundles dependencies and smoke tests the deployed application on Linux, Windows and macOS for pushes to `main`, pull requests, and manual runs. Download development packages from each run's **Artifacts** section (retained for 14 days).

To publish a release, tag the desired commit and push the tag:

```sh
git tag v1.0.1
git push origin v1.0.1
```

Tags starting with `v` publish a GitHub Release only after all three builds and smoke tests pass. Tags containing a hyphen (for example `v1.1.0-rc.1`) publish prereleases. Publication uses the built-in `GITHUB_TOKEN`; no additional repository secrets are needed. A failed run can be rerun for the same tag.
