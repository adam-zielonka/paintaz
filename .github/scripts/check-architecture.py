"""Reject emulated/wrong-architecture desktop builds before packaging."""
from pathlib import Path
import struct
import sys

path, expected = Path(sys.argv[1]), sys.argv[2]
with path.open('rb') as binary:
    header = binary.read(64)
    if header[:4] == b'\x7fELF':
        endian = '<' if header[5] == 1 else '>'
        machine = struct.unpack_from(endian + 'H', header, 18)[0]
        actual = {62: 'x86_64', 183: 'arm64'}.get(machine)
    elif header[:2] == b'MZ':
        binary.seek(struct.unpack_from('<I', header, 60)[0])
        pe = binary.read(6)
        if pe[:4] != b'PE\0\0':
            raise SystemExit(f'Invalid PE header: {path}')
        actual = {0x8664: 'x86_64', 0xaa64: 'arm64'}.get(struct.unpack_from('<H', pe, 4)[0])
    elif header[:4] == b'\xcf\xfa\xed\xfe':
        actual = {0x01000007: 'x86_64', 0x0100000c: 'arm64'}.get(struct.unpack_from('<I', header, 4)[0])
    else:
        raise SystemExit(f'Unsupported executable format (expected a thin native binary): {path}')
if actual != expected:
    raise SystemExit(f'{path}: expected {expected}, got {actual}')
print(f'{path}: verified native {actual} executable')
