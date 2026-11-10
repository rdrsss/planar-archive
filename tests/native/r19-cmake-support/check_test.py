#!/usr/bin/env python3
"""Protocol tests for the R19 adapter's bounded idle-build output recognizer."""
import argparse
import importlib.util
from pathlib import Path
import sys
import unittest


parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--adapter', type=Path, default=Path(__file__).with_name('check.py'))
args = parser.parse_args()
sys.path.insert(0, str(Path.cwd() / 'tests/native/r19-cmake-support'))
spec = importlib.util.spec_from_file_location('r19_check_under_test', args.adapter)
adapter = importlib.util.module_from_spec(spec)
spec.loader.exec_module(adapter)


class NoWorkOutputTests(unittest.TestCase):
    def test_plain_idle_output(self):
        for output in [b'ninja: no work to do.', b'ninja: no work to do.\n']:
            with self.subTest(output=output):
                self.assertTrue(adapter.is_no_work_output(output))

    def test_observed_glob_probe_followed_by_idle_output(self):
        observed = b'[0/2] Re-checking globbed directories...\nninja: no work to do.\n'
        for output in [observed, observed.removesuffix(b'\n')]:
            with self.subTest(output=output):
                self.assertTrue(adapter.is_no_work_output(output))

    def test_work_or_unexplained_output_refuses(self):
        invalid = [
            b'', b'\n', b'[0/2] Re-checking globbed directories...\n',
            b'[1/1] Building CXX object example.o\n',
            b'[1/1] Building CXX object example.o\nninja: no work to do.\n',
            b'[1/1] Linking CXX executable example\nninja: no work to do.\n',
            b'[0/1] Re-running CMake...\nninja: no work to do.\n',
            b'unknown diagnostic\nninja: no work to do.\n',
            b'ninja: no work to do.\nunknown diagnostic\n',
            b'[0/2] Re-checking globbed directories...\n'
            b'[0/2] Re-checking globbed directories...\nninja: no work to do.\n',
            b'[1/2] Re-checking globbed directories...\nninja: no work to do.\n',
            b' ninja: no work to do.\n', b'ninja: no work to do. \n',
            b'ninja: no work to do.\n\n', b'ninja: no work to do.\r\n',
        ]
        for output in invalid:
            with self.subTest(output=output):
                self.assertFalse(adapter.is_no_work_output(output))


if __name__ == '__main__':
    unittest.main(argv=[sys.argv[0]])
