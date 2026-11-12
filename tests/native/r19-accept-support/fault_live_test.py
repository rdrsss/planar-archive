#!/usr/bin/env python3
# R19-ACCEPT-A live fault tests (FaultLiveTest): rows F1-F25 of plan/INVARIANTS.md.
# They drive the D28 candidate named by PLANAR_CANDIDATE through fault_driver and
# fault/cmake_rerun/run_heavy with a fresh ledger per scenario under a per-test
# temporary root. They FAIL (never skip) when PLANAR_CANDIDATE is unset. Each
# docstring names the mutant the row must kill.
import sys

sys.dont_write_bytecode = True

import hashlib
import json
import os
from pathlib import Path
import shutil
import signal
import re
import subprocess
import tempfile
import unittest
from unittest import mock

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
sys.path.insert(0, str(HERE / "fault" / "cmake_rerun"))

import accept_lib as lib  # noqa: E402
import fault_driver  # noqa: E402
import run_heavy  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
REAL_CMAKE_SUPPORT = REPO / "tests/native/r19-cmake-support"
PY = sys.executable
CMAKE_CHECK = HERE / "cmake_check.py"
FAULT_LABELS = ("interrupt", "recover", "takeover", "reject", "capped", "load", "approve")
FAKE_CLI = (
    "import subprocess, sys\n"
    "child = subprocess.Popen(sys.argv[1:], start_new_session=True)\n"
    "child.wait()\n"
)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def tree_digest(root):
    """Map every file under root (including __pycache__) to its sha256."""
    root = Path(root)
    return {p.relative_to(root).as_posix(): sha256(p.read_bytes())
            for p in sorted(root.rglob("*")) if p.is_file()}


