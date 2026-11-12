#!/usr/bin/env python3
# R19-ACCEPT-A pure helpers: replay and ledger comparison, host-path scanning,
# temporary-prefix redaction, hash-pin scope, history expansion and state checks,
# trajectory validation, execution counting, verdict construction, guidance
# usability, closed diagnostics, product-change guard, re-pin rule, red-first
# records, base composition, rendezvous pair and recorded-candidate binding.
#
# Error boundary: every validator raises AcceptError (never a bare assert) so
# call sites in the drivers can abort a run with a typed failure. Helpers that
# spawn processes (git) export PYTHONDONTWRITEBYTECODE=1 (D36).
import sys

sys.dont_write_bytecode = True

import hashlib
import json
import os
from pathlib import Path, PurePosixPath
import re
import subprocess
import tempfile

RENDEZVOUS_NAME = "RENDEZVOUS.json"
# Verbs exempt from live orchestrator guidance: the first load/activate only.
UNGUIDED_VERBS = frozenset({"load", "activate"})

TOKEN_DESCRIPTIONS = {
    "<private-temp-root>": "Resolved OS temporary directory on evidence host, without trailing slash",
    "<temp-root>": "Same OS temporary directory with macOS /private alias removed",
    "<scratch-root>": "Literal scratch directory of the acceptance run",
    "<export-root>": "Scratch source export of the candidate or rerun",
}

EVENT_TYPES = frozenset({
    "job_activated", "run_claimed", "check_reserved", "check_recorded",
    "check_interrupted", "verdict_recorded",
})
CLOSED_CAPTURE_LIMIT = {
    "code": "resource_limit",
    "message": "Lua instruction limit exceeded",
    "field": "",
    "details": {"limit": "instructions", "maximum": 100000000},
}
DATA_SUFFIXES = frozenset({".json", ".lua", ".txt"})
COMPILE_OR_IMPORT = re.compile(
    r"(?m)(?:^|: )(?:fatal )?error: |ImportError|ModuleNotFoundError|SyntaxError")
ASSERTION_MARKER = re.compile(r"FAILED:|AssertionError")
ROLE_LINE = re.compile(r"(?m)^- ([^:\n]+): ([^;\n]+); next: ([^\n]*)$")


class AcceptError(Exception):
    """Typed acceptance failure; call sites abort the run with it."""


def child_env(extra=None):
    """Return the process environment for a spawned child, with bytecode writing disabled (D36)."""
    env = dict(os.environ)
    env["PYTHONDONTWRITEBYTECODE"] = "1"
    if extra:
        env.update(extra)
    return env


def _is_int(value):
    return type(value) is int


def _git(repo, *args):
    try:
        result = subprocess.run(["git", "-C", str(repo), *args], check=True,
                                capture_output=True, env=child_env())
    except (OSError, subprocess.CalledProcessError) as error:
        raise AcceptError(f"git {' '.join(args)} failed in {repo}: {error}") from error
    return result.stdout.decode("utf-8", errors="replace")


def assert_replay_identical(first_response, second_response, ledger_before, ledger_after):
    """Raise AcceptError unless the replay bytes are identical and the ledger digest is unchanged."""
    if first_response != second_response:
        raise AcceptError("replayed response bytes differ from the first response")
    if ledger_before != ledger_after:
        raise AcceptError("ledger bytes changed during replay")


def ledger_digest(db_path):
    """Return a hex digest over the main database file and its -wal and -shm siblings, tail bytes included."""
    db = Path(db_path)
    digest = hashlib.sha256()
    for suffix in ("", "-wal", "-shm"):
        path = Path(str(db) + suffix)
        digest.update(("file" + suffix + "\n").encode())
        if path.is_file():
            data = path.read_bytes()
            digest.update(("present:%d\n" % len(data)).encode())
            digest.update(data)
        else:
            digest.update(b"absent\n")
    return digest.hexdigest()


def _json_strings(value):
    if isinstance(value, str):
        yield value
    elif isinstance(value, dict):
        for item in value.values():
            yield from _json_strings(item)
    elif isinstance(value, list):
        for item in value:
            yield from _json_strings(item)


