#!/usr/bin/env python3
"""Independent record_v1 framing oracle; no production codec imports.
Uses standard-library struct and a separately compiled pinned upstream xxHash.
Numeric high/low word formatting verifies canonical digest byte order independently.
"""
import argparse
import ctypes
from pathlib import Path
import struct
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
MAGIC = b'PLANAR-RECORD\x01'
def blob(v):
    return struct.pack('>Q', len(v)) + v

def file(mode, data):
    return MAGIC + b'\x01' + struct.pack('>I', mode) + blob(data)

def directory(children, portable=False):
    return MAGIC + b'\x02' + struct.pack('>II', 493, len(children)) + b''.join(
        blob(name) + bytes([kind]) + (b'' if portable else struct.pack('>Q', identity)) + digest
        for name, kind, identity, digest in children)

def value(encoded):
    return MAGIC + b'\x03' + blob(b'canonical_v1') + blob(encoded)

def request(*parts):
    return MAGIC + b'\x04' + b''.join(blob(p) for p in parts)

class Hash(ctypes.Structure):
    _fields_ = [('low', ctypes.c_uint64), ('high', ctypes.c_uint64)]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='planar-record-oracle-') as temporary:
        library = Path(temporary) / 'xxhash.so'
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(ROOT / 'vendor/xxhash/0.8.3/xxhash.c'), '-o', str(library)], check=True)
        xx = ctypes.CDLL(str(library))
        xx.XXH3_128bits.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        xx.XXH3_128bits.restype = Hash
        def digest(data):
            result = xx.XXH3_128bits(data, len(data))
            return bytes.fromhex(f'{result.high:016x}{result.low:016x}')
        empty = file(420, b'')
        binary = file(493, b'A\x00\xff\r\n')
        children = [(b'A.txt', 1, 1, digest(empty)), (b'bin', 1, 9223372036854775807, digest(binary))]
        fixtures = [
            ('file-empty', empty, empty), ('file-binary-executable', binary, binary),
            ('directory-empty', directory([]), directory([], True)),
            ('directory-two', directory(children), directory(children, True)),
        ]
        # Hand-transcribed canonical_v1 literals, independent of the existing canonical oracle.
        for name, encoded in [('value-map', '050000000000000000'), ('value-list', '040000000000000000'),
                              ('value-minimum', '02ffffffff80000000'), ('value-raw', '03000000000000000300ff0a')]:
            data = value(bytes.fromhex(encoded))
            fixtures.append((name, data, data))
        for name, parts in [
            ('request-none', (b'load', 'driver:é'.encode(), b'none', b'/work/../tree/workspace.lua', b'/registry.lua')),
            ('request-maximum', (b'load', b'actor', b'9223372036854775807', b'./workspace.lua', b'./registry.lua')),
        ]:
            data = request(*parts)
            fixtures.append((name, data, data))
        output = '# name\tstored record hex\tportable preimage hex\tcanonical XXH3-128 hex\n'
        output += ''.join(f'{name}\t{stored.hex()}\t{portable.hex()}\t{digest(portable).hex()}\n'
                          for name, stored, portable in fixtures)
        target = HERE / 'record-v1.tsv'
        if args.check:
            if target.read_text() != output:
                raise SystemExit('record fixture drift')
        else:
            target.write_text(output)
        print(f'{len(fixtures)} independent record vectors verified')
if __name__ == '__main__':
    main()
