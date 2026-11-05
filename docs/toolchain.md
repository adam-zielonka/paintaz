# Release toolchain and native architectures

## Qt selection (verified 2026-11-05)

The newest stable publicly downloadable Qt release was **6.12.0**. Qt's
[release announcement](https://www.qt.io/blog/qt-6.12-released),
[release table](https://doc.qt.io/qt-6/qt-releases.html) and
[official download directory](https://download.qt.io/official_releases/qt/6.12/)
were checked against the actual installer catalog, not only release headlines.
No newer 6.12 patch was listed. These are fixed release pins, not automatic
"latest" selectors.

With aqtinstall from commit `5a49f2b391ae8ee1d3c0dd87f8b33c1d49a4deb7`
(version `3.3.1.dev168`) and py7zr `1.1.4`, these catalog queries returned the
kits used in CI:

```sh
python3 -m aqt list-qt linux desktop --spec 6.12
python3 -m aqt list-qt linux desktop --arch 6.12.0
python3 -m aqt list-qt linux_arm64 desktop --arch 6.12.0
python3 -m aqt list-qt windows desktop --arch 6.12.0
python3 -m aqt list-qt windows_arm64 desktop --arch 6.12.0
python3 -m aqt list-qt mac desktop --arch 6.12.0
python3 -m aqt list-qt all_os wasm --arch 6.12.0
```

Released aqt **3.3.0** can install Linux/macOS/Wasm, but fails Windows x64
6.12.0: it requests `qt6_6120/qt6_6120/Updates.xml`, whereas Qt now splits
Windows repositories by compiler. The pinned upstream aqt commit includes
support for the new layout; it was installed and the Windows x64 query and
native Windows ARM64 and Linux ARM64 Qt Base downloads were verified. This explicit temporary
source pin should be replaced with a tested stable aqt release once available.
The install action's default py7zr 1.1.0 is too old for this aqt commit.

## Desktop builds

[GitHub's runner reference](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)
lists the hosted native x64 and ARM64 machines used here. All six jobs build
the application and QtTest executable with CMake/Ninja, check the application
binary's architecture, execute the shared workflow tests on that architecture,
package it and run the deployed application with SDK search paths removed.
Every architecture produces a separate release asset and checksum.

| Package suffix | Hosted runner | Qt host / aqt architecture | Installed kit |
| --- | --- | --- | --- |
| `linux-x86_64.AppImage` | `ubuntu-24.04` | `linux` / `linux_gcc_64` | `gcc_64` |
| `linux-arm64.AppImage` | `ubuntu-24.04-arm` | `linux_arm64` / `linux_gcc_arm64` | `gcc_arm64` |
| `windows-x86_64.zip` | `windows-2022` | `windows` / `win64_msvc2022_64` | `msvc2022_64` |
| `windows-arm64.zip` | `windows-11-arm` | `windows_arm64` / `win64_msvc2022_arm64` | `msvc2022_arm64` |
| `macos-x86_64.dmg` | `macos-15-intel` | `mac` / `clang_64` | `macos` |
| `macos-arm64.dmg` | `macos-15` | `mac` / `clang_64` | `macos` |

Qt's [supported-platform requirements](https://doc.qt.io/qt-6.12/supported-platforms.html)
apply. Our Linux packages are built on Ubuntu 24.04 and require glibc 2.39+;
this is a conservative application-package baseline even though Qt's x64 kit
itself can support older glibc. The official Linux ARM64 kit is built on
Ubuntu 24.04 (glibc 2.39). macOS packages require 14.4+. Windows x64 requires
Windows 10 1809+; the native ARM64 package requires Windows 11 on Arm.

Linux development needs a C++ compiler, OpenGL development headers/libraries,
fontconfig/freetype and Qt's XCB/XKB dependencies. The install-qt action installs
Linux dependencies on the disposable hosted runners. AppImage tooling is pinned
in `package-linux.sh`: both linuxdeploy `1-alpha-20251107-1` and its Qt plugin
`1-alpha-20250213-1` publish **x86_64** and **aarch64** assets. The script maps
our `arm64` package name to the upstream `aarch64` tool name and extracts tools
instead of relying on FUSE.

Windows uses the **native** Qt ARM64 repository, not the
`win64_msvc2022_arm64_cross_compiled` kit in the x64 repository. Its downloaded
`qmake.exe`, `moc.exe` and `windeployqt.exe` are ARM64 PE binaries. The native
MSVC shell selects v143 toolset **14.44**, including on the Windows ARM image
which now ships Visual Studio 2026 with that older toolset installed. The
[runner image inventory](https://github.com/actions/runner-images/blob/main/images/windows/Windows11-VS2026-Arm64-Readme.md)
lists the 14.44 ARM64 component.

`windeployqt` from the selected kit copies matching Qt libraries/plugins.
Its `--compiler-runtime` release mode copies a `vc_redist.<arch>.exe` installer,
which is insufficient for a portable ZIP on a fresh machine. Instead,
`package-windows-runtime.ps1` copies the matching redistributable release CRT
DLLs from `VCToolsRedistDir` beside the app, then checks **every** packaged DLL's
PE architecture. This follows Microsoft's documented
[app-local deployment mechanism](https://learn.microsoft.com/en-us/cpp/windows/choosing-a-deployment-method?view=msvc-170).
App-local CRT updates must be delivered with future application packages.

macOS uses Qt's combined `macos` kit with an explicit
`CMAKE_OSX_ARCHITECTURES=x86_64` or `arm64`. The app executable is a thin native
binary; Qt frameworks may contain both architectures. `macdeployqt` bundles
frameworks/plugins and the application is ad-hoc signed before packaging.
Notarization and a Developer ID certificate are not configured.

The qmake projects retain Qt 5.15 compatibility for development/tests, use
C++17 consistently, and do not choose the release Qt version. Official release
builds use the CMake Qt 6.12 target and CTest.

## Browser and corresponding source

See [WebAssembly instructions](webassembly.md). Qt 6.12.0 requires
**Emscripten 5.0.5**, as specified in its
[official platform notes](https://doc.qt.io/qt-6.12/wasm.html#installing-emscripten).
The matching native host tools and single-thread Wasm Qt Base kit were actually
downloaded and used for a local Release compilation. The native Linux x64
CMake build and shared QtTest suite also passed on this toolchain.

The source package pins were checked by downloading and hashing the archives:

| Source | Immutable revision | SHA256 |
| --- | --- | --- |
| Qt Base 6.12.0 | official `6.12.0/submodules` archive | `a951bd163c7b80fc6b8c88d7668fb56abf91c152373e13c10666763238131307` |
| Emscripten 5.0.5 | `bc569045b14c3fa7b960734ed2c50da7b20adea4` | `80aff1befd18387bc05c582ddc7d78abd5b5dea4c4977c54f9b3b4199c963ff3` |
| emsdk 5.0.5 | `c9ef2c9d6000341d667a32a807ba47ca489d1e1b` | `bdedc297aa32a5ca7a7a796b1e9c89bb1b05b36242906f11428a244c42101e17` |

The Qt Base digest also matches the official `.sha256` file. Wasm licensing is
still GPLv3/commercial; the site retains complete corresponding source and all
upstream license/attribution texts beside the executable. App sources stay MIT.
The Qt 6.12.0 `qwasmlocalfileaccess.cpp` source still detects the browser File
System Access API via `showOpenFilePicker`, preserving the custom page's
file-input/download fallback. Browser smoke checks must pass for the integrated
UI changes before publication.

## Versioning and release gate

The only remote application tag found was `v1.0.0`. The Qt migration and new
platforms justify a minor release: the proposed next tag is **v1.1.0**.
CMake and qmake package metadata use `1.1.0`; the release tag remains an explicit
maintainer action. Run all six hosted builds and browser checks on the integrated
commit before tagging/publishing. Local Linux x64 validation cannot substitute
for Windows, macOS or ARM64 hosted execution.
