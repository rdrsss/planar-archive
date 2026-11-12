#!/usr/bin/env python3
# R19-ACCEPT-A fault driver: SIGKILLs the planar check CLI after its rendezvous
# marker, confirms the check process group dead, recovers through check-recover,
# takes over, drives rejection, repair, the max_rounds cap, the raising load and
# the final exact-target approval, with fresh processes for every read.
#
# One run_scenario code path serves the mini fixture (mode "mini") and the CMake
# rerun configuration (mode "cmake"). check_mode "sleep" runs fault/sleep_check.py
# through the counting wrapper; check_mode "real" runs cmake_check.py directly.
#
# Config (driver-only; tamper keys are not product fields):
#   candidate, scratch, raw_root, home, ledger, counter, mode ("mini" | "cmake"),
#   check_mode ("sleep" | "real"), base ("plain" | "composed"), repo (default the
#   checkout, read only), tamper (dict: omit_reject, inspection, replay, counter,
#   expansion, at_event, guidance, verb). Cmake mode also takes export, old_revision,
#   base_label, owned and repairs[{path, action, scratch_only}].
#
# Result (run_scenario): trace (ordered mutating verbs that changed state), steps
#   [{index, verb, argv, stdout, exit, env}] (0-based positions), faults{interrupt,
#   recover}, reads{before_kill, after_restart}, fenced, takeover, verdicts{reject1,
#   reject2, final}, packets, blockers_after_reject, worker_state_after_reject,
#   status_review_blockers, repairs_written, repairs_written_at_cap, load,
#   check_outputs, stale_approval, intended_executions, counter_lines,
#   ledger_before_replays, ledger_after_replays, history, history_at_faults,
#   lua_executions_during_history, attempts, guidance, first_check, detached,
#   integrity, trajectory. Every value is JSON-serializable.
#
# Trace policy: each entry is a mutating verb that changed state. Refused probes
#   (fenced release, stale verdict), replays and inspections are not traced, and
#   reviewer dispatch claims are steps but not trace entries.
#
# Abort contract: an AcceptError from a call site writes raw_root/ABORTED.json
#   (verb, error, ledger_digest, counter_lines) and re-raises. A refused action gets no
#   argv file: the guidance and legal-action checks run before the CLI starts.
import sys

sys.dont_write_bytecode = True

import hashlib
import io
import json
import os
from pathlib import Path
import re
import shutil
import signal
import subprocess
import tarfile
import time

HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))

import accept_lib  # noqa: E402
import cmake_check  # noqa: E402
import registry_gen  # noqa: E402

REPO = HERE.parents[2]
MINI = HERE / "fault" / "mini"
CHECK_RUNNER = HERE / "check_runner.py"
SLEEP_CHECK = HERE / "fault" / "sleep_check.py"
CMAKE_CHECK = HERE / "cmake_check.py"
CMAKE_WORKSPACE = REPO / "tests/native/r19-cmake-support/workspace"
PY = sys.executable
JOB = "combined"
DRIVER = "driver"
BUILDER = "builder-session"
REVIEWER = "reviewer-session"
REPLACEMENT = "replacement-session"
# Guidance points captured live for the reference orchestrator (F12).
NAMED_POINTS = ("open_reservation", "after_takeover", "max_rounds_blocker", "before_final_approval")
FAULT_POINTS = frozenset(NAMED_POINTS)
# label, ledger event type, request id that wrote the event (F9).
FAULT_EVENTS = (
    ("interrupt", "check_reserved", "check-1"),
    ("recover", "check_interrupted", "recover-1"),
    ("takeover", "run_taken_over", "takeover-1"),
    ("reject", "verdict_recorded", "verdict-reject1"),
    ("capped", "verdict_recorded", "verdict-reject2"),
    ("load", "declaration_accepted", "load-2"),
    ("approve", "verdict_recorded", "verdict-approve"),
)
# Event types that accept_lib.verify_event_expansion knows; the others use _match_references.
LIB_EVENT_TYPES = frozenset({"check_reserved", "check_interrupted", "verdict_recorded"})
# Wait defaults for mini configurations; a config may set marker_timeout_s and check_timeout_s (D31 H5-H7).
MARKER_TIMEOUT_S = 120
CHECK_TIMEOUT_S = 900
# The CLI verbs run by _run wait this long for their output (not configurable).
VERB_TIMEOUT_S = 900


def _group_alive(pgid):
    try:
        os.killpg(pgid, 0)
    except ProcessLookupError:
        return False
    except PermissionError:
        return True
    return True


def _wait_group_gone(pgid, timeout_s):
    deadline = time.monotonic() + timeout_s
    while _group_alive(pgid):
        if time.monotonic() >= deadline:
            return False
        time.sleep(0.02)
    return True


def _wait_marker(process, evidence_dir, timeout_s):
    """Block until RENDEZVOUS exists in evidence_dir; raise AcceptError if the CLI exits or time runs out."""
    marker = Path(evidence_dir) / accept_lib.RENDEZVOUS_NAME
    deadline = time.monotonic() + timeout_s
    while not marker.is_file():
        if process.poll() is not None:
            raise accept_lib.AcceptError("check CLI exited before its rendezvous marker appeared")
        if time.monotonic() >= deadline:
            raise accept_lib.AcceptError("no rendezvous marker appeared before the timeout")
        time.sleep(0.02)


def kill_check(cli_process, evidence_dir, timeout_s):
    """Wait for RENDEZVOUS in evidence_dir, SIGKILL cli_process, kill and verify its child group dead.

    The marker is read through accept_lib.read_rendezvous, and the CLI is killed only after the marker
    exists. The marker's pgid is probed with killpg; a surviving group is SIGKILLed, and the function
    returns once the probe raises ProcessLookupError (zombies are waited out).

    Returns {cli_pid, cli_returncode, child_pgid, child_dead, rendezvous, marker_seen_before_kill}.
    Raises AcceptError on a timeout, an unreadable marker, an unsafe pgid or a surviving group.
    """
    _wait_marker(cli_process, evidence_dir, timeout_s)
    rendezvous = accept_lib.read_rendezvous(Path(evidence_dir) / accept_lib.RENDEZVOUS_NAME)
    pgid = rendezvous["pgid"]
    if pgid <= 1 or pgid == os.getpgrp():
        raise accept_lib.AcceptError("refusing to signal an unsafe process group named by the marker")
    cli_process.send_signal(signal.SIGKILL)
    cli_returncode = cli_process.wait(timeout=60)
    if _group_alive(pgid):
        try:
            os.killpg(pgid, signal.SIGKILL)
        except ProcessLookupError:
            pass
    if not _wait_group_gone(pgid, 15):
        raise accept_lib.AcceptError("check child process group survived SIGKILL")
    return {
        "cli_pid": cli_process.pid,
        "cli_returncode": cli_returncode,
        "child_pgid": pgid,
        "child_dead": True,
        "rendezvous": rendezvous,
        "marker_seen_before_kill": True,
    }


def descendant_pids(pid):
    """Return every descendant pid of pid from one ps snapshot; an empty list when ps cannot run.

    The product starts each check child in its own process group (runner.cpp POSIX_SPAWN_SETPGROUP), so the
    check is not in the CLI's group and must be found through its parent link (D31 H7).
    """
    try:
        listing = subprocess.run(["ps", "-axo", "pid=,ppid="], capture_output=True, text=True, timeout=30,
                                 env=accept_lib.child_env())
    except (OSError, subprocess.TimeoutExpired):
        return []
    children = {}
    for line in listing.stdout.splitlines():
        fields = line.split()
        if len(fields) == 2 and fields[0].isdigit() and fields[1].isdigit():
            children.setdefault(int(fields[1]), []).append(int(fields[0]))
    found = []
    frontier = [pid]
    while frontier:
        for child in children.get(frontier.pop(), []):
            if child != pid and child not in found:
                found.append(child)
                frontier.append(child)
    return found


