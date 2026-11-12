#!/usr/bin/env python3
# R19-ACCEPT-A acceptance command: quota, mini-fault, limits, packet, index,
# scan, verify-redaction and retained-dir subcommands (rows A1-A7 of
# plan/INVARIANTS.md). Live subcommands (quota, mini-fault) call
# accept_lib.require_recorded_candidate before any run directory exists or the
# engine is invoked (A6, D28).
#
# CLI contract (AcceptanceCliTest and the evidence runs):
#   scan --root DIR_OR_FILE [--root ...] --repo-path P --home H [--exclude REL]...
#        [--scratch S] [--temp-root T] [--private-temp-root R] [--export-root E]
#        exit 0 when no host prefix is found outside the explicit exclusions,
#        exit 1 and one "HOST-PATH <root>/<rel>: <label>" line per hit otherwise.
#   index --raw-root RAW --out-dir OUT --work-cwd WORK --repo-path P --home H
#        [temporary options] [--dry-run]
#        OUT must be the same directory as WORK (the tree root that holds the
#        standard manifest). --dry-run computes every index in a staging copy
#        outside the checkout, prints "expected-index-files: N", exits 1 naming
#        each raw file that would keep a host prefix, and writes nothing under OUT.
#        Writes one compact digest index per raw file under
#        OUT/index/ (raw-file name, raw bytes, raw sha256 and the host-bearing
#        lines only); raw files are never copied. Temporary prefixes are redacted
#        first (accept_lib.redact_text) and recorded in OUT/TEMP-PATH-REDACTION.json,
#        then scripts/redact-local-paths.py redact runs in WORK for repo and home.
#   limits --out-dir OUT --engine-argv ARG [--engine-argv ARG ...]
#        runs the engine argv; OUT/limits.json is written only when
#        accept_lib.closed_capture_limit_ok holds, otherwise exit 1.
#   packet --source FILE --lines N,N --observed-text TEXT --marker MARK --out-dir OUT
#        OUT/packet.json holds the literal source lines and the observed text.
#   retained-dir --raw-root RAW --name NAME
#        creates RAW/NAME (creating RAW when absent); an existing RAW/NAME is
#        refused and left untouched.
#   quota --candidate EXE --record REC --repo-path P --raw-root RAW
#        --grouping combined|split [--regroup] [--run-name NAME]
#   mini-fault --candidate EXE --record REC --repo-path P --raw-root RAW
#        [--run-name NAME]
#        Both live commands verify the candidate record first, then create
#        RAW/NAME (default quota-GROUPING or mini-fault) with a fresh scratch
#        directory inside it, call quota_driver.run_grouping or
#        fault_driver.run_scenario, and write result.json (bytes as base64).
#   verify-redaction --tree T --repo-path P --home H [temporary options]
#        restores the TEMP-PATH-REDACTION.json entries of T (standard repo/home
#        restoration, then the temporary prefixes given on the command line),
#        rejects scratch or temporary tokens and real prefixes in the standard
#        manifest and the TEMP record, then runs scripts/redact-local-paths.py
#        verify in T and propagates its exit status.
#   suites --support-dir DIR [--module NAME]... [--live-module NAME]...
#        [--record REC] --out FILE
#        runs each module as `sys.executable -B -m unittest -v NAME` in DIR (live
#        modules only after PLANAR_CANDIDATE matches REC), writes one count line per
#        module and a total line to FILE and stdout, and exits 1 (naming the module)
#        unless every module ran tests and has no failure, error, skip or nonzero exit.
#   rerun-index --result RESULT --out OUT
#        validates every D38 field of a heavy rerun result.json and writes the compact
#        index to OUT; any missing or contradicting field exits 1 before OUT is written.
#
# Temporary options: --scratch, --temp-root, --private-temp-root, --export-root.
# Error boundary: validators raise accept_lib.AcceptError; main() prints it on
# stderr and exits 1. Usage errors exit 2 (argparse). Real prefixes live only in
# memory; TEMP-PATH-REDACTION.json holds tokens and descriptions only (D19).
import sys

sys.dont_write_bytecode = True

import argparse
import base64
import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import shutil
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import accept_lib as lib  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
STANDARD = REPO / "scripts" / "redact-local-paths.py"
PY = sys.executable
TEMP_RECORD = "TEMP-PATH-REDACTION.json"
MANIFEST = Path("tests/native/path-redaction.json")
USERS_PREFIX = "/Users/"
ENGINE_TIMEOUT_S = 600
TEMP_SCOPE = ("Round-local temporary, scratch, export and private-temp prefixes of the R19-ACCEPT-A "
              "indexes; rules name tokens and descriptions only, never the real prefixes.")
TEMP_ORDER = ("Temporary prefixes first, then repository/home redactor (scripts/redact-local-paths.py). "
              "Inverse: standard repo/home restoration, then the temporary rules below.")
# (token, argparse destination) for every temporary root the commands accept.
TEMP_OPTIONS = (
    ("<scratch-root>", "scratch"),
    ("<temp-root>", "temp_root"),
    ("<private-temp-root>", "private_temp_root"),
    ("<export-root>", "export_root"),
)


def sha256(data):
    """Return the hex sha256 of bytes."""
    return hashlib.sha256(data).hexdigest()


def temp_prefixes(args):
    """Return {token: real prefix} for the temporary roots given on the command line (memory only)."""
    prefixes = {}
    for token, dest in TEMP_OPTIONS:
        value = getattr(args, dest, None)
        if value:
            prefixes[token] = str(value).rstrip("/")
    return prefixes


