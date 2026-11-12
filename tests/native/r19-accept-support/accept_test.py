#!/usr/bin/env python3
# R19-ACCEPT-A pure-helper tests (AcceptLibTest): one test per invariant row I1-I35
# of plan/INVARIANTS.md. Each docstring names the mutant the test must kill.
# Synthetic fixtures live in per-test temporary roots; the real trees
# r19-cmake-support and fixtures/render/quota are only read or copied.
import sys

sys.dont_write_bytecode = True

import copy
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import accept_lib as lib  # noqa: E402
import check_runner  # noqa: E402
import cmake_check  # noqa: E402
import registry_gen  # noqa: E402
import run_acceptance  # noqa: E402

sys.path.insert(0, str(HERE / "fault/cmake_rerun"))
import fault_driver  # noqa: E402
import run_heavy  # noqa: E402

REPO = HERE.parents[2]
REAL_CMAKE_SUPPORT = REPO / "tests/native/r19-cmake-support"
FIXTURE_REGISTRY = REPO / "fixtures/render/quota/registry/quota-tools.json"
PY = sys.executable
RUNNER = HERE / "check_runner.py"
CMAKE_CHECK = HERE / "cmake_check.py"
OWNED_CMAKE = [
    "CMakeLists.txt", "acceptance/CMakeLists.txt", "budget/CMakeLists.txt",
    "canonical/CMakeLists.txt", "capture/CMakeLists.txt", "execution/CMakeLists.txt",
    "export/CMakeLists.txt", "hash/CMakeLists.txt", "history/CMakeLists.txt",
    "json_codec/CMakeLists.txt", "ledger/CMakeLists.txt", "lua/CMakeLists.txt",
    "process/CMakeLists.txt", "runtime/CMakeLists.txt",
    "runtime/closure_guard/CMakeLists.txt", "value/CMakeLists.txt",
]
GUIDANCE = (
    "# Planar orchestrator prompt for job combined\n\n"
    "# Orchestrator state\n\n"
    "Ledger ledger.db revision 3 event 12 activation 1.\n"
    "- builder: running; next: check, refresh, checkpoint, release, recover, takeover.\n"
    "- reviewer: blocked; next: wait for prerequisites, status.\n"
    "Always run check before treating a worker role as done.\n"
)
LIVE = {"job": "combined", "roles": {"builder": "running", "reviewer": "blocked"}}


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def tree_digest(root):
    """Map every file under root (including __pycache__) to its sha256."""
    root = Path(root)
    return {p.relative_to(root).as_posix(): sha256(p.read_bytes())
            for p in sorted(root.rglob("*")) if p.is_file()}


def target(output, unit="value"):
    return {"unit": unit, "output": output, "work_fp": "wf-" + output}


def trajectory():
    """Good D10 order: interrupted check, takeover, reject with repair, capped reject, load, approve."""
    return [
        {"kind": "check_reserved", "reservation": "r1", "request": "q1", "run": "run1"},
        {"kind": "check_interrupted", "reservation": "r1", "via": "check-recover"},
        {"kind": "run_taken_over", "run": "run2", "previous": "run1"},
        {"kind": "fenced_refused", "run": "run1"},
        {"kind": "check_reserved", "reservation": "r2", "request": "q2", "run": "run2"},
        {"kind": "check_recorded", "reservation": "r2", "output": "o2"},
        {"kind": "verdict_recorded", "decision": "reject", "count": 0, "max_rounds": 1,
         "repair": ["builder"], "repair_admitted": True, "targets": [target("o2")],
         "policy": "pol", "blockers_opened": [], "blockers_cleared": []},
        {"kind": "run_claimed", "role": "builder", "run": "run3"},
        {"kind": "check_reserved", "reservation": "r3", "request": "q3", "run": "run3"},
        {"kind": "check_recorded", "reservation": "r3", "output": "o3"},
        {"kind": "verdict_recorded", "decision": "reject", "count": 1, "max_rounds": 1,
         "repair": [], "repair_admitted": False, "targets": [target("o3")],
         "policy": "pol", "blockers_opened": ["max_rounds"], "blockers_cleared": []},
        {"kind": "declaration_accepted", "max_rounds": 2},
        {"kind": "blockers_cleared", "blockers": ["max_rounds"]},
        {"kind": "check_reserved", "reservation": "r4", "request": "q4", "run": "run3"},
        {"kind": "check_recorded", "reservation": "r4", "output": "o4"},
        {"kind": "verdict_recorded", "decision": "approve", "acceptance": "accepted",
         "targets": [target("o4")], "policy": "pol", "blockers_open": []},
    ]


def without(events, index):
    return events[:index] + events[index + 1:]


def replaced(events, index, **changes):
    copied = copy.deepcopy(events)
    copied[index].update(changes)
    return copied


class AcceptLibTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="r19-accept-lib-"))
        self.addCleanup(shutil.rmtree, str(self.tmp), True)

    def write(self, relative, data):
        path = self.tmp / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data if isinstance(data, bytes) else data.encode())
        return path

    def env(self, **extra):
        env = os.environ.copy()
        env["PYTHONDONTWRITEBYTECODE"] = "1"
        env.update(extra)
        return env

    def git(self, repo, *args):
        identity = {"GIT_AUTHOR_NAME": "t", "GIT_AUTHOR_EMAIL": "t@example.invalid",
                    "GIT_COMMITTER_NAME": "t", "GIT_COMMITTER_EMAIL": "t@example.invalid"}
        return subprocess.run(["git", "-C", str(repo), *args], check=True, capture_output=True,
                              env=self.env(**identity)).stdout.decode().strip()

    def support_copy(self):
        dest = self.tmp / "support"
        shutil.copytree(REAL_CMAKE_SUPPORT, dest,
                        ignore=shutil.ignore_patterns("__pycache__", "*.pyc"))
        # The adapter runs from the copied tree and imports accept_lib from its own directory.
        for name in ("cmake_check.py", "accept_lib.py"):
            shutil.copyfile(HERE / name, dest / name)
        return dest

    # I1
    def test_replay_requires_identical_bytes_and_unchanged_ledger(self):
        """Kills: assert_replay_identical compares only len() of responses, or skips the ledger digest compare."""
        lib.assert_replay_identical(b'{"ok":true,"value":1}', b'{"ok":true,"value":1}', "d0", "d0")
        with self.assertRaises(lib.AcceptError):
            lib.assert_replay_identical(b'{"ok":true,"value":1}', b'{"ok":true,"value":2}', "d0", "d0")
        with self.assertRaises(lib.AcceptError):
            lib.assert_replay_identical(b'{"a":1}', b'{"a":1,"b":2}', "d0", "d0")
        with self.assertRaises(lib.AcceptError):
            lib.assert_replay_identical(b"x", b"x", "d0", "d1")

    # I2
    def test_ledger_digest_detects_single_byte_write_and_sidecars(self):
        """Kills: ledger_digest hashes only the main db file, ignoring -wal/-shm siblings and tail bytes."""
        db = self.write("ledger.db", b"A" * 4096)
        d0 = lib.ledger_digest(db)
        self.assertEqual(d0, lib.ledger_digest(db), "digest must be stable for unchanged bytes")
        data = bytearray(db.read_bytes())
        data[-1] ^= 0x01
        db.write_bytes(bytes(data))
        d1 = lib.ledger_digest(db)
        self.assertNotEqual(d0, d1, "a single trailing byte write must change the digest")
        data[2048] ^= 0x01
        db.write_bytes(bytes(data))
        d2 = lib.ledger_digest(db)
        self.assertNotEqual(d1, d2, "a single interior byte write must change the digest")
        wal = Path(str(db) + "-wal")
        wal.write_bytes(b"W" * 512)
        d3 = lib.ledger_digest(db)
        self.assertNotEqual(d2, d3, "creating the -wal sibling must change the digest")
        wal.write_bytes(b"W" * 511 + b"X")
        d4 = lib.ledger_digest(db)
        self.assertNotEqual(d3, d4, "a single byte in the -wal sibling must change the digest")
        shm = Path(str(db) + "-shm")
        shm.write_bytes(b"S" * 32)
        d5 = lib.ledger_digest(db)
        self.assertNotEqual(d4, d5, "creating the -shm sibling must change the digest")
        shm.write_bytes(b"S" * 31 + b"T")
        d6 = lib.ledger_digest(db)
        self.assertNotEqual(d5, d6, "a single byte in the -shm sibling must change the digest")

    # I3
    def test_host_path_scan_finds_repo_home_scratch_in_nested_files(self):
        """Kills: scan_host_paths skips subdirectories, or checks only the /Users prefix."""
        repo_prefix = "/opt/fake-planar-repo"
        home_prefix = "/opt/fake-home"
        scratch_prefix = "/opt/fake-scratch"
        self.write("tree/a/b/c/nested.md", "see " + repo_prefix + "/docs\n")
        self.write("tree/json/doc.json", json.dumps({"where": home_prefix + "/x"}))
        self.write("tree/scratch/notes/x.txt", scratch_prefix + "/run\n")
        self.write("tree/users/y.txt", "owner /Users/fixture-owner/private\n")
        self.write("tree/clean/ok.txt", "nothing to see\n")
        hits = lib.scan_host_paths(self.tmp / "tree", [repo_prefix, home_prefix, scratch_prefix, "/Users/"])
        expected = sorted([
            ("a/b/c/nested.md", repo_prefix),
            ("json/doc.json", home_prefix),
            ("scratch/notes/x.txt", scratch_prefix),
            ("users/y.txt", "/Users/"),
        ])
        self.assertEqual(sorted(tuple(h) for h in hits), expected)

    # I4
    def test_redaction_round_trips_with_recorded_hash(self):
        """Kills: redact_text omits the scratch-root substitution, substitutes without original_sha256, or writes a real prefix into the record."""
        temp = "/private/var/folders/r19-temp-fixture"
        scratch = temp + "/scratch"
        prefixes = {"<temp-root>": temp, "<scratch-root>": scratch}
        original = "log " + scratch + "/out.txt and " + temp + "/x and " + scratch + "/again\n"
        redacted, entry = lib.redact_text(original, prefixes)
        self.assertNotIn(scratch, redacted, "longest prefix must be replaced by its token")
        self.assertNotIn(temp, redacted)
        self.assertIn("<scratch-root>/out.txt", redacted)
        self.assertIn("<temp-root>/x", redacted)
        self.assertIn("original_sha256", entry)
        self.assertEqual(entry["original_sha256"], sha256(original.encode()))
        self.assertEqual(entry["redacted_sha256_before_standard"], sha256(redacted.encode()))
        self.assertEqual(entry["bytes_before"], len(original.encode()))
        self.assertEqual(entry["bytes_after"], len(redacted.encode()))
        self.assertEqual(lib.restore_text(redacted, prefixes), original)
        self.assertEqual(sha256(lib.restore_text(redacted, prefixes).encode()), entry["original_sha256"])
        recorded = json.dumps(entry, sort_keys=True)
        self.assertNotIn(scratch, recorded, "the record must store token descriptions only")
        self.assertNotIn(temp, recorded)

    # I5
    def test_pin_scope_admits_only_src_and_data(self):
        """Kills: pin_scope_allowed also admits docs/** or round records, or drops the quota and CMake lua admissions."""
        admitted = [
            "src/core/quota.cpp",
            "fixtures/render/quota/workspace/text/quota-api.md",
            "fixtures/render/quota/base/src/core/quota.py",
            "tests/native/r19-cmake-support/workspace/spec.lua",
            "tests/native/r19-cmake-support/workspace/combined.lua",
        ]
        refused = [
            "docs/vernacular/roadmap.md",
            "docs/vernacular/rounds/R19-ACCEPT-A.md",
            "docs/vernacular/rounds/R19-ACCEPT-A-review.md",
            "docs/vernacular/rounds/R19-ACCEPT-A-REPORT.md",
            "tests/native/r19-accept-evidence/REPORT.md",
            "tests/native/r19-accept-evidence/README.md",
            "tests/native/r15-r18-final-evidence/raw/stdout.log",
        ]
        for path in admitted:
            with self.subTest(admitted=path):
                self.assertTrue(lib.pin_scope_allowed(path))
        for path in refused:
            with self.subTest(refused=path):
                self.assertFalse(lib.pin_scope_allowed(path))

    # I6
    def test_event_expansion_binds_declaration_and_dispatch_references(self):
        """Kills: verify_event_expansion skips the dispatch/manifest id comparison or the no-token check."""
        token = "tok-SECRET-0123456789"
        activated = {"activation": {"id": "act-1"}, "revision": {"id": "rev-7"}, "job": "combined"}
        lib.verify_event_expansion("job_activated", activated,
                                   {"activation.id": "act-1", "revision.id": "rev-7"}, token=token)
        claimed = {"run": {"id": "run-3", "role": "builder", "generation": 1},
                   "dispatch": {"id": "disp-9", "kind": "work"},
                   "packet": {"id": "pkt-4"}, "manifest": {"id": "man-5"}}
        expected = {"run.id": "run-3", "dispatch.id": "disp-9", "packet.id": "pkt-4", "manifest.id": "man-5"}
        lib.verify_event_expansion("run_claimed", claimed, expected, token=token)
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("run_claimed", claimed, {**expected, "dispatch.id": "disp-OTHER"}, token=token)
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("run_claimed", claimed, {**expected, "manifest.id": "man-OTHER"}, token=token)
        leaking = copy.deepcopy(claimed)
        leaking["run"]["token"] = token
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("run_claimed", leaking, expected, token=token)

    # I7
    def test_event_expansion_binds_check_state_references(self):
        """Kills: verify_event_expansion ignores the reservation id and the interrupted outcome for check events."""
        reserved = {"reservation": {"id": "res-1", "request_id": "req-1", "run": "run-1"}}
        lib.verify_event_expansion("check_reserved", reserved,
                                   {"reservation.id": "res-1", "reservation.request_id": "req-1"})
        recorded = {"reservation": {"id": "res-1", "request_id": "req-1", "run": "run-1"},
                    "observation": {"id": "obs-1", "value": "o1"}}
        lib.verify_event_expansion("check_recorded", recorded,
                                   {"reservation.id": "res-1", "observation.value": "o1"})
        interrupted = {"reservation": {"id": "res-2", "request_id": "req-2", "run": "run-1"},
                       "request": {"outcome": "interrupted"}}
        expected = {"reservation.id": "res-2", "request.outcome": "interrupted"}
        lib.verify_event_expansion("check_interrupted", interrupted, expected)
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("check_interrupted", interrupted, {**expected, "reservation.id": "res-9"})
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("check_interrupted", interrupted, {**expected, "request.outcome": "completed"})
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("check_recorded", recorded, {"reservation.id": "res-1", "observation.value": "o9"})

    # I8
    def test_event_expansion_binds_verdict_targets_policy_blockers(self):
        """Kills: verify_event_expansion ignores targets, policy or blockers for verdict events."""
        targets = [target("o4")]
        verdict = {"targets": targets, "policy": "pol-A", "decision": "reject",
                   "repair": {"admitted": True, "count": 1},
                   "blockers": {"opened": ["max_rounds"], "cleared": []}}
        expected = {"targets": targets, "policy": "pol-A", "decision": "reject",
                    "repair.admitted": True, "blockers.opened": ["max_rounds"], "blockers.cleared": []}
        lib.verify_event_expansion("verdict_recorded", verdict, expected)
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("verdict_recorded", verdict, {**expected, "targets": [target("o3")]})
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("verdict_recorded", verdict, {**expected, "policy": "pol-B"})
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("verdict_recorded", verdict, {**expected, "blockers.opened": []})

    # I9
    def test_at_event_state_checks_roles_and_runs(self):
        """Kills: verify_at_event_state compares only active_job."""
        recorded = {"active_job": "combined", "activation": "act-1",
                    "roles": {"builder": {"state": "running", "drifted": False, "run": "run-1"},
                              "reviewer": {"state": "blocked", "drifted": False, "run": None}}}
        lib.verify_at_event_state(recorded, copy.deepcopy(recorded))
        mutations = {
            "builder state": lambda r: r["roles"]["builder"].update(state="stale"),
            "builder drifted": lambda r: r["roles"]["builder"].update(drifted=True),
            "builder run": lambda r: r["roles"]["builder"].update(run="run-2"),
            "reviewer missing": lambda r: r["roles"].pop("reviewer"),
            "active_job": lambda r: r.update(active_job="other"),
            "activation": lambda r: r.update(activation="act-2"),
        }
        for label, mutate in mutations.items():
            with self.subTest(mutation=label):
                replayed = copy.deepcopy(recorded)
                mutate(replayed)
                with self.assertRaises(lib.AcceptError):
                    lib.verify_at_event_state(recorded, replayed)

    # I10
    def test_trajectory_requires_interrupted_check_recovery(self):
        """Kills: validate_trajectory drops the check_interrupted requirement."""
        events = trajectory()
        lib.validate_trajectory(events)
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(without(events, 1))
        completed_too = events[:2] + [{"kind": "check_recorded", "reservation": "r1", "output": "ox"}] + events[2:]
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(completed_too)

    # I11
    def test_trajectory_requires_takeover_and_fencing(self):
        """Kills: validate_trajectory drops the run_taken_over or fenced refusal requirement."""
        events = trajectory()
        lib.validate_trajectory(events)
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(without(events, 2))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(without(events, 3))

    # I12
    def test_trajectory_requires_rejection_then_repair(self):
        """Kills: validate_trajectory accepts a first approve instead of reject, or skips repair_admitted or the worker return."""
        first_approve = [
            {"kind": "check_reserved", "reservation": "r9", "request": "q9", "run": "run1"},
            {"kind": "check_recorded", "reservation": "r9", "output": "o9"},
            {"kind": "verdict_recorded", "decision": "approve", "acceptance": "accepted",
             "targets": [target("o9")], "policy": "pol", "blockers_open": []},
        ]
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(first_approve)
        events = trajectory()
        lib.validate_trajectory(events)
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 6, repair_admitted=False, repair=[]))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(without(events, 7))

    # I13
    def test_trajectory_requires_capped_rejection_and_max_rounds_blocker(self):
        """Kills: validate_trajectory treats the capped rejection as admitted, or ignores blockers_opened."""
        events = trajectory()
        lib.validate_trajectory(events)
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 10, repair_admitted=True, repair=["builder"]))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 10, blockers_opened=[]))

    # I14
    def test_trajectory_requires_blocker_cleared_by_load(self):
        """Kills: validate_trajectory accepts a missing blockers_cleared or a clear without a declaration_accepted raising max_rounds."""
        events = trajectory()
        lib.validate_trajectory(events)
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(without(events, 12))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(without(events, 11))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 11, max_rounds=1))

    # I15
    def test_trajectory_requires_exact_target_final_approval(self):
        """Kills: validate_trajectory accepts an approval of an older output, one with open blockers, or a later check."""
        events = trajectory()
        lib.validate_trajectory(events)
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 15, targets=[target("o3")]))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 15, blockers_open=["max_rounds"]))
        later = events + [{"kind": "check_reserved", "reservation": "r5", "request": "q5", "run": "run3"}]
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(later)

    # I16
    def test_reexecution_counter_counts_all_attempt_folders(self):
        """Kills: count_check_executions counts only the debug preset, only the newest attempt, or ignores the counter file."""
        evidence = self.tmp / "evidence"
        for name in ["debug/111-1", "debug/222-1", "release/333-2"]:
            (evidence / "checks" / name).mkdir(parents=True)
        counter = self.write("check-counter.log", '{"argv":"a"}\n{"argv":"b"}\n')
        self.assertEqual(lib.count_check_executions(evidence, counter), 5)
        empty = self.tmp / "empty"
        empty.mkdir()
        self.assertEqual(lib.count_check_executions(empty, counter), 2)

    # I17
    def test_cmake_adapter_inventory_rejects_skips_missing_and_duplicates(self):
        """Kills: cmake_check inventory_verdict tolerates skipped or notrun cases, or missing names."""
        inventory = ["alpha", "beta"]
        good = [{"name": "alpha", "status": "passed"}, {"name": "beta", "status": "passed"}]
        self.assertTrue(cmake_check.inventory_verdict(inventory, good))
        bad = {
            "skipped": [{"name": "alpha", "status": "passed"}, {"name": "beta", "status": "skipped"}],
            "notrun": [{"name": "alpha", "status": "passed"}, {"name": "beta", "status": "notrun"}],
            "failed": [{"name": "alpha", "status": "passed"}, {"name": "beta", "status": "failed"}],
            "missing name": [{"name": "alpha", "status": "passed"}],
            "extra name": good + [{"name": "gamma", "status": "passed"}],
            "duplicate": [{"name": "alpha", "status": "passed"}, {"name": "alpha", "status": "passed"}],
            "empty": [],
        }
        for label, cases in bad.items():
            with self.subTest(case=label):
                self.assertFalse(cmake_check.inventory_verdict(inventory, cases))
        self.assertFalse(cmake_check.inventory_verdict([], []))

    # I18
    def test_cmake_adapter_command_plan_drops_equivalence_keeps_gates(self):
        """Kills: cmake_check command_plan drops lint, repeat-build or the rendezvous step, or re-adds the verify.py equivalence step."""
        expected = ["configure", "build", "inventory", "rendezvous", "ctest", "lint", "repeat-build"]
        for preset in ("debug", "release"):
            with self.subTest(preset=preset):
                self.assertEqual(cmake_check.command_plan(preset), expected)
        source = CMAKE_CHECK.read_text()
        self.assertNotIn("def is_no_work_output", source, "the idle recognizer must be imported, not duplicated")

    # I19
    def test_verdict_document_binds_exact_targets_and_reviewer_policy(self):
        """Kills: build_verdict takes policy from the worker claim, or targets from the claim instead of the observation."""
        observation = {"targets": [target("oz", "zeta"), target("oa", "alpha")]}
        claim = {"work_fp": "WORKER-FP", "policy": "WORKER-POLICY",
                 "targets": [{"unit": "alpha", "output": "STALE-OUT", "work_fp": "STALE-FP"}]}
        reviewer = {"work_fp": "REVIEWER-FP"}
        verdict = lib.build_verdict(observation, claim, reviewer, "approve")
        self.assertIn("decision", verdict)
        self.assertEqual(verdict["decision"], "approve")
        self.assertIn("policy", verdict)
        self.assertEqual(verdict["policy"], "REVIEWER-FP")
        self.assertIn("targets", verdict)
        self.assertEqual(verdict["targets"], [
            {"unit": "alpha", "output": "oa", "work_fp": "wf-oa"},
            {"unit": "zeta", "output": "oz", "work_fp": "wf-oz"},
        ])
        text = json.dumps(verdict, sort_keys=True)
        for leaked in ("WORKER-FP", "WORKER-POLICY", "STALE-OUT", "STALE-FP"):
            self.assertNotIn(leaked, text)

    # I20
    def test_guidance_usable_predicate_rejects_empty_and_generic(self):
        """Kills: guidance_is_usable returns True for any nonempty text, or ignores next_verb."""
        self.assertTrue(lib.guidance_is_usable(GUIDANCE, LIVE, "check"))
        self.assertTrue(lib.guidance_is_usable(GUIDANCE, LIVE, "takeover"))
        self.assertTrue(lib.guidance_is_usable(
            GUIDANCE.replace("- builder: running; next: check,",
                             "- builder: running; next: check-recover (only after confirming the check process stopped),"),
            LIVE, "check-recover"))
        self.assertFalse(lib.guidance_is_usable(GUIDANCE, LIVE, "verdict"), "verdict is not legal for this state")
        self.assertFalse(lib.guidance_is_usable("", LIVE, "check"))
        self.assertFalse(lib.guidance_is_usable("Continue with the next step.", LIVE, "check"))
        self.assertFalse(lib.guidance_is_usable(GUIDANCE.replace("reviewer: blocked", "reviewer: ready"), LIVE, "check"))
        self.assertFalse(lib.guidance_is_usable(GUIDANCE.replace("combined", "other"), LIVE, "check"))

    # I21
    def test_carried_capture_limit_requires_exact_closed_diagnostic(self):
        """Kills: closed_capture_limit_ok accepts any resource_limit or any maximum."""
        def diagnostic(**changes):
            body = {"code": "resource_limit", "message": "Lua instruction limit exceeded", "field": "",
                    "details": {"limit": "instructions", "maximum": 100000000}}
            for key, value in changes.items():
                if key == "maximum":
                    body["details"]["maximum"] = value
                elif key == "limit":
                    body["details"]["limit"] = value
                else:
                    body[key] = value
            return json.dumps(body).encode()

        self.assertTrue(lib.closed_capture_limit_ok(3, b"", diagnostic()))
        refused = {
            "stdout not empty": (3, b"partial", diagnostic()),
            "exit 2": (2, b"", diagnostic()),
            "exit 0": (0, b"", diagnostic()),
            "other code": (3, b"", diagnostic(code="schema")),
            "other maximum": (3, b"", diagnostic(maximum=20000000)),
            "other limit": (3, b"", diagnostic(limit="stack")),
            "other message": (3, b"", diagnostic(message="Lua stack exceeded")),
        }
        for label, (code, out, err) in refused.items():
            with self.subTest(case=label):
                self.assertFalse(lib.closed_capture_limit_ok(code, out, err))

    def test_capture_limit_accepts_exact_real_error_envelope_only(self):
        """Kills H2a: closed_capture_limit_ok accepts only the bare closed object, so the real envelope is refused; H2b: it accepts any envelope whatever its inner error, kind or version."""
        def closed_error(**changes):
            error = {"code": "resource_limit", "message": "Lua instruction limit exceeded", "field": "",
                     "details": {"limit": "instructions", "maximum": 100000000}}
            for key, value in changes.items():
                if key == "maximum":
                    error["details"]["maximum"] = value
                elif key == "limit":
                    error["details"]["limit"] = value
                else:
                    error[key] = value
            return error

        def envelope(error=None, **top):
            body = {"error": closed_error() if error is None else error, "kind": "error", "version": 1}
            body.update(top)
            return json.dumps(body).encode()

        self.assertTrue(lib.closed_capture_limit_ok(3, b"", json.dumps(closed_error()).encode()),
                        "the bare closed object stays accepted")
        self.assertTrue(lib.closed_capture_limit_ok(3, b"", envelope()), "the exact real envelope is accepted")
        refused = {
            "envelope other maximum": (3, b"", envelope(error=closed_error(maximum=20000000))),
            "envelope other code": (3, b"", envelope(error=closed_error(code="schema"))),
            "envelope other message": (3, b"", envelope(error=closed_error(message="Lua stack exceeded"))),
            "envelope other field": (3, b"", envelope(error=closed_error(field="item"))),
            "envelope other limit": (3, b"", envelope(error=closed_error(limit="stack"))),
            "envelope extra top-level key": (3, b"", envelope(extra=1)),
            "envelope kind other than error": (3, b"", envelope(kind="warning")),
            "envelope version other than 1": (3, b"", envelope(version=2)),
            "envelope with nonempty stdout": (3, b"partial", envelope()),
            "envelope with exit not 3": (2, b"", envelope()),
        }
        for label, (code, out, err) in refused.items():
            with self.subTest(case=label):
                self.assertFalse(lib.closed_capture_limit_ok(code, out, err))

    # I22
    def test_product_change_guard_sees_tracked_and_untracked_src_edits(self):
        """Kills: product_changes ignores untracked files, or diffs only tracked paths."""
        repo = self.tmp / "repo"
        repo.mkdir()
        self.git(repo, "init", "-q")
        self.write("repo/src/a.cpp", "int a;\n")
        self.write("repo/docs/x.md", "doc\n")
        self.git(repo, "add", "-A")
        self.git(repo, "commit", "-q", "-m", "start")
        start = self.git(repo, "rev-parse", "HEAD")
        (repo / "src/a.cpp").write_text("int a = 2;\n")
        self.write("repo/src/new.cppm", "export module n;\n")
        (repo / "docs/x.md").write_text("changed doc\n")
        self.write("repo/docs/y.md", "untracked doc\n")
        self.assertEqual(lib.product_changes(repo, start), ["src/a.cpp", "src/new.cppm"])

    # I23
    def test_repin_rule_flags_runtime_cpp_and_embedded_lua(self):
        """Kills: repin_required omits the src/stdlib/** lua pattern."""
        for changed in (["src/engine/runtime/runtime.cpp"], ["src/stdlib/planar/render.lua"],
                        ["src/stdlib/planar/orchestrator.lua"], ["src/stdlib/planar/commands.lua"]):
            with self.subTest(changed=changed):
                self.assertTrue(lib.repin_required(changed))
        for unchanged in (["docs/vernacular/roadmap.md"], ["tests/native/r19-accept-support/accept_test.py"],
                          ["src/engine/capture/capture.cpp"], ["src/cmd/planar/bootstrap.cpp"],
                          ["fixtures/render/quota/workspace/spec.lua"]):
            with self.subTest(unchanged=unchanged):
                self.assertFalse(lib.repin_required(unchanged))

    # I24
    def test_red_record_must_be_behavioural_and_hashed(self):
        """Kills: validate_red_record accepts a nonzero exit without an assertion marker."""
        good = {"exit": 1, "output": "FAILED:\n  REQUIRE( x == 2 )\n", "snapshot_sha256": "a" * 64,
                "command": "planar_tests [name]"}
        lib.validate_red_record(good)
        refused = {
            "zero exit": {**good, "exit": 0},
            "import error without assertion": {**good, "output": "ImportError: no module named x"},
            "compile error": {**good, "output": "src/x.cpp:3:1: error: no member named foo"},
            "missing snapshot hash": {**good, "snapshot_sha256": None},
            "non-hex snapshot hash": {**good, "snapshot_sha256": "zz" * 32},
        }
        for label, record in refused.items():
            with self.subTest(case=label):
                with self.assertRaises(lib.AcceptError):
                    lib.validate_red_record(record)

    # I25
    def test_cmake_adapter_rendezvous_after_inventory_before_ctest(self):
        """Kills: cmake_check writes the marker before the build step, omits pid or pgid, or bypasses accept_lib.write_rendezvous."""
        support = self.support_copy()
        repo = self.tmp / "repo"
        (repo / "src").mkdir(parents=True)
        (repo / "src/a.cpp").write_text("int a;\n")
        evidence = self.tmp / "evidence"
        evidence.mkdir()
        rv = "{evidence}/RENDEZVOUS.json"
        stubs = {
            "configure": [PY, "-c", "pass"],
            "build": [PY, "-c", "import pathlib,sys; pathlib.Path(sys.argv[1]).write_text(str(pathlib.Path(sys.argv[2]).exists()))",
                      "{evidence}/build-saw.txt", rv],
            "inventory": [PY, "-c", "import json; print(json.dumps({'tests': [{'name': 'alpha'}]}))"],
            "ctest": [PY, "-c", "import pathlib,shutil,sys; pathlib.Path(sys.argv[1]).write_text('<testsuite><testcase name=\"alpha\" status=\"run\"/></testsuite>'); shutil.copyfile(sys.argv[3], sys.argv[2])",
                      "{attempt}/ctest.xml", "{evidence}/ctest-saw.json", rv],
            "lint": [PY, "-c", "pass"],
            "repeat-build": [PY, "-c", "print('ninja: no work to do.')"],
        }
        stub_file = self.write("stubs.json", json.dumps(stubs))
        proc = subprocess.Popen([PY, str(support / "cmake_check.py"), "--stub-commands", str(stub_file),
                                 "--support-dir", str(support), "--repo", str(repo),
                                 "--evidence", str(evidence), "--preset", "debug"],
                                env=self.env(), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        # Without start_new_session the adapter inherits this test's process group, so the expected
        # pgid is known before the child can exit (no race with os.getpgid on a reaped pid) and
        # differs from the pid, so a pid-for-pgid mutant is visible.
        adapter_pid = proc.pid
        adapter_pgid = os.getpgrp()
        out, err = proc.communicate(timeout=120)
        self.assertEqual(proc.returncode, 0, err.decode(errors="replace"))
        self.assertEqual((evidence / "build-saw.txt").read_text(), "False",
                         "the rendezvous must not exist during build")
        saw = evidence / "ctest-saw.json"
        self.assertTrue(saw.is_file(), "ctest must observe the rendezvous")
        self.assertEqual(lib.read_rendezvous(saw),
                         {"version": 1, "phase": "ctest", "pid": adapter_pid, "pgid": adapter_pgid})

    # I26
    def test_cmake_adapter_post_kill_integrity_requires_noop_build(self):
        """Kills: cmake_check integrity_verdict returns True for any build output."""
        self.assertTrue(cmake_check.integrity_verdict(b"ninja: no work to do.\n"))
        self.assertFalse(cmake_check.integrity_verdict(b"[1/1] Building CXX object x.o\n"))
        self.assertFalse(cmake_check.integrity_verdict(b""))
        self.assertFalse(cmake_check.integrity_verdict(b"[1/2] Re-checking globbed directories...\nninja: no work to do.\n"))

    # I27
    def test_compose_base_diff_touches_only_owned_cmake_paths(self):
        """Kills: compose_base copies the old tree whole or keeps owned files absent there, or boundary_paths_ok accepts any path."""
        self.assertEqual(len(OWNED_CMAKE), 16)
        head = {p: b"head:" + p.encode() for p in OWNED_CMAKE}
        head.update({"runtime/runtime.cpp": b"head-runtime", "runtime/commands.t.cpp": b"head-commands",
                     "capture/capture.cpp": b"head-capture"})
        old = {p: b"old:" + p.encode() for p in OWNED_CMAKE if p != "runtime/closure_guard/CMakeLists.txt"}
        old.update({"runtime/runtime.cpp": b"old-runtime", "capture/capture.cpp": b"head-capture"})
        composed = lib.compose_base(head, old, OWNED_CMAKE)
        self.assertIn("value/CMakeLists.txt", composed)
        self.assertEqual(composed["value/CMakeLists.txt"], old["value/CMakeLists.txt"])
        self.assertNotIn("runtime/closure_guard/CMakeLists.txt", composed,
                         "an owned file absent in the old tree must be absent in the base")
        self.assertEqual(composed["runtime/runtime.cpp"], head["runtime/runtime.cpp"])
        self.assertEqual(composed["runtime/commands.t.cpp"], head["runtime/commands.t.cpp"])
        self.assertTrue(lib.boundary_paths_ok(composed, head, OWNED_CMAKE))
        self.assertFalse(lib.boundary_paths_ok(old, head, OWNED_CMAKE),
                         "the plain 58128234 base differs outside the owned paths")

    # I28
    def test_check_runner_derives_honest_result_from_unittest_child(self):
        """Kills: check_runner writes passed=matched or constants regardless of the child, or counts skips as passed."""
        child = self.tmp / "child"
        child.mkdir()
        (child / "test_mix.py").write_text(
            "import unittest\n"
            "class T(unittest.TestCase):\n"
            "    def test_a(self):\n        pass\n"
            "    def test_b(self):\n        pass\n"
            "    def test_fail(self):\n        self.assertTrue(False)\n"
            "    def test_error(self):\n        raise RuntimeError('boom')\n"
            "    @unittest.skip('later')\n"
            "    def test_skip(self):\n        pass\n")
        result = self.tmp / "result.json"
        proc = subprocess.run(
            [PY, str(RUNNER), "--counter", str(self.tmp / "counter.log"), "--mode", "unittest", "--",
             PY, "-m", "unittest", "discover", "-s", str(child), "-p", "test_*.py"],
            cwd=child, env=self.env(PLANAR_CHECK_RESULT=str(result)), capture_output=True, text=True)
        self.assertTrue(result.is_file(), proc.stderr)
        self.assertEqual(json.loads(result.read_text()), {"version": 1, "matched": 5, "passed": 2, "failed": 2})
        empty = self.tmp / "empty"
        empty.mkdir()
        result2 = self.tmp / "result-empty.json"
        subprocess.run(
            [PY, str(RUNNER), "--counter", str(self.tmp / "counter2.log"), "--mode", "unittest", "--",
             PY, "-m", "unittest", "discover", "-s", str(empty), "-p", "test_*.py"],
            cwd=empty, env=self.env(PLANAR_CHECK_RESULT=str(result2)), capture_output=True, text=True)
        self.assertTrue(result2.is_file())
        self.assertEqual(json.loads(result2.read_text()), {"version": 1, "matched": 0, "passed": 0, "failed": 0})

    # I29
    def test_check_runner_propagates_exit_counts_once_and_passthrough_keeps_result(self):
        """Kills: check_runner exits 0 regardless, overwrites a child result in passthrough, or counts only successful runs."""
        counter = self.tmp / "counter.log"
        failing = self.write("failing_child.py", (
            "import sys\n"
            "with open(sys.argv[1]) as h:\n"
            "    n = sum(1 for line in h if line.strip())\n"
            "print('child-out count-at-start=%d' % n)\n"
            "print('child-err', file=sys.stderr)\n"
            "sys.exit(3)\n"))
        first = subprocess.run([PY, str(RUNNER), "--counter", str(counter), "--mode", "unittest", "--",
                                PY, str(failing), str(counter)],
                               env=self.env(), capture_output=True, text=True)
        self.assertEqual(first.returncode, 3, "the runner exits with the child status")
        self.assertIn("child-out count-at-start=1", first.stdout, "the counter line precedes the child run")
        self.assertIn("child-err", first.stderr)
        second = subprocess.run([PY, str(RUNNER), "--counter", str(counter), "--mode", "unittest", "--",
                                 PY, str(failing), str(counter)],
                                env=self.env(), capture_output=True, text=True)
        self.assertEqual(second.returncode, 3)
        lines = [line for line in counter.read_text().splitlines() if line.strip()]
        self.assertEqual(len(lines), 2, "one counter line per real execution, failures included")

        writer = self.write("writer_child.py", (
            "import os, sys\n"
            "with open(os.environ['PLANAR_CHECK_RESULT'], 'x') as h:\n"
            "    h.write('{\"version\": 1, \"matched\": 3, \"passed\": 3, \"failed\": 0}\\n')\n"
            "sys.exit(0)\n"))
        result = self.tmp / "passthrough-result.json"
        counter3 = self.tmp / "counter3.log"
        third = subprocess.run([PY, str(RUNNER), "--counter", str(counter3), "--mode", "passthrough", "--",
                                PY, str(writer)],
                               env=self.env(PLANAR_CHECK_RESULT=str(result)), capture_output=True, text=True)
        self.assertEqual(third.returncode, 0)
        self.assertTrue(result.is_file())
        self.assertEqual(result.read_bytes(), b'{"version": 1, "matched": 3, "passed": 3, "failed": 0}\n',
                         "passthrough mode leaves a child-written valid result untouched")
        self.assertEqual(len([l for l in counter3.read_text().splitlines() if l.strip()]), 1)

    # I30
    def test_rendezvous_pair_round_trips_atomically_and_rejects_partial(self):
        """Kills: write_rendezvous stores pgid under another key or writes non-atomically; read_rendezvous accepts missing keys."""
        evidence = self.tmp / "ev"
        evidence.mkdir()
        lib.write_rendezvous(evidence, "ctest", 4242, 4141)
        target_path = evidence / lib.RENDEZVOUS_NAME
        self.assertTrue(target_path.is_file())
        self.assertEqual(sorted(os.listdir(evidence)), [lib.RENDEZVOUS_NAME],
                         "no temporary file may remain beside the rendezvous")
        self.assertEqual(lib.read_rendezvous(target_path),
                         {"version": 1, "phase": "ctest", "pid": 4242, "pgid": 4141})
        good = target_path.read_text()
        partial = self.tmp / "partial.json"
        partial.write_text(good[:-6])
        with self.assertRaises(lib.AcceptError):
            lib.read_rendezvous(partial)
        missing = self.tmp / "missing-key.json"
        missing.write_text(json.dumps({"version": 1, "phase": "ctest", "pid": 4242}))
        with self.assertRaises(lib.AcceptError):
            lib.read_rendezvous(missing)
        unknown = self.tmp / "unknown-key.json"
        unknown.write_text(json.dumps({"version": 1, "phase": "ctest", "pid": 4242, "pgid": 4141, "extra": 1}))
        with self.assertRaises(lib.AcceptError):
            lib.read_rendezvous(unknown)
        wrong_version = self.tmp / "wrong-version.json"
        wrong_version.write_text(json.dumps({"version": 2, "phase": "ctest", "pid": 4242, "pgid": 4141}))
        with self.assertRaises(lib.AcceptError):
            lib.read_rendezvous(wrong_version)

    # I31
    def test_registry_gen_changes_only_toolchain_command_prefix(self):
        """Kills: registry_gen also alters a check argv or timeout, or drops a toolchain environment."""
        registry = json.loads(FIXTURE_REGISTRY.read_text())
        generated = registry_gen.generate(registry, runner=Path("/scratch/check_runner.py"),
                                          counter=Path("/scratch/check-counter.log"),
                                          mode="unittest", python="/usr/bin/python3")
        self.assertIn("toolchains", generated)
        self.assertEqual(generated["toolchains"][0]["command"], [
            "/usr/bin/python3", "/scratch/check_runner.py", "--counter", "/scratch/check-counter.log",
            "--mode", "unittest", "--", "python3", "-B"])
        self.assertEqual(generated.get("checks"), registry["checks"], "check argv, timeouts and bounds are unchanged")
        self.assertEqual(generated.get("id"), registry["id"])
        self.assertEqual(generated.get("revision"), registry["revision"])
        self.assertEqual(generated["toolchains"][0].get("environment"), registry["toolchains"][0]["environment"])
        self.assertEqual(generated["toolchains"][0].get("id"), registry["toolchains"][0]["id"])

    # I32
    def test_candidate_record_hashes_bytes_and_refuses_shared_build_dirs(self):
        """Kills: candidate_record drops the shared-build-dir refusal, or hashes a fixed string."""
        repo = self.tmp / "repo"
        (repo / "build/rewrite-debug").mkdir(parents=True)
        (repo / "build/rewrite-release").mkdir(parents=True)
        exe = self.write("scratch/planar_authoring_stack", b"\x7fELF-fixture" * 100)
        exe.chmod(0o755)
        record = lib.candidate_record(exe, repo, "a" * 40)
        self.assertIn("sha256", record)
        self.assertEqual(record["sha256"], sha256(exe.read_bytes()))
        self.assertEqual(record.get("size"), exe.stat().st_size)
        self.assertEqual(record.get("head"), "a" * 40)
        with self.assertRaises(lib.AcceptError):
            lib.candidate_record(self.tmp / "missing-binary", repo, "a" * 40)
        plain = self.write("scratch/not_exec", b"data")
        plain.chmod(0o644)
        with self.assertRaises(lib.AcceptError):
            lib.candidate_record(plain, repo, "a" * 40)
        debug_copy = repo / "build/rewrite-debug/planar_authoring_stack"
        debug_copy.write_bytes(exe.read_bytes())
        debug_copy.chmod(0o755)
        with self.assertRaises(lib.AcceptError):
            lib.candidate_record(debug_copy, repo, "a" * 40)
        release_copy = repo / "build/rewrite-release/planar_authoring_stack"
        release_copy.write_bytes(exe.read_bytes())
        release_copy.chmod(0o755)
        with self.assertRaises(lib.AcceptError):
            lib.candidate_record(release_copy, repo, "a" * 40)

    # I33
    def test_recorded_candidate_verification_rejects_mismatch(self):
        """Kills: require_recorded_candidate compares only size, or returns without comparing sha256 or HEAD."""
        repo = self.tmp / "repo"
        repo.mkdir()
        self.git(repo, "init", "-q")
        self.write("repo/README.md", "start\n")
        self.git(repo, "add", "-A")
        self.git(repo, "commit", "-q", "-m", "start")
        head = self.git(repo, "rev-parse", "HEAD")
        original = b"\x7fELF-candidate" * 64
        exe = self.write("scratch/candidate", original)
        exe.chmod(0o755)
        record = self.tmp / "candidate.json"
        record.write_text(json.dumps({"head": head, "sha256": sha256(original), "size": len(original)}))
        lib.require_recorded_candidate(exe, record, repo)
        flipped = bytearray(original)
        flipped[7] ^= 0x01
        exe.write_bytes(bytes(flipped))
        with self.assertRaises(lib.AcceptError):
            lib.require_recorded_candidate(exe, record, repo)
        exe.write_bytes(original + b"x")
        with self.assertRaises(lib.AcceptError):
            lib.require_recorded_candidate(exe, record, repo)
        exe.write_bytes(original)
        self.git(repo, "commit", "-q", "--allow-empty", "-m", "second")
        with self.assertRaises(lib.AcceptError):
            lib.require_recorded_candidate(exe, record, repo)
        with self.assertRaises(lib.AcceptError):
            lib.require_recorded_candidate(exe, self.tmp / "no-record.json", repo)
        built = repo / "build/rewrite-debug/planar_authoring_stack"
        built.parent.mkdir(parents=True)
        built.write_bytes(original)
        built.chmod(0o755)
        with self.assertRaises(lib.AcceptError):
            lib.require_recorded_candidate(built, record, repo)

    # I34
    def test_require_legal_action_rejects_verb_absent_from_guidance(self):
        """Kills: require_legal_action returns without checking membership, or UNGUIDED_VERBS contains check."""
        legal = lib.require_legal_action(GUIDANCE, "check")
        self.assertIn("check", legal)
        self.assertIn("takeover", legal)
        with self.assertRaises(lib.AcceptError):
            lib.require_legal_action(GUIDANCE, "verdict")
        with self.assertRaises(lib.AcceptError):
            lib.require_legal_action("# Planar orchestrator prompt\n", "check")
        self.assertEqual(lib.UNGUIDED_VERBS, frozenset({"load", "activate"}))
        for verb in ("check", "check-recover", "verdict", "takeover", "claim"):
            self.assertNotIn(verb, lib.UNGUIDED_VERBS)

    # I35
    def test_cmake_adapter_never_writes_bytecode_into_support_tree(self):
        """Kills: cmake_check removes the sys.dont_write_bytecode setting (or the child env flag) before importing the support check module."""
        support = self.support_copy()
        repo = self.tmp / "repo"
        (repo / "src").mkdir(parents=True)
        (repo / "src/a.cpp").write_text("int a;\n")
        evidence = self.tmp / "evidence"
        evidence.mkdir()
        real_before = tree_digest(REAL_CMAKE_SUPPORT)
        stubs = {
            "configure": [PY, "-c", "import os,pathlib,sys; pathlib.Path(sys.argv[1]).write_text(os.environ.get('PYTHONDONTWRITEBYTECODE', ''))", "{evidence}/env.txt"],
            "build": [PY, "-c", "pass"],
            "inventory": [PY, "-c", "import json; print(json.dumps({'tests': [{'name': 'alpha'}]}))"],
            "ctest": [PY, "-c", "import pathlib,sys; pathlib.Path(sys.argv[1]).write_text('<testsuite><testcase name=\"alpha\" status=\"run\"/></testsuite>')", "{attempt}/ctest.xml"],
            "lint": [PY, "-c", "pass"],
            "repeat-build": [PY, "-c", "print('ninja: no work to do.')"],
        }
        stub_file = self.write("stubs.json", json.dumps(stubs))
        proc = subprocess.run([PY, str(support / "cmake_check.py"), "--stub-commands", str(stub_file),
                               "--support-dir", str(support), "--repo", str(repo),
                               "--evidence", str(evidence), "--preset", "debug"],
                              env=self.env(), capture_output=True, text=True, timeout=120)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        leftovers = [p for p in support.rglob("*") if p.name == "__pycache__" or p.suffix == ".pyc"]
        self.assertEqual(leftovers, [], "no bytecode may be written into the support copy")
        self.assertEqual((evidence / "env.txt").read_text(), "1", "child processes inherit PYTHONDONTWRITEBYTECODE=1")
        self.assertEqual(tree_digest(REAL_CMAKE_SUPPORT), real_before,
                         "the real r19-cmake-support tree, including __pycache__, is unchanged")


class AcceptLibGapTest(unittest.TestCase):
    """Second-pass tests for invariants that mutation probing found unprotected (R19-ACCEPT-A repair).

    Each docstring names the mutant the test must kill.
    """

    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="r19-accept-gap-"))
        self.addCleanup(shutil.rmtree, str(self.tmp), True)

    def write(self, relative, data):
        path = self.tmp / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data if isinstance(data, bytes) else data.encode())
        return path

    def test_scan_finds_prefix_only_present_as_decoded_json_value(self):
        """Kills I3b: scan_host_paths skips the decoded JSON string search (raw bytes hold only an escape)."""
        prefix = "/opt/f\u00e4ke-home"
        payload = json.dumps({"deep": [{"where": prefix + "/x"}]})
        self.assertNotIn(prefix.encode(), payload.encode(), "the raw bytes must carry only the escaped form")
        self.write("tree/doc.json", payload)
        self.assertEqual(lib.scan_host_paths(self.tmp / "tree", [prefix]), [("doc.json", prefix)])

    def test_scan_and_event_expansion_refuse_vacuous_inputs(self):
        """Kills E15 (empty scan prefix accepted) and E14 (empty expected references accepted)."""
        self.write("tree/a.txt", "x\n")
        with self.assertRaises(lib.AcceptError):
            lib.scan_host_paths(self.tmp / "tree", ["/opt/real", ""])
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("check_reserved", {"reservation": {"id": "r"}}, {})
        with self.assertRaises(lib.AcceptError):
            lib.verify_event_expansion("check_reserved", {"reservation": {"id": "r"}}, None)

    def test_at_event_state_refuses_missing_roles_and_extra_roles(self):
        """Kills E17 (missing roles mapping accepted) and E18 (replayed role set with an extra role accepted)."""
        role = {"state": "running", "drifted": False, "run": "run-1"}
        recorded = {"active_job": "combined", "activation": "act-1", "roles": {"builder": dict(role)}}
        no_roles = {"active_job": "combined", "activation": "act-1"}
        with self.assertRaises(lib.AcceptError):
            lib.verify_at_event_state(no_roles, copy.deepcopy(no_roles))
        with self.assertRaises(lib.AcceptError):
            lib.verify_at_event_state(recorded, no_roles)
        with self.assertRaises(lib.AcceptError):
            lib.verify_at_event_state({**recorded, "roles": None}, {**recorded, "roles": None})
        extra = copy.deepcopy(recorded)
        extra["roles"]["reviewer"] = dict(role)
        with self.assertRaises(lib.AcceptError):
            lib.verify_at_event_state(recorded, extra)

    def test_trajectory_interruption_must_close_by_check_recover(self):
        """Kills I10 (drops the check-recover requirement) and I10c (a missing interrupted check is tolerated)."""
        events = trajectory()
        lib.validate_trajectory(events)
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 1, via="check-complete"))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 1, via=None))
        # No interrupted check at all, while the first event looks recovered: still refused.
        no_interrupted = copy.deepcopy(without(events, 1))
        no_interrupted[0]["via"] = "check-recover"
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(no_interrupted)

    def test_trajectory_rejection_admission_predicate_is_exact(self):
        """Kills I12 (repair_admitted ignored), I12c (count < max_rounds ignored) and I12d (first approve admitted)."""
        events = trajectory()
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 6, repair_admitted=False))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 6, count=1))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 6, decision="approve"))

    def test_trajectory_capped_rejection_requires_repair_not_admitted(self):
        """Kills I13b: the capped rejection is accepted with repair_admitted true and no repair roles."""
        events = trajectory()
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 10, repair_admitted=True))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 10, repair_admitted=None))
        with self.assertRaises(lib.AcceptError):
            lib.validate_trajectory(replaced(events, 10, repair=["builder"]))

    def test_suite_refusals_each_clause_stands_alone(self):
        """Kills S2b (ran=0 accepted) and S2c (child exit ignored): each row trips only one clause."""
        clean = {"ran": 3, "failures": 0, "errors": 0, "skipped": 0, "exit": 0}
        self.assertEqual(run_acceptance.suite_refusals(clean), [])
        self.assertTrue(run_acceptance.suite_refusals({**clean, "ran": 0}))
        self.assertTrue(run_acceptance.suite_refusals({**clean, "exit": 5}))
        self.assertTrue(run_acceptance.suite_refusals({**clean, "skipped": 1}))

    def test_inventory_verdict_refuses_duplicate_inventory_names(self):
        """Kills I17c: cmake_check tolerates a duplicated name in the live inventory when the cases repeat it."""
        twice = [{"name": "alpha", "status": "passed"}, {"name": "alpha", "status": "passed"}]
        self.assertFalse(cmake_check.inventory_verdict(["alpha", "alpha"], twice))
        self.assertTrue(cmake_check.inventory_verdict(["alpha"], twice[:1]))

    def test_red_record_each_marker_rule_stands_alone(self):
        """Kills I24 (no assertion marker required) and I24b (compile or import error accepted)."""
        base = {"exit": 1, "snapshot_sha256": "a" * 64, "command": "planar_tests [name]"}
        with self.assertRaises(lib.AcceptError):
            lib.validate_red_record({**base, "output": "Segmentation fault\nnothing else\n"})
        with self.assertRaises(lib.AcceptError):
            lib.validate_red_record({**base, "output": "x.cpp:1:1: error: no member\nFAILED:\n  REQUIRE( x )\n"})
        with self.assertRaises(lib.AcceptError):
            lib.validate_red_record({**base, "output": "FAILED:\nImportError: no module named x\n"})
        with self.assertRaises(lib.AcceptError):
            lib.validate_red_record({**base, "output": "AssertionError: 1 != 2\nModuleNotFoundError: y\n"})
        lib.validate_red_record({**base, "output": "AssertionError: 1 != 2\n"})

    def test_check_runner_passthrough_never_writes_a_result_document(self):
        """Kills I29b: passthrough mode also derives and writes PLANAR_CHECK_RESULT from unittest-looking output."""
        child = self.write("summary_child.py", (
            "import sys\n"
            "print('Ran 2 tests in 0.001s', file=sys.stderr)\n"
            "print('', file=sys.stderr)\n"
            "print('OK', file=sys.stderr)\n"))
        result = self.tmp / "must-not-exist.json"
        env = os.environ.copy()
        env["PYTHONDONTWRITEBYTECODE"] = "1"
        env["PLANAR_CHECK_RESULT"] = str(result)
        proc = subprocess.run([PY, str(RUNNER), "--counter", str(self.tmp / "counter.log"), "--mode", "passthrough",
                               "--", PY, str(child)], env=env, capture_output=True, text=True)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertFalse(result.exists(), "passthrough mode must leave the result path to the child")
        proc = subprocess.run([PY, str(RUNNER), "--counter", str(self.tmp / "counter2.log"), "--mode", "unittest",
                               "--", PY, str(child)], env=env, capture_output=True, text=True)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertEqual(json.loads(result.read_text()), {"version": 1, "matched": 2, "passed": 2, "failed": 0},
                         "the same output in unittest mode does produce the honest result")

    def test_rendezvous_write_replaces_the_file_instead_of_rewriting_it_in_place(self):
        """Kills I30c: write_rendezvous rewrites the existing file in place (a reader could see a partial document)."""
        evidence = self.tmp / "ev"
        evidence.mkdir()
        lib.write_rendezvous(evidence, "ctest", 1, 2)
        held = self.tmp / "held-first-version.json"
        os.link(evidence / lib.RENDEZVOUS_NAME, held)
        lib.write_rendezvous(evidence, "ctest", 3, 4)
        self.assertEqual(lib.read_rendezvous(evidence / lib.RENDEZVOUS_NAME)["pid"], 3)
        self.assertEqual(json.loads(held.read_text())["pid"], 1,
                         "the earlier inode must be untouched: the new document arrives by atomic replace")
        self.assertNotEqual(os.stat(held).st_ino, os.stat(evidence / lib.RENDEZVOUS_NAME).st_ino)

    def test_child_env_sets_bytecode_flag_even_when_the_parent_lacks_it(self):
        """Kills I35b: child_env no longer sets PYTHONDONTWRITEBYTECODE (the harness env already exports it)."""
        saved = os.environ.pop("PYTHONDONTWRITEBYTECODE", None)
        try:
            self.assertEqual(lib.child_env()["PYTHONDONTWRITEBYTECODE"], "1")
            self.assertEqual(lib.child_env({"X": "y"})["X"], "y")
        finally:
            if saved is not None:
                os.environ["PYTHONDONTWRITEBYTECODE"] = saved

    def test_build_verdict_refuses_a_unit_named_twice(self):
        """Kills E13: build_verdict accepts an observation that names a target unit twice."""
        twice = {"targets": [target("o1", "alpha"), target("o2", "alpha")]}
        with self.assertRaises(lib.AcceptError):
            lib.build_verdict(twice, {}, {"work_fp": "R"}, "approve")

    # Negative fixtures for the C02 survivors (PKG-FIX). New names only; no existing test is renamed.

    def test_check_runner_passed_excludes_failures_and_skips(self):
        """Kills Q13-b: check_runner.unittest_result sets passed to matched, ignoring failures and skips."""
        failing = "Ran 3 tests in 0.001s\n\nFAILED (failures=1)\n"
        skipping = "Ran 3 tests in 0.001s\n\nOK (skipped=1)\n"
        self.assertEqual(check_runner.unittest_result(failing),
                         {"version": 1, "matched": 3, "passed": 2, "failed": 1})
        self.assertEqual(check_runner.unittest_result(skipping),
                         {"version": 1, "matched": 3, "passed": 2, "failed": 0})

    def test_guidance_predicate_refuses_text_without_job_role_line_or_verb(self):
        """Kills L7-a: accept_lib.guidance_is_usable returns True for every input, so unusable guidance passes."""
        live = {"job": "combined", "roles": {"builder": "ready"}}
        usable = "Job combined is active.\n- builder: ready; next: check\n"
        self.assertTrue(lib.guidance_is_usable(usable, live, "check"))
        self.assertFalse(lib.guidance_is_usable("- builder: ready; next: check\n", live, "check"),
                         "the live job must be named")
        self.assertFalse(lib.guidance_is_usable("Job combined is active.\n", live, "check"),
                         "each live role line must be present")
        self.assertFalse(lib.guidance_is_usable(usable, live, "verdict"), "the next verb must be listed")
        self.assertFalse(lib.guidance_is_usable("", live, "check"), "empty guidance is never usable")