def scan_host_paths(root, prefixes):
    """Return (posix relative path, prefix) pairs for every file under root, nested or not, containing a prefix.

    Raw bytes are searched, and JSON files are also searched by decoded string value so escaped
    occurrences are found. The caller supplies repo, home, scratch and /Users prefixes.
    """
    root = Path(root)
    prefixes = list(prefixes)
    if any(not prefix for prefix in prefixes):
        raise AcceptError("an empty prefix would match every file")
    hits = set()
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        rel = path.relative_to(root).as_posix()
        data = path.read_bytes()
        for prefix in prefixes:
            if prefix.encode() in data:
                hits.add((rel, prefix))
        if path.suffix == ".json":
            try:
                decoded = json.loads(data.decode("utf-8"))
            except (UnicodeDecodeError, ValueError):
                decoded = None
            if decoded is not None:
                for text in _json_strings(decoded):
                    for prefix in prefixes:
                        if prefix in text:
                            hits.add((rel, prefix))
    return sorted(hits)


def redact_text(text, prefixes):
    """Replace each real prefix by its token, longest prefix first.

    prefixes maps token to the real prefix (in memory only; never written to a rules file).
    Returns (redacted_text, entry) where entry is {original_sha256, redacted_sha256_before_standard,
    bytes_before, bytes_after, rules} and rules maps each used token to a description only.
    """
    redacted = text
    used = {}
    for token, prefix in sorted(prefixes.items(), key=lambda item: len(item[1]), reverse=True):
        if token not in TOKEN_DESCRIPTIONS:
            raise AcceptError(f"unknown redaction token {token!r}")
        if not prefix:
            raise AcceptError(f"empty real prefix for {token}")
        if prefix in redacted:
            used[token] = TOKEN_DESCRIPTIONS[token]
            redacted = redacted.replace(prefix, token)
    original_bytes = text.encode()
    redacted_bytes = redacted.encode()
    entry = {
        "original_sha256": hashlib.sha256(original_bytes).hexdigest(),
        "redacted_sha256_before_standard": hashlib.sha256(redacted_bytes).hexdigest(),
        "bytes_before": len(original_bytes),
        "bytes_after": len(redacted_bytes),
        "rules": dict(sorted(used.items())),
    }
    return redacted, entry


def restore_text(text, prefixes):
    """Inverse of redact_text: replace each token of the token-to-prefix mapping by its prefix."""
    restored = text
    for token, prefix in sorted(prefixes.items(), key=lambda item: len(item[0]), reverse=True):
        restored = restored.replace(token, prefix)
    return restored


def pin_scope_allowed(relpath):
    """Return True only for src/**, r19-accept-support data, fixtures/render/quota/** and r19-cmake-support/workspace/*.lua."""
    path = PurePosixPath(relpath)
    if path.is_absolute() or not path.parts or ".." in path.parts:
        return False
    if any(part == "raw" or part.endswith("-evidence") for part in path.parts):
        return False
    if path.name.upper().startswith("REPORT") or path.suffix == ".log":
        return False
    text = path.as_posix()
    if text.startswith("src/"):
        return True
    if text.startswith("fixtures/render/quota/"):
        return True
    if text.startswith("tests/native/r19-accept-support/") and path.suffix in DATA_SUFFIXES:
        return True
    return path.parent.as_posix() == "tests/native/r19-cmake-support/workspace" and path.suffix == ".lua"


def _dig(obj, dotted):
    current = obj
    for key in dotted.split("."):
        if not isinstance(current, dict) or key not in current:
            return False, None
        current = current[key]
    return True, current


def verify_event_expansion(event_type, expansion, expected, token=None):
    """Raise AcceptError unless each dotted path in expected equals the expansion value and no token appears.

    expected maps dotted paths (for example "dispatch.id" or "blockers.opened") to the values
    recorded at the time. token, when given, must not occur anywhere in the serialized expansion.
    event_type is one of job_activated, run_claimed, check_reserved, check_recorded,
    check_interrupted or verdict_recorded.
    """
    if event_type not in EVENT_TYPES:
        raise AcceptError(f"unknown event type {event_type!r}")
    if not isinstance(expected, dict) or not expected:
        raise AcceptError(f"{event_type}: no recorded references to compare")
    for dotted, value in expected.items():
        found, actual = _dig(expansion, dotted)
        if not found:
            raise AcceptError(f"{event_type}: expansion lacks {dotted}")
        if actual != value:
            raise AcceptError(f"{event_type}: {dotted} is {actual!r}, recorded {value!r}")
    if token:
        serialized = json.dumps(expansion, sort_keys=True, default=str)
        if token in serialized:
            raise AcceptError(f"{event_type}: expansion contains a bearer token")


