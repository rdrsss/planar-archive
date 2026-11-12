"""X08 Linux recipe pin check (I20): the committed gate image, runner and dockerignore keep their pins and exclusions.

argv: case name (linux_recipe_pins), unused argument, repository root.
Reads the three recipe files as text. Comment lines are stripped before every check so a comment
cannot satisfy a requirement. Requires no Docker daemon, network or build; it only reads files.
"""
from pathlib import Path
import re
import sys

DOCKERFILE = "scripts/linux-gate.Dockerfile"
DOCKERIGNORE = "scripts/linux-gate.Dockerfile.dockerignore"
RUNNER = "scripts/linux-gate.sh"
SCHEMA = "fixtures/ledger/expected-schema.sql"
OLD_SCHEMA_DIR = "r12b-evidence"
STALE_CMAKE_REFERENCE = "src/engine/CMakeLists.txt:254"
PINNED_PACKAGES = ("clang-23", "libc++-23-dev", "libc++abi-23-dev", "lld-23")


def fail(message):
    print(f"linux_recipe_pins FAILED: {message}", file=sys.stderr)
    sys.exit(1)


def read_full(root, rel):
    """Return the file's complete text, comments included."""
    try:
        return (root / rel).read_text(encoding="utf-8")
    except OSError as error:
        fail(f"cannot read {rel}: {error}")


def read_active(root, rel):
    """Return the file's text with comment lines removed."""
    text = read_full(root, rel)
    return "\n".join(line for line in text.splitlines() if not line.lstrip().startswith("#"))


def instructions(text):
    """Join backslash-continued lines into logical lines, dropping blank lines."""
    out, pending = [], ""
    for raw in text.splitlines():
        line = raw.rstrip()
        if line.endswith("\\"):
            pending += line[:-1] + " "
            continue
        out.append((pending + line).strip())
        pending = ""
    if pending.strip():
        out.append(pending.strip())
    return [line for line in out if line]


def check_base_image(lines):
    froms = [line for line in lines if re.match(r"FROM\s", line, re.IGNORECASE)]
    if not froms:
        fail(f"{DOCKERFILE} has no FROM instruction")
    for line in froms:
        if re.search(r"\sscratch(\s|$)", line):
            continue
        if not re.search(r"@sha256:[0-9a-f]{64}(\s|$)", line):
            fail(f"FROM is not pinned by a 64-hex sha256 digest: {line}")


def check_cmake(text):
    if not re.search(r"^ARG CMAKE_VERSION=4\.[34]\.\d+$", text, re.MULTILINE):
        fail("ARG CMAKE_VERSION must be a 4.3.x or 4.4.x release")
    if not re.search(r"^ARG CMAKE_SHA256=[0-9a-f]{64}$", text, re.MULTILINE):
        fail("ARG CMAKE_SHA256 must be a 64-hex SHA256 default")
    if "sha256sum -c" not in text:
        fail("the CMake download is not verified with sha256sum -c")


def check_llvm_pins(text):
    for package in PINNED_PACKAGES:
        pattern = r"(?<![\w.+-])" + re.escape(package) + r"=\$\{LLVM_APT_VERSION\}(?=[\s\\]|$)"
        if not re.search(pattern, text):
            fail(f"{package} is not installed with the =${{LLVM_APT_VERSION}} pin")
    match = re.search(r"^ARG LLVM_APT_VERSION=(\S+)$", text, re.MULTILINE)
    if not match:
        fail("ARG LLVM_APT_VERSION has no default")
    version = re.match(r"^(?:1:)?(23)\.(\d+)\.(\d+)", match.group(1))
    if not version:
        fail(f"LLVM_APT_VERSION does not match ^(1:)?23\\.\\d+\\.\\d+: {match.group(1)}")
    major, minor, patch = (int(part) for part in version.groups())
    if (major, minor, patch) < (23, 1, 0):
        fail(f"LLVM_APT_VERSION is below 23.1.0: {match.group(1)}")


def check_single_source_copy(lines):
    source_copies = [
        index
        for index, line in enumerate(lines)
        if re.match(r"COPY\s", line, re.IGNORECASE) and "--from=" not in line
    ]
    if len(source_copies) != 1:
        fail(f"expected exactly one COPY that brings in the source, found {len(source_copies)}")
    if not re.match(r"COPY\s+(--\S+\s+)*\.\s+/home/planar/src$", lines[source_copies[0]]):
        fail(f"the source COPY is not `COPY ... . /home/planar/src`: {lines[source_copies[0]]}")
    return source_copies[0]


def check_no_mounts(dockerfile_text, runner_text):
    for label, text in (("Dockerfile", dockerfile_text), ("runner", runner_text)):
        if re.search(r"^\s*VOLUME\b", text, re.MULTILINE | re.IGNORECASE):
            fail(f"{label} declares a VOLUME")
        if re.search(r"(^|\s)-v\s", text) or "--volume" in text:
            fail(f"{label} uses a bind mount (-v or --volume)")
        if "type=bind" in text:
            fail(f"{label} uses a bind mount")