def standard_restore(data, repo, home):
    """Invert scripts/redact-local-paths.py redact on bytes: home token first, then repo token."""
    return data.replace(b"<home>", home.encode()).replace(b"<repo>", repo.encode())


def write_new_json(path, payload, default=None):
    """Write payload as JSON to a new file; an existing file is refused and never overwritten."""
    path = Path(path)
    if path.exists():
        raise lib.AcceptError(f"{path.name} already exists; refusing to overwrite it")
    path.parent.mkdir(parents=True, exist_ok=True)
    try:
        text = json.dumps(payload, indent=1, sort_keys=True, default=default) + "\n"
    except TypeError as error:
        raise lib.AcceptError(f"{path.name} is not serializable: {error}") from error
    path.write_text(text, encoding="utf-8")
    return path


def _jsonable(value):
    """Encode bytes as base64 for JSON; anything else is a typed failure."""
    if isinstance(value, bytes):
        return {"bytes_base64": base64.b64encode(value).decode("ascii")}
    raise TypeError(f"not JSON serializable: {type(value).__name__}")


def retained_dir(raw_root, name):
    """Create raw_root/name and return it; an existing run directory is refused and left untouched (A5)."""
    if not name or name in (".", "..") or "/" in name or "\\" in name:
        raise lib.AcceptError(f"run directory name {name!r} must be a single path component")
    raw = Path(raw_root)
    raw.mkdir(parents=True, exist_ok=True)
    target = raw / name
    try:
        target.mkdir()
    except FileExistsError as error:
        raise lib.AcceptError(f"run directory {name} already exists under the raw root; refusing to overwrite") from error
    return target


def require_candidate(args):
    """Abort with AcceptError unless the candidate binary matches the recorded candidate (A6, D28)."""
    lib.require_recorded_candidate(args.candidate, args.record, args.repo_path)


def load_temp_record(path, required=False):
    """Return the TEMP-PATH-REDACTION.json mapping; a missing file is an empty record unless required."""
    path = Path(path)
    if not path.exists():
        if required:
            raise lib.AcceptError(f"{path.name} is missing from the tree")
        return {"scope": TEMP_SCOPE, "order": TEMP_ORDER, "rules": {}, "files": {}}
    try:
        record = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, ValueError) as error:
        raise lib.AcceptError(f"{path.name} is unreadable") from error
    if (not isinstance(record, dict) or not isinstance(record.get("files"), dict)
            or not isinstance(record.get("rules"), dict)):
        raise lib.AcceptError(f"{path.name} lacks its files and rules mappings")
    return record


CONTROL_BYTES = re.compile(r"[\x00-\x1f\x7f]")


def escape_control(line):
    """Return line with every NUL, control byte (< 0x20) and 0x7f shown as \\xNN, so the index is text (D31 H8)."""
    return CONTROL_BYTES.sub(lambda match: "\\x%02x" % ord(match.group()), line)


def compact_index(raw_rel, data, host_prefixes):
    """Return the digest index of one raw file: its name, size, sha256 and the host-bearing lines only.

    Control bytes of each hit line are escaped (escape_control); the host path text stays contiguous, and the
    digest, size and hit count are those of the raw bytes.
    """
    text = data.decode("utf-8", errors="replace")
    hits = [(number, line) for number, line in enumerate(text.splitlines(), start=1)
            if any(prefix in line for prefix in host_prefixes)]
    lines = [
        f"raw-file: {raw_rel}",
        f"raw-bytes: {len(data)}",
        f"raw-sha256: {sha256(data)}",
        f"host-lines: {len(hits)}",
    ]
    lines.extend(f"L{number}: {escape_control(line)}" for number, line in hits)
    return "\n".join(lines) + "\n"


def cmd_scan(args):
    """Scan committed roots for host prefixes; exit 1 with the hits when any is found (A1)."""
    named = [("repo", args.repo_path.rstrip("/")), ("home", args.home.rstrip("/")),
             ("users", USERS_PREFIX)]
    named += [(token.strip("<>"), prefix) for token, prefix in temp_prefixes(args).items()]
    label_of = {}
    for label, prefix in named:
        if not prefix:
            raise lib.AcceptError(f"empty prefix for {label}")
        label_of.setdefault(prefix, label)
    prefixes = list(label_of)
    excluded = {PurePosixPath(item).as_posix() for item in args.exclude}
    hits = []
    for root in args.root:
        root = Path(root)
        if root.is_dir():
            found = lib.scan_host_paths(root, prefixes)
            display = root.name + "/"
        elif root.is_file():
            # A single file is scanned as a one-file tree so that no sibling is read.
            with tempfile.TemporaryDirectory(prefix="r19-accept-scan-") as staging:
                shutil.copyfile(root, Path(staging) / root.name)
                found = lib.scan_host_paths(staging, prefixes)
            display = ""
        else:
            raise lib.AcceptError(f"scan root {root.name} is missing")
        for rel, prefix in found:
            if rel in excluded:
                continue
            hits.append((display + rel, label_of[prefix]))
    for where, label in hits:
        print(f"HOST-PATH {where}: {label}")
    return 1 if hits else 0


