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
    'qtbase-everywhere-src-6.10.3.tar.xz': (
        'https://download.qt.io/archive/qt/6.10/6.10.3/submodules/qtbase-everywhere-src-6.10.3.tar.xz',
        '383dc907816338f0cba72088a524c07458dfc69ce684ca9132fcc4fe91c24b0b'),
    'emscripten-4.0.7.tar.gz': (
        'https://codeload.github.com/emscripten-core/emscripten/tar.gz/8dc91db45bf96c174531006839472a3924d105aa', None),
    'emsdk-4.0.7.tar.gz': (
        'https://codeload.github.com/emscripten-core/emsdk/tar.gz/c69d433d8509c5c64564c2f0d054bf102a5cf67e', None),
}
manifest = {'commit': sha, 'qt': '6.10.3', 'emscripten': '4.0.7', 'files': {}}
for name, (url, expected) in archives.items():
    dest = source_dir / name
    if not dest.exists():
        urllib.request.urlretrieve(url, dest)
    digest = hashlib.sha256(dest.read_bytes()).hexdigest()
    if expected and digest != expected:
        raise SystemExit(f'Checksum mismatch: {name}')
    manifest['files']['source/' + name] = {'sha256': digest, 'source': url}
# Preserve all upstream license texts, notices, attributions and third-party source.
with tarfile.open(source_dir / 'qtbase-everywhere-src-6.10.3.tar.xz') as archive:
    for member in archive.getmembers():
        relative = Path(*Path(member.name).parts[1:])
        if member.isfile() and relative.parts and (relative.parts[0] == 'LICENSES'
                or relative.name in ('qt_attributions.json', 'LICENSE', 'COPYING')):
            dest = out / 'licenses/qtbase' / relative
            dest.parent.mkdir(parents=True, exist_ok=True)
            dest.write_bytes(archive.extractfile(member).read())
for name in ('emscripten-4.0.7.tar.gz', 'emsdk-4.0.7.tar.gz'):
    with tarfile.open(source_dir / name) as archive:
        member = next(m for m in archive.getmembers() if m.name.endswith('/LICENSE'))
        dest = out / 'licenses' / name.replace('.tar.gz', '.txt')
        dest.parent.mkdir(parents=True, exist_ok=True)
        dest.write_bytes(archive.extractfile(member).read())
shutil.copyfile('LICENSE', out / 'licenses/paintAZ-MIT.txt')
for required in ('licenses/qtbase/LICENSES/GPL-3.0-only.txt',
                 'licenses/paintAZ-MIT.txt', 'licenses/emscripten-4.0.7.txt',
                 'licenses/emsdk-4.0.7.txt', 'source/paintAZ.tar.gz'):
    if not (out / required).is_file():
        raise SystemExit(f'Missing distribution file: {required}')
for name in ('paintAZ.js', 'paintAZ.wasm', 'qtloader.js'):
    data = (out / name).read_bytes()
    manifest['files'][name] = {'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()}
(out / 'build-info.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(json.dumps(manifest, indent=2))
