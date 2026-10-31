# WebAssembly build and distribution

The browser app is the same six C++ implementation files, three Qt Designer
forms and `resources.qrc` as the desktop app. `CMakeLists.txt` adds a Qt6 target;
`paintAZ.pro` and the Qt5 desktop packaging remain supported. There is no
image upload API or application server.

## Toolchain

The `WebAssembly` workflow uses an isolated Ubuntu 24.04 GitHub runner:

- Qt **6.10.3**, `wasm_singlethread`, Qt Base (including JPEG plugin).
- Qt **6.10.3**, `linux_gcc_64` host tools, installed as `gcc_64`.
- Emscripten **4.0.7**, emsdk commit
  `c69d433d8509c5c64564c2f0d054bf102a5cf67e`, Emscripten commit
  `8dc91db45bf96c174531006839472a3924d105aa`.
- aqtinstall **3.3.0** and SHA-pinned install-qt/checkout/artifact actions.
- CMake/Ninja from the Ubuntu runner, C++17, Release.
- Browser checks: Playwright **1.64.1** and its pinned Chromium revision.

Qt6.10's [official platform notes](https://doc.qt.io/qt-6.10/wasm.html)
require Emscripten 4.0.7; patch versions must match the host tools.
The official binary repositories are
[Wasm](https://download.qt.io/online/qtsdkrepository/all_os/wasm/qt6_6103/qt6_6103_wasm_singlethread/)
and [Linux host](https://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt6_6103/qt6_6103/).
The single-threaded build needs neither SharedArrayBuffer nor COOP/COEP.
There is no Asyncify/JSPI dependency and no nested dialog event loop on the web.

To reproduce in an isolated development environment with Python, aqtinstall,
CMake, Ninja and the Linux Qt host dependencies available:

```sh
python3 -m aqt install-qt linux desktop 6.10.3 linux_gcc_64 -O "$PWD/Qt"
python3 -m aqt install-qt all_os wasm 6.10.3 wasm_singlethread -O "$PWD/Qt" --archives qtbase
# Obtain emsdk at the exact commit above (or use the supplied emsdk source).
./emsdk/emsdk install 4.0.7
./emsdk/emsdk activate 4.0.7
. ./emsdk/emsdk_env.sh
em++ --version
./Qt/6.10.3/gcc_64/libexec/moc --version
cmake -S . -B build/web -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$PWD/Qt/6.10.3/wasm_singlethread/lib/cmake/Qt6/qt.toolchain.cmake" \
  -DCMAKE_BUILD_TYPE=Release -DQT_HOST_PATH="$PWD/Qt/6.10.3/gcc_64"
cmake --build build/web --parallel 3
python3 .github/scripts/package-web.py
npm install --prefix tests/web --ignore-scripts --package-lock=false
tests/web/node_modules/.bin/playwright install chromium
node tests/web/smoke.mjs --dir dist/web
```

`package-web.py` requires a committed Git checkout: it archives the exact
`HEAD` source, not uncommitted edits. It verifies the official Qt Base source
SHA256, obtains Emscripten/emsdk source by immutable commit and records every
source checksum in `build-info.json`. Reuse the local supplied source archives
in `dist/web/source` when rebuilding offline. The emsdk compiler binaries and
Qt development kits must have been obtained before an offline build.
CMake/Ninja/runner patch versions are recorded in CI logs; byte-for-byte
reproducibility across different build machines is not claimed.

To rebuild Qt itself from the supplied complete Qt Base source, activate the
same emsdk and configure Qt with a matching native Qt6.10.3 host build:

```sh
/path/to/qtbase/configure -platform wasm-emscripten -qt-host-path /path/to/host-qt \
  -prefix /path/to/install-qt-wasm -release -static -no-feature-thread \
  -nomake examples -nomake tests
cmake --build . --parallel 3
cmake --install .
```

Qt embeds its bundled third-party sources and font; all their notices remain
in the source archive. Use Qt's configure documentation when choosing optional
features. The normal app workflow uses the official prebuilt kit, not this
optional Qt rebuild.

## File behavior and limits

`New` and color selection use `open()` and accepted/selected signals. Cancel
never applies a partially selected value. Desktop open/save keep their native
path behavior. Web open uses `QFileDialog::getOpenFileContent`; saving encodes
PNG with `QBuffer` and uses `QFileDialog::saveFileContent`.
[Qt's file API documentation](https://doc.qt.io/qt-6.10/qfiledialog.html#getOpenFileContent)
explains browser sandbox access. Qt6.10 normally prefers the File System Access
API on Chromium, which opens an OS save picker rather than generating a download.
This page intentionally selects Qt's HTML file-input/download fallback by hiding
`showOpenFilePicker` within the app page before loading Qt. This is a pinned
Qt6.10 compatibility choice, also used in the tests, with actual browser file
selection/downloads (no mocked file data or test-only app hooks). Recheck this
detection when upgrading Qt. The browser controls download name and
location; Save and Save As both request a new download and cannot promise to
overwrite the original file. An imported JPEG/BMP is downloaded as PNG.

PNG, JPEG and BMP import are included. TIFF is deliberately not advertised by
the web picker: this minimal Qt Base kit does not bundle Qt Image Formats/TIFF.
Unsupported or malformed input is rejected without clearing the current
image/history. Dimensions must be 1–4096 per side and at most 4,194,304 pixels;
web encoded input is capped at 32 MiB. These checks reduce memory pressure,
but do not guarantee that a memory-constrained device can allocate every
allowed image. Undo retains full image snapshots; long sessions can consume
substantial memory. Reload/close loses the in-memory drawing: download first.
There is no persistence, autosave or service worker cache.

A browser with WebAssembly and WebGL is required. Upstream tests desktop
Chrome/Firefox/Safari/Edge; our automated check exercises Chromium only.
The Qt Widgets UI remains a desktop interface. Toolbars offer overflow menus
and the image area scrolls on small viewports; touch uses Qt's synthesized
mouse input. This does not claim full mobile ergonomics, native device testing
or screen-reader parity. The bundled font is available to Text; device font
access is restricted by the browser sandbox.

## Licensing and complete source

`LICENSE` stays MIT for the application's original sources. The Qt Wasm port
is [GPLv3 or commercial](https://doc.qt.io/qt-6.10/wasm.html#license).
This web distribution uses GPLv3 for the combined statically linked work,
without purchasing a license. Icons8 attribution remains in the UI and web
notices. `web/notices.html` links license texts and the complete corresponding
source at the **same public location**, without relying on expiring Actions
artifacts or a third-party source URL. The archive includes all Qt Base source
(including third parties), app/build scripts and Emscripten runtime/library
source, plus emsdk scripts. All upstream copyright and license notices are
preserved. No other Qt module is linked.

A verified Release build of this port produced a 14,044,742-byte Wasm,
226,862-byte app JS and 12,354-byte Qt loader (about 13.62 MiB combined,
uncompressed). The complete downloadable distribution with source was about
95 MiB on disk. Exact numbers for each revision remain in the manifest.

`build-info.json` identifies the exact commit, toolchain, file hashes and raw
JS/Wasm sizes for that deployment. Source archives are downloaded only when
requested; they are not part of application startup. Server compression may
reduce transfer size; consult the actual response headers rather than assuming
GitHub Pages compresses every asset.

## Validation and rollback

The browser smoke test drives actual keyboard, mouse, file picker and downloads
in the compiled app, checks saved image dimensions/pixels, and captures only
synthetic drawings. It also checks a 390x844 viewport and a CDP-emulated touch
stroke by its exported pixels; this is not evidence from a physical phone. It does not replace an independent reviewer. The desktop
workflow retains Linux/Windows/macOS build, packaging and native startup smoke,
and adds QtTest checks for image transactions, history and asynchronous dialogs.

Pages publication is limited to the repository's `main` workflow with Pages
and OIDC permissions only in its deployment job. Before publication all web
checks must pass. The published URL must be checked again with
`node tests/web/smoke.mjs --url https://adam-zielonka.github.io/paintaz/`.

Rollback uses an ordinary revert/forward commit restoring the previous app and
workflow, then a successful web deployment of that revision. For the first
publication, disable the Pages deployment job or unpublish the Pages site if
necessary; retain source archives for any binaries still distributed. Never
force-push or change installed desktop applications as part of a web rollback.
