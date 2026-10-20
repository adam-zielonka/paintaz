#!/usr/bin/env bash
set -euo pipefail

cd build

# Pinned releases; extracting the tools avoids requiring FUSE on the runner.
curl --fail --location --retry 3 \
  https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/linuxdeploy-x86_64.AppImage \
  --output linuxdeploy.AppImage
curl --fail --location --retry 3 \
  https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/linuxdeploy-plugin-qt-x86_64.AppImage \
  --output linuxdeploy-plugin-qt.AppImage
chmod +x linuxdeploy.AppImage linuxdeploy-plugin-qt.AppImage
./linuxdeploy.AppImage --appimage-extract >/dev/null
mv squashfs-root linuxdeploy
./linuxdeploy-plugin-qt.AppImage --appimage-extract >/dev/null
mv squashfs-root linuxdeploy-plugin-qt
export PATH="$PWD/linuxdeploy-plugin-qt/usr/bin:$PATH"
export QMAKE="${QT_ROOT_DIR}/bin/qmake"
export OUTPUT="${GITHUB_WORKSPACE}/dist/${PACKAGE_NAME}.AppImage"
export APPIMAGE_EXTRACT_AND_RUN=1

mkdir -p AppDir/usr/share/doc/paintAZ
cp ../LICENSE AppDir/usr/share/doc/paintAZ/
./linuxdeploy/AppRun --appdir AppDir --executable paintAZ \
  --desktop-file ../packaging/paintAZ.desktop \
  --icon-file ../packaging/paintAZ.svg --plugin qt --output appimage