class FaultLiveTest(unittest.TestCase):
    def setUp(self):
        self.candidate = os.environ.get("PLANAR_CANDIDATE", "")
        if not self.candidate:
            self.fail("PLANAR_CANDIDATE must name the D28 candidate binary; live rows fail, never skip")
        self.root = Path(tempfile.mkdtemp(prefix="r19-fault-live-"))
        self.addCleanup(shutil.rmtree, str(self.root), True)
        self.serial = 0

    def scenario(self, mode="mini", base="plain", tamper=None, check_mode="sleep", **extra):
        """Fresh scratch HOME, ledger, counter and raw root for one fault scenario."""
        self.serial += 1
        scratch = self.root / ("run-%02d" % self.serial)
        config = {
            "candidate": self.candidate, "repo": str(REPO), "mode": mode, "check_mode": check_mode,
            "base": base, "scratch": str(scratch), "raw_root": str(scratch / "raw"),
            "home": str(scratch / "home"), "ledger": str(scratch / "ledger.db"),
            "counter": str(scratch / "check-counter.log"), "tamper": dict(tamper or {}),
        }
        config.update(extra)
        return config

    def need(self, value, key):
        """Return value[key], failing with an assertion (never a KeyError) when it is absent."""
        self.assertIsInstance(value, dict, "result section missing for " + repr(key))
        self.assertIn(key, value, "result lacks " + repr(key))
        return value[key]

    def env(self):
        env = os.environ.copy()
        env["PYTHONDONTWRITEBYTECODE"] = "1"
        return env

    # F1
    def test_interrupted_check_is_recovered_by_check_recover(self):
        """Kills: kill_check kills only the check child group while the CLI survives, kills before the rendezvous exists, or leaves the child group alive."""
        result = fault_driver.run_scenario(self.scenario())
        faults = self.need(result, "faults")
        interrupt = self.need(faults, "interrupt")
        self.assertTrue(interrupt.get("rendezvous_seen"))
        self.assertTrue(interrupt.get("child_dead") is True, "the check child group must be confirmed dead")
        self.assertEqual(interrupt.get("cli_returncode"), -signal.SIGKILL)
        self.assertEqual(interrupt.get("reservation_outcome"), "in_progress",
                         "the reservation stays open with request outcome in_progress after the kill")
        recover = self.need(faults, "recover")
        self.assertEqual(recover.get("exit"), 0)
        self.assertEqual(recover.get("outcome"), "interrupted")
        argv = self.need(recover, "argv")
        self.assertIn("--confirm-stopped", argv)
        self.assertEqual(argv[argv.index("--confirm-stopped") + 1], "true")

    # F2
    def test_state_survives_process_restart_between_phases(self):
        """Kills: fault_driver reuses a cached pre-kill status instead of re-reading after the kill."""
        result = fault_driver.run_scenario(self.scenario())
        reads = self.need(result, "reads")
        before = self.need(reads, "before_kill")
        after = self.need(reads, "after_restart")
        pids = []
        for verb in ("status", "history", "orchestrator"):
            with self.subTest(verb=verb):
                pre = self.need(before, verb)
                post = self.need(after, verb)
                self.assertEqual(pre.get("exit"), 0)
                self.assertEqual(post.get("exit"), 0)
                self.assertTrue(post.get("executed") is True, "the post-restart read must run")
                self.assertNotEqual(pre["pid"], post["pid"], "each read after restart is a fresh process")
                pids.extend([pre["pid"], post["pid"]])
        self.assertEqual(len(set(pids)), len(pids))
        self.assertEqual(self.need(before["status"], "revision"), self.need(after["status"], "revision"))
        self.assertEqual(self.need(before["status"], "activation"), self.need(after["status"], "activation"))

    # F3
    def test_takeover_fences_old_run_token(self):
        """Kills: fault_driver presents the NEW token in the fenced probe."""
        result = fault_driver.run_scenario(self.scenario())
        fenced = self.need(result, "fenced")
        self.assertEqual(fenced.get("token_role"), "old")
        self.assertNotEqual(fenced.get("exit"), 0)
        self.assertTrue(fenced.get("error_code"), "the refusal must be typed")
        self.assertEqual(fenced.get("ledger_before"), fenced.get("ledger_after"))
        takeover = self.need(result, "takeover")
        self.assertEqual(takeover.get("exit"), 0)
        self.assertGreater(self.need(takeover, "generation_after"), self.need(takeover, "generation_before"))

    # F4
    def test_rejecting_verdict_admits_repair_and_worker_returns(self):
        """Kills: fault_driver rejects with an empty repair list, which opens unowned_rejection instead."""
        result = fault_driver.run_scenario(self.scenario())
        reject1 = self.need(self.need(result, "verdicts"), "reject1")
        self.assertEqual(reject1.get("decision"), "reject")
        self.assertEqual(reject1.get("exit"), 0)
        self.assertTrue(reject1.get("repair_admitted") is True)
        self.assertEqual(reject1.get("count_before"), 0)
        kinds = [b.get("kind") for b in self.need(result, "blockers_after_reject")]
        self.assertNotIn("unowned_rejection", kinds)
        packet = self.need(self.need(result, "packets"), "builder_after_reject")
        self.assertTrue(self.need(packet, "findings"), "the worker packet carries the findings")
        self.assertIn(self.need(result, "worker_state_after_reject"), ("ready", "stale"))

    # F5
    def test_second_rejection_is_capped_and_opens_max_rounds_blocker(self):
        """Kills: fault_driver writes the first-revision job file with max_rounds 2."""
        result = fault_driver.run_scenario(self.scenario())
        reject2 = self.need(self.need(result, "verdicts"), "reject2")
        self.assertEqual(reject2.get("decision"), "reject")
        self.assertTrue(reject2.get("capped") is True)
        self.assertFalse(reject2.get("repair_admitted"))
        self.assertEqual(reject2.get("max_rounds"), 1)
        kinds = [b.get("kind") for b in self.need(result, "status_review_blockers")]
        self.assertIn("max_rounds", kinds)
        # D10 also scripts a repair after the load, so the run total is 2; the cap rule is that no
        # repair was written after the capped rejection, which the count taken at the cap records.
        self.assertEqual(self.need(result, "repairs_written_at_cap"), 1, "no repair is written after the cap")

    # F6
    def test_load_raising_max_rounds_clears_blocker(self):
        """Kills: fault_driver loads the same job bytes, leaving max_rounds unchanged."""
        result = fault_driver.run_scenario(self.scenario())
        load = self.need(result, "load")
        self.assertEqual(load.get("exit"), 0)
        self.assertGreater(self.need(load, "max_rounds_after"), self.need(load, "max_rounds_before"))
        self.assertIn("max_rounds", self.need(load, "blockers_cleared"))
        self.assertTrue(load.get("work_fp_rebound") is True, "work fingerprints are re-bound by the load")

    # F7
    def test_final_approval_after_faults_is_exact_target(self):
        """Kills: fault_driver builds the final verdict from the first check output."""
        result = fault_driver.run_scenario(self.scenario())
        final = self.need(self.need(result, "verdicts"), "final")
        self.assertEqual(final.get("decision"), "approve")
        self.assertEqual(final.get("exit"), 0)
        self.assertEqual(final.get("acceptance"), "accepted")
        self.assertEqual(final.get("blockers_open"), [])
        self.assertTrue(final.get("targets_equal_latest_output") is True)
        outputs = self.need(result, "check_outputs")
        self.assertGreaterEqual(len(set(outputs)), 3, "repairs must change the output between rechecks")
        stale = self.need(result, "stale_approval")
        # Test expectation corrected to the contract: mismatched targets are not refusals. The verdict is
        # recorded historical with exit 0, approves nothing, and appends one verdict event (R18-3,
        # execution-contract-r18.md:144; execution-design.md A06). The final approval above stays exact.
        self.assertEqual(stale.get("exit"), 0)
        self.assertEqual(stale.get("applicability"), "historical")
        self.assertIs(stale.get("admitted"), False)
        self.assertFalse(stale.get("ledger_unchanged"), "the historical verdict is one recorded event")

    # F8
    def test_replays_and_inspections_do_not_reexecute_checks_or_write(self):
        """Kills: fault_driver replays a check with a fresh request id."""
        result = fault_driver.run_scenario(self.scenario())
        # Four check rounds run one counted execution each: check 1 (killed) and checks 2 to 4.
        self.assertEqual(self.need(result, "intended_executions"), 4)
        self.assertEqual(self.need(result, "counter_lines"), 4)
        self.assertEqual(self.need(result, "ledger_before_replays"), self.need(result, "ledger_after_replays"))

    # F9
    def test_history_references_through_faults(self):
        """Kills: fault_driver restricts event inspection to the first and last event."""
        result = fault_driver.run_scenario(self.scenario())
        history = self.need(result, "history")
        labels = {h.get("label") for h in history}
        for label in FAULT_LABELS:
            with self.subTest(label=label):
                self.assertIn(label, labels)
        for record in history:
            with self.subTest(event=record.get("label")):
                self.assertTrue(record.get("matched") is True, "references must match those recorded at the time")
        self.assertEqual(self.need(result, "lua_executions_during_history"), 0)

    # F10
    def test_failed_and_interrupted_attempts_remain_retained(self):
        """Kills: fault_driver rmtrees the interrupted attempt folder after recovery."""
        result = fault_driver.run_scenario(self.scenario())
        attempts = self.need(result, "attempts")
        interrupted = [a for a in attempts if a.get("outcome") == "interrupted"]
        self.assertTrue(interrupted, "the interrupted attempt must be recorded")
        for attempt in attempts:
            with self.subTest(path=attempt.get("path")):
                self.assertTrue(Path(attempt["path"]).is_dir(), "attempt folders are never deleted")
        for attempt in interrupted:
            with self.subTest(interrupted=attempt["path"]):
                self.assertTrue(any(Path(attempt["path"]).iterdir()), "interrupted attempt keeps its raw logs")

    # F11
    def test_scenario_driver_is_config_parameterized(self):
        """Kills: fault_driver.run_scenario forks a separate cmake-only path with a different verb order or an extra action."""
        # After the load the builder is re-claimed (the load made it stale), and the stale-output verdict is
        # recorded historical with exit 0, a state change, so it is traced (R18-3; fault_driver trace policy).
        expected_trace = ["activate", "claim", "check", "check-recover", "takeover", "check", "verdict",
                          "claim", "check", "verdict", "load", "claim", "check", "verdict", "verdict"]
        mini = fault_driver.run_scenario(self.scenario(mode="mini"))
        self.assertEqual(self.need(mini, "trace"), expected_trace)
        heavy_config = run_heavy.build_config(self.root / "heavy-scratch", self.candidate)
        self.assertTrue(heavy_config, "build_config must return the cmake-rerun configuration")
        heavy_config["check_mode"] = "sleep"
        heavy = fault_driver.run_scenario(heavy_config)
        self.assertEqual(self.need(heavy, "trace"), self.need(mini, "trace"))

    # F12
    def test_orchestrator_guidance_usable_through_fault_phases(self):
        """Kills: fault_driver captures the orchestrator once or stores a canned string, or passes next_verb=None."""
        result = fault_driver.run_scenario(self.scenario())
        guidance = {g.get("point"): g for g in self.need(result, "guidance")}
        expected = {"open_reservation": "check-recover", "after_takeover": "check",
                    "max_rounds_blocker": "load", "before_final_approval": "verdict"}
        steps = self.need(result, "steps")
        for point, verb in expected.items():
            with self.subTest(point=point):
                self.assertIn(point, guidance)
                record = guidance[point]
                self.assertEqual(record.get("next_verb"), verb)
                if verb in lib.UNGUIDED_VERBS:
                    # load is exempt from the legal-action rule (accept_lib.UNGUIDED_VERBS, D37). The role lines
                    # read "next: none" while the only resolving action is a load, so the capture must instead name
                    # the live job and the open max_rounds blocker with its clearing condition (R18-8).
                    self.assertIn(record["live_state"]["job"], record["text"])
                    self.assertRegex(record["text"], r"(?m)^- blocker \d+: max_rounds \(verdict \d+\); "
                                                     r"clears when a later revision raises max_rounds above \d+$")
                else:
                    self.assertTrue(lib.guidance_is_usable(record["text"], record["live_state"], verb))
                self.assertEqual(steps[record["step_index"] + 1]["verb"], verb)
        states = {json.dumps(g.get("live_state"), sort_keys=True) for g in guidance.values()}
        self.assertGreaterEqual(len(states), 3, "guidance is captured live at each point")

    # F13
    def test_first_completed_check_passes_boundary_rows(self):
        """Kills: the cmake configuration uses the plain 58128234:src/engine snapshot as its base."""
        composed = fault_driver.run_scenario(self.scenario(mode="cmake", base="composed"))
        first = self.need(composed, "first_check")
        self.assertEqual(first.get("outcome"), "completed")
        self.assertEqual(self.need(first, "withheld"), [])
        rows = self.need(first, "boundary_rows")
        self.assertTrue(rows)
        for row in rows:
            self.assertEqual(row.get("status"), "pass")
        plain = fault_driver.run_scenario(self.scenario(mode="cmake", base="plain"))
        first_plain = self.need(plain, "first_check")
        # Check outcomes are completed, recorded or interrupted (execution-contract-r17.md:408). A boundary failure
        # records the evidence and withholds completion: outcome "recorded" with withheld ["boundary"].
        self.assertEqual(first_plain.get("outcome"), "recorded")
        self.assertTrue(any(row.get("status") == "fail" for row in self.need(first_plain, "boundary_rows")))
        # Engine withheld vocabulary (src/stdlib/planar/checks.lua): a failed boundary row reads "boundary".
        self.assertIn("boundary", self.need(first_plain, "withheld"))

    # F14
    def test_run_scenario_aborts_when_trajectory_omits_rejection(self):
        """Kills: fault_driver.run_scenario removes the accept_lib.validate_trajectory call."""
        # Composed base (the R19-CMAKE configuration, run_heavy.build_config): with the plain base the first
        # completed check is withheld, the run returns before this call site, and no abort can be observed.
        with self.assertRaises(lib.AcceptError):
            fault_driver.run_scenario(self.scenario(mode="cmake", base="composed", tamper={"omit_reject": True}))

    # F15
    def test_run_scenario_aborts_when_inspection_changes_ledger(self):
        """Kills: fault_driver removes the before/after ledger_digest comparison inside inspect()."""
        # Composed base: see test_run_scenario_aborts_when_trajectory_omits_rejection.
        with self.assertRaises(lib.AcceptError):
            fault_driver.run_scenario(self.scenario(mode="cmake", base="composed", tamper={"inspection": True}))

    # F16
    def test_run_scenario_aborts_on_tampered_replay(self):
        """Kills: fault_driver removes the accept_lib.assert_replay_identical call."""
        # Composed base: see test_run_scenario_aborts_when_trajectory_omits_rejection.
        with self.assertRaises(lib.AcceptError):
            fault_driver.run_scenario(self.scenario(mode="cmake", base="composed", tamper={"replay": True}))

    # F17
    def test_run_scenario_aborts_when_counter_increases_on_replay(self):
        """Kills: fault_driver removes the accept_lib.count_check_executions comparison after replays and inspections."""
        # Composed base: see test_run_scenario_aborts_when_trajectory_omits_rejection.
        with self.assertRaises(lib.AcceptError):
            fault_driver.run_scenario(self.scenario(mode="cmake", base="composed", tamper={"counter": True}))

    # F18
    def test_run_scenario_aborts_on_mismatched_event_expansion(self):
        """Kills: fault_driver removes the accept_lib.verify_event_expansion call."""
        # Composed base: see test_run_scenario_aborts_when_trajectory_omits_rejection.
        with self.assertRaises(lib.AcceptError):
            fault_driver.run_scenario(self.scenario(mode="cmake", base="composed", tamper={"expansion": True}))

    # F19
    def test_run_scenario_aborts_on_mismatched_at_event_state(self):
        """Kills: fault_driver removes the accept_lib.verify_at_event_state call."""
        # Composed base: see test_run_scenario_aborts_when_trajectory_omits_rejection.
        with self.assertRaises(lib.AcceptError):
            fault_driver.run_scenario(self.scenario(mode="cmake", base="composed", tamper={"at_event": True}))

    # F20
    def test_run_scenario_aborts_on_unusable_guidance(self):
        """Kills: fault_driver removes the accept_lib.guidance_is_usable call at the fault points."""
        with self.assertRaises(lib.AcceptError):
            fault_driver.run_scenario(self.scenario(mode="cmake", tamper={"guidance": "empty"}))

    # F21
    def test_run_heavy_config_is_composed_base_and_owned_repairs(self):
        """Kills: run_heavy.build_config uses base plain_58128234, targets a non-owned path, or builds only one preset."""
        config = run_heavy.build_config(self.root / "heavy-scratch", self.candidate)
        self.assertEqual(self.need(config, "mode"), "cmake")
        self.assertEqual(self.need(config, "check_mode"), "real")
        self.assertEqual(self.need(config, "base_label"), "baseline_head_cmake58128234")
        self.assertEqual(self.need(config, "max_rounds"), 1)
        self.assertEqual(self.need(config, "kill_target"), "cli")
        repairs = self.need(config, "repairs")
        self.assertEqual(len(repairs), 2, "one repair after reject 1 and one after the load")
        for repair in repairs:
            with self.subTest(path=repair.get("path")):
                self.assertIn(repair.get("path"), run_heavy.OWNED_CMAKE_PATHS)
                self.assertEqual(repair.get("action"), "append_comment")
                self.assertTrue(repair.get("scratch_only") is True)
        export = self.need(config, "export")
        registry = run_heavy.build_registry(config)
        checks = self.need(registry, "checks")
        self.assertEqual(sorted(c.get("id") for c in checks), ["cmake_debug", "cmake_release"])
        presets = set()
        for check in checks:
            argv = self.need(check, "argv")
            self.assertIn("cmake_check.py", " ".join(argv))
            self.assertIn("--repo", argv)
            self.assertEqual(argv[argv.index("--repo") + 1], export, "build dirs live under the scratch export")
            self.assertIn("--preset", argv)
            presets.add(argv[argv.index("--preset") + 1])
        self.assertEqual(presets, {"debug", "release"})

    # F22
    def test_kill_check_consumes_marker_written_by_cmake_check(self):
        """Kills: kill_check reads a different field or file name than cmake_check writes, or skips waiting for the marker."""
        repo = self.root / "f22-repo"
        (repo / "src").mkdir(parents=True)
        (repo / "src/a.cpp").write_text("int a;\n")
        evidence = self.root / "f22-evidence"
        evidence.mkdir()
        stubs = {
            "configure": [PY, "-c", "pass"],
            "build": [PY, "-c", "pass"],
            "inventory": [PY, "-c", "import json; print(json.dumps({'tests': [{'name': 'alpha'}]}))"],
            "ctest": [PY, "-c", "import time; time.sleep(120)"],
            "lint": [PY, "-c", "pass"],
            "repeat-build": [PY, "-c", "print('ninja: no work to do.')"],
        }
        stub_file = self.root / "f22-stubs.json"
        stub_file.write_text(json.dumps(stubs))
        fake = self.root / "fake_cli.py"
        fake.write_text(FAKE_CLI)
        real_before = tree_digest(REAL_CMAKE_SUPPORT)
        cli = subprocess.Popen([PY, str(fake), PY, str(CMAKE_CHECK), "--stub-commands", str(stub_file),
                                "--support-dir", str(REAL_CMAKE_SUPPORT), "--repo", str(repo),
                                "--evidence", str(evidence), "--preset", "debug"],
                               start_new_session=True, env=self.env(),
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        def reap_group():
            marker = evidence / lib.RENDEZVOUS_NAME
            if marker.is_file():
                try:
                    os.killpg(lib.read_rendezvous(marker)["pgid"], signal.SIGKILL)
                except (OSError, ValueError, TypeError, KeyError, lib.AcceptError):
                    pass  # cleanup only: the assertions below report the real failure
            if cli.poll() is None:
                cli.kill()
            cli.wait()
        self.addCleanup(reap_group)
        result = fault_driver.kill_check(cli, evidence, 60)
        self.assertEqual(self.need(result, "cli_pid"), cli.pid)
        self.assertEqual(self.need(result, "cli_returncode"), -signal.SIGKILL)
        self.assertTrue(result.get("child_dead") is True, "the marker pgid must be dead after the kill")
        self.assertTrue(result.get("marker_seen_before_kill") is True, "no kill happens before the marker exists")
        rendezvous = lib.read_rendezvous(evidence / lib.RENDEZVOUS_NAME)
        self.assertEqual(rendezvous["phase"], "ctest")
        self.assertEqual(self.need(result, "child_pgid"), rendezvous["pgid"])
        self.assertEqual(self.need(result, "rendezvous"), rendezvous)
        self.assertEqual(tree_digest(REAL_CMAKE_SUPPORT), real_before,
                         "the real r19-cmake-support tree, including __pycache__, is unchanged")

    # F23
    def test_fault_history_inspection_is_independent_of_scratch_files(self):
        """Kills: fault_driver passes --workspace or --registry, or skips the scratch rename before inspection."""
        result = fault_driver.run_scenario(self.scenario())
        detached = self.need(result, "detached")
        self.assertTrue(detached.get("renamed") is True)
        self.assertTrue(detached.get("restored") is True)
        self.assertTrue(detached.get("counter_unchanged") is True)
        before = self.need(detached, "before")
        after = self.need(detached, "after")
        self.assertIn("export", before)
        self.assertEqual(before.keys(), after.keys())
        for key in before:
            with self.subTest(inspection=key):
                self.assertEqual(before[key], after[key])
        for argv in self.need(detached, "argv"):
            for flag in ("--workspace", "--registry", "--root"):
                self.assertNotIn(flag, argv)

    # F24
    def test_run_heavy_aborts_on_unrecorded_candidate(self):
        """Kills: run_heavy.main removes the require_recorded_candidate call before the first live action."""
        scratch = self.root / "f24"
        scratch.mkdir()
        sentinel = scratch / "sentinel.txt"
        stub = scratch / "stub-candidate"
        stub.write_text("#!%s\nimport sys\nopen(%r, 'a').write('ran\\n')\n" % (PY, str(sentinel)))
        stub.chmod(0o755)
        record = scratch / "candidate.json"
        record.write_text(json.dumps({"head": "0" * 40, "sha256": "0" * 64, "size": 1}))
        with self.assertRaises(lib.AcceptError):
            run_heavy.main(["--scratch", str(scratch), "--candidate", str(stub),
                            "--record", str(record), "--repo", str(REPO)])
        self.assertFalse(sentinel.exists(), "the stub candidate must never run")
        self.assertEqual(list(scratch.rglob("*.argv.json")), [], "no action file may be written")

    # F25
    def test_run_scenario_aborts_before_action_not_listed_as_legal(self):
        """Kills: fault_driver.run_scenario removes the accept_lib.require_legal_action call before a live action."""
        for verb in ("check-recover", "takeover", "verdict", "load"):
            with self.subTest(verb=verb):
                config = self.scenario(mode="mini", tamper={"verb": verb})
                with self.assertRaises(lib.AcceptError):
                    fault_driver.run_scenario(config)
                aborted_path = Path(config["raw_root"]) / "ABORTED.json"
                self.assertTrue(aborted_path.is_file(), "the driver records the aborted point")
                aborted = json.loads(aborted_path.read_text())
                self.assertEqual(aborted.get("verb"), verb)
                self.assertEqual(aborted.get("ledger_digest"), lib.ledger_digest(Path(config["ledger"])))
                # The initial load is unguided (accept_lib.UNGUIDED_VERBS) and runs before the refused load-2, so
                # exactly one load argv file (request load-1) legitimately exists; a refused action has none.
                files = sorted(Path(config["raw_root"]).rglob("*-%s.argv.json" % verb))
                self.assertEqual(len(files), 1 if verb == "load" else 0,
                                 "no argv file may exist for the refused action")
                for path in files:
                    self.assertIn("load-1", path.read_text(encoding="utf-8"), "only the initial load may have an argv file")

    # Wait-timeout helpers (H5-H7): the delayed-marker fixture replaces the mini sleep check for one test.
    def capture_error(self, config):
        """Run one scenario and return the exception it raised, or None when it completed; the caller asserts on its type."""
        try:
            fault_driver.run_scenario(config)
        except Exception as error:  # noqa: BLE001 - every failure is asserted on by type in the caller
            return error
        return None

    def recording_popen(self, launched):
        """Return a Popen subclass that appends every process it starts to launched (test-only observer)."""
        class RecordingPopen(subprocess.Popen):
            def __init__(self, *args, **kwargs):
                super().__init__(*args, **kwargs)
                launched.append(self)
        return RecordingPopen

    def kill_leftover_clis(self, launched):
        """Kill every launched CLI that still runs, with its descendants and their groups (fault_driver.kill_session)."""
        for process in launched:
            if process.poll() is None:
                try:
                    fault_driver.kill_session(process)
                except lib.AcceptError:
                    pass  # cleanup only: the test assertions report the real failure

    def live_lines_naming(self, text):
        """Return the ps lines whose full command line contains text (test-only process observer)."""
        listing = subprocess.run(["ps", "-axww", "-o", "pid=,command="], capture_output=True, text=True,
                                 timeout=30, env=self.env())
        return [line for line in listing.stdout.splitlines() if text in line]

    # H5
    def test_run_heavy_config_wait_timeouts_cover_registry_budget(self):
        """Kills H5: run_heavy.build_config omits marker_timeout_s and check_timeout_s, or keeps the 120 s and 900 s constants."""
        config = run_heavy.build_config(self.root / "heavy-timeouts", self.candidate)
        registry = run_heavy.build_registry(config)
        budget_ms = sum(int(check["timeout_ms"]) for check in self.need(registry, "checks"))
        source = Path(fault_driver.__file__).read_text(encoding="utf-8")
        requests = len(re.findall(r"= self\.check_round\(", source))
        self.assertGreater(requests, 0, "the scenario must issue check requests")
        budget_s = budget_ms / 1000 * requests
        for key in ("marker_timeout_s", "check_timeout_s"):
            with self.subTest(key=key):
                value = self.need(config, key)
                self.assertIsInstance(value, (int, float))
                self.assertGreaterEqual(value, budget_s, "the wait must cover the registry budget of every request")
                for recorded_ms in (1458551, 819557):
                    self.assertLess(recorded_ms / 1000, value, "the recorded R19-CMAKE duration must fit the wait")

    # H6
    def test_marker_wait_honors_configured_timeout(self):
        """Kills H6: fault_driver.check_round hard-codes the 120 s marker wait and ignores marker_timeout_s."""
        launched = []
        self.addCleanup(self.kill_leftover_clis, launched)
        with mock.patch.object(fault_driver, "SLEEP_CHECK", HERE / "fault" / "delayed_marker_check.py"), \
                mock.patch.object(fault_driver.subprocess, "Popen", self.recording_popen(launched)):
            short = self.scenario(mode="mini", marker_timeout_s=1)
            error = self.capture_error(short)
            self.assertIsInstance(error, lib.AcceptError, "a marker that appears after the configured wait must abort")
            self.assertIn("no rendezvous marker appeared before the timeout", str(error))
            longer = self.scenario(mode="mini", marker_timeout_s=60)
            error = self.capture_error(longer)
            self.assertIsNone(error, "a marker within the configured wait must complete the kill step: %r" % (error,))
            self.assertFalse((Path(longer["raw_root"]) / "ABORTED.json").exists(),
                             "a completed run writes no ABORTED.json")

    # H7
    def test_check_completion_timeout_aborts_with_accept_error_and_kills_cli(self):
        """Kills H7: fault_driver lets subprocess.TimeoutExpired escape the completion wait (no AcceptError, no ABORTED.json), or converts it without killing the check CLI and its group."""
        launched = []
        self.addCleanup(self.kill_leftover_clis, launched)
        config = self.scenario(mode="mini", check_timeout_s=1)
        with mock.patch.object(fault_driver, "SLEEP_CHECK", HERE / "fault" / "delayed_marker_check.py"), \
                mock.patch.object(fault_driver.subprocess, "Popen", self.recording_popen(launched)):
            error = self.capture_error(config)
        self.assertIsInstance(error, lib.AcceptError,
                              "a completion wait that expires must raise AcceptError, not %r" % (error,))
        aborted_path = Path(config["raw_root"]) / "ABORTED.json"
        self.assertTrue(aborted_path.is_file(), "the abort must be recorded in ABORTED.json")
        self.assertEqual(json.loads(aborted_path.read_text()).get("verb"), "check")
        checks = [p for p in launched if len(p.args) > 1 and p.args[0] == self.candidate and p.args[1] == "check"]
        self.assertTrue(checks, "the scenario must have started a check CLI")
        cli = checks[-1]
        self.assertIsNotNone(cli.returncode, "the check CLI must have been reaped")
        self.assertEqual(cli.returncode, -signal.SIGKILL, "the check CLI must be SIGKILLed")
        with self.assertRaises(ProcessLookupError, msg="the check CLI group must be dead"):
            os.killpg(cli.pid, 0)
        with self.assertRaises(ProcessLookupError, msg="the check CLI must be dead"):
            os.kill(cli.pid, 0)
        self.assertEqual(self.live_lines_naming(config["scratch"]), [],
                         "no check child of the aborted scenario may survive (its own process group)")

    # Negative fixtures for the C02 survivors (PKG-FIX). New names only; no existing test is renamed.

    def test_trace_is_derived_from_executed_steps_in_plain_cmake_run(self):
        """Kills F11-b and L17: the trace is hard-coded instead of derived from execution. Covers both the stopped-run
        trace (the plain base run stops after its first withheld check, so its executed traced verbs are a shorter
        sequence) and the finish() trace (a mini run whose repairs append a sentinel verb to the instance trace, which
        a hard-coded list in finish cannot contain)."""
        result = fault_driver.run_scenario(self.scenario(mode="cmake", base="plain"))
        traced = {"activate", "claim", "check", "check-recover", "takeover", "verdict", "load"}
        steps = self.need(result, "steps")
        # The trace begins at the first activation: init and the setup load before it are not traced.
        first_activate = [step.get("verb") for step in steps].index("activate")
        derived = []
        for step in steps[first_activate:]:
            verb = step.get("verb")
            if verb not in traced:
                continue
            # An interrupted check (SIGKILL) is traced, a refused probe (nonzero exit) is not.
            if step.get("exit") == 0 or (verb == "check" and step.get("exit") == -signal.SIGKILL):
                derived.append(verb)
        self.assertTrue(derived, "the plain run must execute traced verbs")
        self.assertEqual(self.need(result, "trace"), derived)
        # finish() trace: wrap apply_repair so each call appends a sentinel to the instance trace after the real call.
        real_apply_repair = fault_driver._Scenario.apply_repair
        instances = []

        def sentinel_repair(scenario, *args, **kwargs):
            outcome = real_apply_repair(scenario, *args, **kwargs)
            scenario.trace.append("repair-sentinel")
            if scenario not in instances:
                instances.append(scenario)
            return outcome

        with mock.patch.object(fault_driver._Scenario, "apply_repair", autospec=True,
                               side_effect=sentinel_repair):
            mini = fault_driver.run_scenario(self.scenario(mode="mini"))
        self.assertIn("trajectory", mini, "the mini run must reach finish()")
        self.assertEqual(len(instances), 1, "exactly one scenario instance applies repairs")
        mini_trace = self.need(mini, "trace")
        self.assertEqual(mini_trace, instances[0].trace)
        self.assertIn("repair-sentinel", mini_trace)
        hard_coded = ["activate", "claim", "check", "check-recover", "takeover", "check", "verdict",
                      "claim", "check", "verdict", "load", "claim", "check", "verdict", "verdict"]
        self.assertNotEqual(mini_trace, hard_coded)

    def test_blank_guidance_is_refused_by_the_usability_predicate(self):
        """Kills F20: fault_driver.guided no longer calls guidance_is_usable. Blank guidance is then refused only by
        require_legal_action, with the legal-list message instead of the predicate refusal."""
        error = self.capture_error(self.scenario(mode="cmake", tamper={"guidance": "empty"}))
        self.assertIsInstance(error, lib.AcceptError)
        self.assertIn("orchestrator guidance is not usable for", str(error))

    def test_run_heavy_config_names_the_composed_base(self):
        """Kills F21-a: build_config sets base to plain while base_label still reads baseline_head_cmake58128234."""
        config = run_heavy.build_config(self.root / "heavy-base", self.candidate)
        self.assertEqual(self.need(config, "base"), "composed")

    def test_kill_check_signals_the_marker_pgid_when_it_differs_from_the_marker_pid(self):
        """Kills F22-a: kill_check signals the group named by the marker pid. The marker names the CLI as pid and a
        separate group leader as pgid, and only that pgid group may be killed."""
        evidence = self.root / "f22a-evidence"
        evidence.mkdir()
        sleeper = [PY, "-c", "import time; time.sleep(120)"]
        cli = subprocess.Popen(sleeper, start_new_session=True, env=self.env(),
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        # The child is started by a short-lived parent, so it is reparented after the parent exits and no zombie
        # of this process can keep its group looking alive (fault_driver._group_alive counts zombies as alive).
        spawner_code = ("import subprocess, sys\n"
                        "child = subprocess.Popen(sys.argv[1:], start_new_session=True,\n"
                        "                         stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)\n"
                        "print(child.pid, flush=True)\n")
        spawner = subprocess.Popen([PY, "-c", spawner_code, *sleeper], stdout=subprocess.PIPE, env=self.env())
        grandchild = int(spawner.stdout.readline())
        spawner.stdout.close()
        spawner.wait(timeout=30)

        def reap():
            try:
                os.killpg(grandchild, signal.SIGKILL)
            except ProcessLookupError:
                pass
            if cli.poll() is None:
                cli.kill()
            cli.wait()
        self.addCleanup(reap)
        lib.write_rendezvous(evidence, "ctest", cli.pid, grandchild)
        result = fault_driver.kill_check(cli, evidence, 60)
        self.assertEqual(self.need(result, "child_pgid"), grandchild, "the marker pgid group is the one killed")
        self.assertTrue(result.get("child_dead") is True)
        self.assertEqual(self.need(result, "cli_returncode"), -signal.SIGKILL)
        with self.assertRaises(ProcessLookupError, msg="the marker pgid group must be dead"):
            os.killpg(grandchild, 0)

    def test_every_check_attempt_folder_keeps_its_command_logs(self):
        """Kills L15: fault_driver._copy_attempt returns before copying, so attempt folders keep no argv, stdout,
        stderr or exit record. The interrupted folder then keeps only the rendezvous copy."""
        result = fault_driver.run_scenario(self.scenario())
        attempts = self.need(result, "attempts")
        self.assertTrue(attempts)
        for attempt in attempts:
            folder = Path(attempt["path"])
            for pattern in ("*.argv.json", "*.stdout", "*.stderr", "*.exit"):
                with self.subTest(attempt=folder.name, pattern=pattern):
                    self.assertTrue(list(folder.glob(pattern)), "an attempt folder keeps " + pattern)

if __name__ == "__main__":
    unittest.main()