def verify_at_event_state(recorded, replayed):
    """Raise AcceptError unless active_job, activation and each role's state, drifted and run agree."""
    for key in ("active_job", "activation"):
        if key not in recorded or key not in replayed:
            raise AcceptError(f"state lacks {key}")
        if recorded[key] != replayed[key]:
            raise AcceptError(f"{key} is {replayed[key]!r}, recorded {recorded[key]!r}")
    recorded_roles = recorded.get("roles")
    replayed_roles = replayed.get("roles")
    if not isinstance(recorded_roles, dict) or not isinstance(replayed_roles, dict):
        raise AcceptError("state lacks a roles mapping")
    if set(recorded_roles) != set(replayed_roles):
        raise AcceptError(f"role set {sorted(replayed_roles)} differs from {sorted(recorded_roles)}")
    for role in sorted(recorded_roles):
        try:
            want = {field: recorded_roles[role][field] for field in ("state", "drifted", "run")}
            got = {field: replayed_roles[role][field] for field in ("state", "drifted", "run")}
        except (KeyError, TypeError) as error:
            raise AcceptError(f"role {role} lacks state, drifted or run") from error
        if want != got:
            raise AcceptError(f"role {role} is {got}, recorded {want}")


def _trajectory_error(message):
    return AcceptError("trajectory: " + message)


def _matching(events, start, kind, predicate=lambda event: True):
    return [j for j in range(start + 1, len(events))
            if events[j]["kind"] == kind and predicate(events[j])]


def _admitted_rejection(event):
    count, limit = event.get("count"), event.get("max_rounds")
    return (event.get("decision") == "reject" and event.get("repair_admitted") is True
            and bool(event.get("repair")) and _is_int(count) and _is_int(limit) and count < limit)


def _capped_rejection(event):
    count, limit = event.get("count"), event.get("max_rounds")
    return (event.get("decision") == "reject" and event.get("repair_admitted") is False
            and not event.get("repair") and _is_int(count) and _is_int(limit) and count >= limit
            and "max_rounds" in (event.get("blockers_opened") or []))