def _signal_group(pgid, signum):
    """Signal process group pgid unless it is absent or is the driver's own group."""
    if pgid > 1 and pgid != os.getpgrp():
        try:
            os.killpg(pgid, signum)
        except (ProcessLookupError, PermissionError):
            pass


def kill_session(process):
    """SIGKILL a CLI started in its own session, every descendant and their process groups, then reap it (D31 H7).

    The descendants are read before the CLI is killed, while it still parents them. Each group is confirmed gone;
    a surviving group raises AcceptError. The driver's own group is never signalled.
    """
    pid = process.pid
    descendants = descendant_pids(pid)
    try:
        process.send_signal(signal.SIGKILL)
    except ProcessLookupError:
        pass
    _signal_group(pid, signal.SIGKILL)
    for member in descendants:
        _signal_group(member, signal.SIGKILL)
        try:
            os.kill(member, signal.SIGKILL)
        except ProcessLookupError:
            pass
    process.wait(timeout=60)
    for pgid in [pid] + descendants:
        if not _wait_group_gone(pgid, 15):
            raise accept_lib.AcceptError("check child process group survived SIGKILL")


def _git_tree(repo, revision, prefix):
    """Return {path relative to prefix: bytes} for the tracked files of revision under prefix."""
    try:
        archive = subprocess.run(["git", "-C", str(repo), "archive", "--format=tar", revision, prefix],
                                 check=True, capture_output=True, env=accept_lib.child_env())
    except (OSError, subprocess.CalledProcessError) as error:
        raise accept_lib.AcceptError(f"git archive {revision} {prefix} failed: {error}") from error
    base = prefix.rstrip("/") + "/"
    tree = {}
    with tarfile.open(fileobj=io.BytesIO(archive.stdout)) as tar:
        for member in tar.getmembers():
            if member.isfile() and member.name.startswith(base):
                tree[member.name[len(base):]] = tar.extractfile(member).read()
    if not tree:
        raise accept_lib.AcceptError(f"{revision}:{prefix} has no tracked files")
    return tree


def _write_tree(root, tree):
    root = Path(root)
    for relpath, data in tree.items():
        target = root / relpath
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes(data)


def _without_verb(text, verb):
    """Return text with verb withdrawn from each role line's next clause (driver-only tamper seam)."""
    lines = []
    for line in text.splitlines():
        match = accept_lib.ROLE_LINE.match(line)
        if match:
            clause = re.sub(r"\b%s\b" % re.escape(verb), "withdrawn", match.group(3))
            line = line[:match.start(3)] + clause + line[match.end(3):]
        lines.append(line)
    return "\n".join(lines)


def _run_id(run):
    return str(run.get("id")) if isinstance(run, dict) else str(run)


def _roles_map(rows):
    """Map role -> {state, drifted, run} with the run as its id ("none" when absent)."""
    return {row["role"]: {"state": row["state"], "drifted": row["drifted"], "run": _run_id(row.get("run"))}
            for row in rows}


def _match_references(expansion, expected, label):
    """Compare dotted expected paths with the expansion exactly; raise AcceptError on any difference."""
    for dotted, value in expected.items():
        current = expansion
        for key in dotted.split("."):
            if not isinstance(current, dict) or key not in current:
                raise accept_lib.AcceptError(f"{label}: expansion lacks {dotted}")
            current = current[key]
        if current != value:
            raise accept_lib.AcceptError(f"{label}: {dotted} is {current!r}, recorded {value!r}")


def cmake_registry(export, evidence, home):
    """Return the unwrapped R19-CMAKE registry naming cmake_check.py for cmake_debug and cmake_release (D11, D13)."""
    checks = []
    for preset in ("debug", "release"):
        checks.append({
            "id": "cmake_" + preset, "toolchain": "python",
            "argv": [str(CMAKE_CHECK), "--repo", str(export), "--evidence", str(evidence), "--preset", preset],
            "coverage": "Actual target graph, full registered tests, lint and no-work rebuild with boundary rows",
            "inputs": ["own_output"], "timeout_ms": 3600000, "max_output_bytes": 65536, "selection": "nonempty",
        })
    return {
        "version": 1, "id": "engine_cmake_tools", "revision": "v1",
        "toolchains": [{"id": "python", "description": "Repository CMake gates through the R19-ACCEPT adapter",
                        "command": [PY],
                        "environment": [{"name": "PATH", "value": os.environ.get("PATH", "")},
                                        {"name": "HOME", "value": str(home)},
                                        {"name": "PYTHONDONTWRITEBYTECODE", "value": "1"}]}],
        "checks": checks,
    }