def cmd_index_dry_run(args, raw, host, temp_map):
    """Compute every index in a staging copy outside the checkout and refuse when one keeps a host prefix (D28, D31 H9/H10).

    Writes nothing under --out-dir. Prints the expected index-file count (the regular files under the raw root)
    and returns 0; a raw file whose index would keep a host prefix after the standard redaction is named and the
    dry run fails with AcceptError.
    """
    staging = Path(tempfile.mkdtemp(prefix="r19-accept-index-dry-"))
    try:
        index_dir = staging / "index"
        written = []
        for path in sorted(item for item in raw.rglob("*") if item.is_file()):
            raw_rel = path.relative_to(raw).as_posix()
            redacted, _entry = lib.redact_text(compact_index(raw_rel, path.read_bytes(), host), temp_map)
            target = index_dir / (raw_rel + ".index")
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(redacted.encode("utf-8"))
            written.append(target.relative_to(staging).as_posix())
        if written:
            proc = subprocess.run(
                [PY, str(STANDARD), "redact", "--repo-path", args.repo_path, "--home", args.home, *written],
                cwd=str(staging), env=lib.child_env(), capture_output=True, text=True)
            if proc.returncode != 0:
                raise lib.AcceptError("standard redaction failed on the dry run: " + (proc.stdout + proc.stderr).strip())
        residual = lib.scan_host_paths(index_dir, host) if written else []
        offending = sorted({rel[:-len(".index")] if rel.endswith(".index") else rel for rel, _ in residual})
        if offending:
            raise lib.AcceptError("index dry run: host prefix would remain after redaction in raw file(s): "
                                  + ", ".join(offending))
        print(f"expected-index-files: {len(written)}")
        return 0
    finally:
        shutil.rmtree(staging, ignore_errors=True)


def cmd_index(args):
    """Write redacted digest indexes for the raw tree and the standard repo/home redaction (A2, D19).

    With --dry-run the same computation runs in a staging copy and nothing is written under --out-dir (D28).
    """
    raw = Path(args.raw_root)
    out = Path(args.out_dir)
    work = Path(args.work_cwd)
    if not raw.is_dir():
        raise lib.AcceptError(f"raw root {raw.name} is missing")
    if out.resolve() != work.resolve():
        raise lib.AcceptError("--out-dir must be the working tree root (--work-cwd): the standard "
                              "manifest and the TEMP record are keyed to it")
    repo = args.repo_path.rstrip("/")
    home = args.home.rstrip("/")
    temp_map = temp_prefixes(args)
    host = [repo, home, USERS_PREFIX] + list(temp_map.values())
    if args.dry_run:
        return cmd_index_dry_run(args, raw, host, temp_map)
    out.mkdir(parents=True, exist_ok=True)
    index_dir = out / "index"
    record_path = out / TEMP_RECORD
    record = load_temp_record(record_path)
    written = []
    for path in sorted(item for item in raw.rglob("*") if item.is_file()):
        raw_rel = path.relative_to(raw).as_posix()
        index_text = compact_index(raw_rel, path.read_bytes(), host)
        redacted, entry = lib.redact_text(index_text, temp_map)
        target = index_dir / (raw_rel + ".index")
        if target.exists():
            raise lib.AcceptError(f"index {target.name} already exists; refusing to overwrite it")
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(redacted.encode("utf-8"))
        key = target.relative_to(out).as_posix()
        record["files"][key] = {name: entry[name] for name in (
            "original_sha256", "redacted_sha256_before_standard", "bytes_before", "bytes_after")}
        record["rules"].update({token: lib.TOKEN_DESCRIPTIONS[token] for token in entry["rules"]})
        written.append(key)
    record["scope"] = TEMP_SCOPE
    record["order"] = TEMP_ORDER
    # The TEMP record is recorded before the standard tool runs (D19 order); it merges across index runs.
    record_path.write_text(json.dumps(record, indent=1, sort_keys=True) + "\n", encoding="utf-8")
    if written:
        proc = subprocess.run(
            [PY, str(STANDARD), "redact", "--repo-path", args.repo_path, "--home", args.home, *written],
            cwd=str(work), env=lib.child_env(), capture_output=True, text=True)
        if proc.returncode != 0:
            raise lib.AcceptError("standard redaction failed: " + (proc.stdout + proc.stderr).strip())
        residual = lib.scan_host_paths(index_dir, host)
        if residual:
            names = ", ".join(sorted({rel for rel, _ in residual}))
            raise lib.AcceptError(f"host prefix remains after redaction in: {names}")
    print(f"indexed {len(written)} raw files")
    return 0


def cmd_limits(args):
    """Run the engine argv and record the closed capture-limit reproduction only when it is closed (A3, D16)."""
    out = Path(args.out_dir)
    try:
        proc = subprocess.run(list(args.engine_argv), capture_output=True,
                              env=lib.child_env(), timeout=ENGINE_TIMEOUT_S)
    except (OSError, subprocess.TimeoutExpired) as error:
        raise lib.AcceptError(f"engine run failed: {error}") from error
    stdout = proc.stdout.decode("utf-8", errors="replace")
    stderr = proc.stderr.decode("utf-8", errors="replace")
    if not lib.closed_capture_limit_ok(proc.returncode, stdout, stderr):
        raise lib.AcceptError(f"capture limit is not the closed diagnostic (exit {proc.returncode}); "
                              f"observed stderr: {stderr[:500]}")
    write_new_json(out / "limits.json", {
        "closed": True, "exit": proc.returncode, "stdout": stdout, "stderr": stderr,
    })
    print("recorded the closed capture limit")
    return 0


def parse_line_numbers(text):
    """Return the positive 1-based line numbers named by a comma-separated list."""
    numbers = []
    for item in text.split(","):
        try:
            number = int(item.strip())
        except ValueError as error:
            raise lib.AcceptError(f"line number {item.strip()!r} is not an integer") from error
        if number < 1:
            raise lib.AcceptError(f"line number {number} must be positive")
        numbers.append(number)
    if not numbers:
        raise lib.AcceptError("no line numbers given")
    return numbers


