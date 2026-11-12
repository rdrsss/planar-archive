#!/usr/bin/env python3
# R19-ACCEPT-A quota grouping driver: copies fixtures/render/quota into scratch,
# applies the scripted session through the candidate CLI with fresh ledgers, and
# retains every raw argv, stdout, stderr and exit file.
#
# Config (driver-only; tamper keys are not product fields):
#   candidate, repo, fixture, scratch, raw_root, home, planar_db, ledger, counter,
#   grouping ("combined" | "split"), regroup (bool), tamper (dict: replay, guidance,
#   counter, inspection, expansion, at_event, verb).
# Error boundary: validator refusals raise accept_lib.AcceptError from call sites;
#   an unexpected candidate, environment or history-read failure raises CandidateFault
#   (never AcceptError, so a call-site row cannot pass on a product defect). Either
#   one writes raw_root/ABORTED.json (verb, error, ledger_digest, counter_lines) and
#   re-raises.
# Guidance binding (D37): every scripted mutating action is preceded by a live
#   status read and an orchestrator capture whose legal next verbs contain the verb
#   about to run. Exempt: accept_lib.UNGUIDED_VERBS (load, activate), init (no ledger
#   exists before it), saved-request replays (they re-send a stored request and are
#   not new actions) and the fenced probe (its verb is not a legal next action in the
#   state where it runs; it writes nothing and is recorded as a negative control).
import sys

sys.dont_write_bytecode = True

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

import accept_lib as lib
import registry_gen

SUPPORT = Path(__file__).resolve().parent
BOUNDARY = "planar.boundary_v1"
CONTRACT = "text/quota-api.md"
REVIEWER = "review_session"
NO_GUIDANCE = frozenset(lib.UNGUIDED_VERBS | {"init"})
WORK_OF_ROLE = {
    "combined": {"builder": ["command", "library"]},
    "split": {"library_worker": ["library"], "cli_worker": ["command"]},
}
ACTORS = {"builder": "builder_session", "library_worker": "library_session",
          "cli_worker": "cli_session", "reviewer": REVIEWER}
PLACEHOLDER_OUTPUT = "xxh3_128:" + "0" * 32
WORKSPACE_SPLIT = (
    'local p = planar\n'
    'local L = p.list\n'
    'return p.workspace {\n'
    '  version = 1, id = "quota",\n'
    '  root = { id = "source", base = "initial", excluded = L { "generated/" } },\n'
    '  spec = "spec.lua",\n'
    '  jobs = L { { id = "split", file = "split.lua" } },\n'
    '  registry = { id = "quota_tools", revision = "v1" },\n'
    '  toolchains = L { "python" }, checks = L { "contract", "integration" },\n'
    '  notes = p.immaterial(p.md "The driver supplies stable sibling source snapshots.")\n'
    '}\n')


class CandidateFault(RuntimeError):
    """An unexpected candidate, environment or history-read failure during the run."""


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def tree_files(root):
    """Return sorted posix relative paths of every regular file under root."""
    root = Path(root)
    return sorted(p.relative_to(root).as_posix() for p in root.rglob("*") if p.is_file())


def copy_overlay(source, destination, subtrees):
    """Copy the named subtrees (relative to source) over destination, file by file."""
    for subtree in subtrees:
        base = Path(source) / subtree
        for path in sorted(base.rglob("*")):
            if path.is_file():
                target = Path(destination) / path.relative_to(source)
                target.parent.mkdir(parents=True, exist_ok=True)
                shutil.copyfile(path, target)


def normalize_targets(targets):
    """Return a verdict target vector as sorted {unit, output, work_fp} records."""
    rows = [{"unit": target.get("unit", target.get("work")), "output": target["output"],
             "work_fp": target["work_fp"]} for target in targets]
    return sorted(rows, key=lambda row: row["unit"])


def parse_json(data):
    try:
        return json.loads(data.decode("utf-8"))
    except (ValueError, UnicodeDecodeError):
        return None


def error_code(stderr_bytes):
    body = parse_json(stderr_bytes)
    if isinstance(body, dict) and isinstance(body.get("error"), dict):
        return body["error"].get("code")
    return None


def without_verb(text, verb):
    """Return text with verb removed from every role's next clause (the legal list no longer names it)."""
    out = []
    for line in text.split("\n"):
        match = lib.ROLE_LINE.match(line)
        if match:
            clause = match.group(3)
            trailer = "." if clause.endswith(".") else ""
            entries = [entry.strip() for entry in clause.rstrip(".").split(",")
                       if entry.strip() and entry.strip() != verb]
            line = "- %s: %s; next: %s%s" % (match.group(1), match.group(2), ", ".join(entries), trailer)
        out.append(line)
    return "\n".join(out)