def validate_trajectory(events):
    """Raise AcceptError unless the event list satisfies the R19 acceptance trajectory.

    Event kinds and fields: check_reserved{reservation, request, run}; check_interrupted{reservation,
    via}; check_recorded{reservation, output}; run_taken_over{run, previous}; fenced_refused{run};
    run_claimed{role, run}; verdict_recorded{decision, count (rejections before this one), max_rounds,
    repair, repair_admitted, targets[{unit, output, work_fp}], policy, blockers_opened,
    blockers_cleared | blockers_open, acceptance}; declaration_accepted{max_rounds};
    blockers_cleared{blockers}. Required: an interrupted check with no completed record, a takeover
    and fenced refusal, a rejection admitted while count < max_rounds with the worker returning, a
    capped rejection opening a max_rounds blocker, a load raising max_rounds that clears it, and a
    final approval of the latest check output with no open blockers and no later check.
    """
    events = list(events)
    for index, event in enumerate(events):
        if not isinstance(event, dict) or "kind" not in event:
            raise _trajectory_error(f"event {index} is not a tagged record")

    interrupted = _matching(events, -1, "check_interrupted")
    if not interrupted:
        raise _trajectory_error("no interrupted check")
    i = interrupted[0]
    reservation = events[i].get("reservation")
    if events[i].get("via") != "check-recover":
        raise _trajectory_error("the interrupted check was not closed by check-recover")
    reserved = _matching(events, -1, "check_reserved", lambda e: e.get("reservation") == reservation)
    if not reserved or reserved[0] > i:
        raise _trajectory_error("the interrupted check was never reserved before it closed")
    if _matching(events, -1, "check_recorded", lambda e: e.get("reservation") == reservation):
        raise _trajectory_error("the interrupted check also has a recorded completion")
    run_before = events[reserved[0]].get("run")

    takeover = _matching(events, i, "run_taken_over", lambda e: e.get("previous") == run_before)
    if not takeover:
        raise _trajectory_error("no takeover of the interrupted run after the interrupted check")
    t = takeover[0]
    fenced = _matching(events, t, "fenced_refused", lambda e: e.get("run") == run_before)
    if not fenced:
        raise _trajectory_error("the old run token was not refused after takeover")
    f = fenced[0]

    admitted = _matching(events, f, "verdict_recorded", _admitted_rejection)
    if not admitted:
        raise _trajectory_error("no rejection with repair admitted after the takeover")
    a = admitted[0]
    repair_roles = set(events[a]["repair"])
    if not _matching(events, a, "run_claimed", lambda e: e.get("role") in repair_roles):
        raise _trajectory_error("the repair worker did not return to work after the rejection")

    capped = _matching(events, a, "verdict_recorded", _capped_rejection)
    if not capped:
        raise _trajectory_error("no capped rejection that opens a max_rounds blocker")
    c = capped[0]
    limit = events[c]["max_rounds"]

    declared = _matching(events, c, "declaration_accepted",
                         lambda e: _is_int(e.get("max_rounds")) and e["max_rounds"] > limit)
    if not declared:
        raise _trajectory_error("no load raising max_rounds after the capped rejection")
    d = declared[0]
    cleared = _matching(events, d, "blockers_cleared",
                        lambda e: "max_rounds" in (e.get("blockers") or []))
    if not cleared:
        raise _trajectory_error("the max_rounds blocker was not cleared after the load")
    b = cleared[0]

    verdicts = [j for j, e in enumerate(events) if e["kind"] == "verdict_recorded"]
    if not verdicts or verdicts[-1] <= b:
        raise _trajectory_error("no final verdict after the blocker was cleared")
    final = verdicts[-1]
    approval = events[final]
    if approval.get("decision") != "approve" or approval.get("acceptance") != "accepted":
        raise _trajectory_error("the final verdict is not an accepted approval")
    if approval.get("blockers_open") != []:
        raise _trajectory_error("the final approval has open blockers")
    if any(e["kind"] in ("check_reserved", "check_recorded") for e in events[final + 1:]):
        raise _trajectory_error("a check ran after the final approval")

    reserved_before = [j for j in range(final) if events[j]["kind"] == "check_reserved"]
    recorded_before = [j for j in range(final) if events[j]["kind"] == "check_recorded"]
    if not reserved_before or not recorded_before:
        raise _trajectory_error("the final approval has no recorded check output")
    r_last, m_last = reserved_before[-1], recorded_before[-1]
    if m_last < r_last or events[m_last].get("reservation") != events[r_last].get("reservation"):
        raise _trajectory_error("the latest check was not recorded before the final approval")
    latest = events[m_last].get("output")
    targets = approval.get("targets")
    if not targets:
        raise _trajectory_error("the final approval names no targets")
    for target in targets:
        if not isinstance(target, dict) or target.get("output") != latest:
            raise _trajectory_error("a final approval target is not the latest check output")


def count_check_executions(evidence_dir, counter_file):
    """Return the number of attempt folders under evidence_dir/checks/*/* plus non-empty counter_file lines."""
    checks = Path(evidence_dir) / "checks"
    folders = 0
    if checks.is_dir():
        for preset in checks.iterdir():
            if preset.is_dir():
                folders += sum(1 for attempt in preset.iterdir() if attempt.is_dir())
    lines = 0
    counter = Path(counter_file)
    if counter.is_file():
        lines = sum(1 for line in counter.read_text(encoding="utf-8").splitlines() if line.strip())
    return folders + lines


def build_verdict(observation, claim, reviewer, decision):
    """Return {decision, targets, policy} for a verdict.

    observation["targets"] is a list of {unit, output, work_fp} from the checked observation; the
    result targets are that list sorted by unit. policy is reviewer["work_fp"]. claim (the worker
    claim) must contribute nothing to the result.
    """
    del claim  # The worker claim never supplies targets or policy (I19).
    if decision not in ("approve", "reject"):
        raise AcceptError(f"unknown verdict decision {decision!r}")
    targets = sorted(
        ({"unit": t["unit"], "output": t["output"], "work_fp": t["work_fp"]}
         for t in observation["targets"]),
        key=lambda t: t["unit"])
    units = [t["unit"] for t in targets]
    if len(set(units)) != len(units):
        raise AcceptError("observation names a unit twice")
    return {"decision": decision, "targets": targets, "policy": reviewer["work_fp"]}