class HeavyDriverTest(unittest.TestCase):
    """C03 R1l to R1p: the heavy entry's exit code and overlay, and the export overlay of the scenario."""

    OVERLAY = ["src/engine/ledger/CMakeLists.txt", "fixtures/ledger/expected-schema.sql"]
    FIXTURE = "fixtures/ledger/expected-schema.sql"
    LEDGER_CMAKE = "src/engine/ledger/CMakeLists.txt"

    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="heavy-driver-"))
        self.addCleanup(shutil.rmtree, self.tmp, ignore_errors=True)

    def run_main(self, result):
        """Run run_heavy.main with a stubbed scenario and candidate check; return (rc, stderr, scratch)."""
        import contextlib
        import io
        from unittest import mock
        scratch = self.tmp / "scratch"
        scratch.mkdir()
        err = io.StringIO()
        with mock.patch.object(run_heavy.fault_driver, "run_scenario", return_value=result), \
                mock.patch.object(run_heavy.accept_lib, "require_recorded_candidate", return_value=None), \
                contextlib.redirect_stderr(err):
            rc = run_heavy.main(["--scratch", str(scratch), "--candidate", str(self.tmp / "planar"),
                                 "--record", str(self.tmp / "candidate.json"), "--repo", str(self.tmp)])
        return rc, err.getvalue(), scratch

    def test_run_heavy_exits_nonzero_when_the_scenario_stopped(self):
        """Kills R1l: main returns 0 unconditionally, or returns before writing result.json."""
        stopped = {"trace": ["init", "check-1"], "faults": {}, "counter_lines": 3, "intended_executions": 4}
        rc, stderr, scratch = self.run_main(stopped)
        self.assertNotEqual(rc, 0, "a stopped scenario must exit nonzero")
        self.assertEqual(rc, 3)
        written = scratch / "result.json"
        self.assertTrue(written.is_file(), "result.json is written before the nonzero return")
        self.assertEqual(json.loads(written.read_text(encoding="utf-8")), stopped)
        self.assertIn("stopped", stderr.lower())

    def test_run_heavy_exits_nonzero_when_the_trajectory_is_present_but_invalid(self):
        """Kills R1m-c: the stopped predicate checks only that trajectory is a dict, not that valid is true."""
        for trajectory in ({"valid": False, "events": [1]}, {}, {"valid": "true"}):
            with self.subTest(trajectory=trajectory):
                self.tearDown_scratch()
                rc, stderr, scratch = self.run_main({"trace": ["init"], "faults": {}, "trajectory": trajectory})
                self.assertEqual(rc, 3)
                self.assertIn("stopped", stderr.lower())
                self.assertTrue((scratch / "result.json").is_file())

    def tearDown_scratch(self):
        """Remove the scratch directory so run_main can recreate it between sub-cases."""
        shutil.rmtree(self.tmp / "scratch", ignore_errors=True)

    def test_run_heavy_exits_zero_and_writes_result_for_a_completed_scenario(self):
        """Kills R1m: the stopped predicate is always true, so every result exits 3."""
        completed = {"trace": ["init"], "faults": {}, "counter_lines": 4, "intended_executions": 4,
                     "trajectory": {"valid": True, "events": [1, 2, 3]}}
        from unittest import mock

        with mock.patch.object(run_heavy.run_acceptance, "validate_rerun_result", return_value=None):
            rc, stderr, scratch = self.run_main(completed)
        self.assertEqual(rc, 0)
        self.assertEqual(json.loads((scratch / "result.json").read_text(encoding="utf-8")), completed)
        self.assertNotIn("stopped", stderr.lower())

    def test_run_heavy_exits_nonzero_when_a_completed_result_fails_rerun_validation(self):
        """Kills R19-ACCEPT-A-C03-R1 (exit): a completed run whose result rerun-index refuses exited 0."""
        completed = {"trace": ["init"], "faults": {}, "counter_lines": 7, "intended_executions": 8,
                     "trajectory": {"valid": True, "events": [1, 2, 3]}}
        rc, stderr, scratch = self.run_main(completed)
        self.assertEqual(rc, 4, "a result the rerun validator refuses must exit 4")
        self.assertTrue((scratch / "result.json").is_file(), "result.json is still written for diagnosis")
        self.assertIn("rerun validation", stderr.lower())

    def test_run_heavy_config_overlays_the_fixture_and_ledger_cmake(self):
        """Kills R1n: the overlay is empty, missing, reordered into a set, or carries an extra path."""
        config = run_heavy.build_config(self.tmp / "scratch", self.tmp / "planar")
        self.assertEqual(config.get("overlay"), self.OVERLAY)

    def make_repo(self):
        """Return a git repo whose HEAD differs from its working tree for the overlay paths."""
        repo = self.tmp / "repo"
        (repo / "src/engine/ledger").mkdir(parents=True)
        (repo / "src/engine/value").mkdir(parents=True)
        (repo / self.LEDGER_CMAKE).write_bytes(b"head ledger cmake\n")
        (repo / "src/engine/value/CMakeLists.txt").write_bytes(b"head value cmake\n")
        identity = {"GIT_AUTHOR_NAME": "t", "GIT_AUTHOR_EMAIL": "t@example.invalid",
                    "GIT_COMMITTER_NAME": "t", "GIT_COMMITTER_EMAIL": "t@example.invalid"}
        for args in (("init", "-q"), ("add", "-A"), ("commit", "-q", "-m", "head")):
            subprocess.run(["git", "-C", str(repo), *args], check=True, capture_output=True,
                           env=lib.child_env(identity))
        (repo / self.LEDGER_CMAKE).write_bytes(b"working tree ledger cmake\n")
        (repo / "fixtures/ledger").mkdir(parents=True)
        (repo / self.FIXTURE).write_bytes(b"working tree schema\n")
        return repo

    def scenario(self, repo, name, overlay):
        root = self.tmp / name
        config = {"candidate": str(root / "planar"), "repo": str(repo), "mode": "cmake", "check_mode": "real",
                  "base": "plain", "old_revision": "HEAD", "max_rounds": 1, "kill_target": "cli",
                  "owned": [], "repairs": [], "tamper": {}, "scratch": str(root),
                  "export": str(root / "export"), "evidence": str(root / "evidence"),
                  "raw_root": str(root / "raw"), "home": str(root / "home"),
                  "ledger": str(root / "ledger.db"), "counter": str(root / "counter.log")}
        if overlay is not None:
            config["overlay"] = list(overlay)
        for sub in ("raw", "home", "workspace", "evidence", "check-scratch"):
            (root / sub).mkdir(parents=True, exist_ok=True)
        return fault_driver._Scenario(config)

    def test_intended_executions_counts_one_check_for_an_interrupted_round(self):
        """Kills the R19-ACCEPT-A-C03-R1 formula: every round, interrupted or not, counted all its checks.

        The C03 heavy run had four cmake rounds of two checks; the first round was killed while its debug
        check held, so its release check never started and the counter held 7 lines, not 8.
        """
        scenario = self.scenario(self.make_repo(), "counted", None)
        scenario.checks_per_round = 2
        outcomes = ["interrupted", "fail", "pass", "pass"]
        scenario.check_rounds = len(outcomes)
        scenario.attempts = [{"path": f"a{i}", "outcome": o, "request_id": f"check-{i}"}
                             for i, o in enumerate(outcomes, 1)]
        self.assertEqual(scenario.intended_executions(), 7)
        scenario.attempts[0]["outcome"] = "fail"
        self.assertEqual(scenario.intended_executions(), 8, "no interrupted round: every check of every round")
        scenario.checks_per_round = 1
        scenario.attempts[0]["outcome"] = "interrupted"
        self.assertEqual(scenario.intended_executions(), 4, "one check per round: an interrupt changes nothing")

    def test_export_overlay_copies_working_tree_bytes_of_the_named_paths(self):
        """Kills R1o: the overlay loop is skipped, or a path is written from the HEAD blob."""
        repo = self.make_repo()
        plain = self.scenario(repo, "plain", None)
        plain.prepare_cmake()
        self.assertEqual((plain.export / self.LEDGER_CMAKE).read_bytes(), b"head ledger cmake\n")
        self.assertFalse((plain.export / self.FIXTURE).exists(), "no overlay means no fixture in the export")
        overlaid = self.scenario(repo, "overlaid", self.OVERLAY)
        overlaid.prepare_cmake()
        self.assertEqual((overlaid.export / self.LEDGER_CMAKE).read_bytes(), b"working tree ledger cmake\n")
        self.assertEqual((overlaid.root / "ledger/CMakeLists.txt").read_bytes(), b"working tree ledger cmake\n")
        self.assertEqual((overlaid.export / self.FIXTURE).read_bytes(), b"working tree schema\n")
        self.assertEqual((overlaid.export / "src/engine/value/CMakeLists.txt").read_bytes(),
                         b"head value cmake\n", "paths outside the overlay keep the HEAD bytes")

    def test_export_overlay_refuses_a_path_missing_from_the_working_tree(self):
        """Kills R1p: the missing-file branch is skipped silently, or raises after writing."""
        repo = self.make_repo()
        (repo / self.FIXTURE).unlink()
        scenario = self.scenario(repo, "missing", self.OVERLAY)
        with self.assertRaises(lib.AcceptError) as caught:
            scenario.prepare_cmake()
        self.assertIn(self.FIXTURE, str(caught.exception))
        written = [p for p in scenario.export.rglob("*") if p.is_file()] if scenario.export.exists() else []
        self.assertEqual(written, [], "nothing is written to the export before the refusal")