def state_view(document):
    """Normalize a history --at-event document to active_job, activation and per-role (state, drifted, run)."""
    data = document.get("data") if isinstance(document, dict) else None
    state = data.get("state") if isinstance(data, dict) else None
    if not isinstance(state, dict):
        state = {}
    roles = {}
    for role in state.get("roles") or []:
        if isinstance(role, dict):
            roles[role["role"]] = {"state": role.get("state"), "drifted": role.get("drifted"),
                                   "run": role.get("run")}
    return {"active_job": state.get("active_job", "none"), "activation": state.get("activation", "none"),
            "roles": roles}


def expansion_for(event_type, data):
    """Return the flattened expansion of one history --event document for the types the driver verifies."""
    execution = data.get("execution") if isinstance(data.get("execution"), dict) else {}
    if event_type == "job_activated":
        activation = execution.get("activation", {})
        return {"revision": {"id": activation.get("revision")}, "activation": {"id": activation.get("id")},
                "job": activation.get("job")}
    if event_type == "run_claimed":
        dispatch = execution.get("dispatch", {})
        manifest = execution.get("manifest", {})
        run = execution.get("run")
        return {"dispatch": {"id": dispatch.get("id"), "work_fp": dispatch.get("work_fp")},
                "manifest": {"id": manifest.get("id")},
                "run": {"id": run.get("id") if isinstance(run, dict) else run}}
    if event_type == "check_reserved":
        refs = data["event"]["payload"]["references"]
        return {"reservation": {"id": refs.get("reservation")}, "run": {"id": refs.get("run")}}
    if event_type == "verdict_recorded":
        verdict = execution.get("verdict", {})
        body = verdict.get("body", {})
        return {"decision": verdict.get("decision"), "verdict": {"id": verdict.get("id")},
                "run": verdict.get("run"), "targets": normalize_targets(body.get("targets", [])),
                "policy": body.get("policy"), "blockers": execution.get("blockers")}
    return {"event": data["event"]["type"]}