def cmd_packet(args):
    """Record the literal source lines and the observed packet text when the marker is present (A4, D15)."""
    source = Path(args.source)
    source_bytes = source.read_bytes()
    try:
        lines = source_bytes.decode("utf-8").splitlines()
    except UnicodeDecodeError as error:
        raise lib.AcceptError(f"{source.name} is not UTF-8 text") from error
    selected = {}
    for number in parse_line_numbers(args.lines):
        if number > len(lines):
            raise lib.AcceptError(f"line {number} is outside {source.name} ({len(lines)} lines)")
        selected[str(number)] = lines[number - 1]
    if not args.marker or args.marker not in args.observed_text:
        raise lib.AcceptError("observed packet text does not contain the marker")
    write_new_json(Path(args.out_dir) / "packet.json", {
        "source": source.name, "source_sha256": sha256(source_bytes), "lines": selected,
        "marker": args.marker, "observed": args.observed_text,
    })
    print(f"recorded {len(selected)} literal lines")
    return 0


def cmd_retained_dir(args):
    """Create the retained run directory under raw root; refuse an existing one (A5)."""
    target = retained_dir(args.raw_root, args.name)
    print(target)
    return 0


def run_live(args, default_name, run):
    """Verify the candidate, create the retained run directory, call run(config) and record its result."""
    require_candidate(args)
    run_dir = retained_dir(args.raw_root, args.run_name or default_name)
    scratch = run_dir / "scratch"
    config = {
        "candidate": str(args.candidate), "repo": str(args.repo_path),
        "scratch": str(scratch), "raw_root": str(run_dir / "raw"),
        "home": str(scratch / "home"), "ledger": str(scratch / "ledger.db"),
        "counter": str(scratch / "check-counter.log"),
    }
    result = run(config, args, scratch)
    write_new_json(run_dir / "result.json", result, default=_jsonable)
    print(run_dir)
    return 0


def cmd_quota(args):
    """Run one quota grouping through quota_driver after the candidate check (A6)."""
    def run(config, parsed, scratch):
        import quota_driver  # noqa: PLC0415 - imported after the candidate check
        config.update({
            "fixture": str(Path(parsed.repo_path) / "fixtures" / "render" / "quota"),
            "planar_db": str(scratch / "planar.db"), "grouping": parsed.grouping,
            "regroup": bool(parsed.regroup), "tamper": {},
        })
        return quota_driver.run_grouping(config)
    return run_live(args, f"quota-{args.grouping}", run)


def cmd_mini_fault(args):
    """Run the mini fault scenario through fault_driver after the candidate check (A6)."""
    def run(config, parsed, scratch):
        import fault_driver  # noqa: PLC0415 - imported after the candidate check
        config.update({"mode": "mini", "check_mode": "sleep", "base": "plain", "tamper": {}})
        return fault_driver.run_scenario(config)
    return run_live(args, "mini-fault", run)


def cmd_verify_redaction(args):
    """Restore and check the TEMP record of the tree, then run the standard verify (A7)."""
    tree = Path(args.tree)
    repo = args.repo_path.rstrip("/")
    home = args.home.rstrip("/")
    record_path = tree / TEMP_RECORD
    record = load_temp_record(record_path, required=True)
    temp_map = temp_prefixes(args)
    missing = sorted(set(record["rules"]) - set(temp_map))
    if missing:
        raise lib.AcceptError(f"TEMP rules need their prefixes on the command line: {', '.join(missing)}")
    problems = []
    for rel, entry in sorted(record["files"].items()):
        try:
            before = entry["redacted_sha256_before_standard"]
            original = entry["original_sha256"]
            size = entry["bytes_before"]
        except (KeyError, TypeError) as error:
            raise lib.AcceptError(f"TEMP entry {rel} lacks its recorded hashes") from error
        target = tree / rel
        if not target.is_file():
            problems.append(f"{rel} is missing")
            continue
        restored = standard_restore(target.read_bytes(), repo, home)
        if sha256(restored) != before:
            problems.append(f"{rel}: standard restoration does not reproduce the recorded hash")
            continue
        try:
            text = restored.decode("utf-8")
        except UnicodeDecodeError:
            problems.append(f"{rel}: restored bytes are not UTF-8")
            continue
        restored_original = lib.restore_text(text, temp_map).encode("utf-8")
        if sha256(restored_original) != original or len(restored_original) != size:
            problems.append(f"{rel}: temporary restoration does not reproduce the original hash")
    manifest = tree / MANIFEST
    if manifest.is_file():
        manifest_text = manifest.read_text(encoding="utf-8", errors="replace")
        for token, _ in TEMP_OPTIONS:
            if token in manifest_text:
                problems.append(f"{MANIFEST.as_posix()} holds the scratch or temporary token {token}")
        for prefix in temp_map.values():
            if prefix in manifest_text:
                problems.append(f"{MANIFEST.as_posix()} holds a real temporary prefix")
    record_text = record_path.read_text(encoding="utf-8")
    for prefix in temp_map.values():
        if prefix in record_text:
            problems.append(f"{TEMP_RECORD} holds a real temporary prefix")
    proc = subprocess.run([PY, str(STANDARD), "verify", "--repo-path", args.repo_path, "--home", args.home],
                          cwd=str(tree), env=lib.child_env(), capture_output=True, text=True)
    sys.stdout.write(proc.stdout)
    sys.stderr.write(proc.stderr)
    if proc.returncode != 0:
        problems.append(f"standard verify exited {proc.returncode}")
    if problems:
        raise lib.AcceptError("; ".join(problems))
    print(f"verified {len(record['files'])} TEMP entries")
    return 0


