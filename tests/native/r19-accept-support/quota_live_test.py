#!/usr/bin/env python3
# R19-ACCEPT-A live quota tests (QuotaLiveTest): rows Q1-Q19 of plan/INVARIANTS.md.
# They drive the D28 scratch-built candidate named by PLANAR_CANDIDATE through
# quota_driver.run_grouping with fresh ledgers under a per-test temporary root.
# They FAIL (never skip) when PLANAR_CANDIDATE is unset. Each docstring names the
# mutant the row must kill.
import sys

sys.dont_write_bytecode = True

import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import accept_lib as lib  # noqa: E402
import quota_driver  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
FIXTURE = REPO / "fixtures/render/quota"
SUPPORT_TREES = ["tests/native/r19-cmake-support", "tests/native/r19-authoring-support"]


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def tree_digest(root):
    """Map every file under root (including __pycache__) to its sha256."""
    root = Path(root)
    return {p.relative_to(root).as_posix(): sha256(p.read_bytes())
            for p in sorted(root.rglob("*")) if p.is_file()}


def unittest_counts(text):
    """Derive matched, passed and failed from the raw unittest summary lines."""
    ran = re.search(r"^Ran (\d+) tests?", text, re.M)
    if not ran:
        return None
    summary = [line for line in text.splitlines() if line.startswith(("FAILED (", "OK (", "OK"))]
    line = summary[-1] if summary else "OK"
    counts = {key: int(value) for key, value in re.findall(r"(\w+(?: \w+)?)=(\d+)", line)}
    failed = counts.get("failures", 0) + counts.get("errors", 0) + counts.get("unexpected successes", 0)
    skipped = counts.get("skipped", 0)
    matched = int(ran.group(1))
    return {"matched": matched, "passed": matched - failed - skipped, "failed": failed}


