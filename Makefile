# Direct replacement build; no host queue, install, release or bin/ copying.
PRESET ?= debug
BUILD_DIR ?= build/rewrite-$(PRESET)
PYTHON ?= python3
.DEFAULT_GOAL := build
.PHONY: configure build test test-all cpp-lint install release dist run
configure:
	cmake --preset $(PRESET)
build: configure
	cmake --build $(BUILD_DIR) --target all
test: configure
	$(PYTHON) scripts/rewrite-check.py test --build-dir $(BUILD_DIR)
cpp-lint: configure
	cmake --build $(BUILD_DIR) --target all planar_tests
	$(PYTHON) scripts/rewrite-check.py lint --build-dir $(BUILD_DIR)
test-all: configure
	$(PYTHON) scripts/rewrite-check.py gate --build-dir $(BUILD_DIR)
install release dist run:
	@echo "Unsupported during the render rewrite; distribution requires a separate design and authorization." >&2
	@exit 2