SUITE_RAN = re.compile(r"^Ran (\d+) tests?\b", re.MULTILINE)
SUITE_SUMMARY = re.compile(r"^(?:OK|FAILED)(?: \(([^)]*)\))?\s*$", re.MULTILINE)
SUITE_COUNTS = ("failures", "errors", "skipped")
HOST_PATH_MARKERS = ("/Users/", "/private/", "/home/")
RERUN_REQUIRED_KEYS = ("faults", "reads", "takeover", "fenced", "verdicts", "status_review_blockers", "load",
                       "replays", "ledger_before_replays", "ledger_after_replays", "counter_lines",
                       "intended_executions", "history", "trajectory")


def validate_suite_name(name):
    """Raise AcceptError unless name is a single importable module name passed to unittest."""
    if not name or name.startswith("-") or name in (".", "..") or "/" in name or "\\" in name:
        raise lib.AcceptError(f"suite module name {name!r} must be a plain module name")


def parse_unittest_counts(text):
    """Return (ran, failures, errors, skipped) from a unittest stderr report; no Ran line is an import failure."""
    ran_found = SUITE_RAN.findall(text)
    if not ran_found:
        return 0, 0, 1, 0
    counts = dict.fromkeys(SUITE_COUNTS, 0)
    summaries = SUITE_SUMMARY.findall(text)
    if summaries and summaries[-1]:
        for item in summaries[-1].split(","):
            key, _, value = item.strip().partition("=")
            if key in counts and value.isdigit():
                counts[key] = int(value)
    return int(ran_found[-1]), counts["failures"], counts["errors"], counts["skipped"]


def run_suite_module(support, name):
    """Run one suite module in a child under sys.executable with cwd support; return the completed process."""
    try:
        return subprocess.run([sys.executable, "-B", "-m", "unittest", "-v", name], cwd=str(support),
                              env=lib.child_env(), capture_output=True, encoding="utf-8", errors="replace")
    except OSError as error:
        raise lib.AcceptError(f"suite module {name} could not start: {error}") from error


def write_suite_raw(raw_dir, name, proc):
    """Keep the raw stdout and stderr of one child under raw_dir; an existing raw log is never overwritten."""
    raw_dir.mkdir(parents=True, exist_ok=True)
    for stream, data in (("stdout", proc.stdout), ("stderr", proc.stderr)):
        target = raw_dir / f"{name}.{stream}.txt"
        if target.exists():
            raise lib.AcceptError(f"raw log {target.name} already exists; refusing to overwrite it")
        target.write_text(data, encoding="utf-8")


def suite_refusals(row):
    """Return the reasons a suite row does not count as a full pass; an empty list means it passes."""
    reasons = []
    if row["ran"] <= 0:
        reasons.append("zero matched tests")
    for key in SUITE_COUNTS:
        if row[key]:
            reasons.append(f"{row[key]} {key}")
    if row["exit"] != 0:
        reasons.append(f"exit {row['exit']}")
    return reasons


def suite_report(rows):
    """Return the report text: one line per module, then the total line."""
    lines = [f"{row['name']} ran={row['ran']} failures={row['failures']} errors={row['errors']} "
             f"skipped={row['skipped']} exit={row['exit']}" for row in rows]
    total = {key: sum(row[key] for row in rows) for key in ("ran",) + SUITE_COUNTS}
    lines.append(f"total ran={total['ran']} failures={total['failures']} errors={total['errors']} "
                 f"skipped={total['skipped']}")
    return "\n".join(lines) + "\n"


def require_live_candidate(record):
    """Abort before any child starts unless PLANAR_CANDIDATE matches the recorded candidate (A6, D28)."""
    if record is None:
        raise lib.AcceptError("live suite modules need --record naming the recorded candidate")
    candidate = os.environ.get("PLANAR_CANDIDATE")
    if not candidate:
        raise lib.AcceptError("PLANAR_CANDIDATE must name the recorded candidate before live suite modules run")
    lib.require_recorded_candidate(Path(candidate), record, REPO)


def cmd_suites(args):
    """Run the suite modules (non-live first), write their counts and refuse any module not fully passing (S1-S3)."""
    support = args.support_dir
    if not support.is_dir():
        raise lib.AcceptError(f"support directory {support.name} is missing")
    ordered = list(args.module) + list(args.live_module)
    if not ordered:
        raise lib.AcceptError("no suite modules given")
    for name in ordered:
        validate_suite_name(name)
    out = args.out
    raw_dir = out.parent / (out.name + ".raw")
    for path in (out, raw_dir):
        if path.exists():
            raise lib.AcceptError(f"{path.name} already exists; refusing to overwrite it")
    if args.live_module:
        require_live_candidate(args.record)
    rows = []
    for name in ordered:
        proc = run_suite_module(support, name)
        write_suite_raw(raw_dir, name, proc)
        ran, failures, errors, skipped = parse_unittest_counts(proc.stderr)
        rows.append({"name": name, "ran": ran, "failures": failures, "errors": errors,
                     "skipped": skipped, "exit": proc.returncode})
    text = suite_report(rows)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(text, encoding="utf-8")
    sys.stdout.write(text)
    problems = []
    for row in rows:
        reasons = suite_refusals(row)
        if reasons:
            problems.append(f"suite module {row['name']} did not fully pass: {', '.join(reasons)}")
    if problems:
        raise lib.AcceptError("; ".join(problems))
    return 0


def rerun_field(result, dotted):
    """Return the value at a dotted path of the rerun result; a missing path is a typed failure naming it."""
    current = result
    for key in dotted.split("."):
        if not isinstance(current, dict) or key not in current:
            raise lib.AcceptError(f"rerun result lacks {dotted}")
        current = current[key]
    return current