def _legal_actions(text):
    """Return the ordered, de-duplicated legal verbs from every role line's next clause."""
    verbs = []
    for match in ROLE_LINE.finditer(text):
        clause = re.sub(r"\([^)]*\)", "", match.group(3)).rstrip(".")
        for entry in clause.split(","):
            verb = entry.strip()
            if verb and verb not in verbs:
                verbs.append(verb)
    return verbs


def guidance_is_usable(text, live_state, next_verb):
    """Return True when text is nonempty, names live_state's job and each role state, and lists next_verb as legal.

    live_state is {job, roles: {role: state}}. Role lines have the orchestrator form
    "- ROLE: STATE[ (drifted)]; next: verb, verb." and the legal list is the union of the next clauses,
    with parenthetical notes removed from each entry.
    """
    try:
        if not isinstance(text, str) or not text.strip():
            return False
        if live_state["job"] not in text:
            return False
        for role, state in live_state["roles"].items():
            pattern = (r"(?m)^- " + re.escape(role) + ": " + re.escape(state)
                       + r"(?: \(drifted\))?; next: ")
            if not re.search(pattern, text):
                return False
        return next_verb in _legal_actions(text)
    except (KeyError, TypeError, AttributeError):
        return False


def require_legal_action(guidance_text, verb):
    """Return the parsed legal-action list (see guidance_is_usable); raise AcceptError when verb is absent or the list is empty."""
    legal = _legal_actions(guidance_text if isinstance(guidance_text, str) else "")
    if not legal:
        raise AcceptError("orchestrator guidance lists no legal actions")
    if verb not in legal:
        raise AcceptError(f"{verb!r} is not a legal action in the captured guidance")
    return legal


def _is_error_envelope(body):
    """Return True only for the engine error envelope: keys exactly error, kind and version, kind error, version 1, error closed."""
    if not isinstance(body, dict) or set(body) != {"error", "kind", "version"}:
        return False
    if body["kind"] != "error" or not _is_int(body["version"]) or body["version"] != 1:
        return False
    return body["error"] == CLOSED_CAPTURE_LIMIT


def closed_capture_limit_ok(exit_code, stdout, stderr):
    """Return True only for exit 3, empty stdout and stderr equal to the closed native error object or its envelope.

    The expected stderr parses to {code: "resource_limit", message: "Lua instruction limit exceeded",
    field: "", details: {limit: "instructions", maximum: 100000000}}, either bare or wrapped in the engine
    error envelope {error: <that object>, kind: "error", version: 1} (the form the engine prints, D31 H2).
    """
    if not _is_int(exit_code) or exit_code != 3:
        return False
    if stdout:
        return False
    try:
        body = json.loads(stderr)
    except (TypeError, ValueError):
        return False
    return body == CLOSED_CAPTURE_LIMIT or _is_error_envelope(body)


def product_changes(repo, start_revision):
    """Return sorted src/ paths changed since start_revision, tracked and untracked."""
    tracked = _git(repo, "diff", "--name-only", start_revision, "--", "src").splitlines()
    untracked = _git(repo, "ls-files", "--others", "--exclude-standard", "--", "src").splitlines()
    return sorted({path for path in tracked + untracked if path.startswith("src/")})


def repin_required(paths):
    """Return True when runtime.cpp or embedded Lua (src/stdlib/**/*.lua, commands.lua) is among paths."""
    for path in paths:
        if path == "src/engine/runtime/runtime.cpp":
            return True
        if path.startswith("src/stdlib/") and path.endswith(".lua"):
            return True
        if path.startswith("src/") and PurePosixPath(path).name == "commands.lua":
            return True
    return False


def validate_red_record(record):
    """Raise AcceptError unless the record is a nonzero behavioural failure with a hashed snapshot.

    record is {exit, output, snapshot_sha256, command}. The output must carry an assertion marker
    (FAILED: for Catch2 or AssertionError) and no compile or import marker (error:, ImportError,
    ModuleNotFoundError, SyntaxError). snapshot_sha256 must be 64 lowercase hex characters.
    """
    if not isinstance(record, dict):
        raise AcceptError("red record is not an object")
    exit_code = record.get("exit")
    if not _is_int(exit_code) or exit_code == 0:
        raise AcceptError("red run must exit nonzero")
    output = record.get("output")
    if not isinstance(output, str):
        raise AcceptError("red record lacks its output")
    if COMPILE_OR_IMPORT.search(output):
        raise AcceptError("red run failed to compile or import, not on an assertion")
    if not ASSERTION_MARKER.search(output):
        raise AcceptError("red run output carries no assertion marker")
    snapshot = record.get("snapshot_sha256")
    if not isinstance(snapshot, str) or not re.fullmatch(r"[0-9a-f]{64}", snapshot):
        raise AcceptError("red record lacks a 64-hex snapshot sha256")
    command = record.get("command")
    if not isinstance(command, str) or not command:
        raise AcceptError("red record lacks its command")