def check_user_before_build(lines):
    user_index = next((i for i, line in enumerate(lines) if re.fullmatch(r"USER\s+planar", line)), None)
    if user_index is None:
        fail("no `USER planar` instruction")
    build_pattern = re.compile(r"(^|[\s;&|(])(cmake\s+-|ctest\s)")
    build_index = next(
        (i for i, line in enumerate(lines) if line.startswith("RUN") and build_pattern.search(line)),
        None,
    )
    if build_index is None:
        fail("no RUN instruction calls cmake or ctest")
    if user_index > build_index:
        fail("USER planar does not precede the first RUN that calls cmake or ctest")


def check_schema_assertion(lines, source_index, full_text=""):
    assertion = f"test -s {SCHEMA}"
    indices = [i for i, line in enumerate(lines) if line.startswith("RUN") and assertion in line]
    if not indices:
        fail(f"no RUN asserts `{assertion}`")
    if indices[0] < source_index:
        fail("the schema assertion does not run after the source COPY")
    for stale in (OLD_SCHEMA_DIR, STALE_CMAKE_REFERENCE):
        if stale in full_text:
            fail(f"{DOCKERFILE} still mentions {stale}")


def check_dockerignore(text, full_text=""):
    entries = [line.strip() for line in text.splitlines() if line.strip()]
    for required in ("build", ".git"):
        if required not in entries and "/" + required not in entries:
            fail(f"{DOCKERIGNORE} does not exclude {required}")
    evidence = "tests/native/*-evidence/**"
    if evidence not in entries:
        fail(f"{DOCKERIGNORE} does not exclude {evidence}")
    reincluded = [entry for entry in entries if entry.startswith("!tests/native/")]
    if reincluded:
        fail(f"{DOCKERIGNORE} re-includes an evidence path: {reincluded[0]}")
    if any(entry.lstrip("/").startswith("fixtures") for entry in entries if not entry.startswith("!")):
        fail(f"{DOCKERIGNORE} excludes fixtures")
    if OLD_SCHEMA_DIR in full_text:
        fail(f"{DOCKERIGNORE} still mentions {OLD_SCHEMA_DIR}")


def check_runner(text, full_text=""):
    if "expected-schema" in text or OLD_SCHEMA_DIR in text:
        fail(f"{RUNNER} special-cases the schema in active text")
    for stale in (OLD_SCHEMA_DIR, "re-included"):
        if stale in full_text:
            fail(f"{RUNNER} still mentions {stale}")
    if not re.search(r"/tests/native/\[!/\]\*-evidence/\*\)\s*return 0", text):
        fail(f"{RUNNER} is_excluded no longer excludes the evidence trees")
    for needle in ("df -g", "docker builder prune -f", "--output type=local", "host-manifest"):
        if needle not in text:
            fail(f"{RUNNER} lacks `{needle}`")
    if not re.search(r"(?<![\w-])15(?![\w.-])", text):
        fail(f"{RUNNER} lacks the 15 GiB disk threshold")
    forbidden = (
        (r"(^|\s)-t(\s|$)", "a tag flag (-t)"),
        (r"--tag\b", "--tag"),
        (r"system\s+prune", "system prune"),
        (r"prune\s+(-a|--all)\b", "prune -a"),
        (r"\brmi\b", "rmi"),
        (r"image\s+rm\b", "image rm"),
    )
    for pattern, label in forbidden:
        if re.search(pattern, text):
            fail(f"{RUNNER} uses {label}")


def linux_recipe_pins(root):
    dockerfile_full = read_full(root, DOCKERFILE)
    runner_full = read_full(root, RUNNER)
    ignore_full = read_full(root, DOCKERIGNORE)
    dockerfile = read_active(root, DOCKERFILE)
    lines = instructions(dockerfile)
    runner = read_active(root, RUNNER)
    ignore = read_active(root, DOCKERIGNORE)

    check_base_image(lines)
    check_cmake(dockerfile)
    check_llvm_pins(dockerfile)
    source_index = check_single_source_copy(lines)
    check_no_mounts(dockerfile, runner)
    check_user_before_build(lines)
    check_schema_assertion(lines, source_index, dockerfile_full)
    check_dockerignore(ignore, ignore_full)
    check_runner(runner, runner_full)


def main(argv):
    if len(argv) != 4 or argv[1] != "linux_recipe_pins":
        print("usage: x08_recipe.py linux_recipe_pins <unused> <repo-root>", file=sys.stderr)
        return 2
    root = Path(argv[3]).resolve()
    linux_recipe_pins(root)
    print("linux_recipe_pins passed")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
