# Linux gate image for the Planar rewrite (V5-02-X08 operator decision Q5 A;
# DECISIONS rows 3-10c). Built only by scripts/linux-gate.sh, which runs ONE
# `docker build --target evidence --output type=local,dest=<out>/container`
# from the repository root with no image tag and no bind mount.
#
# Building needs network access to apt.llvm.org (LLVM 23 apt packages and the
# signing key), github.com (the Kitware CMake release tarball) and the base image
# registry. Only linux/arm64 is covered; x86_64 is not covered (recorded limit).
#
# Gate RUN steps never fail the layer on a test or build failure. Each gate
# command writes its raw log and its exit status to /home/planar/out, and
# tests/native/v5-02-x08-support/validate.py judges the raw logs. A missing
# expected-schema.sql fails the build at the first RUN after COPY instead.

# Base image pinned by the multi-architecture index digest (linux/arm64/v8 resolves
# to sha256:19cad59657037cffa0ac9a48107019b768900ff1071140965827513ac866bd5e).
FROM ubuntu:24.04@sha256:24737cef17f4e22b1977df7d9a05e77c4e28b7da5b8911f8d8ccda136bf3573c AS gate

SHELL ["/bin/bash", "-o", "pipefail", "-c"]

# JOBS is the parallelism of the gate builds.
ARG JOBS=4
# Exact apt version of the LLVM 23 packages, read from the apt.llvm.org noble
# arm64 Packages index (llvm-toolchain-noble-23, main) on 2026-10-29. apt.llvm.org drops superseded builds, so refresh this pin from the
# index when the build reports the version as not found (first pin 2026-10-28 was dropped on 2026-10-29). It must match
# ^(1:)?23\.\d+\.\d+ and be >= 23.1.0 (docs/toolchain-parity.md).
ARG LLVM_APT_VERSION=1:23.1.5~++20261028083854+a738f2737189-1~exp1~20261028203912.92
# CMake 4.4.x (>= 4.3; the import-std UUID table covers 4.3.x and 4.4.x only).
# CMAKE_SHA256 is the linux-aarch64 tarball line of the published
# https://cmake.org/files/v4.4/cmake-4.4.5-SHA-256.txt.
ARG CMAKE_VERSION=4.4.5
ARG CMAKE_SHA256=301d5063ab259577c1b57ce5490b1aab9ea5c56dc4c822f73499553dab94925f

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      ca-certificates curl gnupg lsb-release wget git python3 ninja-build \
      pkg-config xz-utils sqlite3 \
 && rm -rf /var/lib/apt/lists/*

# apt.llvm.org signing key, dearmored into a keyring used with signed-by.
RUN curl -fsSL -o /tmp/llvm-snapshot.gpg.key https://apt.llvm.org/llvm-snapshot.gpg.key \
 && gpg --batch --dearmor -o /usr/share/keyrings/llvm-snapshot.gpg /tmp/llvm-snapshot.gpg.key \
 && rm -f /tmp/llvm-snapshot.gpg.key \
 && echo "deb [arch=arm64 signed-by=/usr/share/keyrings/llvm-snapshot.gpg] https://apt.llvm.org/noble/ llvm-toolchain-noble-23 main" \
      > /etc/apt/sources.list.d/llvm-toolchain-noble-23.list

# Every LLVM package is pinned to LLVM_APT_VERSION. The clang-23 dependency closure
# (llvm-23-linker-tools, libllvm23, libclang-cpp23, libclang1-23) is pinned by the
# index's own exact dependencies to the same version.
RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      clang-23=${LLVM_APT_VERSION} \
      libc++-23-dev=${LLVM_APT_VERSION} \
      libc++abi-23-dev=${LLVM_APT_VERSION} \
      lld-23=${LLVM_APT_VERSION} \
      llvm-23=${LLVM_APT_VERSION} \
      libclang-common-23-dev=${LLVM_APT_VERSION} \
 && rm -rf /var/lib/apt/lists/*

# Kitware linux-aarch64 tarball, verified against the pinned SHA256 before extraction.
RUN curl -fsSL -o /tmp/cmake.tar.gz "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-aarch64.tar.gz" \
 && echo "${CMAKE_SHA256}  /tmp/cmake.tar.gz" | sha256sum -c - \
 && tar -xzf /tmp/cmake.tar.gz -C /usr/local --strip-components=1 \
 && rm -f /tmp/cmake.tar.gz

# tests/acceptance/bootstrap_cli.py compiles vendored xxhash with `cc`; the image has no cc, so point it at the pinned clang.
RUN ln -s /usr/bin/clang-23 /usr/local/bin/cc

# Non-root user uid 1000. Ubuntu 24.04 ships a default "ubuntu" user at uid 1000,
# which is removed first. Tests branch on geteuid() == 0, so the gate must not run
# as root (DECISIONS row 8).
RUN if getent passwd ubuntu >/dev/null; then userdel --remove ubuntu; fi \
 && if getent group ubuntu >/dev/null; then groupdel ubuntu; fi \
 && groupadd --gid 1000 planar \
 && useradd --uid 1000 --gid 1000 --create-home --shell /bin/bash planar \
 && mkdir -p /home/planar/src /home/planar/out \
 && chown -R planar:planar /home/planar

USER planar
WORKDIR /home/planar/src

# The only source entry point. Source is copied into the image filesystem; the
# checkout is never bind-mounted and no VOLUME is declared (DECISIONS rows 4, 10c).
COPY --chown=planar:planar . /home/planar/src

# First RUN after COPY: the R12B ledger schema is required by
# src/engine/CMakeLists.txt:254 and is gitignored, so it is re-included by the
# dockerignore. Then record the image context as sorted "sha256  path" lines.
RUN test -s tests/native/r12b-evidence/expected-schema.sql \
 && find . \( -type f -o -type l \) -print | LC_ALL=C sort | sed 's#^\./##' \
      | xargs -d '\n' -r sha256sum > /home/planar/out/context-manifest.txt

# Gate. Each command runs to completion (no set -e); its raw log goes to
# /home/planar/out/<preset>-<step>.log and its exit status to a matching .exit file.
RUN cd /home/planar/src \
 && for p in debug release; do \
      cmake --preset "$p" > "/home/planar/out/$p-configure.log" 2>&1; echo $? > "/home/planar/out/$p-configure.exit"; \
      cmake --build "build/rewrite-$p" --target planar_tests -j"${JOBS}" > "/home/planar/out/$p-build.log" 2>&1; echo $? > "/home/planar/out/$p-build.exit"; \
      ctest --test-dir "build/rewrite-$p" --no-tests=error --output-on-failure > "/home/planar/out/$p-ctest.log" 2>&1; echo $? > "/home/planar/out/$p-ctest.exit"; \
      cmake --build "build/rewrite-$p" --target planar_tests > "/home/planar/out/$p-nowork.log" 2>&1; echo $? > "/home/planar/out/$p-nowork.exit"; \
      ctest --test-dir "build/rewrite-$p" -N > "/home/planar/out/$p-inventory.txt" 2>&1; echo $? > "/home/planar/out/$p-inventory.exit"; \
      sed -n 's/^ *Test *#[0-9]*: //p' "/home/planar/out/$p-inventory.txt" | LC_ALL=C sort > "/home/planar/out/$p-inventory-sorted.txt"; \
    done; \
    true

# Toolchain identity for the round record. dpkg output is kept raw and filtered.
RUN { \
      uname -a; \
      cat /etc/os-release; \
      clang++-23 --version; \
      cmake --version; \
      ninja --version; \
      python3 --version; \
      echo "base image: ubuntu:24.04@sha256:24737cef17f4e22b1977df7d9a05e77c4e28b7da5b8911f8d8ccda136bf3573c"; \
      echo "LLVM_APT_VERSION=${LLVM_APT_VERSION}"; \
      echo "CMAKE_VERSION=${CMAKE_VERSION} CMAKE_SHA256=${CMAKE_SHA256}"; \
    } > /home/planar/out/toolchain.txt 2>&1; \
    dpkg -l > /home/planar/out/dpkg-list.txt 2>&1; \
    grep -E 'clang-23|libc\+\+-23|lld-23' /home/planar/out/dpkg-list.txt >> /home/planar/out/toolchain.txt; \
    true

# Export stage: only the gate outputs, no image tag. Built with --output type=local.
FROM scratch AS evidence
COPY --from=gate /home/planar/out/ /