def compose_base(head_tree, old_tree, owned):
    """Return HEAD's tree (path to bytes) with each owned path taken from old_tree, removed when absent there."""
    base = dict(head_tree)
    for path in owned:
        if path in old_tree:
            base[path] = old_tree[path]
        else:
            base.pop(path, None)
    return base


def boundary_paths_ok(base_tree, head_tree, owned):
    """Return True when every path whose presence or bytes differ between base_tree and head_tree is in owned."""
    owned = set(owned)
    for path in set(base_tree) | set(head_tree):
        if base_tree.get(path) != head_tree.get(path) and path not in owned:
            return False
    return True


def write_rendezvous(evidence_dir, phase, pid, pgid, extra=None):
    """Atomically write RENDEZVOUS.json with exactly version, phase, pid and pgid; return its path."""
    if extra:
        raise AcceptError("the rendezvous document has exactly version, phase, pid and pgid")
    if not isinstance(phase, str) or not phase or not _is_int(pid) or not _is_int(pgid):
        raise AcceptError("rendezvous needs a phase name and integer pid and pgid")
    evidence = Path(evidence_dir)
    target = evidence / RENDEZVOUS_NAME
    payload = json.dumps({"version": 1, "phase": phase, "pid": pid, "pgid": pgid},
                         sort_keys=True) + "\n"
    descriptor, temporary = tempfile.mkstemp(prefix=".rendezvous-", suffix=".tmp", dir=str(evidence))
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8") as stream:
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, target)
    except BaseException:
        try:
            os.unlink(temporary)
        except FileNotFoundError:
            pass
        raise
    return target


def read_rendezvous(path):
    """Return {version, phase, pid, pgid}; raise AcceptError on truncation, missing, unknown keys or a wrong version."""
    try:
        data = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, ValueError) as error:
        raise AcceptError(f"rendezvous {path} is unreadable or truncated") from error
    if not isinstance(data, dict) or set(data) != {"version", "phase", "pid", "pgid"}:
        raise AcceptError("rendezvous keys are not exactly version, phase, pid and pgid")
    if not _is_int(data["version"]) or data["version"] != 1:
        raise AcceptError("rendezvous version is not 1")
    if not isinstance(data["phase"], str) or not _is_int(data["pid"]) or not _is_int(data["pgid"]):
        raise AcceptError("rendezvous fields have the wrong types")
    return dict(data)


def candidate_record(exe, repo, head):
    """Return {head, sha256, size} of exe; raise AcceptError when missing, not executable or under build/rewrite-*."""
    exe = Path(exe)
    repo = Path(repo).resolve()
    if not exe.is_file():
        raise AcceptError(f"candidate binary {exe.name} is missing")
    resolved = exe.resolve()
    if not os.access(resolved, os.X_OK):
        raise AcceptError("candidate binary is not executable")
    try:
        parts = resolved.relative_to(repo).parts
    except ValueError:
        parts = ()
    if len(parts) >= 2 and parts[0] == "build" and parts[1].startswith("rewrite-"):
        raise AcceptError("candidate must not live in a shared build/rewrite-* directory")
    digest = hashlib.sha256()
    with resolved.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return {"head": head, "sha256": digest.hexdigest(), "size": resolved.stat().st_size}


def require_recorded_candidate(exe, record_path, repo):
    """Raise AcceptError unless exe matches the recorded head, sha256 and size of record_path."""
    try:
        recorded = json.loads(Path(record_path).read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, ValueError) as error:
        raise AcceptError(f"candidate record {record_path} is missing or unreadable") from error
    if not isinstance(recorded, dict):
        raise AcceptError("candidate record is not an object")
    head = _git(repo, "rev-parse", "HEAD").strip()
    current = candidate_record(exe, repo, head)
    for key in ("head", "sha256", "size"):
        if recorded.get(key) != current[key]:
            raise AcceptError(f"candidate {key} does not match the recorded candidate")
