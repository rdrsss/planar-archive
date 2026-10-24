The initial independent audit stopped at audit.py line 82 because its expected
CTest summary string was `100% tests passed, 0 tests failed out of 430`.
The archived CMake 4.4.4 CTest output uses `100% tests passed out of 430`.
The JUnit case/status checks had already passed. The audit expectation was
corrected; no candidate or author evidence changed. The initial traceback was
tool-returned output, not a separately captured raw process stderr file.