class SchemaFixtureContextTest(unittest.TestCase):
    """C03 R1a, R1b, R1k, R1q: executable observers of the clean-export and Linux-context rows.

    Each observer reads the working tree through git and shell only; none runs the runner, Docker or a build.
    """

    FIXTURE = "fixtures/ledger/expected-schema.sql"
    OLD_COPY = "tests/native/r12b-evidence/expected-schema.sql"

    def git(self, *args):
        return subprocess.run(["git", "-C", str(REPO), *args], capture_output=True, check=False)

    def export_list(self):
        """The clean-export file list: the files a fresh public checkout plus new files would hold."""
        out = self.git("ls-files", "-z", "--cached", "--others", "--exclude-standard")
        self.assertEqual(out.returncode, 0, out.stderr)
        return set(out.stdout.decode().split("\0")) - {""}

    def test_ledger_schema_compile_definition_names_a_file_in_the_clean_export(self):
        """Kills R1b: PLANAR_LEDGER_SCHEMA points back at an ignored path a clean export lacks."""
        import re
        text = (REPO / "src/engine/ledger/CMakeLists.txt").read_text()
        match = re.search(r'PLANAR_LEDGER_SCHEMA="\$\{PROJECT_SOURCE_DIR\}/([^"]+)"', text)
        self.assertIsNotNone(match, "the format tests must define PLANAR_LEDGER_SCHEMA")
        rel = match.group(1)
        self.assertTrue((REPO / rel).is_file(), rel + " must exist in the working tree")
        self.assertTrue(rel in self.export_list(), rel + " is git-ignored: a clean export lacks it and SCHEMA612 fails")

    def test_schema_fixture_is_listed_not_ignored_and_matches_the_evidence_copy(self):
        """Kills R1a: the fixture is missing, drifts by one byte, or sits under an ignored path."""
        fixture = REPO / self.FIXTURE
        self.assertTrue(fixture.is_file(), self.FIXTURE + " must exist")
        self.assertEqual(self.git("check-ignore", "-q", self.FIXTURE).returncode, 1, "the fixture must not be ignored")
        self.assertTrue(self.FIXTURE in self.export_list(), "the fixture must be in the clean-export list")
        old = REPO / self.OLD_COPY
        if old.exists():
            self.assertEqual(fixture.read_bytes(), old.read_bytes(), "the fixture must equal the evidence copy")

    def test_testing_doc_no_longer_calls_the_schema_ignored_or_unrunnable(self):
        """Kills R1k: either stale passage of docs/testing.md is left."""
        import re
        text = (REPO / "docs/testing.md").read_text()
        self.assertTrue("r12b-evidence/expected-schema" not in text, "docs/testing.md still names the old schema path")
        self.assertIsNone(re.search(r"src/engine/CMakeLists\.txt. points", text))

    def test_no_ignored_file_survives_the_linux_context_exclusion_mirror(self):
        """Kills R1q: is_excluded keeps an ignored file (the old schema path) in the Linux context."""
        runner = (REPO / "scripts/linux-gate.sh").read_text()
        start = runner.index("is_excluded() {")
        func = runner[start:runner.index("\n}\n", start) + 3]
        ignored = self.git("ls-files", "-z", "--others", "--ignored", "--exclude-standard")
        self.assertEqual(ignored.returncode, 0, ignored.stderr)
        script = func + 'while IFS= read -r -d "" p; do is_excluded "$p" || printf "%s\\n" "$p"; done'
        proc = subprocess.run(["bash", "-c", script], input=ignored.stdout, capture_output=True, check=False)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        kept = proc.stdout.decode().splitlines()
        self.assertEqual(kept, [], "ignored files the Linux context would still include")


if __name__ == "__main__":
    unittest.main()