def require(condition, message):
    """Raise AcceptError with message unless condition holds."""
    if not condition:
        raise lib.AcceptError(message)


def is_int(value):
    """Return True for an exact JSON integer (booleans are not integers here)."""
    return type(value) is int


def names_max_rounds(entry):
    """Return True when a status review blocker entry is the max_rounds blocker."""
    return entry == "max_rounds" or (isinstance(entry, dict) and entry.get("kind") == "max_rounds")


def history_event_type(entry):
    """Return the recorded event type of one history entry, or None when the expansion lacks it."""
    try:
        return entry["expansion"]["data"]["event"]["type"]
    except (KeyError, TypeError):
        return None


def check_faults_and_restart(result):
    """Require the interrupt, the check-recover step and the restart reads of a rerun result (D38)."""
    require(rerun_field(result, "faults.interrupt.child_dead") is True,
            "faults.interrupt.child_dead must be true")
    rerun_field(result, "faults.interrupt.reservation_outcome")
    recover_exit = rerun_field(result, "faults.recover.exit")
    require(is_int(recover_exit) and recover_exit == 0, "faults.recover.exit must be 0")
    rerun_field(result, "faults.recover.outcome")
    rerun_field(result, "reads.before_kill")
    rerun_field(result, "reads.after_restart")


def check_takeover_and_fence(result):
    """Require the takeover generation to advance and the old token to be fenced with an unchanged ledger (D38)."""
    before = rerun_field(result, "takeover.generation_before")
    after = rerun_field(result, "takeover.generation_after")
    require(is_int(before) and is_int(after) and after > before,
            "takeover.generation_after must exceed takeover.generation_before")
    fenced_exit = rerun_field(result, "fenced.exit")
    require(is_int(fenced_exit) and fenced_exit != 0, "fenced.exit must be nonzero (the old token is refused)")
    require(rerun_field(result, "fenced.error_code"), "fenced.error_code must name the refusal")
    require(rerun_field(result, "fenced.ledger_before") == rerun_field(result, "fenced.ledger_after"),
            "fenced.ledger_before must equal fenced.ledger_after")


def check_verdicts(result):
    """Require the reject, capped reject, blocker and final accepted verdicts of the rerun (D38)."""
    require(rerun_field(result, "verdicts.reject1.repair_admitted") is True,
            "verdicts.reject1.repair_admitted must be true")
    require(rerun_field(result, "verdicts.reject1.capped") is False, "verdicts.reject1.capped must be false")
    require(rerun_field(result, "verdicts.reject2.capped") is True, "verdicts.reject2.capped must be true")
    blockers = rerun_field(result, "status_review_blockers")
    require(isinstance(blockers, list) and any(names_max_rounds(item) for item in blockers),
            "status_review_blockers must name a max_rounds blocker")
    require(rerun_field(result, "verdicts.final.targets_equal_latest_output") is True,
            "verdicts.final.targets_equal_latest_output must be true")
    require(rerun_field(result, "verdicts.final.acceptance") == "accepted",
            "verdicts.final.acceptance must be accepted")


def check_load(result):
    """Require the load to raise max_rounds and clear the max_rounds blocker (D38)."""
    before = rerun_field(result, "load.max_rounds_before")
    after = rerun_field(result, "load.max_rounds_after")
    require(is_int(before) and is_int(after) and after > before,
            "load.max_rounds_after must exceed load.max_rounds_before")
    cleared = rerun_field(result, "load.blockers_cleared")
    require(isinstance(cleared, list) and "max_rounds" in cleared, "load.blockers_cleared must name max_rounds")


def check_replays_and_ledger(result):
    """Require every replay identical in response and ledger, and an unchanged counter and ledger digest (D38)."""
    replays = rerun_field(result, "replays")
    require(isinstance(replays, list) and replays, "replays must be a non-empty list")
    for item in replays:
        require(isinstance(item, dict), "replays entries must be objects")
        require(item.get("response_identical") is True,
                f"replays: {item.get('request')} response_identical must be true")
        require(item.get("ledger_identical") is True,
                f"replays: {item.get('request')} ledger_identical must be true")
    lines = rerun_field(result, "counter_lines")
    intended = rerun_field(result, "intended_executions")
    require(is_int(lines) and lines == intended, "counter_lines must equal intended_executions")
    require(rerun_field(result, "ledger_before_replays") == rerun_field(result, "ledger_after_replays"),
            "ledger_before_replays must equal ledger_after_replays (the ledger digest is unchanged)")


def check_history_and_trajectory(result):
    """Require the recover history entry to name check_interrupted and the trajectory to validate non-empty (D38)."""
    history = rerun_field(result, "history")
    require(isinstance(history, list) and all(isinstance(entry, dict) for entry in history),
            "history must be a list of recorded events")
    recover = [entry for entry in history if entry.get("label") == "recover"]
    require(recover, "history has no recover entry naming check_interrupted")
    require(history_event_type(recover[0]) == "check_interrupted" and recover[0].get("matched") is True,
            "history recover entry must be a matched check_interrupted event")
    trajectory = rerun_field(result, "trajectory")
    require(isinstance(trajectory, dict) and trajectory.get("valid") is True, "trajectory must validate")
    events = trajectory.get("events")
    require(isinstance(events, list) and events, "trajectory events must be non-empty")


def validate_rerun_result(result):
    """Raise AcceptError naming the first required field of a rerun result that is missing or contradicts D38."""
    for key in RERUN_REQUIRED_KEYS:
        rerun_field(result, key)
    check_faults_and_restart(result)
    check_takeover_and_fence(result)
    check_verdicts(result)
    check_load(result)
    check_replays_and_ledger(result)
    check_history_and_trajectory(result)


