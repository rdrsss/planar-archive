#!/usr/bin/env python3
"""Independent request_v2 framing oracle; no production codec imports.
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
def blob(v):
    return struct.pack('>Q', len(v)) + v

class Hash(ctypes.Structure):
    _fields_ = [('low', ctypes.c_uint64), ('high', ctypes.c_uint64)]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    with tempfile.TemporaryDirectory(prefix='planar-request-v2-oracle-') as temporary:
        library = Path(temporary) / 'xxhash.so'
        subprocess.run(['cc', '-shared', '-fPIC', '-O2', str(ROOT / 'vendor/xxhash/0.8.3/xxhash.c'), '-o', str(library)], check=True)
        xx = ctypes.CDLL(str(library))
        xx.XXH3_128bits.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        xx.XXH3_128bits.restype = Hash
        def digest(data):
            result = xx.XXH3_128bits(data, len(data))
            return bytes.fromhex(f'{result.high:016x}{result.low:016x}')
        fixtures = []
        for name, actor, head, profile in [
            ('embedded-none', 'driver:é', 'none', None),
            ('profile-maximum', 'actor', '9223372036854775807', '/private/profiles/é.json'),
        ]:
            parts = ['load', actor, head, '../workspace.lua', './registry.lua']
            data = b'PLANAR-REQUEST\x02' + b''.join(blob(p.encode()) for p in parts)
            data += b'\x00' if profile is None else b'\x01' + blob(profile.encode())
            fixtures.append((name, data))
        output = '# name\tcomplete frame hex\tcanonical seed-zero XXH3-128 hex\n'
        output += ''.join(f'{name}\t{data.hex()}\t{digest(data).hex()}\n' for name, data in fixtures)
        target = HERE / 'request-v2.tsv'
        if args.check:
            if target.read_text() != output:
                raise SystemExit('request-v2 fixture drift')
        else:
            target.write_text(output)
        print(f'{len(fixtures)} independent request-v2 vectors verified')
if __name__ == '__main__':
    main()