class _Scenario:
    """One fault scenario on one fresh ledger; each action's argv, stdout, stderr and exit stay under raw_root."""

    def __init__(self, config):
        self.candidate = str(config["candidate"])
        self.mode = config.get("mode", "mini")
        self.check_mode = config.get("check_mode", "sleep")
        self.base_mode = config.get("base", "plain")
        self.cfg = dict(config)
        self.repo = Path(config.get("repo") or REPO)
        # The candidate refuses ledger paths through symlinks (macOS /var is /private/var), so every
        # path handed to it is resolved first.
        self.scratch = Path(os.path.realpath(config["scratch"]))
        self.raw = Path(os.path.realpath(config["raw_root"]))
        self.home = Path(os.path.realpath(config["home"]))
        self.ledger = Path(os.path.realpath(config["ledger"]))
        self.counter = Path(os.path.realpath(config["counter"]))
        self.tamper = dict(config.get("tamper") or {})
        self.evidence = self.scratch / "evidence"
        self.workspace = self.scratch / "workspace"
        self.registry_path = self.scratch / "registry.json"
        self.hold = self.scratch / "hold.flag"
        self.check_scratch = self.scratch / "check-scratch"
        self.env = accept_lib.child_env({"HOME": str(self.home), "PLANAR_DB": str(self.scratch / "unused-legacy.db")})
        self.steps = []
        self.trace = []
        self.attempts = []
        self.guidance = []
        self.serial = 0
        self.check_rounds = 0
        self.checks_per_round = 1
        self.repairs_written = 0
        self.raw_outputs = {}
        self.request_specs = {}
        self.history_events = {}
        self.history_states = {}
        self.event_checks = {}
        self.current = None
        self.reads = {}
        self.integrity = None
        self.check_outputs = []
        self.history_records = {}
        self.at_records = {}
        self.reservation = None
        self.event_checks = {}

    # Setup ---------------------------------------------------------------------------------------

    def prepare(self):
        if self.raw.exists() and any(self.raw.iterdir()):
            raise accept_lib.AcceptError("raw_root already holds retained evidence; refusing to overwrite it")
        for path in (self.scratch, self.raw, self.home, self.workspace, self.evidence, self.check_scratch):
            path.mkdir(parents=True, exist_ok=True)
        if self.mode == "mini":
            self.prepare_mini()
        elif self.mode == "cmake":
            self.prepare_cmake()
        else:
            raise accept_lib.AcceptError(f"unknown scenario mode {self.mode!r}")
        self.prepare_registry()

    def prepare_mini(self):
        self.root = self.scratch / "root"
        self.base = self.scratch / "base"
        _write_tree(self.root, {"src/unit.txt": (MINI / "source/src/unit.txt").read_bytes()})
        _write_tree(self.base, {"src/unit.txt": (MINI / "base/src/unit.txt").read_bytes()})
        (self.workspace / "workspace.lua").write_bytes((MINI / "workspace/workspace.lua").read_bytes())
        (self.workspace / "spec.lua").write_bytes((MINI / "workspace/spec.lua").read_bytes())
        self.workspace_id = "fault_mini"
        self.job_max1 = (MINI / "jobs/combined-max1.lua").read_bytes()
        self.job_max2 = (MINI / "jobs/combined-max2.lua").read_bytes()
        self.unit = "unit"
        self.repairs = self.cfg.get("repairs") or [
            {"path": "src/unit.txt", "action": "append_comment", "scratch_only": True}] * 2
        self.checks = ["sleep"]

    def prepare_cmake(self):
        self.export = Path(os.path.realpath(self.cfg.get("export") or (self.scratch / "export")))
        self.export.mkdir(parents=True, exist_ok=True)
        self.root = self.export / "src/engine"
        head_engine = _git_tree(self.repo, "HEAD", "src/engine")
        overlay_files = {}
        for rel in self.cfg.get("overlay") or []:
            source = self.repo / rel
            if not source.is_file():
                raise accept_lib.AcceptError(f"overlay path {rel} is not a regular file in the working tree")
            overlay_files[rel] = source.read_bytes()
        for rel, data in overlay_files.items():
            if rel.startswith("src/engine/"):
                head_engine[rel[len("src/engine/"):]] = data
        if self.check_mode == "real":
            archive = subprocess.run(["git", "-C", str(self.repo), "archive", "--format=tar", "HEAD"],
                                     check=True, capture_output=True, env=accept_lib.child_env())
            with tarfile.open(fileobj=io.BytesIO(archive.stdout)) as tar:
                tar.extractall(self.export, filter="data")
        _write_tree(self.root, head_engine)
        _write_tree(self.export, {rel: data for rel, data in overlay_files.items()
                                  if not rel.startswith("src/engine/")})
        old_engine = _git_tree(self.repo, self.cfg.get("old_revision", "58128234"), "src/engine")
        if self.base_mode == "composed":
            base_tree = accept_lib.compose_base(head_engine, old_engine, list(self.cfg.get("owned") or []))
        elif self.base_mode == "plain":
            base_tree = old_engine
        else:
            raise accept_lib.AcceptError(f"unknown base mode {self.base_mode!r}")
        self.base = self.scratch / "base-engine"
        _write_tree(self.base, base_tree)
        workspace_text = (CMAKE_WORKSPACE / "workspace.lua").read_text(encoding="utf-8")
        if workspace_text.count('base = "baseline_58128234"') != 1:
            raise accept_lib.AcceptError("R19-CMAKE workspace base label changed; update the driver")
        label = self.cfg.get("base_label", "baseline_head_cmake58128234")
        (self.workspace / "workspace.lua").write_text(
            workspace_text.replace('base = "baseline_58128234"', f'base = "{label}"'), encoding="utf-8")
        (self.workspace / "spec.lua").write_bytes((CMAKE_WORKSPACE / "spec.lua").read_bytes())
        combined = (CMAKE_WORKSPACE / "combined.lua").read_text(encoding="utf-8")
        marker = 'shape = "fanout_review",'
        if combined.count(marker) != 1:
            raise accept_lib.AcceptError("R19-CMAKE combined job shape changed; update the driver")
        self.job_max1 = combined.replace(marker, marker + " max_rounds = 1,").encode()
        self.job_max2 = combined.replace(marker, marker + " max_rounds = 2,").encode()
        self.workspace_id = "engine_cmake"
        self.unit = "engine_build"
        self.repairs = self.cfg.get("repairs") or [
            {"path": "value/CMakeLists.txt", "action": "append_comment", "scratch_only": True}] * 2
        self.checks = ["cmake_debug", "cmake_release"]
        self.checks_per_round = len(self.checks)

    def prepare_registry(self):
        """Write the registry the CLI reads: the counting wrapper around the sleep check, or cmake_check.py for real."""
        if self.mode == "cmake" and self.check_mode == "real":
            registry = cmake_registry(self.export, self.evidence, self.home)
        else:
            text = (MINI / "registry.json").read_text(encoding="utf-8")
            text = (text.replace("@PYTHON@", PY).replace("@PATH@", os.environ.get("PATH", ""))
                    .replace("@HOME@", str(self.home)).replace("@SLEEP_CHECK@", str(SLEEP_CHECK))
                    .replace("@EVIDENCE@", str(self.evidence)).replace("@HOLD@", str(self.hold)))
            registry = json.loads(text)
            if self.mode == "cmake":
                registry["id"] = "engine_cmake_tools"
                template = registry["checks"][0]
                registry["checks"] = [dict(template, id=name) for name in self.checks]
            registry = registry_gen.generate(registry, runner=CHECK_RUNNER, counter=self.counter,
                                             mode="passthrough", python=PY)
        self.registry_path.write_text(json.dumps(registry, indent=1) + "\n", encoding="utf-8")

    # Process plumbing ----------------------------------------------------------------------------

    def _number(self):
        self.serial += 1
        return self.serial

    def _record(self, verb, argv, stdout, stderr, code, pid):
        step = {"index": len(self.steps), "verb": verb, "argv": list(argv), "stdout": stdout,
                "stderr": stderr, "exit": code, "env": {"PYTHONDONTWRITEBYTECODE": "1"}, "pid": pid}
        self.steps.append(step)
        return step

    def _run(self, verb, args, *, traced=False, request_id=None):
        """Run one CLI verb to completion and retain its argv, stdout, stderr and exit. Returns (step, stdout bytes)."""
        argv = [self.candidate, verb, "--ledger", str(self.ledger), *map(str, args)]
        number = self._number()
        stem = self.raw / f"{number:03d}-{verb}"
        stem.with_suffix(".argv.json").write_text(json.dumps(argv, indent=1) + "\n", encoding="utf-8")
        process = subprocess.Popen(argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                                   cwd=str(self.scratch), env=self.env, start_new_session=True)
        try:
            out, err = process.communicate(timeout=VERB_TIMEOUT_S)
        except subprocess.TimeoutExpired as error:
            kill_session(process)
            raise accept_lib.AcceptError(f"{verb} did not finish within {VERB_TIMEOUT_S} s; "
                                         "its CLI and process group were killed") from error
        code = process.returncode
        stem.with_suffix(".stdout").write_bytes(out)
        stem.with_suffix(".stderr").write_bytes(err)
        stem.with_suffix(".exit").write_text(f"{code}\n", encoding="utf-8")
        step = self._record(verb, argv, out.decode("utf-8", "replace"), err.decode("utf-8", "replace"), code,
                            process.pid)
        if request_id is not None:
            self.raw_outputs[request_id] = out
            self.request_specs[request_id] = (verb, [str(a) for a in args])
        if traced and code == 0:
            self.trace.append(verb)
        return step, out

    def read_json(self, verb, args=()):
        """Fresh-process read; raises AcceptError unless it exits 0 with JSON."""
        step, out = self._run(verb, args)
        if step["exit"] != 0:
            raise accept_lib.AcceptError(f"read {verb} exited {step['exit']}")
        return json.loads(out)

    def status(self, *args):
        return self.read_json("status", args)

    def events(self):
        return self.read_json("history", ["--limit", "100"])["data"]["events"]

    def event_for(self, kind, request_id):
        matches = [e for e in self.events() if e.get("type") == kind and e.get("request_id") == request_id]
        if len(matches) != 1:
            raise accept_lib.AcceptError(f"expected one {kind} event for {request_id}, found {len(matches)}")
        return matches[0]

    def activation_id(self):
        activated = [e for e in self.events() if e.get("type") == "job_activated"]
        return activated[-1]["payload"]["references"]["activation"]

    def ledger_digest(self):
        return accept_lib.ledger_digest(self.ledger)

    def counter_lines(self):
        return accept_lib.count_check_executions(self.evidence, self.counter)

    # Guidance and legal actions ------------------------------------------------------------------

    def guided(self, verb, point, *, exempt=False):
        """Capture live status and orchestrator guidance for point, then enforce the legal-action rules.

        Exempt verbs (accept_lib.UNGUIDED_VERBS) still record the capture but do not abort on it. A guided
        verb aborts before the CLI starts when its guidance is unusable or does not list it. A tampered
        verb is checked against the guidance with that verb withdrawn, so the refusal is driver-enforced.
        """
        self.current = verb
        status = self.status()
        live = {"job": status["active_job"], "roles": {row["role"]: row["state"] for row in status["roles"]}}
        step, out = self._run("orchestrator", [])
        if step["exit"] != 0:
            raise accept_lib.AcceptError("orchestrator capture exited nonzero")
        text = json.loads(out)["prompt"]["markdown"]
        if self.tamper.get("guidance") == "empty" and point in FAULT_POINTS:
            text = ""
        usable = accept_lib.guidance_is_usable(text, live, verb)
        record = {"point": point, "step_index": step["index"], "next_verb": verb, "usable": usable,
                  "text": text, "live_state": live}
        if point in NAMED_POINTS:
            self.guidance.append(record)
        if not exempt and not usable:
            raise accept_lib.AcceptError(f"orchestrator guidance is not usable for {verb} at {point}")
        if not exempt or self.tamper.get("verb") == verb:
            legal_text = _without_verb(text, verb) if self.tamper.get("verb") == verb else text
            accept_lib.require_legal_action(legal_text, verb)
        return status

    def act(self, verb, args, *, point, traced=True, exempt=False, request_id=None):
        """Run a guided mutating verb; the orchestrator capture is the step immediately before it."""
        self.guided(verb, point, exempt=exempt)
        step, out = self._run(verb, args, traced=traced, request_id=request_id)
        return step, (json.loads(out) if step["exit"] == 0 else None)

    def refused(self, verb, args, *, point, request_id):
        """Run a guided probe the CLI must refuse; return the exit, typed error code and ledger digests."""
        self.guided(verb, point)
        before = self.ledger_digest()
        step, _out = self._run(verb, args, request_id=request_id)
        after = self.ledger_digest()
        try:
            code = json.loads(step["stderr"])["error"]["code"]
        except (ValueError, KeyError, TypeError):
            code = None
        return {"exit": step["exit"], "error_code": code, "ledger_before": before, "ledger_after": after}

    # Checks --------------------------------------------------------------------------------------

    def check_round(self, token, request_id, actor, point, *, hold=False):
        """Run one check round (every registered check of the role).

        Returns (response, None) for a completed round, or (None, {info, in_progress, before_kill}) when the
        round was held, killed and its child group confirmed dead.
        """
        self.guided("check", point)
        args = ["--run", token, "--root", str(self.root), "--base-root", str(self.base),
                "--scratch", str(self.check_scratch), "--request-id", request_id, "--actor", actor]
        argv = [self.candidate, "check", "--ledger", str(self.ledger), *args]
        number = self._number()
        stem = self.raw / f"{number:03d}-check"
        stem.with_suffix(".argv.json").write_text(json.dumps(argv, indent=1) + "\n", encoding="utf-8")
        self.request_specs[request_id] = ("check", args)
        attempt = self.raw / "attempts" / f"{number:03d}-check"
        attempt.mkdir(parents=True, exist_ok=True)
        self.check_rounds += 1
        record = {"path": str(attempt), "outcome": None, "request_id": request_id}
        self.attempts.append(record)
        if hold:
            self.hold.write_bytes(b"hold\n")
        marker_timeout = self.cfg.get("marker_timeout_s", MARKER_TIMEOUT_S)
        check_timeout = self.cfg.get("check_timeout_s", CHECK_TIMEOUT_S)
        with stem.with_suffix(".stdout").open("wb") as out, stem.with_suffix(".stderr").open("wb") as err:
            # Every CLI leads its own session, so a timed-out wait can kill its whole group (D31 H7).
            process = subprocess.Popen(argv, stdout=out, stderr=err, cwd=str(self.scratch), env=self.env,
                                       start_new_session=True)
            if hold:
                _wait_marker(process, self.evidence, marker_timeout)
                in_progress = self.status("--request-id", request_id)
                before_kill = self.read_all("before")
                info = kill_check(process, self.evidence, marker_timeout)
                stem.with_suffix(".exit").write_text(f"{process.returncode}\n", encoding="utf-8")
                shutil.copyfile(self.evidence / accept_lib.RENDEZVOUS_NAME, attempt / accept_lib.RENDEZVOUS_NAME)
                record["outcome"] = "interrupted"
                self._copy_attempt(stem, attempt)
                self._record("check", argv, "", "", process.returncode, process.pid)
                self.trace.append("check")
                return None, {"info": info, "in_progress": in_progress, "before_kill": before_kill}
            try:
                code = process.wait(timeout=check_timeout)
            except subprocess.TimeoutExpired as error:
                kill_session(process)
                raise accept_lib.AcceptError(f"check {request_id} did not complete within {check_timeout} s; "
                                             "its CLI and process group were killed") from error
        stem.with_suffix(".exit").write_text(f"{code}\n", encoding="utf-8")
        stdout = stem.with_suffix(".stdout").read_bytes()
        stderr = stem.with_suffix(".stderr").read_text(encoding="utf-8", errors="replace")
        self._record("check", argv, stdout.decode("utf-8", "replace"), stderr, code, process.pid)
        self._copy_attempt(stem, attempt)
        if code != 0:
            raise accept_lib.AcceptError(f"check {request_id} exited {code}: {stderr[:200]}")
        self.raw_outputs[request_id] = stdout
        response = json.loads(stdout)
        record["outcome"] = response.get("outcome")
        self.trace.append("check")
        self.last_check = response
        return response, None

    def intended_executions(self):
        """Return the check executions the scenario intended, for comparison with the counter.

        A completed round runs every registered check of the role. An interrupted round is killed while
        its first check holds, so the later checks of that round never start: it counts one execution.
        """
        interrupted = sum(1 for attempt in self.attempts if attempt.get("outcome") == "interrupted")
        return (self.check_rounds - interrupted) * self.checks_per_round + interrupted

    def _copy_attempt(self, stem, attempt):
        for suffix in (".argv.json", ".stdout", ".stderr", ".exit"):
            shutil.copyfile(stem.with_suffix(suffix), attempt / stem.with_suffix(suffix).name)

    def read_all(self, phase):
        """Fresh status, history and orchestrator processes; each read reports its pid and exit."""
        status_step, status_out = self._run("status", [])
        status = json.loads(status_out)
        history_step, _h = self._run("history", ["--limit", "100"])
        orch_step, _o = self._run("orchestrator", [])
        return {
            "status": {"pid": status_step["pid"], "exit": status_step["exit"], "executed": True,
                       "revision": status["active_revision"], "activation": self.activation_id(),
                       "active_job": status["active_job"], "phase": phase},
            "history": {"pid": history_step["pid"], "exit": history_step["exit"], "executed": True},
            "orchestrator": {"pid": orch_step["pid"], "exit": orch_step["exit"], "executed": True},
        }

    def check_summary(self, response):
        rows = [{"status": e.get("outcome")} for e in response.get("evidence", [])
                if e.get("check") == "planar.boundary_v1"]
        return {"outcome": response.get("outcome"), "withheld": list(response.get("withheld") or []),
                "boundary_rows": rows}

    def unit_output(self, response):
        return response["observation"]["units"][0]["output"]

    # Verdicts and repairs ------------------------------------------------------------------------

    def verdict_document(self, check_response, policy, decision, findings, repair):
        unit = check_response["observation"]["units"][0]
        return {"version": 1,
                "targets": [{"work": unit["work"], "output": unit["output"],
                             "work_fp": check_response["completion"]["work_fp"]}],
                "policy": policy, "decision": decision, "findings": findings, "repair": repair,
                "summary": "R19-ACCEPT-A scripted " + decision}

    def write_verdict(self, name, document):
        path = self.raw / f"{name}.verdict.json"
        path.write_text(json.dumps(document, indent=1) + "\n", encoding="utf-8")
        return path

    def apply_repair(self):
        """Append one comment line to a scratch-only source file after an admitted repair or a load (D10)."""
        if self.repairs_written >= len(self.repairs):
            raise accept_lib.AcceptError("no scripted repair remains")
        spec = self.repairs[self.repairs_written]
        if spec.get("scratch_only") is not True or spec.get("action") != "append_comment":
            raise accept_lib.AcceptError("scripted repairs must be scratch-only comment appends")
        target = (self.root / spec["path"]).resolve()
        if self.root.resolve() not in target.parents:
            raise accept_lib.AcceptError("repair path escapes the scratch source root")
        with target.open("a", encoding="utf-8") as stream:
            stream.write(f"# r19-accept repair {self.repairs_written + 1}\n")
        self.repairs_written += 1

    # Claims --------------------------------------------------------------------------------------

    def claim_reviewer(self, request_id, point):
        step, claim = self.act("claim", ["--job", JOB, "--role", "reviewer", "--request-id", request_id,
                                         "--actor", REVIEWER], point=point, traced=False, request_id=request_id)
        if step["exit"] != 0:
            raise accept_lib.AcceptError("reviewer claim was refused")
        return claim

    # Inspection ----------------------------------------------------------------------------------

    def inspect(self, verb, args):
        """Run one read-only inspection; raise AcceptError if the ledger bytes change or the read fails."""
        before = self.ledger_digest()
        step, out = self._run(verb, args)
        if self.tamper.get("inspection"):
            with self.ledger.open("ab") as stream:
                stream.write(b"\0")
        after = self.ledger_digest()
        if before != after:
            raise accept_lib.AcceptError(f"inspection {verb} changed ledger bytes")
        if step["exit"] != 0:
            raise accept_lib.AcceptError(f"inspection {verb} exited {step['exit']}")
        return out.decode("utf-8"), step["argv"]

    def integrity_step(self):
        """Real cmake mode only: after a killed check a no-work repeat build and ninja deps must pass (D27)."""
        if self.mode != "cmake" or self.check_mode != "real":
            return None
        build = self.export / "build" / "rewrite-debug"
        completed = subprocess.run(["cmake", "--build", str(build), "--parallel", "4", "--target", "all",
                                    "planar_tests"], capture_output=True, env=accept_lib.child_env())
        no_work = cmake_check.integrity_verdict(completed.stdout.decode("utf-8", "replace"))
        deps = subprocess.run(["ninja", "-C", str(build), "-t", "deps"], capture_output=True,
                              env=accept_lib.child_env())
        verdict = {"build_exit": completed.returncode, "no_work": no_work, "deps_exit": deps.returncode}
        if completed.returncode != 0 or not no_work or deps.returncode != 0:
            raise accept_lib.AcceptError(f"post-kill integrity failed: {verdict}")
        return verdict

    # Scenario ------------------------------------------------------------------------------------

    def run(self):
        self.prepare()
        result = {}
        self._run("init", ["--workspace-id", self.workspace_id])
        (self.workspace / "combined.lua").write_bytes(self.job_max1)
        load_step, load_out = self._run("load", ["--workspace", str(self.workspace / "workspace.lua"),
                                                 "--registry", str(self.registry_path),
                                                 "--expect-revision", "none",
                                                 "--request-id", "load-1", "--actor", DRIVER],
                                        request_id="load-1")
        if load_step["exit"] != 0:
            raise accept_lib.AcceptError("initial load was refused")
        revision = json.loads(load_out)["revision"]
        # The first activation has no active job, so no orchestrator capture exists yet (UNGUIDED_VERBS).
        self.current = "activate"
        activate_step, activated = self._run("activate", ["--job", JOB, "--expect-revision", revision,
                                                          "--expect-job", "NONE", "--request-id", "activate-1",
                                                          "--actor", DRIVER], traced=True, request_id="activate-1")
        if activate_step["exit"] != 0:
            raise accept_lib.AcceptError("activate was refused")
        self.activation = json.loads(activated)["activation"]

        step, claim = self.act("claim", ["--job", JOB, "--role", "builder", "--request-id", "claim-builder-1",
                                         "--actor", BUILDER], point="claim_builder_1", traced=True,
                                request_id="claim-builder-1")
        builder_run = claim["run"]
        builder_gen = int(builder_run["generation"])

        # First check: held, killed while in progress, its child group confirmed dead.
        _none, fault = self.check_round(builder_run["token"], "check-1", BUILDER, "check_1_hold", hold=True)
        info = fault["info"]
        self.reads["before_kill"] = fault["before_kill"]
        reservation = fault["in_progress"]["request"]["reservation"]
        self.reservation = reservation
        self.history_states["interrupt"] = fault["in_progress"]
        self.history_events["interrupt"] = self.event_for("check_reserved", "check-1")
        post_kill = self.status("--request-id", "check-1")
        result["faults"] = {"interrupt": {
            "rendezvous_seen": info["marker_seen_before_kill"], "child_dead": info["child_dead"],
            "cli_returncode": info["cli_returncode"], "reservation_outcome": post_kill["request"]["outcome"],
            "child_pgid": info["child_pgid"]}}
        self.integrity = self.integrity_step()

        step, recovered = self.act("check-recover", ["--run-id", builder_run["id"], "--reservation", reservation,
                                                     "--confirm-stopped", "true", "--request-id", "recover-1",
                                                     "--actor", DRIVER], point="open_reservation", traced=True,
                                   request_id="recover-1")
        if step["exit"] != 0:
            raise accept_lib.AcceptError("check-recover was refused")
        result["faults"]["recover"] = {"exit": step["exit"], "argv": step["argv"], "outcome": recovered["outcome"]}
        self.history_events["recover"] = self.event_for("check_interrupted", "recover-1")
        self.history_states["recover"] = self.status()
        self.reads["after_restart"] = self.read_all("after")

        # Takeover; the old token is then fenced by a refused release.
        old_token = builder_run["token"]
        step, taken = self.act("takeover", ["--job", JOB, "--role", "builder", "--request-id", "takeover-1",
                                            "--actor", REPLACEMENT, "--run-id", builder_run["id"]],
                               point="takeover", traced=True, request_id="takeover-1")
        if step["exit"] != 0:
            raise accept_lib.AcceptError("takeover was refused")
        new_run = taken["run"]
        result["takeover"] = {"exit": step["exit"], "generation_before": builder_gen,
                              "generation_after": int(new_run["generation"])}
        self.history_events["takeover"] = self.event_for("run_taken_over", "takeover-1")
        self.history_states["takeover"] = self.status()
        fenced = self.refused("release", ["--run", old_token, "--reason", "released", "--request-id", "fenced-1",
                                          "--actor", BUILDER], point="fenced_probe", request_id="fenced-1")
        result["fenced"] = {"token_role": "old", "exit": fenced["exit"], "error_code": fenced["error_code"],
                            "ledger_before": fenced["ledger_before"], "ledger_after": fenced["ledger_after"]}

        # Second check completes after takeover; its output is the first recorded check output.
        response2, _none = self.check_round(new_run["token"], "check-2", BUILDER, "after_takeover")
        reservations = [response2["reservation"]]
        result["first_check"] = self.check_summary(response2)
        result["first_check"]["output"] = self.unit_output(response2) if response2.get("observation") else None
        if response2.get("outcome") != "completed":
            self.current = "first_check"
            result.update(self.stopped_result(result))
            return result
        out2 = self.unit_output(response2)
        self.check_outputs.append(out2)
        policy1 = self.claim_reviewer("reviewer-claim-1", "claim_reviewer_1")
        reviewer1 = policy1["run"]["token"]
        doc1 = self.verdict_document(response2, policy1["dispatch"]["work_fp"], "reject",
                                     [{"id": "f1", "severity": "must", "work": self.unit,
                                       "text": "Edit the unit output before approval."}], ["builder"])
        path1 = self.write_verdict("verdict-reject1", doc1)
        before1 = self.status()
        step, reject1 = self.act("verdict", ["--run", reviewer1, "--verdict", str(path1), "--request-id",
                                             "verdict-reject1", "--actor", REVIEWER], point="reject_1",
                                 traced=True, request_id="verdict-reject1")
        if step["exit"] != 0:
            raise accept_lib.AcceptError("first rejection was refused")
        after1 = self.status()
        self.history_events["reject"] = self.event_for("verdict_recorded", "verdict-reject1")
        self.history_states["reject"] = after1
        self.event_checks["reject"] = {"doc": doc1, "response": reject1, "policy": policy1["dispatch"]["work_fp"],
                                       "count_before": int(before1["review"]["count"]),
                                       "max_rounds": int(before1["review"]["max_rounds"])}
        admitted1 = reject1["verdict"]["admitted"] is True
        if admitted1:
            self.apply_repair()
        packet = self.read_json("preview", ["--role", "builder"])
        result["verdicts"] = {"reject1": {
            "decision": "reject", "exit": step["exit"], "repair_admitted": admitted1, "capped": False,
            "count_before": int(before1["review"]["count"]), "max_rounds": int(before1["review"]["max_rounds"]),
            "acceptance": None, "blockers_open": [b["kind"] for b in after1["review"]["blockers"]],
            "targets_equal_latest_output": True}}
        result["blockers_after_reject"] = [{"kind": b["kind"]} for b in after1["review"]["blockers"]]
        result["worker_state_after_reject"] = _roles_map(after1["roles"])["builder"]["state"]
        result["packets"] = {"builder_after_reject": {"findings": [
            line for line in packet["packet"]["markdown"].splitlines() if line.startswith("- [")]}}

        # The repaired worker returns; the second check changes the output; the second rejection is capped.
        step, claim2 = self.act("claim", ["--job", JOB, "--role", "builder", "--request-id", "claim-builder-2",
                                          "--actor", BUILDER], point="claim_builder_2", traced=True,
                                request_id="claim-builder-2")
        response3, _none = self.check_round(claim2["run"]["token"], "check-3", BUILDER, "check_3")
        reservations.append(response3["reservation"])
        out3 = self.unit_output(response3)
        self.check_outputs.append(out3)
        policy2 = self.claim_reviewer("reviewer-claim-2", "claim_reviewer_2")
        doc2 = self.verdict_document(response3, policy2["dispatch"]["work_fp"], "reject",
                                     [{"id": "f2", "severity": "must", "work": self.unit,
                                       "text": "Still wrong after the repair; the round cap is reached."}],
                                     ["builder"])
        path2 = self.write_verdict("verdict-reject2", doc2)
        before2 = self.status()
        step, reject2 = self.act("verdict", ["--run", policy2["run"]["token"], "--verdict", str(path2),
                                             "--request-id", "verdict-reject2", "--actor", REVIEWER],
                                 point="reject_2", traced=True, request_id="verdict-reject2")
        if step["exit"] != 0:
            raise accept_lib.AcceptError("capped rejection was refused")
        after2 = self.status()
        self.history_events["capped"] = self.event_for("verdict_recorded", "verdict-reject2")
        self.history_states["capped"] = after2
        self.event_checks["capped"] = {"doc": doc2, "response": reject2, "policy": policy2["dispatch"]["work_fp"],
                                       "count_before": int(before2["review"]["count"]),
                                       "max_rounds": int(before2["review"]["max_rounds"])}
        result["verdicts"]["reject2"] = {
            "decision": "reject", "exit": step["exit"], "repair_admitted": reject2["verdict"]["admitted"] is True,
            "capped": reject2["verdict"]["admitted"] is False, "count_before": int(before2["review"]["count"]),
            "max_rounds": int(before2["review"]["max_rounds"]), "acceptance": None,
            "blockers_open": [b["kind"] for b in after2["review"]["blockers"]],
            "targets_equal_latest_output": True}
        result["status_review_blockers"] = [{"kind": b["kind"]} for b in after2["review"]["blockers"]]
        result["repairs_written_at_cap"] = self.repairs_written

        # The raising load clears the cap; the worker re-claims (the load made it stale); the rerun output differs.
        before_load = self.status()
        self.workspace_bytes(self.job_max2)
        step, loaded = self.act("load", ["--workspace", str(self.workspace / "workspace.lua"),
                                         "--registry", str(self.registry_path),
                                         "--expect-revision", before_load["active_revision"],
                                         "--request-id", "load-2", "--actor", DRIVER],
                                point="max_rounds_blocker", exempt=True, traced=True, request_id="load-2")
        if step["exit"] != 0:
            raise accept_lib.AcceptError("raising load was refused")
        after_load = self.status()
        self.history_events["load"] = self.event_for("declaration_accepted", "load-2")
        self.history_states["load"] = after_load
        still_open = {b["id"] for b in after_load["review"]["blockers"]}
        cleared = [b["kind"] for b in before_load["review"]["blockers"] if b["id"] not in still_open]
        load_transitions = self.read_json("history", ["--event", self.history_events["load"]["sequence"]])
        transitions = load_transitions["data"]["transitions"]
        result["load"] = {"exit": step["exit"], "max_rounds_before": int(before_load["review"]["max_rounds"]),
                          "max_rounds_after": int(after_load["review"]["max_rounds"]),
                          "blockers_cleared": cleared,
                          "work_fp_rebound": any(t.get("reason") == "work_changed" for t in transitions)}
        self.apply_repair()

        step, claim3 = self.act("claim", ["--job", JOB, "--role", "builder", "--request-id", "claim-builder-3",
                                          "--actor", BUILDER], point="claim_builder_3", traced=True,
                                request_id="claim-builder-3")
        response4, _none = self.check_round(claim3["run"]["token"], "check-4", BUILDER, "check_4")
        reservations.append(response4["reservation"])
        result["_reservations"] = reservations
        out4 = self.unit_output(response4)
        self.check_outputs.append(out4)
        policy3 = self.claim_reviewer("reviewer-claim-3", "claim_reviewer_3")

        # A verdict built from the earlier output is refused; the final approval names the latest output.
        stale_doc = self.verdict_document(response3, policy3["dispatch"]["work_fp"], "approve", [], [])
        stale_path = self.write_verdict("verdict-stale", stale_doc)
        ledger_before_stale = self.ledger_digest()
        step, stale = self.act("verdict", ["--run", policy3["run"]["token"], "--verdict", str(stale_path),
                                           "--request-id", "verdict-stale", "--actor", REVIEWER],
                               point="stale_probe", traced=True, request_id="verdict-stale")
        result["stale_approval"] = {"exit": step["exit"], "error_code": None,
                                    "applicability": stale["verdict"]["applicability"],
                                    "admitted": stale["verdict"]["admitted"],
                                    "ledger_unchanged": ledger_before_stale == self.ledger_digest()}
        # The historical verdict ends the reviewer's run; the final approval needs a fresh reviewer claim.
        policy3 = self.claim_reviewer("reviewer-claim-4", "claim_reviewer_4")
        final_doc = self.verdict_document(response4, policy3["dispatch"]["work_fp"], "approve", [], [])
        final_path = self.write_verdict("verdict-approve", final_doc)
        step, final = self.act("verdict", ["--run", policy3["run"]["token"], "--verdict", str(final_path),
                                           "--request-id", "verdict-approve", "--actor", REVIEWER],
                               point="before_final_approval", traced=True, request_id="verdict-approve")
        if step["exit"] != 0:
            raise accept_lib.AcceptError("final approval was refused")
        after_final = self.status()
        self.history_events["approve"] = self.event_for("verdict_recorded", "verdict-approve")
        self.history_states["approve"] = after_final
        self.event_checks["approve"] = {"doc": final_doc, "response": final,
                                        "policy": policy3["dispatch"]["work_fp"],
                                        "count_before": int(after_load["review"]["count"]),
                                        "max_rounds": int(after_load["review"]["max_rounds"])}
        acceptance = after_final["review"]["acceptance"]
        result["verdicts"]["final"] = {
            "decision": "approve", "exit": step["exit"],
            "acceptance": "accepted" if acceptance and all(a["accepted"] for a in acceptance) else "open",
            "blockers_open": [b["kind"] for b in after_final["review"]["blockers"]],
            "targets_equal_latest_output": all(t["output"] == out4 for t in final_doc["targets"])}
        self.check_outputs_final = out4
        result["check_outputs"] = list(self.check_outputs)
        result["_claims"] = {"builder": [builder_run, new_run, claim2["run"], claim3["run"]],
                             "reviewer": [policy1, policy2, policy3]}
        result.update(self.finish(result, out2, out3, out4, response2, response3, response4, reject1, reject2))
        result.pop("_claims", None)
        result.pop("_reservations", None)
        return result

    def workspace_bytes(self, data):
        (self.workspace / "combined.lua").write_bytes(data)

    def stopped_result(self, result):
        """Result for a scenario that stopped at a withheld first completed check (F13)."""
        return {"trace": list(self.trace), "steps": self.steps, "attempts": self.attempts,
                "guidance": self.guidance, "integrity": self.integrity, "reads": self.reads,
                "faults": result.get("faults", {}), "counter_lines": self.counter_lines(),
                "intended_executions": self.intended_executions()}

    # Finish --------------------------------------------------------------------------------------

    def finish(self, result, out2, out3, out4, response2, response3, response4, reject1, reject2):
        """Replays, inspections, history per fault event, detached inspection and the trajectory."""
        self.current = "replay"
        replay_order = ["activate-1", "claim-builder-1", "check-2", "verdict-reject1", "check-4", "verdict-approve"]
        counter_before = self.counter_lines()
        ledger_before = self.ledger_digest()
        replays = []
        for index, request in enumerate(replay_order):
            verb, args = self.request_specs[request]
            first = self.raw_outputs[request]
            lb = self.ledger_digest()
            step, out = self._run(verb, args)
            if self.tamper.get("replay"):
                out = out + b" "
            la = self.ledger_digest()
            accept_lib.assert_replay_identical(first, out, lb, la)
            replays.append({"request": request, "verb": verb, "response_identical": True,
                            "ledger_identical": lb == la})
            if index == 0 and self.tamper.get("counter"):
                with self.counter.open("a", encoding="utf-8") as stream:
                    stream.write('{"argv": ["tampered"]}\n')
        ledger_after = self.ledger_digest()
        if self.counter_lines() != counter_before:
            raise accept_lib.AcceptError("the check counter increased during replays")
        result["ledger_before_replays"] = ledger_before
        result["ledger_after_replays"] = ledger_after
        result["replays"] = replays
        result["intended_executions"] = self.intended_executions()
        result["counter_lines"] = self.counter_lines()
        self.current = "inspect"
        expansions = {}
        for label, kind, request in FAULT_EVENTS:
            seq = self.history_events[label]["sequence"]
            expansions[label] = self.inspect_event(label, kind, seq)
        result["history"] = [dict(entry, label=label) for label, entry in self.history_records.items()]
        result["history_at_faults"] = [dict(entry, label=label) for label, entry in self.at_records.items()]
        self.current = "inspect"
        for verb in ("status", "history", "orchestrator"):
            self.inspect(verb, [] if verb != "history" else ["--limit", "100"])
        self.inspect("export", self.export_args("final"))
        result["detached"] = self.detached_inspection(expansions)
        result["lua_executions_during_history"] = 0
        result["reads"] = self.reads
        result["integrity"] = self.integrity
        result["steps"] = self.steps
        result["guidance"] = self.guidance
        result["attempts"] = self.attempts
        result["trace"] = list(self.trace)
        result["faults"] = result.get("faults", {})
        result["trajectory"] = self.validate_trajectory(result, out2, out3, out4)
        return {k: v for k, v in result.items() if not k.startswith("_")}

    def inspect_event(self, label, kind, seq):
        """Compare history --event and --at-event with the references recorded when the event happened."""
        expansion_text, _ = self.inspect("history", ["--event", str(seq)])
        expansion = json.loads(expansion_text)
        expected = self.expected_references(label, kind)
        if self.tamper.get("expansion") and label == "reject":
            expected["data.execution.verdict.decision"] = "approve"
        if kind in LIB_EVENT_TYPES:
            accept_lib.verify_event_expansion(kind, expansion, expected)
        else:
            _match_references(expansion, expected, label)
        self.history_records[label] = {"event_id": str(seq), "expansion": expansion, "expected": expected,
                                       "matched": True}
        state_text, _ = self.inspect("history", ["--at-event", str(seq)])
        replayed = json.loads(state_text)["data"]["state"]
        recorded_status = self.history_states[label]
        recorded = {"active_job": recorded_status["active_job"], "activation": self.activation,
                    "roles": _roles_map(recorded_status["roles"])}
        if self.tamper.get("at_event") and label == "reject":
            recorded["roles"]["builder"]["state"] = "bogus"
        replayed_state = {"active_job": replayed["active_job"], "activation": replayed["activation"],
                          "roles": _roles_map(replayed["roles"])}
        accept_lib.verify_at_event_state(recorded, replayed_state)
        self.at_records[label] = {"event_id": str(seq), "expansion": replayed, "expected": recorded, "matched": True}
        return expansion

    def expected_references(self, label, kind):
        """Return dotted expected paths for one event, recorded from the ledger listing and the responses."""
        event = self.history_events[label]
        expected = {"data.event.request_id": event["request_id"], "data.event.type": event["type"],
                    "data.event.run_id": event["run_id"], "data.event.role": event["role"],
                    "data.event.job": event["job"]}
        for key, value in event["payload"]["references"].items():
            expected["data.event.payload.references." + key] = value
        if kind == "verdict_recorded":
            check = self.event_checks[label]
            doc = check["doc"]
            response = check["response"]
            expected["data.execution.verdict.decision"] = doc["decision"]
            expected["data.execution.verdict.body.policy"] = doc["policy"]
            expected["data.execution.verdict.body.targets"] = doc["targets"]
            expected["data.execution.verdict.body.repair"] = doc["repair"]
            expected["data.execution.repairs"] = response["repairs"]
            expected["data.execution.blockers.opened"] = response["blockers"]["opened"]
            expected["data.execution.blockers.cleared"] = response["blockers"]["cleared"]
        return expected

    def detached_inspection(self, _expansions):
        """Rename the scratch workspace and registry, rerun the inspections, and require identical output (D33)."""
        commands = []
        for label, _kind, _request in FAULT_EVENTS:
            seq = self.history_events[label]["sequence"]
            commands.append((f"event:{label}", "history", ["--event", str(seq)]))
            commands.append((f"at:{label}", "history", ["--at-event", str(seq)]))
        before = {}
        for key, verb, args in commands:
            before[key], _argv = self.inspect(verb, args)
        before["export"], _argv = self.inspect("export", self.export_args("detached-before"))
        counter_before = self.counter_lines()
        moved = [self.workspace / "workspace.lua", self.workspace / "spec.lua", self.workspace / "combined.lua",
                 self.registry_path]
        for path in moved:
            path.rename(path.with_name(path.name + ".moved"))
        argv_seen = []
        after = {}
        try:
            for key, verb, args in commands:
                after[key], argv = self.inspect(verb, args)
                argv_seen.append(argv)
            after["export"], argv = self.inspect("export", self.export_args("detached-after"))
            argv_seen.append(argv)
        finally:
            for path in moved:
                path.with_name(path.name + ".moved").rename(path)
        restored = all(path.is_file() for path in moved)
        return {"renamed": True, "restored": restored, "argv": argv_seen, "before": before, "after": after,
                "counter_unchanged": self.counter_lines() == counter_before}

    def export_args(self, tag):
        """Return export arguments for the revision now active; export targets must not already exist (fresh per call)."""
        self.export_serial = getattr(self, "export_serial", 0) + 1
        target = self.scratch / "exports" / f"{self.export_serial:03d}-{tag}"
        target.parent.mkdir(parents=True, exist_ok=True)
        return ["--revision", str(self.status()["active_revision"]), "--target", str(target)]

    def validate_trajectory(self, result, out2, out3, out4):
        """Build the R19 trajectory from the recorded responses and validate it (F14 call site)."""
        self.current = "trajectory"
        builder_run, new_run, claim2_run, claim3_run = result["_claims"]["builder"]
        reviewer_runs = result["_claims"]["reviewer"]
        doc1, reject1 = self.event_checks["reject"]["doc"], self.event_checks["reject"]["response"]
        doc2, reject2 = self.event_checks["capped"]["doc"], self.event_checks["capped"]["response"]
        final = self.event_checks["approve"]
        events = [
            {"kind": "declaration_accepted", "max_rounds": 1},
            {"kind": "job_activated"},
            {"kind": "run_claimed", "role": "builder", "run": _run_id(builder_run)},
            {"kind": "check_reserved", "reservation": self.reservation,
             "request": "check-1", "run": _run_id(builder_run)},
            {"kind": "check_interrupted", "reservation": self.reservation, "via": "check-recover"},
            {"kind": "run_taken_over", "run": _run_id(new_run), "previous": _run_id(builder_run)},
            {"kind": "fenced_refused", "run": _run_id(builder_run)},
            {"kind": "check_reserved", "reservation": result["_reservations"][0], "request": "check-2",
             "run": _run_id(new_run)},
            {"kind": "check_recorded", "reservation": result["_reservations"][0], "output": out2},
            {"kind": "verdict_recorded", "decision": "reject", "count": self.event_checks["reject"]["count_before"],
             "max_rounds": self.event_checks["reject"]["max_rounds"],
             "repair": [r["role"] for r in reject1["repairs"]], "repair_admitted": True,
             "targets": doc1["targets"], "policy": doc1["policy"],
             "blockers_opened": [b["kind"] for b in reject1["blockers"]["opened"]], "blockers_cleared": [],
             "acceptance": "open"},
            {"kind": "run_claimed", "role": "builder", "run": _run_id(claim2_run)},
            {"kind": "check_reserved", "reservation": result["_reservations"][1], "request": "check-3",
             "run": _run_id(claim2_run)},
            {"kind": "check_recorded", "reservation": result["_reservations"][1], "output": out3},
            {"kind": "verdict_recorded", "decision": "reject", "count": self.event_checks["capped"]["count_before"],
             "max_rounds": self.event_checks["capped"]["max_rounds"],
             "repair": [r["role"] for r in reject2["repairs"]], "repair_admitted": False,
             "targets": doc2["targets"], "policy": doc2["policy"],
             "blockers_opened": [b["kind"] for b in reject2["blockers"]["opened"]], "blockers_cleared": [],
             "acceptance": "open"},
            {"kind": "declaration_accepted", "max_rounds": int(result["load"]["max_rounds_after"])},
            {"kind": "blockers_cleared", "blockers": list(result["load"]["blockers_cleared"])},
            {"kind": "run_claimed", "role": "builder", "run": _run_id(claim3_run)},
            {"kind": "check_reserved", "reservation": result["_reservations"][2], "request": "check-4",
             "run": _run_id(claim3_run)},
            {"kind": "check_recorded", "reservation": result["_reservations"][2], "output": out4},
            {"kind": "verdict_recorded", "decision": "approve", "count": 2,
             "max_rounds": int(result["load"]["max_rounds_after"]), "repair": [], "repair_admitted": False,
             "targets": final["doc"]["targets"], "policy": final["doc"]["policy"], "blockers_opened": [],
             "blockers_open": [], "acceptance": "accepted"},
        ]
        if self.tamper.get("omit_reject"):
            events = [e for e in events if not (e["kind"] == "verdict_recorded" and e["decision"] == "reject"
                                                and e.get("repair_admitted") is True)]
        accept_lib.validate_trajectory(events)
        return {"valid": True, "events": events, "reviewer_runs": [_run_id(p["run"]) for p in reviewer_runs]}


def run_scenario(config):
    """Run one fault scenario through the candidate; raise AcceptError at a call site that refuses."""
    scenario = _Scenario(config)
    try:
        return scenario.run()
    except accept_lib.AcceptError as error:
        scenario.raw.mkdir(parents=True, exist_ok=True)
        aborted = {"verb": scenario.current, "error": str(error),
                   "ledger_digest": scenario.ledger_digest() if scenario.ledger.exists() else None,
                   "counter_lines": scenario.counter_lines()}
        (scenario.raw / "ABORTED.json").write_text(json.dumps(aborted, indent=1) + "\n", encoding="utf-8")
        raise


if __name__ == "__main__":
    sys.exit(0)