def normalize_argv(argv):
    """Return argv with every absolute path element rewritten to <path>/<basename>; other elements are kept (D31 H1)."""
    normalized = []
    for element in argv:
        if isinstance(element, str) and element.startswith("/"):
            normalized.append("<path>/" + PurePosixPath(element).name)
        else:
            normalized.append(element)
    return normalized


def build_rerun_index(result):
    """Return the compact D38 index: the validated fields only, with the history reduced to labels and types.

    The recover argv keeps its verb and non-path elements; its absolute candidate and ledger paths become
    <path>/<basename> so that a real-shaped argv is not refused as a host path (D31 H1). No other field changes.
    """
    history = [{"label": entry.get("label"), "event_id": entry.get("event_id"),
                "event_type": history_event_type(entry), "matched": entry.get("matched")}
               for entry in rerun_field(result, "history")]
    trajectory = rerun_field(result, "trajectory")
    recover = dict(rerun_field(result, "faults.recover"))
    if isinstance(recover.get("argv"), list):
        recover["argv"] = normalize_argv(recover["argv"])
    return {
        "counters": {"counter_lines": rerun_field(result, "counter_lines"),
                     "intended_executions": rerun_field(result, "intended_executions")},
        "fenced": dict(rerun_field(result, "fenced")),
        "faults": {"interrupt": dict(rerun_field(result, "faults.interrupt")),
                   "recover": recover},
        "history": history,
        "ledger": {"after_replays": rerun_field(result, "ledger_after_replays"),
                   "before_replays": rerun_field(result, "ledger_before_replays")},
        "load": dict(rerun_field(result, "load")),
        "reads": {"after_restart": rerun_field(result, "reads.after_restart"),
                  "before_kill": rerun_field(result, "reads.before_kill")},
        "replays": [dict(item) for item in rerun_field(result, "replays")],
        "status_review_blockers": rerun_field(result, "status_review_blockers"),
        "takeover": dict(rerun_field(result, "takeover")),
        "trajectory": {"events": trajectory.get("events"), "reviewer_runs": trajectory.get("reviewer_runs"),
                       "valid": trajectory.get("valid")},
        "verdicts": {name: dict(rerun_field(result, "verdicts." + name))
                     for name in ("reject1", "reject2", "final")},
    }


def cmd_rerun_index(args):
    """Validate every D38 field of a rerun result and write its compact index; nothing is written on failure."""
    try:
        result = json.loads(args.result.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, ValueError) as error:
        raise lib.AcceptError(f"rerun result {args.result.name} is missing or unreadable") from error
    if not isinstance(result, dict):
        raise lib.AcceptError("rerun result is not a JSON object")
    validate_rerun_result(result)
    index = build_rerun_index(result)
    text = json.dumps(index, indent=1, sort_keys=True)
    if any(marker in text for marker in HOST_PATH_MARKERS):
        raise lib.AcceptError("rerun index would hold a host path; refusing to write it unredacted")
    write_new_json(args.out, index)
    print(f"rerun index written: {args.out.name}")
    return 0


COMMIT_LIST_SUPERSEDED_ALLOWED = re.compile(
    r"^tests/native/r19-accept-evidence/(plan/superseded-[0-9]+/|gate/candidate/superseded-[0-9]+/candidate[.]json$)"
    r"|^tests/native/r19-accept-a-c03-evidence/plan/superseded-[0-9]+/")
# Redacted digests written by `index` mirror the raw tree layout; they are committed evidence (C02-D28, L21),
# so a raw component is allowed only for an .index file under the evidence index tree.
COMMIT_LIST_INDEX_ALLOWED = re.compile(r"^tests/native/r19-accept-evidence/index/.+[.]index$")
COMMIT_LIST_REPORT_PIN = re.compile(r"[0-9a-fA-F]{64}")


def commit_list_path_rules(entry):
    """Return the rule words a listed path breaks (C3-D16); a missing file is checked by the caller."""
    words = []
    path = PurePosixPath(entry)
    parts = path.parts
    directories = parts[:-1]
    if entry.startswith("/") or path.is_absolute() or Path(entry).is_absolute():
        words.append("absolute")
    if ".." in parts:
        words.append("parent")
    if (any(part in ("raw", "raw2") or part.endswith(".raw") for part in directories)
            and not COMMIT_LIST_INDEX_ALLOWED.search(entry)):
        words.append("raw")
    if "__pycache__" in parts or entry.endswith(".pyc"):
        words.append("bytecode")
    if entry.endswith(".log"):
        words.append("log")
    if any(part.startswith("snapshot") for part in directories):
        words.append("snapshot")
    if "redaction/original/" in entry:
        words.append("original")
    if any(part.startswith("superseded-") for part in parts) and not COMMIT_LIST_SUPERSEDED_ALLOWED.search(entry):
        words.append("superseded")
    return words


def cmd_commit_list(args):
    """Check a commit list: sorted, unique, existing, rule-clean files that pin no REPORT hash (C3-D16)."""
    entries = args.list.read_text(encoding="utf-8").splitlines()
    problems = []
    if entries != sorted(entries):
        problems.append("list: unsorted (entries must be in byte order)")
    seen = set()
    for entry in entries:
        if entry in seen:
            problems.append(f"{entry}: duplicate")
        seen.add(entry)
    for entry in dict.fromkeys(entries):
        words = commit_list_path_rules(entry)
        for word in words:
            problems.append(f"{entry}: {word}")
        if "absolute" in words or "parent" in words:
            continue
        target = args.repo_root / entry
        if not target.is_file():
            problems.append(f"{entry}: missing")
            continue
        try:
            text = target.read_text(encoding="utf-8")
        except (UnicodeDecodeError, OSError):
            continue
        if any("REPORT" in line and COMMIT_LIST_REPORT_PIN.search(line) for line in text.splitlines()):
            problems.append(f"{entry}: pins a REPORT hash")
    if problems:
        for problem in problems:
            print(f"commit-list: {problem}", file=sys.stderr)
        return 1
    print(f"commit-list: {len(entries)} entries")
    return 0