class Session:
    """One grouping run: fresh scratch tree, fresh ledger, and every action retained."""

    def __init__(self, config):
        self.cfg = config
        # The candidate refuses ledger paths whose parents are not real directories (macOS temp roots are
        # symlinked under /var), so every scratch path handed to the CLI is resolved first.
        self.candidate = Path(config["candidate"])
        self.scratch = Path(os.path.realpath(config["scratch"]))
        self.raw = Path(os.path.realpath(config["raw_root"]))
        self.home = Path(os.path.realpath(config["home"]))
        self.ledger = Path(os.path.realpath(config["ledger"]))
        self.counter = Path(os.path.realpath(config["counter"]))
        self.grouping = config["grouping"]
        self.tamper = dict(config.get("tamper") or {})
        self.env = {"PATH": "/usr/bin:/bin", "HOME": str(self.home),
                    "PLANAR_DB": os.path.realpath(config["planar_db"]), "PYTHONDONTWRITEBYTECODE": "1"}
        self.steps = []
        self.guidance = []
        self.replays = []
        self.inspections = []
        self.checks = []
        self.controls = []
        self.verdicts = []
        self.responses = {}
        self.submitted = {}
        self.first_stdout = {}
        self.completed_targets = {}
        self.at_event = {}
        self.known_seq = 0
        self.check_runs = 0
        self.exports = []
        self.current_verb = "setup"
        self.tampered_replay = False
        self.tampered_counter = False
        self.final_targets = []
        self.phase_start_claimable = []
        self.status_value = "not_accepted"

    # ----- raw recording -------------------------------------------------
    def _digest(self):
        return lib.ledger_digest(self.ledger)

    def _counter_lines(self):
        if not self.counter.is_file():
            return 0
        return sum(1 for line in self.counter.read_text(encoding="utf-8").splitlines() if line.strip())

    def _exec(self, verb, args, request_id=""):
        """Run the candidate, retain argv, stdout, stderr and exit under raw_root, and return (process, step)."""
        argv = [str(self.candidate), verb, *args]
        index = len(self.steps) + 1
        stem = "%03d-%s" % (index, verb)
        files = {kind: str(self.raw / (stem + suffix)) for kind, suffix in
                 (("argv", ".argv.json"), ("stdout", ".stdout"), ("stderr", ".stderr"), ("exit", ".exit"))}
        Path(files["argv"]).write_text(json.dumps(argv, indent=2) + "\n", encoding="utf-8")
        process = subprocess.run(argv, cwd=str(self.scratch), env=self.env, capture_output=True)
        Path(files["stdout"]).write_bytes(process.stdout)
        Path(files["stderr"]).write_bytes(process.stderr)
        Path(files["exit"]).write_text("%d\n" % process.returncode, encoding="utf-8")
        step = {"index": index, "verb": verb, "request_id": request_id, "argv": argv,
                "stdout": process.stdout.decode("utf-8", errors="replace"),
                "stderr": process.stderr.decode("utf-8", errors="replace"),
                "exit": process.returncode, "env": dict(self.env), "files": files}
        self.steps.append(step)
        return process, step

    # ----- reads (Q8, Q16) -----------------------------------------------
    def inspect(self, verb, args):
        """Run a read-only verb; the ledger bytes and counter must be unchanged, else AcceptError."""
        self.current_verb = verb
        before, counter_before = self._digest(), self._counter_lines()
        process, _ = self._exec(verb, args)
        after = self._digest()
        if self.tamper.get("inspection"):
            after = sha256(b"tampered-ledger-view:" + before.encode("ascii"))
        counter_after = self._counter_lines()
        self.inspections.append({"verb": verb, "args": list(args), "exit": process.returncode,
                                 "ledger_before": before, "ledger_after": after,
                                 "counter_before": counter_before, "counter_after": counter_after})
        if before != after:
            raise lib.AcceptError("inspection %s changed ledger bytes" % verb)
        if process.returncode != 0:
            raise CandidateFault("inspection %s exited %d: %r" % (verb, process.returncode, process.stderr))
        return parse_json(process.stdout)

    def status(self, role=None):
        args = ["--ledger", str(self.ledger)] + (["--role", role] if role else [])
        return self.inspect("status", args)

    def history(self, *selector):
        return self.inspect("history", ["--ledger", str(self.ledger), *selector])

    # ----- guidance (D14, D37) -------------------------------------------
    def guide(self, verb):
        """Capture live status and orchestrator guidance; require that verb is a legal next action."""
        status = self.status()
        self.current_verb = verb
        live = {"job": status["active_job"], "roles": {r["role"]: r["state"] for r in status["roles"]}}
        prompt = self.inspect("orchestrator", ["--ledger", str(self.ledger)])
        # inspect() names its own read as current_verb; the aborted point is the refused action (Q19).
        self.current_verb = verb
        step_index = len(self.steps) - 1
        text = prompt["prompt"]["markdown"]
        mode = self.tamper.get("guidance")
        if mode:
            text = "" if mode == "empty" else "Continue the run."
        if not lib.guidance_is_usable(text, live, verb):
            raise lib.AcceptError("orchestrator guidance is not usable for next verb %r" % verb)
        visible = without_verb(text, verb) if self.tamper.get("verb") == verb else text
        legal = lib.require_legal_action(visible, verb)
        self.guidance.append({"phase": verb, "step_index": step_index, "next_verb": verb,
                              "text": text, "live_state": live, "legal": legal})

    # ----- actions -------------------------------------------------------
    def act(self, verb, args, request_id):
        """Run one scripted mutating action (guarded unless exempt), require success, observe its events."""
        if verb not in NO_GUIDANCE:
            self.guide(verb)
        self.current_verb = verb
        process, _ = self._exec(verb, args, request_id)
        if process.returncode != 0:
            raise CandidateFault("%s %s exited %d: %r" % (verb, request_id, process.returncode, process.stderr))
        response = parse_json(process.stdout)
        self.responses[request_id] = response
        self.first_stdout[(verb, request_id)] = (process.stdout, list(args))
        self.observe()
        return response

    def control(self, name, verb, args, request_id):
        """Run a negative control; record a refusal or a withheld result without requiring success."""
        self.guide(verb)
        self.current_verb = verb
        before, counter_before = self._digest(), self._counter_lines()
        process, _ = self._exec(verb, args, request_id)
        after, counter_after = self._digest(), self._counter_lines()
        document = parse_json(process.stdout) or {}
        reasons = list(document.get("withheld") or [])
        for row in document.get("evidence") or []:
            if row.get("outcome") != "pass" and row.get("reason") not in reasons:
                reasons.append(row.get("reason"))
        self.controls.append({
            "name": name, "request_id": request_id, "exit": process.returncode,
            "error_code": error_code(process.stderr), "outcome": self._outcome(document, process.returncode),
            "reasons": reasons, "ledger_before": before, "ledger_after": after,
            "counter_before": counter_before, "counter_after": counter_after})
        self.check_runs += self._executed(document)
        self.responses[request_id] = document
        self.observe()
        return process

    def replay(self, verb, request_id):
        """Re-send one saved request in a fresh process; bytes, ledger and counter must not change (Q5, Q14)."""
        first, args = self.first_stdout[(verb, request_id)]
        before, counter_before = self._digest(), self._counter_lines()
        process, _ = self._exec(verb, args, request_id)
        output = process.stdout
        if self.tamper.get("replay") and not self.tampered_replay:
            self.tampered_replay = True
            output = output + b" "
        if self.tamper.get("counter") and not self.tampered_counter:
            self.tampered_counter = True
            with self.counter.open("a", encoding="utf-8") as handle:
                handle.write("{}\n")
        after, counter_after = self._digest(), self._counter_lines()
        lib.assert_replay_identical(first, output, before, after)
        self.replays.append({"verb": verb, "request_id": request_id, "exit": process.returncode,
                             "response_identical": output == first, "ledger_identical": before == after,
                             "counter_unchanged": counter_before == counter_after})
        if counter_before != counter_after:
            raise lib.AcceptError("replay %s %s executed a check" % (verb, request_id))

    def observe(self):
        """Record the state at each newly appended event with history --at-event, read live."""
        document = self.history("--after-event", str(self.known_seq))
        for event in document["data"]["events"]:
            seq = int(event["sequence"])
            at = self.history("--at-event", str(seq))
            self.at_event[seq] = {"type": event["type"], "request_id": event["request_id"],
                                  "recorded": state_view(at)}
            self.known_seq = max(self.known_seq, seq)

    # ----- evidence shaping ----------------------------------------------
    @staticmethod
    def _outcome(document, returncode):
        if returncode != 0:
            return "refused"
        if isinstance(document.get("verdict"), dict):
            return document["verdict"].get("applicability", "unknown")
        if document.get("outcome") == "completed":
            return "completed"
        if document.get("withheld"):
            return "withheld"
        return document.get("outcome", "unknown")

    @staticmethod
    def _executed(document):
        return sum(1 for row in document.get("evidence") or []
                   if row.get("check") != BOUNDARY and row.get("exit_status") != "none")

    def _executions_since(self, start):
        """Return the counted check executions after counter line start, each with its retained transcript.

        A counter line's argv ends with the argv of the registered check it ran. check_runner
        --transcript wrote the child output of line N to transcripts/NNN.txt; a missing file is None.
        """
        lines = [json.loads(line) for line in self.counter.read_text(encoding="utf-8").splitlines() if line.strip()]
        found = []
        for number in range(start + 1, len(lines) + 1):
            argv = lines[number - 1]["argv"]
            name = None
            for check_id, tail in self.check_argv.items():
                if tail and argv[-len(tail):] == tail:
                    name = check_id
            transcript = self.transcripts / ("%03d.txt" % number)
            found.append({"check": name, "counter_line": number,
                          "unittest_output": transcript.read_bytes() if transcript.is_file() else None})
        return found

    def check(self, root_name, root, base, token, role, request_id):
        """Run one scripted check and record one entry per registered check name it executed.

        The observation may cover every unit visible in the root; only the units owned by the
        checking role (WORK_OF_ROLE) form that role's targets.
        """
        args = ["--ledger", str(self.ledger), "--run", token, "--root", str(root), "--base-root", str(base),
                "--scratch", str(self.scratch / "check-scratch"), "--request-id", request_id,
                "--actor", ACTORS[role]]
        start = self._counter_lines()
        document = self.act("check", args, request_id)
        executions = self._executions_since(start)
        self.check_runs += self._executed(document)
        rows = [row for row in document.get("evidence") or [] if row.get("check") != BOUNDARY]
        owned = WORK_OF_ROLE[self.grouping][role]
        units = [unit for unit in (document.get("observation") or {}).get("units") or []
                 if unit["work"] in owned]
        completion = document.get("completion")
        work_fp = completion.get("work_fp") if isinstance(completion, dict) else None
        outcome = self._outcome(document, 0)
        withheld = list(document.get("withheld") or [])
        targets = [{"unit": unit["work"], "output": unit["output"], "work_fp": work_fp} for unit in units]
        if outcome == "completed":
            self.completed_targets[request_id] = normalize_targets(targets)
        # The output vector is the whole observed vector the check consumed (every unit visible in the root;
        # the integration check lists output_vector among its inputs). Only the owned units are targets.
        observed_units = (document.get("observation") or {}).get("units") or []
        output_vector = {"targets": [{"unit": unit["work"], "output": unit["output"]} for unit in observed_units]}
        for name in dict.fromkeys(row["check"] for row in rows):
            mine = [row for row in rows if row["check"] == name]
            matched = sum(int(row["matched"]) for row in mine if row.get("matched") not in (None, "none"))
            passed = sum(int(row["matched"]) for row in mine if row["outcome"] == "pass"
                         and row.get("matched") not in (None, "none"))
            failed = sum(1 for row in mine if row["outcome"] != "pass")
            reasons = list(withheld)
            for row in mine:
                if row["outcome"] != "pass" and row["reason"] not in reasons:
                    reasons.append(row["reason"])
            self.checks.append({
                "name": name, "unit": "+".join(sorted({row["work"] for row in mine})),
                "root": root_name, "request_id": request_id, "outcome": outcome,
                "completed": outcome == "completed",
                "result": {"version": 1, "matched": matched, "passed": passed, "failed": failed},
                # One entry per counted execution of this check name; unittest_output is the transcript
                # check_runner retained for that execution (None when it is missing).
                "executions": [item for item in executions if item["check"] == name],
                "output_vector": output_vector, "withheld": withheld, "reasons": reasons,
                "work_fp": work_fp, "targets": targets})
        return document

    # ----- scratch tree --------------------------------------------------
    def setup(self):
        """Copy the fixture, generate the counting registry, and build the base and root views (no ledger writes)."""
        self.scratch.mkdir(parents=True, exist_ok=False)
        self.home.mkdir(parents=True)
        self.raw.mkdir(parents=True, exist_ok=False)
        self.fixture = self.scratch / "fixture"
        shutil.copytree(Path(self.cfg["fixture"]), self.fixture)
        self.workspace = self.fixture / "workspace"
        self.runner = self.scratch / "check_runner.py"
        shutil.copyfile(SUPPORT / "check_runner.py", self.runner)
        source = json.loads((self.fixture / "registry" / "quota-tools.json").read_text(encoding="utf-8"))
        self.check_argv = {check["id"]: list(check["argv"]) for check in source["checks"]}
        self.transcripts = self.scratch / "transcripts"
        self.registry = self.scratch / "registry-counting.json"
        generated = registry_gen.generate(source, runner=self.runner, counter=self.counter, mode="unittest",
                                          python=sys.executable, transcript=self.transcripts)
        self.registry.write_text(json.dumps(generated, indent=2) + "\n", encoding="utf-8")
        self.contract_sha = sha256((self.workspace / CONTRACT).read_bytes())
        self.base = self.scratch / "base"
        shutil.copytree(self.fixture / "base", self.base)
        (self.base / "text").mkdir()
        shutil.copyfile(self.workspace / CONTRACT, self.base / CONTRACT)
        self.scratch_check = self.scratch / "check-scratch"
        self.scratch_check.mkdir()
        if self.grouping == "combined":
            self.roots = {"builder": self._root("root_combined", self.base, ["src", "tests"])}
            self.root_for = {"builder": self.roots["builder"]}
            self.root_cli_noctr = self._copy_without("root_cli_noctr", self.roots["builder"])
        else:
            early = self._root("root_cli_early", self.base, ["src/cli", "tests/cli"])
            library = self._root("root_library", self.base, ["src/core", "tests/core"])
            cli = self._root("root_cli", library, ["src/cli", "tests/cli"])
            self.roots = {"root_cli_early": early, "root_library": library, "root_cli": cli}
            self.root_cli_noctr = self._copy_without("root_cli_noctr", cli)
        self.base_noctr = self._copy_without("base_noctr", self.base)

    def _root(self, name, source, subtrees):
        """Build one root view: a copy of source, the current/ subtrees overlaid, and the contract placed."""
        destination = self.scratch / name
        shutil.copytree(source, destination)
        copy_overlay(self.fixture / "current", destination, subtrees)
        (destination / "text").mkdir(exist_ok=True)
        shutil.copyfile(self.workspace / CONTRACT, destination / CONTRACT)
        return destination

    def _copy_without(self, name, source):
        destination = self.scratch / name
        shutil.copytree(source, destination)
        (destination / CONTRACT).unlink()
        return destination

    def export(self, revision):
        target = self.scratch / ("export-%02d" % (len(self.exports) + 1))
        document = self.inspect("export", ["--ledger", str(self.ledger), "--revision", str(revision),
                                           "--target", str(target)])
        record = {"revision": str(revision), "target": target.name,
                  "spec_bytes": (target / "spec.lua").read_bytes(), "source": document.get("source")}
        self.exports.append(record)
        return record

    # ----- the scripted session ------------------------------------------
    def run(self):
        self.setup()
        init = self._init()
        loaded = self.act("load", ["--ledger", str(self.ledger), "--workspace", str(self.workspace / "workspace.lua"),
                                   "--registry", str(self.registry), "--expect-revision", "none",
                                   "--request-id", "load", "--actor", "driver"], "load")
        revision = str(loaded["revision"])
        job = "combined" if self.grouping == "combined" else "split"
        self.act("activate", ["--ledger", str(self.ledger), "--job", job, "--expect-revision", revision,
                              "--expect-job", "NONE", "--request-id", "activate", "--actor", "driver"], "activate")
        self.replay("activate", "activate")
        self.phase_start_claimable = sorted(r["role"] for r in self.status()["roles"] if r["state"] == "ready")
        if self.grouping == "combined":
            self.combined()
        else:
            self.split()
        final = self.status()
        acceptance = final["review"]["acceptance"]
        accepted = bool(acceptance) and all(entry["accepted"] for entry in acceptance) \
            and not final["review"]["blockers"]
        self.status_value = "accepted" if accepted else "not_accepted"
        self.accepted_revision = revision
        self.export(revision)
        self.verify_history(init)
        detached = self.detached() if self.grouping == "combined" else None
        regroup = self.regroup(revision) if self.cfg.get("regroup") and self.grouping == "combined" else None
        return self.finish(detached, regroup)

    def _init(self):
        """Create the ledger (not guided: no ledger exists before init) and record the initial event."""
        self.current_verb = "init"
        process, _ = self._exec("init", ["--ledger", str(self.ledger), "--workspace-id", "quota"], "init")
        if process.returncode != 0:
            raise CandidateFault("init exited %d: %r" % (process.returncode, process.stderr))
        self.known_seq = 1
        self.at_event[1] = {"type": "ledger_initialized", "request_id": "none",
                            "recorded": state_view(self.history("--at-event", "1"))}
        return parse_json(process.stdout)

    def combined(self):
        ledger = str(self.ledger)
        builder = self.act("claim", ["--ledger", ledger, "--job", "combined", "--role", "builder",
                                     "--request-id", "claim-builder", "--actor", ACTORS["builder"]],
                           "claim-builder")
        self.builder_token = builder["run"]["token"]
        self.replay("claim", "claim-builder")
        self.check("builder", self.roots["builder"], self.base, self.builder_token, "builder", "check-builder")
        self.replay("check", "check-builder")
        self.final_targets = normalize_targets(self.completed_targets["check-builder"])
        reviewer = self.act("claim", ["--ledger", ledger, "--job", "combined", "--role", "reviewer",
                                      "--request-id", "claim-reviewer", "--actor", REVIEWER], "claim-reviewer")
        self.reviewer = reviewer
        self.reviewer_token = reviewer["run"]["token"]
        self.wrong_target_control(self.final_targets, reviewer)
        # A historical verdict leaves the reviewer stale and claimable again (R18-3, DR18-1): claim a fresh run.
        reviewer = self.act("claim", ["--ledger", ledger, "--job", "combined", "--role", "reviewer",
                                      "--request-id", "claim-reviewer-2", "--actor", REVIEWER],
                            "claim-reviewer-2")
        self.reviewer = reviewer
        self.reviewer_token = reviewer["run"]["token"]
        self.worker_token_control(self.final_targets, reviewer)
        self.submit_verdict(self.reviewer_token, self.final_targets, self.reviewer)
        self.replay("verdict", "verdict")

    def split(self):
        ledger = str(self.ledger)
        library = self.act("claim", ["--ledger", ledger, "--job", "split", "--role", "library_worker",
                                     "--request-id", "claim-library", "--actor", ACTORS["library_worker"]],
                           "claim-library")
        self.library_token = library["run"]["token"]
        cli = self.act("claim", ["--ledger", ledger, "--job", "split", "--role", "cli_worker",
                                 "--request-id", "claim-cli", "--actor", ACTORS["cli_worker"]], "claim-cli")
        self.cli_token = cli["run"]["token"]
        self.replay("claim", "claim-library")
        self.control("contract_missing_from_root", "check",
                     ["--ledger", ledger, "--run", self.cli_token, "--root", str(self.root_cli_noctr),
                      "--base-root", str(self.base), "--scratch", str(self.scratch_check),
                      "--request-id", "control-root-no-contract", "--actor", ACTORS["cli_worker"]],
                     "control-root-no-contract")
        self.control("contract_missing_from_base", "check",
                     ["--ledger", ledger, "--run", self.library_token, "--root", str(self.roots["root_library"]),
                      "--base-root", str(self.base_noctr), "--scratch", str(self.scratch_check),
                      "--request-id", "control-base-no-contract", "--actor", ACTORS["library_worker"]],
                     "control-base-no-contract")
        self.check("root_cli_early", self.roots["root_cli_early"], self.base, self.cli_token, "cli_worker",
                   "check-cli-early")
        self.cli_worker_status = self.status("cli_worker")
        self.check("root_library", self.roots["root_library"], self.base, self.library_token, "library_worker",
                   "check-library")
        self.check("root_cli", self.roots["root_cli"], self.base, self.cli_token, "cli_worker", "check-cli")
        self.replay("check", "check-library")
        reviewer = self.act("claim", ["--ledger", ledger, "--job", "split", "--role", "reviewer",
                                      "--request-id", "claim-reviewer", "--actor", REVIEWER], "claim-reviewer")
        self.reviewer = reviewer
        self.reviewer_token = reviewer["run"]["token"]
        self.final_targets = normalize_targets(self.completed_targets["check-library"]
                                               + self.completed_targets["check-cli"])
        self.submit_verdict(self.reviewer_token, self.final_targets, reviewer)
        self.replay("verdict", "verdict")

    def wrong_target_control(self, targets, reviewer):
        """Submit an approval whose command output is not the observed one; the engine records it historical (R18-3)."""
        wrong = [dict(target) for target in targets]
        wrong[-1]["output"] = PLACEHOLDER_OUTPUT
        path = self.write_verdict("wrong-target.json", wrong, reviewer["dispatch"]["work_fp"], "wrong target")
        self.submitted["verdict-wrong-target"] = {"decision": "approve", "targets": normalize_targets(wrong),
                                                  "policy": reviewer["dispatch"]["work_fp"]}
        self.control("wrong_target", "verdict",
                     ["--ledger", str(self.ledger), "--run", self.reviewer_token, "--verdict", str(path),
                      "--request-id", "verdict-wrong-target", "--actor", REVIEWER], "verdict-wrong-target")

    def worker_token_control(self, targets, reviewer):
        """Submit a correct-looking approval with the worker's saved run token; the engine must refuse it."""
        path = self.write_verdict("worker-token.json", targets, reviewer["dispatch"]["work_fp"], "worker token")
        self.submitted["verdict-worker-token"] = {"decision": "approve", "targets": targets,
                                                  "policy": reviewer["dispatch"]["work_fp"]}
        self.control("worker_token", "verdict",
                     ["--ledger", str(self.ledger), "--run", self.builder_token, "--verdict", str(path),
                      "--request-id", "verdict-worker-token", "--actor", REVIEWER], "verdict-worker-token")

    def write_verdict(self, name, targets, policy, summary):
        document = {"version": 1,
                    "targets": [{"work": t["unit"], "output": t["output"], "work_fp": t["work_fp"]}
                                for t in targets],
                    "policy": policy, "decision": "approve", "findings": [], "repair": [],
                    "summary": "Independent exact-target approval" if summary == "approve" else summary}
        path = self.scratch / name
        path.write_text(json.dumps(document, indent=1) + "\n", encoding="utf-8")
        return path

    def submit_verdict(self, token, targets, reviewer):
        ledger = str(self.ledger)
        path = self.write_verdict("verdict-approve.json", targets, reviewer["dispatch"]["work_fp"], "approve")
        self.submitted["verdict"] = {"decision": "approve", "targets": normalize_targets(targets),
                                     "policy": reviewer["dispatch"]["work_fp"]}
        response = self.act("verdict", ["--ledger", ledger, "--run", token, "--verdict", str(path),
                                        "--request-id", "verdict", "--actor", REVIEWER], "verdict")
        status = self.status()
        entries = status["review"]["acceptance"]
        acceptance = "accepted" if entries and all(entry["accepted"] for entry in entries) else "not_accepted"
        self.final_response = response
        for role, works in WORK_OF_ROLE[self.grouping].items():
            self.verdicts.append({
                "unit": role, "decision": "approve", "acceptance": acceptance, "actor": REVIEWER,
                "targets": [t for t in targets if t["unit"] in works], "policy": reviewer["dispatch"]["work_fp"],
                "exit": 0, "submission": "verdict", "request_id": "verdict",
                "run": reviewer["run"]["id"], "verdict_id": response["verdict"]["id"]})

    # ----- history verification (I6-I9, Q7, Q17) ---------------------------
    def verify_history(self, init):
        del init
        events = self.history()["data"]["events"]
        self.event_count = len(events)
        for event in events:
            seq = int(event["sequence"])
            replayed = state_view(self.history("--at-event", str(seq)))
            recorded = self.at_event[seq]["recorded"]
            if self.tamper.get("at_event"):
                recorded = dict(recorded, activation="999")
            lib.verify_at_event_state(recorded, replayed)
            self.at_event[seq]["replayed"] = replayed
        self.history_records = []
        for event in events:
            seq = int(event["sequence"])
            document = self.inspect("history", ["--ledger", str(self.ledger), "--event", str(seq)])
            kind = event["type"]
            expansion = expansion_for(kind, document["data"])
            self.history_records.append({"sequence": seq, "type": kind, "request_id": event["request_id"],
                                         "expansion": expansion, "stdout": json.dumps(document, sort_keys=True)})
            if kind in ("job_activated", "run_claimed", "check_reserved", "verdict_recorded"):
                self.verify_expansion(kind, event, expansion)

    def verify_expansion(self, kind, event, expansion):
        """Compare one history expansion with the references recorded in the response that caused the event."""
        request = self.responses.get(event["request_id"])
        if not isinstance(request, dict):
            raise lib.AcceptError("%s has no recorded response to compare" % kind)
        token = None
        if kind == "job_activated":
            expected = {"revision.id": str(request.get("revision")), "activation.id": str(request.get("activation")),
                        "job": request.get("job")}
        elif kind == "run_claimed":
            expected = {"dispatch.id": str(request["dispatch"]["id"]),
                        "dispatch.work_fp": request["dispatch"]["work_fp"], "run.id": str(request["run"]["id"])}
            token = request["run"]["token"]
        elif kind == "check_reserved":
            expected = {"reservation.id": str(request.get("reservation")), "run.id": str(request["run"]["id"])}
        else:
            submitted = self.submitted[event["request_id"]]
            expected = {"decision": request["verdict"]["decision"], "verdict.id": str(request["verdict"]["id"]),
                        "run": str(request["run"]), "targets": submitted["targets"],
                        "policy": submitted["policy"], "blockers": request.get("blockers")}
        if self.tamper.get("expansion") and kind == "job_activated":
            expected = dict(expected, **{"revision.id": "999"})
        lib.verify_event_expansion(kind, expansion, expected, token=token)

    # ----- detached inspection (Q18, D33) ----------------------------------
    def detached_outputs(self, tag):
        """Run every history and export inspection of the ledger alone; an export gets a fresh target per pass."""
        outputs, argv = {}, []
        ledger = str(self.ledger)
        for seq in sorted(self.at_event):
            for key, args in (("event-%d" % seq, ["--event", str(seq)]),
                              ("at-event-%d" % seq, ["--at-event", str(seq)])):
                full = ["--ledger", ledger, *args]
                process, _ = self._exec("history", full)
                outputs[key] = [process.returncode, process.stdout.decode("utf-8", errors="replace")]
                argv.append(["history", *full])
        target = self.scratch / ("export-detached-" + tag)
        full = ["--ledger", ledger, "--revision", self.accepted_revision, "--target", str(target)]
        process, _ = self._exec("export", full)
        outputs["export"] = [process.returncode, process.stdout.decode("utf-8", errors="replace")]
        argv.append(["export", *full])
        return outputs, argv

    def detached(self):
        """Rename the scratch workspace .lua files and registry, rerun history and export, then restore."""
        before_counter = self._counter_lines()
        before, _ = self.detached_outputs("before")
        names = sorted(self.workspace.glob("*.lua")) + [self.registry]
        for path in names:
            os.rename(path, Path(str(path) + ".moved"))
        try:
            after, argv = self.detached_outputs("after")
        finally:
            for path in names:
                os.rename(Path(str(path) + ".moved"), path)
        return {"renamed": True, "restored": all(path.is_file() for path in names), "argv": argv,
                "before": before, "after": after,
                "counter_unchanged": before_counter == self._counter_lines()}

    # ----- regrouping (Q10, D20) -------------------------------------------
    def regroup(self, revision):
        """Load the split workspace variant over the finished combined ledger, activate it, probe the old token."""
        ledger = str(self.ledger)
        old = {r["role"]: r for r in self.status()["roles"]}
        variant = self.workspace / "workspace-split.lua"
        variant.write_text(WORKSPACE_SPLIT, encoding="utf-8")
        loaded = self.act("load", ["--ledger", ledger, "--workspace", str(variant), "--registry", str(self.registry),
                                   "--expect-revision", str(revision), "--request-id", "regroup-load",
                                   "--actor", "driver"], "regroup-load")
        new_revision = str(loaded["revision"])
        self.act("activate", ["--ledger", ledger, "--job", "split", "--expect-revision", new_revision,
                              "--expect-job", "NONE", "--request-id", "regroup-activate", "--actor", "driver"],
                 "regroup-activate")
        activated = self.status()
        new = {r["role"]: r for r in activated["roles"]}
        before = self._digest()
        process, _ = self._exec("release", ["--ledger", ledger, "--run", self.builder_token, "--reason", "released",
                                            "--request-id", "fenced-probe", "--actor", ACTORS["builder"]],
                                "fenced-probe")
        probe = {"exit": process.returncode, "error_code": error_code(process.stderr),
                 "ledger_before": before, "ledger_after": self._digest()}
        # activated is read back from the ledger (active job and revision), never assumed (Q10).
        return {"activated": activated["active_job"] == "split" and activated["active_revision"] == new_revision,
                "job_after": activated["active_job"], "load_status": loaded.get("outcome"), "fenced_probe": probe,
                "reviewer_generation_old": int(old["reviewer"]["generation"]),
                "reviewer_generation_new": int(new["reviewer"]["generation"]),
                "inherited_approval": any(entry["accepted"] for entry in activated["review"]["acceptance"]),
                "claimable": sorted(role for role, record in new.items() if record["state"] == "ready")}

    # ----- result ----------------------------------------------------------
    def finish(self, detached, regroup):
        at_event = [{"number": seq, "type": self.at_event[seq]["type"],
                     "request_id": self.at_event[seq]["request_id"],
                     "recorded": self.at_event[seq]["recorded"],
                     "replayed": self.at_event[seq].get("replayed")} for seq in sorted(self.at_event)]
        return {
            "status": self.status_value,
            "spec_sha256": sha256((self.workspace / "spec.lua").read_bytes()),
            "exports": self.exports, "raw_root": str(self.raw), "steps": self.steps,
            "checks": self.checks, "observed_targets": self.final_targets,
            "builder_actor": ACTORS["builder"], "verdicts": self.verdicts,
            "phase_start_claimable": self.phase_start_claimable,
            "cli_worker_status": getattr(self, "cli_worker_status", None),
            "replays": self.replays, "inspections": self.inspections, "guidance": self.guidance,
            "history": self.history_records, "event_count": self.event_count, "at_event": at_event,
            "accepted_revision": self.accepted_revision, "final_targets": self.final_targets,
            "roots": [{"role": name, "root": name, "has_contract": (root / CONTRACT).is_file(),
                       "sha256": sha256((root / CONTRACT).read_bytes()) if (root / CONTRACT).is_file() else None,
                       "tree": tree_files(root)} for name, root in self.roots.items()],
            "base_contract_sha256": self.contract_sha,
            "registry_path": str(self.registry), "counter_path": str(self.counter),
            "check_runs": self.check_runs, "negative_controls": self.controls,
            "detached": detached, "regroup": regroup,
        }


def run_grouping(config):
    """Run one quota grouping end to end; on any failure write ABORTED.json and re-raise."""
    session = Session(config)
    try:
        return session.run()
    except (lib.AcceptError, CandidateFault) as error:
        session.raw.mkdir(parents=True, exist_ok=True)
        lines = session._counter_lines()
        (session.raw / "ABORTED.json").write_text(json.dumps({
            "verb": session.current_verb, "error": str(error), "error_type": type(error).__name__,
            "ledger_digest": lib.ledger_digest(session.ledger), "counter_lines": lines}, indent=2) + "\n",
            encoding="utf-8")
        raise


if __name__ == "__main__":
    sys.exit(0)
