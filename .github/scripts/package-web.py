"""Package static output and same-location corresponding source (GPLv3 section 6d)."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tarfile
import urllib.request

out = Path('dist/web')
out.mkdir(parents=True, exist_ok=True)
for name in ('paintAZ.js', 'paintAZ.wasm', 'qtloader.js'):
    shutil.copyfile(Path('build/web') / name, out / name)
for source in Path('web').iterdir():
    if source.is_file():
        shutil.copyfile(source, out / source.name)
source_dir = out / 'source'
source_dir.mkdir(exist_ok=True)
sha = subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True).strip()
subprocess.run(['git', 'archive', '--format=tar.gz', '--prefix=paintAZ/',
                '-o', str(source_dir / 'paintAZ.tar.gz'), 'HEAD'], check=True)
archives = {
    'qtbase-everywhere-src-6.12.0.tar.xz': (
        'https://download.qt.io/official_releases/qt/6.12/6.12.0/submodules/qtbase-everywhere-src-6.12.0.tar.xz',
        'a951bd163c7b80fc6b8c88d7668fb56abf91c152373e13c10666763238131307'),
    'emscripten-5.0.5.tar.gz': (
        'https://codeload.github.com/emscripten-core/emscripten/tar.gz/bc569045b14c3fa7b960734ed2c50da7b20adea4', '80aff1befd18387bc05c582ddc7d78abd5b5dea4c4977c54f9b3b4199c963ff3'),
    'emsdk-5.0.5.tar.gz': (
        'https://codeload.github.com/emscripten-core/emsdk/tar.gz/c9ef2c9d6000341d667a32a807ba47ca489d1e1b', 'bdedc297aa32a5ca7a7a796b1e9c89bb1b05b36242906f11428a244c42101e17'),
}
manifest = {'commit': sha, 'qt': '6.12.0', 'emscripten': '5.0.5', 'files': {}}
for name, (url, expected) in archives.items():
    dest = source_dir / name
    if not dest.exists():
        urllib.request.urlretrieve(url, dest)
    digest = hashlib.sha256(dest.read_bytes()).hexdigest()
    if expected and digest != expected:
        raise SystemExit(f'Checksum mismatch: {name}')
    manifest['files']['source/' + name] = {'sha256': digest, 'source': url}
# Preserve all upstream license texts, notices, attributions and third-party source.
with tarfile.open(source_dir / 'qtbase-everywhere-src-6.12.0.tar.xz') as archive:
    for member in archive.getmembers():
        relative = Path(*Path(member.name).parts[1:])
        if member.isfile() and relative.parts and (relative.parts[0] == 'LICENSES'
                or relative.name in ('qt_attributions.json', 'LICENSE', 'COPYING')):
            dest = out / 'licenses/qtbase' / relative
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(archive.extractfile(member).read())
for name in ('emscripten-5.0.5.tar.gz', 'emsdk-5.0.5.tar.gz'):
    with tarfile.open(source_dir / name) as archive:
        member = next(m for m in archive.getmembers() if m.name.endswith('/LICENSE'))
        dest = out / 'licenses' / name.replace('.tar.gz', '.txt')
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(archive.extractfile(member).read())
shutil.copyfile('LICENSE', out / 'licenses/paintAZ-MIT.txt')
for required in ('licenses/qtbase/LICENSES/GPL-3.0-only.txt',
                 'licenses/paintAZ-MIT.txt', 'licenses/emscripten-5.0.5.txt',
                 'licenses/emsdk-5.0.5.txt', 'source/paintAZ.tar.gz'):
    if not (out / required).is_file():
        raise SystemExit(f'Missing distribution file: {required}')
for name in ('paintAZ.js', 'paintAZ.wasm', 'qtloader.js', 'index.html', 'notices.html', 'source/paintAZ.tar.gz'):
    data = (out / name).read_bytes()
    manifest['files'][name] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
(out / 'build-info.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps(manifest, indent=2))