def add_temp_options(parser):
    """Add the temporary-root options shared by scan, index and verify-redaction."""
    parser.add_argument("--scratch", default=None)
    parser.add_argument("--temp-root", dest="temp_root", default=None)
    parser.add_argument("--private-temp-root", dest="private_temp_root", default=None)
    parser.add_argument("--export-root", dest="export_root", default=None)


def add_live_options(parser):
    """Add the candidate, record, repo and raw-root options shared by quota and mini-fault."""
    parser.add_argument("--candidate", required=True, type=Path)
    parser.add_argument("--record", required=True, type=Path)
    parser.add_argument("--repo-path", dest="repo_path", required=True)
    parser.add_argument("--raw-root", dest="raw_root", required=True)
    parser.add_argument("--run-name", dest="run_name", default=None)


def build_parser():
    """Return the argument parser with one subcommand per acceptance step."""
    parser = argparse.ArgumentParser(prog="run_acceptance.py", description="R19-ACCEPT-A acceptance commands.",
                                     allow_abbrev=False)
    sub = parser.add_subparsers(dest="command", required=True)

    scan = sub.add_parser("scan", help="fail on host paths in committed evidence")
    scan.set_defaults(handler=cmd_scan)
    scan.add_argument("--root", action="append", required=True)
    scan.add_argument("--repo-path", dest="repo_path", required=True)
    scan.add_argument("--home", required=True)
    scan.add_argument("--exclude", action="append", default=[])
    add_temp_options(scan)

    index = sub.add_parser("index", help="write redacted digest indexes of raw logs")
    index.set_defaults(handler=cmd_index)
    index.add_argument("--raw-root", dest="raw_root", required=True)
    index.add_argument("--out-dir", dest="out_dir", required=True)
    index.add_argument("--work-cwd", dest="work_cwd", required=True)
    index.add_argument("--repo-path", dest="repo_path", required=True)
    index.add_argument("--home", required=True)
    index.add_argument("--dry-run", dest="dry_run", action="store_true",
                       help="compute the indexes in a staging copy, print the expected count, write nothing")
    add_temp_options(index)

    limits = sub.add_parser("limits", help="record the closed capture-limit reproduction")
    limits.set_defaults(handler=cmd_limits)
    limits.add_argument("--out-dir", dest="out_dir", required=True)
    limits.add_argument("--engine-argv", dest="engine_argv", action="append", required=True)

    packet = sub.add_parser("packet", help="record literal packet source lines")
    packet.set_defaults(handler=cmd_packet)
    packet.add_argument("--source", required=True, type=Path)
    packet.add_argument("--lines", required=True)
    packet.add_argument("--observed-text", dest="observed_text", required=True)
    packet.add_argument("--marker", required=True)
    packet.add_argument("--out-dir", dest="out_dir", required=True)

    retained = sub.add_parser("retained-dir", help="create a new retained run directory")
    retained.set_defaults(handler=cmd_retained_dir)
    retained.add_argument("--raw-root", dest="raw_root", required=True)
    retained.add_argument("--name", required=True)

    quota = sub.add_parser("quota", help="run a quota grouping through the external driver")
    quota.set_defaults(handler=cmd_quota)
    add_live_options(quota)
    quota.add_argument("--grouping", required=True, choices=("combined", "split"))
    quota.add_argument("--regroup", action="store_true")

    mini = sub.add_parser("mini-fault", help="run the mini fault scenario")
    mini.set_defaults(handler=cmd_mini_fault)
    add_live_options(mini)

    verify = sub.add_parser("verify-redaction", help="verify the TEMP record and the standard manifest")
    verify.set_defaults(handler=cmd_verify_redaction)
    verify.add_argument("--tree", required=True)
    verify.add_argument("--repo-path", dest="repo_path", required=True)
    verify.add_argument("--home", required=True)
    add_temp_options(verify)

    suites = sub.add_parser("suites", help="run Python suite modules and report their counts")
    suites.set_defaults(handler=cmd_suites)
    suites.add_argument("--support-dir", dest="support_dir", required=True, type=Path)
    suites.add_argument("--module", action="append", default=[])
    suites.add_argument("--live-module", dest="live_module", action="append", default=[])
    suites.add_argument("--record", type=Path, default=None)
    suites.add_argument("--out", required=True, type=Path)

    rerun = sub.add_parser("rerun-index", help="write the compact index of a heavy rerun result")
    rerun.set_defaults(handler=cmd_rerun_index)
    rerun.add_argument("--result", required=True, type=Path)
    rerun.add_argument("--out", required=True, type=Path)

    commit_list = sub.add_parser("commit-list", help="check a commit list of repository files")
    commit_list.set_defaults(handler=cmd_commit_list)
    commit_list.add_argument("--list", required=True, type=Path)
    commit_list.add_argument("--repo-root", dest="repo_root", required=True, type=Path)
    return parser


def main(argv=None):
    """Dispatch a subcommand; typed failures print on stderr and return 1."""
    args = build_parser().parse_args(argv)
    try:
        return args.handler(args)
    except (lib.AcceptError, OSError) as error:
        print(f"run_acceptance: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
