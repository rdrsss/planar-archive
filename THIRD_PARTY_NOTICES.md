# Third-party notices

Planar is licensed under the MIT license (see [LICENSE](LICENSE)). It includes
the third-party software below, cached unmodified under `vendor/` so a
configured build does not touch the network. Each package's own license text
travels with its source. Where a package ships no separate license file, the
notice is given here or in the source header noted.

| Package | License | Notice |
| --- | --- | --- |
| CLI11 2.7.2 | BSD-3-Clause | `vendor/cli11/9d6a/LICENSE` |
| Catch2 3.15.3 (tests only) | BSL-1.0 | `vendor/catch2/c601/LICENSE.txt` |
| Lua 5.5.0 | MIT | `vendor/lua/0b10/src/lua.h` (reproduced below) |
| Glaze 8.1.0 | MIT | `vendor/glaze/a4a7/LICENSE` |
| xxHash 0.8.3 | BSD-2-Clause | `vendor/xxhash/0.8.3/LICENSE` |

Archive URLs and SHA256 pins are recorded in `cmake/dependencies.cmake`.

## Lua

Copyright (C) 1994-2025 Lua.org, PUC-Rio.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