class QuotaLiveTest(unittest.TestCase):
    def setUp(self):
        self.candidate = os.environ.get("PLANAR_CANDIDATE", "")
        if not self.candidate:
            self.fail("PLANAR_CANDIDATE must name the D28 candidate binary; live rows fail, never skip")
        self.root = Path(tempfile.mkdtemp(prefix="r19-quota-live-"))
        self.addCleanup(shutil.rmtree, str(self.root), True)
        self.serial = 0

    def config(self, grouping, regroup=False, tamper=None):
        """Fresh scratch HOME, PLANAR_DB, ledger and raw root for one run."""
        self.serial += 1
        scratch = self.root / ("run-%02d" % self.serial)
        return {
            "candidate": self.candidate, "repo": str(REPO), "fixture": str(FIXTURE),
            "scratch": str(scratch), "raw_root": str(scratch / "raw"), "home": str(scratch / "home"),
            "planar_db": str(scratch / "planar.db"), "ledger": str(scratch / "ledger.db"),
            "counter": str(scratch / "check-counter.log"), "grouping": grouping,
            "regroup": regroup, "tamper": dict(tamper or {}),
        }

    def need(self, value, key):
        """Return value[key], failing with an assertion (never a KeyError) when it is absent."""
        self.assertIsInstance(value, dict, "result section missing for " + repr(key))
        self.assertIn(key, value, "result lacks " + repr(key))
        return value[key]

    def named(self, items, name):
        """Return the item whose name is name, failing with an assertion when absent."""
        matches = [item for item in items if item.get("name") == name]
        self.assertEqual(len(matches), 1, "expected exactly one control named " + repr(name))
        return matches[0]

    # Q1
    def test_combined_and_split_share_spec_bytes_and_load_accepted(self):
        """Kills: quota_driver gives the split run a spec copy with one changed byte."""
        spec = (FIXTURE / "workspace/spec.lua").read_bytes()
        for grouping in ("combined", "split"):
            with self.subTest(grouping=grouping):
                result = quota_driver.run_grouping(self.config(grouping))
                self.assertEqual(self.need(result, "status"), "accepted")
                self.assertEqual(self.need(result, "spec_sha256"), sha256(spec))
                exports = self.need(result, "exports")
                self.assertTrue(exports, "export must return the accepted source")
                self.assertEqual(self.need(exports[-1], "spec_bytes"), spec)

    # Q2
    def test_combined_grouping_completes_with_exact_target_approval(self):
        """Kills: quota_driver skips applying current/ over base/ for src/core, or writes a non-atomic try_take, so the contract check fails."""
        result = quota_driver.run_grouping(self.config("combined"))
        self.assertEqual(self.need(result, "status"), "accepted")
        checks = self.need(result, "checks")
        for name in ("contract", "integration"):
            with self.subTest(check=name):
                check = self.named(checks, name)
                self.assertEqual(check["outcome"], "completed")
                self.assertGreater(check["result"]["matched"], 0)
                self.assertEqual(check["result"]["failed"], 0)
        roots = self.need(result, "roots")
        self.assertEqual(len(roots), 1)
        self.assertTrue(roots[0]["has_contract"])
        for path in ("src/cli/main.py", "src/core/quota.py", "text/quota-api.md"):
            self.assertIn(path, roots[0]["tree"])
        approvals = [v for v in self.need(result, "verdicts") if v.get("decision") == "approve"]
        self.assertEqual(len(approvals), 1)
        self.assertEqual(approvals[0]["acceptance"], "accepted")
        self.assertEqual(approvals[0]["exit"], 0)
        self.assertEqual(approvals[0]["targets"], self.need(result, "observed_targets"))
        self.assertNotEqual(approvals[0]["actor"], self.need(result, "builder_actor"))

    # Q3
    def test_split_grouping_orders_dependency_and_completes(self):
        """Kills: the split fixture drops the integrate dependency (cli completes early), or the cli check runs on a mixed root with library edits."""
        result = quota_driver.run_grouping(self.config("split"))
        self.assertEqual(self.need(result, "status"), "accepted")
        self.assertIn("cli_worker", self.need(result, "phase_start_claimable"))
        checks = self.need(result, "checks")
        early = [c for c in checks if c.get("root") == "root_cli_early"]
        # The early root executes both registered checks and each is withheld (per work and check rows,
        # execution-contract-r17.md:408), so the root has one entry per check name, not one per root.
        self.assertEqual(sorted(c["name"] for c in early), ["contract", "integration"])
        for entry in early:
            with self.subTest(check=entry["name"]):
                self.assertEqual(entry["outcome"], "withheld")
                self.assertIn("complete_prerequisite", entry["reasons"])
                self.assertFalse(entry.get("completed", False))
        status = self.need(result, "cli_worker_status")
        explain = self.need(self.need(status, "role_detail"), "explain")
        prerequisites = self.need(explain, "prerequisite")
        self.assertEqual([p["id"] for p in prerequisites], ["integrate"], "keyed by the dependency id, not library")
        self.assertEqual(prerequisites[0]["phase"], "complete")
        library = [c for c in checks if c.get("root") == "root_library"]
        self.assertEqual(len(library), 1)
        self.assertEqual(library[0]["outcome"], "completed")
        fresh = [c for c in checks if c.get("root") == "root_cli"]
        self.assertEqual(sorted(c["name"] for c in fresh), ["contract", "integration"])
        for entry in fresh:
            with self.subTest(check=entry["name"]):
                self.assertEqual(entry["outcome"], "completed")
        self.assertNotEqual(fresh[0]["request_id"], early[0]["request_id"], "the cli check on root_cli is a fresh request")
        self.assertIn("library", [t["unit"] for t in self.need(fresh[0], "output_vector")["targets"]])
        approved = {v["unit"] for v in self.need(result, "verdicts") if v.get("decision") == "approve"}
        self.assertEqual(approved, {"library_worker", "cli_worker"})

    # Q4
    def test_every_action_retains_raw_argv_stdout_stderr_exit(self):
        """Kills: quota_driver invoke does not write the .stdout file for check, or skips the .exit file."""
        result = quota_driver.run_grouping(self.config("combined"))
        steps = self.need(result, "steps")
        self.assertTrue(steps)
        self.assertEqual([s["index"] for s in steps], list(range(1, len(steps) + 1)), "numbering must be gapless")
        for step in steps:
            files = self.need(step, "files")
            for kind in ("argv", "stdout", "stderr", "exit"):
                with self.subTest(step=step["index"], kind=kind):
                    self.assertIn(kind, files)
                    self.assertTrue(Path(files[kind]).is_file(), "raw " + kind + " file missing")
        raw = Path(self.need(result, "raw_root"))
        self.assertEqual(len(list(raw.rglob("*.exit"))), len(steps), "one .exit file per action")
        self.assertEqual(len(list(raw.rglob("*.argv.json"))), len(steps), "one argv file per action")

    # Q5
    def test_restart_replay_is_byte_identical_without_reexecution(self):
        """Kills: quota_driver replay helper uses a fresh request id, which re-executes the check."""
        for grouping in ("combined", "split"):
            with self.subTest(grouping=grouping):
                result = quota_driver.run_grouping(self.config(grouping))
                replays = self.need(result, "replays")
                verbs = {r.get("verb") for r in replays}
                self.assertTrue({"activate", "claim", "check", "verdict"} <= verbs, verbs)
                for replay in replays:
                    self.assertTrue(replay["response_identical"], replay)
                    self.assertTrue(replay["ledger_identical"], replay)
                    self.assertTrue(replay["counter_unchanged"], replay)

    # Q6
    def test_wrong_target_verdict_is_refused_and_ledger_unchanged(self):
        """Kills: the negative-control builder uses the correct target vector instead of the wrong one."""
        result = quota_driver.run_grouping(self.config("combined"))
        controls = self.need(result, "negative_controls")
        for name in ("wrong_target", "worker_token"):
            with self.subTest(control=name):
                control = self.named(controls, name)
                if name == "wrong_target":
                    # Test name kept (no renames). Mismatched targets are not refusals: the approval is recorded
                    # historical with exit 0, appends one verdict event and approves nothing (R18-3,
                    # execution-contract-r18.md:144; execution-design.md A06).
                    self.assertEqual(control["exit"], 0)
                    self.assertEqual(control["outcome"], "historical")
                    self.assertNotEqual(control["ledger_before"], control["ledger_after"],
                                        "the historical verdict is one recorded event")
                    self.assertEqual(control["counter_before"], control["counter_after"])
                else:
                    self.assertNotEqual(control["exit"], 0)
                    self.assertTrue(control["error_code"], "the refusal must be typed")
                    self.assertEqual(control["ledger_before"], control["ledger_after"])

    # Q7
    def test_history_event_and_at_event_reproduce_accepted_records(self):
        """Kills: the inspection queries --at-event at the latest event only, skipping per-event expansion."""
        result = quota_driver.run_grouping(self.config("combined"))
        history = self.need(result, "history")
        types = {h.get("type") for h in history}
        for kind in ("job_activated", "run_claimed", "check_recorded", "verdict_recorded"):
            self.assertIn(kind, types)
        self.assertEqual(len(history), self.need(result, "event_count"), "every event must be expanded")
        self.assertEqual(len(self.need(result, "at_event")), len(history))
        accepted = self.need(result, "accepted_revision")
        activated = [h for h in history if h.get("type") == "job_activated"]
        self.assertEqual(self.need(self.need(activated[0], "expansion"), "revision")["id"], accepted)
        approvals = [h for h in history if h.get("type") == "verdict_recorded"
                     and h["expansion"].get("decision") == "approve"]
        self.assertTrue(approvals)
        self.assertEqual(approvals[-1]["expansion"]["targets"], self.need(result, "final_targets"))

    # Q8
    def test_inspection_reads_do_not_write_ledger(self):
        """Kills: quota_driver inspect() adds a mutating claim with a new request id."""
        result = quota_driver.run_grouping(self.config("combined"))
        inspections = self.need(result, "inspections")
        verbs = {i.get("verb") for i in inspections}
        self.assertTrue({"status", "history", "orchestrator", "export"} <= verbs, verbs)
        for inspection in inspections:
            with self.subTest(verb=inspection.get("verb")):
                self.assertIn(inspection["verb"], {"status", "history", "orchestrator", "export"})
                self.assertEqual(inspection["ledger_before"], inspection["ledger_after"])
                self.assertEqual(inspection["counter_before"], inspection["counter_after"])

    # Q9
    def test_orchestrator_guidance_is_usable_at_every_phase(self):
        """Kills: quota_driver captures the orchestrator once (or stores a canned string), or passes next_verb=None."""
        for grouping in ("combined", "split"):
            with self.subTest(grouping=grouping):
                result = quota_driver.run_grouping(self.config(grouping))
                guidance = self.need(result, "guidance")
                self.assertGreaterEqual(len(guidance), 3)
                steps = self.need(result, "steps")
                live_states = set()
                for record in guidance:
                    self.assertIsNotNone(record.get("next_verb"), "each capture stores the next scripted verb")
                    self.assertTrue(lib.guidance_is_usable(record["text"], record["live_state"], record["next_verb"]),
                                    "unusable guidance at phase " + str(record.get("phase")))
                    following = record["step_index"] + 1
                    self.assertLess(following, len(steps))
                    self.assertEqual(steps[following]["verb"], record["next_verb"])
                    live_states.add(json.dumps(record["live_state"], sort_keys=True))
                self.assertGreaterEqual(len(live_states), 2, "guidance must be captured live at each phase")

    # Q10
    def test_regroup_combined_to_split_fences_old_tokens(self):
        """Kills: quota_driver regroup skips the activate step, or presents the new token in the fenced probe."""
        result = quota_driver.run_grouping(self.config("combined", regroup=True))
        regroup = self.need(result, "regroup")
        self.assertTrue(self.need(regroup, "activated"))
        self.assertEqual(self.need(regroup, "load_status"), "accepted")
        probe = self.need(regroup, "fenced_probe")
        self.assertNotEqual(probe["exit"], 0, "the retired builder token must be refused")
        self.assertTrue(probe["error_code"])
        self.assertEqual(probe["ledger_before"], probe["ledger_after"])
        self.assertEqual(self.need(regroup, "job_after"), "split", "the split job is the active job after regroup")
        # M1 (execution-contract.md:820): the first activation of a (job, role) lineage opens generation 1. Ordinals
        # restart per job, so the combined reviewer's ordinal is not a lower bound for the split reviewer's.
        self.assertEqual(self.need(regroup, "reviewer_generation_new"), 1)
        self.assertFalse(self.need(regroup, "inherited_approval"))
        self.assertTrue({"library_worker", "cli_worker"} <= set(self.need(regroup, "claimable")))

    # Q11
    def test_fixture_copied_unmodified_and_counter_counts_checks(self):
        """Kills: the session root is the checkout fixture, registry_gen changes a check argv, or children run without PYTHONDONTWRITEBYTECODE=1."""
        before_fixture = tree_digest(FIXTURE)
        before_support = {tree: tree_digest(REPO / tree) for tree in SUPPORT_TREES}
        result = quota_driver.run_grouping(self.config("combined"))
        self.assertEqual(tree_digest(FIXTURE), before_fixture, "the checkout fixture must be unmodified")
        for tree in SUPPORT_TREES:
            with self.subTest(tree=tree):
                self.assertEqual(tree_digest(REPO / tree), before_support[tree])
        for step in self.need(result, "steps"):
            with self.subTest(step=step.get("index")):
                self.assertEqual(self.need(step, "env").get("PYTHONDONTWRITEBYTECODE"), "1")
        generated = json.loads(Path(self.need(result, "registry_path")).read_text())
        original = json.loads((FIXTURE / "registry/quota-tools.json").read_text())
        self.assertEqual([c["id"] for c in generated["checks"]], [c["id"] for c in original["checks"]])
        for gen_check, orig_check in zip(generated["checks"], original["checks"]):
            for key in ("argv", "timeout_ms", "max_output_bytes", "selection", "inputs", "coverage", "toolchain"):
                with self.subTest(check=orig_check["id"], key=key):
                    self.assertEqual(gen_check.get(key), orig_check[key])
        gen_toolchain = generated["toolchains"][0]
        orig_toolchain = original["toolchains"][0]
        self.assertEqual(gen_toolchain.get("environment"), orig_toolchain["environment"])
        self.assertEqual(gen_toolchain["command"][-len(orig_toolchain["command"]):], orig_toolchain["command"],
                         "the original command is retained as the tail of the counting prefix")
        self.assertIn("check_runner.py", " ".join(gen_toolchain["command"]))
        self.assertIn("--counter", gen_toolchain["command"])
        lines = [line for line in Path(self.need(result, "counter_path")).read_text().splitlines() if line.strip()]
        self.assertGreater(len(lines), 0)
        self.assertEqual(len(lines), self.need(result, "check_runs"), "each real check execution appends one line")

    # Q12
    def test_contract_delivered_to_base_and_every_root(self):
        """Kills: the driver omits the text/quota-api.md copy into the scratch base (boundary withheld) or into a root view (revision_conflict)."""
        contract = (FIXTURE / "workspace/text/quota-api.md").read_bytes()
        result = quota_driver.run_grouping(self.config("split"))
        self.assertEqual(self.need(result, "base_contract_sha256"), sha256(contract))
        roots = self.need(result, "roots")
        self.assertEqual(len(roots), 3, "root_cli_early, root_library and root_cli")
        for root in roots:
            with self.subTest(role=root.get("role")):
                self.assertTrue(root["has_contract"])
                self.assertEqual(root["sha256"], sha256(contract))
        controls = self.need(result, "negative_controls")
        from_root = self.named(controls, "contract_missing_from_root")
        self.assertEqual(from_root["error_code"], "revision_conflict")
        self.assertEqual(from_root["ledger_before"], from_root["ledger_after"])
        self.assertEqual(from_root["counter_before"], from_root["counter_after"])
        from_base = self.named(controls, "contract_missing_from_base")
        self.assertEqual(from_base["outcome"], "withheld")
        # Engine withheld vocabulary (src/stdlib/planar/checks.lua): a failed boundary row reads "boundary".
        self.assertIn("boundary", from_base["reasons"])

    # Q13
    def test_quota_check_results_match_actual_unittest_counts(self):
        """Kills: check_runner writes constant matched=1 and passed=1, or passed=matched while a failing test is present."""
        result = quota_driver.run_grouping(self.config("combined"))
        completed = [c for c in self.need(result, "checks") if c.get("outcome") == "completed"]
        self.assertTrue(completed)
        for check in completed:
            with self.subTest(check=check.get("name")):
                executions = self.need(check, "executions")
                self.assertTrue(executions, "a completed check has at least one counted execution")
                totals = {"matched": 0, "passed": 0, "failed": 0}
                for execution in executions:
                    output = self.need(execution, "unittest_output")
                    self.assertIsNotNone(output, "check_runner must retain the execution's transcript")
                    counts = unittest_counts(output.decode(errors="replace"))
                    self.assertIsNotNone(counts, "the raw unittest output must contain a Ran line")
                    self.assertGreater(counts["matched"], 0)
                    self.assertEqual(counts["failed"], 0)
                    for key in totals:
                        totals[key] += counts[key]
                # The entry sums its executions (combined runs the contract check once per unit).
                self.assertEqual(self.need(check, "result"), {"version": 1, **totals})

    # Q14
    def test_run_grouping_aborts_on_tampered_replay(self):
        """Kills: quota_driver removes the accept_lib.assert_replay_identical call at the replay site."""
        with self.assertRaises(lib.AcceptError):
            quota_driver.run_grouping(self.config("combined", tamper={"replay": True}))

    # Q14
    def test_run_grouping_aborts_when_replay_reexecutes_a_check(self):
        """Kills: quota_driver replay ignores the counter comparison (only a counter line is added; bytes and ledger match)."""
        with self.assertRaises(lib.AcceptError) as caught:
            quota_driver.run_grouping(self.config("combined", tamper={"counter": True}))
        self.assertIn("executed a check", str(caught.exception))

    # Q15
    def test_run_grouping_aborts_on_unusable_guidance(self):
        """Kills: quota_driver removes the accept_lib.guidance_is_usable call after the orchestrator capture."""
        with self.assertRaises(lib.AcceptError):
            quota_driver.run_grouping(self.config("combined", tamper={"guidance": "empty"}))

    # Q16
    def test_run_grouping_aborts_when_inspection_changes_ledger(self):
        """Kills: quota_driver removes the before/after ledger_digest comparison inside inspect()."""
        with self.assertRaises(lib.AcceptError):
            quota_driver.run_grouping(self.config("combined", tamper={"inspection": True}))

    # Q17
    def test_run_grouping_aborts_on_mismatched_history_reference(self):
        """Kills: quota_driver removes the verify_event_expansion and verify_at_event_state calls."""
        with self.assertRaises(lib.AcceptError):
            quota_driver.run_grouping(self.config("combined", tamper={"expansion": True}))
        with self.assertRaises(lib.AcceptError):
            quota_driver.run_grouping(self.config("combined", tamper={"at_event": True}))

    # Q18
    def test_history_inspection_is_independent_of_scratch_workspace_and_registry(self):
        """Kills: the inspection helper passes --workspace or --registry, or skips the rename of the scratch .lua files and registry."""
        result = quota_driver.run_grouping(self.config("combined"))
        detached = self.need(result, "detached")
        self.assertTrue(detached.get("renamed") is True, "scratch workspace and registry must be renamed")
        self.assertTrue(detached.get("restored") is True, "files are restored afterwards")
        before = self.need(detached, "before")
        after = self.need(detached, "after")
        self.assertTrue(before, "pre-rename inspection output must exist")
        self.assertEqual(before.keys(), after.keys())
        for key in before:
            with self.subTest(inspection=key):
                self.assertEqual(before[key], after[key])
        for argv in self.need(detached, "argv"):
            for flag in ("--workspace", "--registry", "--root"):
                self.assertNotIn(flag, argv)

    # Q19
    def test_run_grouping_aborts_before_action_not_listed_as_legal(self):
        """Kills: quota_driver removes the accept_lib.require_legal_action call before each live action."""
        config = self.config("combined", tamper={"verb": "check"})
        with self.assertRaises(lib.AcceptError):
            quota_driver.run_grouping(config)
        aborted_path = Path(config["raw_root"]) / "ABORTED.json"
        self.assertTrue(aborted_path.is_file(), "the driver records the aborted point")
        aborted = json.loads(aborted_path.read_text())
        self.assertEqual(aborted.get("verb"), "check")
        self.assertEqual(aborted.get("ledger_digest"), lib.ledger_digest(Path(config["ledger"])))
        counter = Path(config["counter"])
        lines = [line for line in counter.read_text().splitlines() if line.strip()] if counter.exists() else []
        self.assertEqual(aborted.get("counter_lines"), len(lines))
        argv_files = [p for p in Path(config["raw_root"]).rglob("*check*.argv.json")]
        self.assertEqual(argv_files, [], "no argv file may exist for the refused action")

    # L24
    def test_split_grouping_retains_raw_exact_targets_and_non_builder_actor(self):
        """Kills: quota_driver run_grouping (split) skips the raw retention write for one verb, or approves a split unit with the builder actor or with another unit's targets."""
        result = quota_driver.run_grouping(self.config("split"))
        steps = self.need(result, "steps")
        self.assertTrue(steps)
        self.assertEqual([s["index"] for s in steps], list(range(1, len(steps) + 1)), "numbering must be gapless")
        for step in steps:
            files = self.need(step, "files")
            for kind in ("argv", "stdout", "stderr", "exit"):
                with self.subTest(step=step["index"], kind=kind):
                    self.assertIn(kind, files)
                    self.assertTrue(Path(files[kind]).is_file(), "raw " + kind + " file missing")
        raw = Path(self.need(result, "raw_root"))
        self.assertEqual(len(list(raw.rglob("*.exit"))), len(steps), "one .exit file per action")
        self.assertEqual(len(list(raw.rglob("*.argv.json"))), len(steps), "one argv file per action")
        # The approving verdict as the CLI received it: the --actor and the document's targets.
        submitted = [s for s in steps if s["verb"] == "verdict"
                     and s["argv"][s["argv"].index("--request-id") + 1] == "verdict"]
        # The first submission is the approval; the replay re-sends the same saved request (Q5), not a new approval.
        self.assertTrue(submitted, "the split grouping submits an approving verdict")
        argv = submitted[0]["argv"]
        actor = argv[argv.index("--actor") + 1]
        document = json.loads(Path(argv[argv.index("--verdict") + 1]).read_text(encoding="utf-8"))
        submitted_targets = {t["work"]: t for t in document["targets"]}
        # Each unit's observed target comes from the completed check of the root that its role checks.
        observed = {}
        for root in ("root_library", "root_cli"):
            entries = [c for c in self.need(result, "checks") if c.get("root") == root and c.get("completed")]
            self.assertTrue(entries, "the split run completes checks on " + root)
            for target in entries[0]["targets"]:
                observed[target["unit"]] = target
        approvals = {v["unit"]: v for v in self.need(result, "verdicts") if v.get("decision") == "approve"}
        self.assertEqual(set(approvals), {"library_worker", "cli_worker"})
        for role, unit in (("library_worker", "library"), ("cli_worker", "command")):
            with self.subTest(role=role):
                self.assertEqual(approvals[role]["acceptance"], "accepted")
                self.assertEqual(approvals[role]["exit"], 0)
                self.assertNotEqual(actor, self.need(result, "builder_actor"))
                self.assertEqual(submitted_targets[unit]["output"], observed[unit]["output"])
                self.assertEqual(submitted_targets[unit]["work_fp"], observed[unit]["work_fp"])
                self.assertEqual(approvals[role]["targets"], [observed[unit]])

    # Negative fixtures for the C02 survivors (PKG-FIX). New names only; no existing test is renamed.

    def test_combined_builder_root_carries_the_current_overlay_not_the_base_copy(self):
        """Kills Q2-a: the combined builder root omits the current/src/core overlay, so the root keeps the base copy of quota.py."""
        config = self.config("combined")
        quota_driver.run_grouping(config)
        root = Path(config["scratch"]) / "root_combined"
        for relative in ("src/core/quota.py", "src/cli/main.py"):
            with self.subTest(path=relative):
                self.assertEqual(sha256((root / relative).read_bytes()),
                                 sha256((FIXTURE / "current" / relative).read_bytes()))

    def test_every_history_event_has_a_replayed_state_equal_to_its_recorded_state(self):
        """Kills Q7: verify_history replays --at-event only for the latest event, so earlier history events carry no
        replayed state to compare with the recorded one."""
        result = quota_driver.run_grouping(self.config("combined"))
        by_number = {record["number"]: record for record in self.need(result, "at_event")}
        history = self.need(result, "history")
        self.assertTrue(history)
        for event in history:
            number = int(event["sequence"])
            with self.subTest(sequence=number):
                record = self.need(by_number, number)
                self.assertIsNotNone(record.get("replayed"), "each event's state is replayed with --at-event")
                self.assertEqual(record["replayed"], record["recorded"])

    def test_session_argv_names_scratch_paths_and_never_the_checkout_fixture(self):
        """Kills Q11-a: the session binds the checkout fixture directory as its root, so the load argv names the
        checkout workspace instead of the scratch copy."""
        config = self.config("combined")
        result = quota_driver.run_grouping(config)
        steps = self.need(result, "steps")
        loads = [step for step in steps if step.get("verb") == "load"]
        self.assertTrue(loads, "the scenario loads a workspace")
        fixture = str(FIXTURE)
        scratch = os.path.realpath(config["scratch"])
        for step in steps:
            for token in step["argv"]:
                with self.subTest(step=step["index"]):
                    self.assertFalse(token.startswith(fixture), "an argv token names the checkout fixture")
        self.assertTrue(any(token.startswith(scratch) for token in loads[0]["argv"]),
                        "the load names the scratch workspace copy")

    def test_root_view_places_the_contract_even_when_its_source_lacks_it(self):
        """Kills Q12-b: quota_driver._root omits the text/quota-api.md placement. The real roots only inherit the
        contract from the base copy, so a root built from a source without the contract exposes the omission."""
        session = quota_driver.Session(self.config("split"))
        session.setup()
        without = session._copy_without("probe_source", session.base)
        root = session._root("probe_root", without, ["src/cli", "tests/cli"])
        self.assertTrue((root / quota_driver.CONTRACT).is_file(), "every root view carries the contract")
        self.assertEqual((root / quota_driver.CONTRACT).read_bytes(),
                         (session.workspace / quota_driver.CONTRACT).read_bytes())

    def test_counter_only_replay_tamper_leaves_an_aborted_record_naming_the_replay(self):
        """Kills Q14-b (negative fixture): with the counter comparison removed, a replay that only appends a counter
        line completes. The run must abort and write ABORTED.json that names the replay."""
        config = self.config("combined", tamper={"counter": True})
        with self.assertRaises(lib.AcceptError):
            quota_driver.run_grouping(config)
        aborted = json.loads((Path(config["raw_root"]) / "ABORTED.json").read_text(encoding="utf-8"))
        self.assertIn("executed a check", aborted["error"])

    def test_unusable_guidance_is_refused_by_the_usability_predicate(self):
        """Kills Q15: run_grouping refuses unusable guidance only through require_legal_action. The predicate refusal
        message must be raised for both the empty and the generic guidance text."""
        for mode in ("empty", "generic"):
            with self.subTest(mode=mode):
                with self.assertRaises(lib.AcceptError) as caught:
                    quota_driver.run_grouping(self.config("combined", tamper={"guidance": mode}))
                self.assertIn("not usable for next verb", str(caught.exception))

    def test_regroup_fenced_probe_presents_the_retired_builder_token(self):
        """Kills L6: the regroup probe presents the reviewer's pre-regroup token instead of the retired builder token.
        The presented --run token must be a builder claim's token and no reviewer claim's token."""
        result = quota_driver.run_grouping(self.config("combined", regroup=True))
        steps = self.need(result, "steps")

        def claimed_token(step):
            return json.loads(step["stdout"])["run"]["token"]

        def role_of(step):
            return step["argv"][step["argv"].index("--role") + 1]

        probes = [s for s in steps if s.get("request_id") == "fenced-probe"]
        self.assertEqual(len(probes), 1, "the regroup issues exactly one fenced probe")
        argv = probes[0]["argv"]
        presented = argv[argv.index("--run") + 1]
        claims = [s for s in steps if s.get("verb") == "claim" and "--role" in s["argv"] and s.get("exit") == 0]
        builder = {claimed_token(s) for s in claims if role_of(s) == "builder"}
        reviewer = {claimed_token(s) for s in claims if role_of(s) == "reviewer"}
        self.assertTrue(builder, "the combined run claims a builder")
        self.assertIn(presented, builder, "the probe presents the builder token")
        self.assertNotIn(presented, reviewer, "the probe must not present a reviewer token")


if __name__ == "__main__":
    unittest.main()
