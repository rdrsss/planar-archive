#!/usr/bin/env python3
# R19-ACCEPT-A acceptance command tests (AcceptanceCliTest): rows A1-A7 of
# plan/INVARIANTS.md. Each test runs run_acceptance.py as a subprocess with a
# per-test temporary root and scratch cwd; the standard repository tool
# scripts/redact-local-paths.py is run read-only from the checkout. The live
# rows (A6) need PLANAR_CANDIDATE and FAIL, never skip, when it is unset.
import sys

sys.dont_write_bytecode = True

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
import run_acceptance  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
RUN = HERE / "run_acceptance.py"
STANDARD = REPO / "scripts/redact-local-paths.py"
PY = sys.executable


def sha256(data):
    return hashlib.sha256(data).hexdigest()


# Synthetic suite modules for the suites command (S1-S3). Each module is a plain unittest module in its own
# support directory; the child that runs it is started by run_acceptance.py suites.
SKIPPED_SOURCE = ("import unittest\n\nclass SkippedTest(unittest.TestCase):\n"
                  "    def test_one(self):\n        self.skipTest('skipped on purpose')\n")
FAILING_SOURCE = ("import unittest\n\nclass FailingTest(unittest.TestCase):\n"
                  "    def test_one(self):\n        self.assertEqual(1, 2)\n")
PASSING_SOURCE = ("import unittest\n\nclass PassingTest(unittest.TestCase):\n"
                  "    def test_one(self):\n        self.assertTrue(True)\n")
IMPORT_ERROR_SOURCE = "import s2_no_such_module_for_suites\n"
ZERO_TESTS_SOURCE = "VALUE = 1\n"

# Top-level keys of a rerun result that rerun-index requires (D38); R2 drops each one in turn.
REQUIRED_RERUN_KEYS = ("faults", "reads", "takeover", "fenced", "verdicts", "status_review_blockers", "load",
                       "replays", "ledger_before_replays", "ledger_after_replays", "counter_lines",
                       "intended_executions", "history", "trajectory")

# Sections R1 must find in the index. Each entry is a value the synthetic result carries for that section.
RERUN_SECTIONS = (
    ("interrupt and check-recover", ("sentinel-reservation-open", "sentinel-recover-outcome")),
    ("recover event naming check_interrupted", ("check_interrupted",)),
    ("restart reads", ("sentinel-read-before-kill", "sentinel-read-after-restart")),
    ("takeover generations", ("40416", "40417")),
    ("fenced old token", ("sentinel-fenced-error", "sentinel-fenced-ledger")),
    ("reject1 admitted", ("sentinel-reject1-blocker",)),
    ("reject2 capped with a max_rounds blocker", ("max_rounds",)),
    ("load raising max_rounds", ("40418",)),
    ("final approval accepted", ("accepted",)),
    ("replays identical", ("verdict-approve",)),
    ("counter unchanged", ("40419",)),
    ("ledger digests equal", ("sentinel-ledger-digest",)),
    ("trajectory", ("sentinel-reviewer-run",)),
)


def suite_module_source(record, count):
    """Return a passing synthetic unittest module with count tests that records sys.executable at import."""
    lines = ["import sys", "import unittest", "",
             "with open(%r, 'w', encoding='utf-8') as stream:" % str(record),
             "    stream.write(sys.executable)", "", "",
             "class SyntheticTest(unittest.TestCase):"]
    for index in range(count):
        lines += ["    def test_%d(self):" % index, "        self.assertTrue(True)"]
    return "\n".join(lines) + "\n"


def live_module_source(marker):
    """Return a synthetic live module that appends a marker line whenever it is imported by a child run."""
    return ("import unittest\n"
            "with open(%r, 'a', encoding='utf-8') as stream:\n"
            "    stream.write('ran\\n')\n"
            "class LiveSyntheticTest(unittest.TestCase):\n"
            "    def test_marker(self):\n"
            "        self.assertTrue(True)\n") % str(marker)


class AcceptanceCliTest(unittest.TestCase):
    def setUp(self):
        self.tmp = Path(tempfile.mkdtemp(prefix="r19-accept-cli-"))
        self.addCleanup(shutil.rmtree, str(self.tmp), True)
        self.home = self.tmp / "home-fixture"
        self.scratch = self.tmp / "scratch-fixture"
        self.temp = self.tmp / "temp-fixture"

    def write(self, path, data):
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(data if isinstance(data, bytes) else data.encode())
        return path

    def env(self):
        env = os.environ.copy()
        env["PYTHONDONTWRITEBYTECODE"] = "1"
        return env

    def cli(self, *args):
        return subprocess.run([PY, str(RUN)] + [str(a) for a in args], env=self.env(),
                              cwd=str(self.tmp), capture_output=True, text=True, timeout=600)

    def index_fixture(self):
        """Run the index command over a raw tree that mentions repo, home, scratch and temp prefixes."""
        raw = self.tmp / "raw" / "quota"
        self.write(raw / "0001-check.stdout",
                   "ok %s/out.txt %s/tmp.txt %s/src/core.cpp\n" % (self.scratch, self.temp, REPO))
        self.write(raw / "0002-check.stderr", "warn %s/cache\n" % self.home)
        # The evidence tree is also the working directory, so the standard manifest sits at tree/tests/native.
        out = self.tmp / "tree"
        (out / "tests" / "native").mkdir(parents=True)
        proc = self.cli("index", "--raw-root", raw, "--out-dir", out, "--repo-path", REPO,
                        "--home", self.home, "--work-cwd", out, "--scratch", self.scratch,
                        "--temp-root", self.temp)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        return raw, out, out

    # A1
    def test_scan_command_fails_on_host_path_in_committed_evidence(self):
        """Kills: cmd_scan removes the accept_lib.scan_host_paths call, or scans only the top directory."""
        clean = self.tmp / "clean"
        self.write(clean / "top.txt", "plain text with <repo>/x and <home>/y\n")
        self.write(clean / "b" / "c.json", json.dumps({"where": "<scratch-root>/run"}))
        proc = self.cli("scan", "--root", clean, "--repo-path", REPO, "--home", self.home,
                        "--scratch", self.scratch)
        self.assertEqual(proc.returncode, 0, "a clean tree must pass: " + proc.stderr)
        dirty = self.tmp / "dirty"
        self.write(dirty / "top.txt", "clean\n")
        self.write(dirty / "nested" / "deep" / "note.json", json.dumps({"home": str(self.home) + "/cache"}))
        proc = self.cli("scan", "--root", dirty, "--repo-path", REPO, "--home", self.home,
                        "--scratch", self.scratch)
        self.assertNotEqual(proc.returncode, 0, "a host path in a nested JSON string must fail")
        self.assertIn("nested/deep/note.json", proc.stdout + proc.stderr)
        raw_case = self.tmp / "raw-case"
        self.write(raw_case / "clean.txt", "clean\n")
        self.write(raw_case / "raw" / "run.log", "ran in %s\n" % REPO)
        proc = self.cli("scan", "--root", raw_case, "--repo-path", REPO, "--home", self.home,
                        "--scratch", self.scratch)
        self.assertNotEqual(proc.returncode, 0, "unlisted raw logs are scanned")
        proc = self.cli("scan", "--root", raw_case, "--repo-path", REPO, "--home", self.home,
                        "--scratch", self.scratch, "--exclude", "raw/run.log")
        self.assertEqual(proc.returncode, 0, "only the explicit path list is ignored: " + proc.stderr)

    # A2
    def test_index_is_redacted_with_recorded_hashes_and_excludes_raw(self):
        """Kills: cmd_index writes unredacted text, records scratch rules in tests/native/path-redaction.json, skips redact-local-paths.py, or copies raw logs."""
        raw, out, work = self.index_fixture()
        temp_record = out / "TEMP-PATH-REDACTION.json"
        self.assertTrue(temp_record.is_file(), "index must write the round-local TEMP-PATH-REDACTION.json")
        redaction = json.loads(temp_record.read_text())
        files = redaction.get("files", {})
        self.assertTrue(files, "the round-local TEMP-PATH-REDACTION.json must record the indexed files")
        for name, entry in files.items():
            with self.subTest(file=name):
                for key in ("original_sha256", "redacted_sha256_before_standard", "bytes_before", "bytes_after"):
                    self.assertIn(key, entry)
        manifest = work / "tests/native/path-redaction.json"
        self.assertTrue(manifest.is_file(), "the standard repo/home manifest is written by the standard tool")
        manifest_data = json.loads(manifest.read_text())
        self.assertTrue(set(manifest_data.get("tokens", {})) <= {"<repo>", "<home>"},
                        "the standard manifest holds only repo and home tokens")
        self.assertNotIn(str(self.scratch), manifest.read_text())
        self.assertNotIn("<scratch-root>", manifest.read_text())
        outputs = [p for p in out.rglob("*") if p.is_file()]
        self.assertTrue(outputs)
        combined = "\n".join(p.read_text() for p in outputs)
        self.assertIn("<scratch-root>", combined, "scratch prefixes are replaced by their token")
        self.assertIn("<temp-root>", combined)
        self.assertIn("<repo>", combined, "the standard tool redacts the repository root")
        for prefix in (str(self.scratch), str(self.temp), str(REPO)):
            self.assertNotIn(prefix, combined)
        check = subprocess.run([PY, str(STANDARD), "check", "--repo-path", str(REPO), "--home", str(self.home)]
                               + [str(p) for p in outputs], cwd=str(work), env=self.env(),
                               capture_output=True, text=True)
        self.assertEqual(check.returncode, 0, check.stdout + check.stderr)
        copied = {p.name for p in outputs}
        for raw_file in raw.iterdir():
            self.assertNotIn(raw_file.name, copied, "raw logs are never copied")

    # A3
    def test_limits_command_requires_exact_closed_diagnostic(self):
        """Kills: cmd_limits records any nonzero exit without calling accept_lib.closed_capture_limit_ok."""
        stub = self.write(self.tmp / "engine_stub.py", (
            "import json, sys\n"
            "mode = sys.argv[1]\n"
            "diag = {'code': 'resource_limit', 'message': 'Lua instruction limit exceeded', 'field': '',\n"
            "        'details': {'limit': 'instructions', 'maximum': 100000000}}\n"
            "if mode == 'good':\n"
            "    sys.stderr.write(json.dumps(diag)); sys.exit(3)\n"
            "if mode == 'other_maximum':\n"
            "    diag['details']['maximum'] = 20000000; sys.stderr.write(json.dumps(diag)); sys.exit(3)\n"
            "if mode == 'other_code':\n"
            "    diag['code'] = 'schema'; sys.stderr.write(json.dumps(diag)); sys.exit(3)\n"
            "if mode == 'stdout':\n"
            "    sys.stdout.write('partial'); sys.stderr.write(json.dumps(diag)); sys.exit(3)\n"
            "sys.stderr.write(json.dumps(diag)); sys.exit(2)\n"))
        out = self.tmp / "limits-good"
        proc = self.cli("limits", "--out-dir", out, "--engine-argv", PY, "--engine-argv", stub,
                        "--engine-argv", "good")
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertTrue((out / "limits.json").is_file(), "the closed reproduction must be recorded")
        record = json.loads((out / "limits.json").read_text())
        self.assertTrue(record.get("closed") is True)
        self.assertEqual(record.get("exit"), 3)
        for mode in ("other_maximum", "other_code", "stdout", "exit2"):
            with self.subTest(mode=mode):
                out_bad = self.tmp / ("limits-" + mode)
                proc = self.cli("limits", "--out-dir", out_bad, "--engine-argv", PY,
                                "--engine-argv", stub, "--engine-argv", mode)
                self.assertNotEqual(proc.returncode, 0)
                record_path = out_bad / "limits.json"
                if record_path.exists():
                    self.assertFalse(json.loads(record_path.read_text()).get("closed") is True)

    # A4
    def test_packet_command_records_literal_source_lines(self):
        """Kills: cmd_packet records fixed strings instead of the literal lines of the source file."""
        source = self.write(self.tmp / "render.lua",
                            "line one\n  local observed = 'OBSERVED-MARK'\nline three\nline four\n")
        out = self.tmp / "packet"
        proc = self.cli("packet", "--source", source, "--lines", "2,4",
                        "--observed-text", "packet says OBSERVED-MARK here", "--marker", "OBSERVED-MARK",
                        "--out-dir", out)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        self.assertTrue((out / "packet.json").is_file(), "the packet record must be written")
        record = json.loads((out / "packet.json").read_text())
        self.assertEqual(record.get("lines"), {"2": "  local observed = 'OBSERVED-MARK'", "4": "line four"})
        self.assertEqual(record.get("observed"), "packet says OBSERVED-MARK here")
        beyond = self.cli("packet", "--source", source, "--lines", "99",
                          "--observed-text", "OBSERVED-MARK", "--marker", "OBSERVED-MARK",
                          "--out-dir", self.tmp / "packet-beyond")
        self.assertNotEqual(beyond.returncode, 0, "a line number outside the file must fail")
        unmarked = self.cli("packet", "--source", source, "--lines", "2",
                            "--observed-text", "no marker text", "--marker", "OBSERVED-MARK",
                            "--out-dir", self.tmp / "packet-unmarked")
        self.assertNotEqual(unmarked.returncode, 0, "observed text without the marker must fail")

    # A5
    def test_retained_dir_never_overwrites_existing_runs(self):
        """Kills: retained_dir removes and recreates an existing directory, or creates runs under a temporary directory instead of --raw-root."""
        raw = self.tmp / "raw"
        raw.mkdir()
        other = self.write(raw / "other" / "keep.txt", b"other bytes")
        first = self.cli("retained-dir", "--raw-root", raw, "--name", "run-a")
        self.assertEqual(first.returncode, 0, first.stderr)
        run = raw / "run-a"
        self.assertTrue(run.is_dir(), "the run directory is created under --raw-root")
        keep = self.write(run / "keep.txt", b"first run bytes")
        second = self.cli("retained-dir", "--raw-root", raw, "--name", "run-a")
        self.assertNotEqual(second.returncode, 0, "an existing run directory is refused")
        self.assertEqual(keep.read_bytes(), b"first run bytes")
        self.assertEqual(other.read_bytes(), b"other bytes")

    # A6
    def test_live_commands_abort_before_first_action_on_candidate_mismatch(self):
        """Kills: run_acceptance quota or mini-fault removes the require_recorded_candidate call before the first action."""
        if not os.environ.get("PLANAR_CANDIDATE"):
            self.fail("PLANAR_CANDIDATE must name the D28 candidate binary; live rows fail, never skip")
        stub_dir = self.tmp / "stub"
        stub_dir.mkdir()
        sentinel = stub_dir / "sentinel.txt"
        stub = stub_dir / "candidate"
        stub.write_text("#!%s\nimport sys\nopen(%r, 'a').write('ran\\n')\n" % (PY, str(sentinel)))
        stub.chmod(0o755)
        record = stub_dir / "candidate.json"
        record.write_text(json.dumps({"head": "0" * 40, "sha256": "0" * 64, "size": 1}))
        cases = [["quota", "--grouping", "combined"], ["mini-fault"]]
        for case in cases:
            with self.subTest(command=case[0]):
                raw = self.tmp / ("raw-" + case[0])
                raw.mkdir()
                proc = self.cli(case[0], "--candidate", stub, "--record", record, "--repo-path", REPO,
                                "--raw-root", raw, *case[1:])
                self.assertNotEqual(proc.returncode, 0,
                                    "a candidate that differs from the record must abort " + case[0])
                self.assertFalse(sentinel.exists(), "the stub candidate must never run")
                self.assertEqual(list(raw.rglob("*.argv.json")), [], "no action file may exist")

    # A7
    def test_verify_redaction_runs_standard_tool_and_rejects_scratch_tokens_in_manifest(self):
        """Kills: cmd_verify_redaction skips the redact-local-paths.py verify subprocess, or ignores its exit status."""
        raw, out, work = self.index_fixture()
        args = ["verify-redaction", "--tree", out, "--repo-path", REPO, "--home", self.home,
                "--scratch", self.scratch, "--temp-root", self.temp]
        good = self.cli(*args)
        self.assertEqual(good.returncode, 0, "a tree redacted through index must verify: " + good.stderr)
        manifest = out / "tests/native/path-redaction.json"
        self.assertTrue(manifest.is_file(), "the verified tree must hold the standard manifest")
        original_manifest = manifest.read_text()
        data = json.loads(original_manifest)
        data.setdefault("files", {})["scratch-note.txt"] = {
            "original_sha256": "a" * 64, "redacted_sha256": "b" * 64, "note": "<scratch-root>/x"}
        manifest.write_text(json.dumps(data))
        rejected = self.cli(*args)
        self.assertNotEqual(rejected.returncode, 0, "a scratch token in the manifest must fail")
        manifest.write_text(original_manifest)
        data = json.loads(original_manifest)
        candidates = [p for p in out.rglob("*") if p.is_file() and p.name != "TEMP-PATH-REDACTION.json"
                      and p != manifest]
        self.assertTrue(candidates, "the redacted tree must hold indexed files")
        target = candidates[0]
        data.setdefault("files", {})[str(target)] = {
            "original_sha256": "c" * 64, "redacted_sha256": sha256(target.read_bytes())}
        manifest.write_text(json.dumps(data))
        mismatch = self.cli(*args)
        self.assertNotEqual(mismatch.returncode, 0, "the standard verify exit status must propagate")

    # A2 and A7, second pass (mutation probing)
    def test_index_fails_when_a_host_prefix_survives_redaction(self):
        """Kills E6: cmd_index skips the residual host-prefix scan after the standard redaction."""
        raw = self.tmp / "raw" / "quota"
        self.write(raw / "0001-note.stdout", "owner path /Users/someone-else/private/file\n")
        out = self.tmp / "tree"
        (out / "tests" / "native").mkdir(parents=True)
        proc = self.cli("index", "--raw-root", raw, "--out-dir", out, "--repo-path", REPO,
                        "--home", self.home, "--work-cwd", out, "--scratch", self.scratch,
                        "--temp-root", self.temp)
        self.assertNotEqual(proc.returncode, 0, "an unredacted host prefix must fail the index")
        self.assertIn("host prefix remains after redaction", proc.stderr)

    def verify_args(self, out):
        return ["verify-redaction", "--tree", out, "--repo-path", REPO, "--home", self.home,
                "--scratch", self.scratch, "--temp-root", self.temp]

    def test_verify_redaction_checks_original_hash_and_real_prefixes(self):
        """Kills E8 (original-hash check skipped), E10 (manifest real-prefix check removed) and E11 (TEMP record real-prefix check removed)."""
        raw, out, work = self.index_fixture()
        args = self.verify_args(out)
        self.assertEqual(self.cli(*args).returncode, 0)
        record_path = out / "TEMP-PATH-REDACTION.json"
        manifest = out / "tests/native/path-redaction.json"
        original_record = record_path.read_text()
        original_manifest = manifest.read_text()

        record = json.loads(original_record)
        name = sorted(record["files"])[0]
        record["files"][name]["original_sha256"] = "d" * 64
        record_path.write_text(json.dumps(record, indent=1, sort_keys=True) + "\n")
        proc = self.cli(*args)
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("temporary restoration does not reproduce the original hash", proc.stderr)
        record_path.write_text(original_record)

        data = json.loads(original_manifest)
        data["note"] = "real prefix " + str(self.temp) + "/leak"
        manifest.write_text(json.dumps(data))
        proc = self.cli(*args)
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("path-redaction.json holds a real temporary prefix", proc.stderr)
        manifest.write_text(original_manifest)

        record = json.loads(original_record)
        record["note"] = "real prefix " + str(self.temp) + "/leak"
        record_path.write_text(json.dumps(record, indent=1, sort_keys=True) + "\n")
        proc = self.cli(*args)
        self.assertNotEqual(proc.returncode, 0)
        self.assertIn("TEMP-PATH-REDACTION.json holds a real temporary prefix", proc.stderr)
        record_path.write_text(original_record)
        self.assertEqual(self.cli(*args).returncode, 0, "the restored tree verifies again")

    def run_cli_env(self, env, *args):
        """Run run_acceptance.py with an explicit environment, with the per-test temporary root as cwd."""
        return subprocess.run([PY, str(RUN)] + [str(a) for a in args], env=env, cwd=str(self.tmp),
                              capture_output=True, text=True, timeout=600)

    def rerun_result(self):
        """Return a synthetic run_scenario result.json that carries every D38 field and passes every predicate."""
        events = (("interrupt", "check_reserved", "check-1"),
                  ("recover", "check_interrupted", "recover-1"),
                  ("takeover", "run_taken_over", "takeover-1"),
                  ("reject", "verdict_recorded", "verdict-reject1"),
                  ("capped", "verdict_recorded", "verdict-reject2"),
                  ("load", "declaration_accepted", "load-2"),
                  ("approve", "verdict_recorded", "verdict-approve"))
        history = [{"label": label, "event_id": str(9101 + 2 * index), "matched": True,
                    "expansion": {"data": {"event": {"type": kind, "request_id": request}}},
                    "expected": {"data.event.type": kind, "data.event.request_id": request}}
                   for index, (label, kind, request) in enumerate(events)]
        replay_order = (("activate-1", "activate"), ("claim-builder-1", "claim"), ("check-2", "check"),
                        ("verdict-reject1", "verdict"), ("check-4", "check"), ("verdict-approve", "verdict"))
        replays = [{"request": request, "verb": verb, "response_identical": True, "ledger_identical": True}
                   for request, verb in replay_order]
        return {
            "faults": {
                "interrupt": {"rendezvous_seen": True, "child_dead": True, "cli_returncode": -9,
                              "reservation_outcome": "sentinel-reservation-open", "child_pgid": 61001},
                "recover": {"exit": 0, "argv": ["check-recover", "--request-id", "recover-1"],
                            "outcome": "sentinel-recover-outcome"},
            },
            "reads": {"before_kill": {"status": "sentinel-read-before-kill"},
                      "after_restart": {"status": "sentinel-read-after-restart"}},
            "takeover": {"exit": 0, "generation_before": 40416, "generation_after": 40417},
            "fenced": {"token_role": "old", "exit": 1, "error_code": "sentinel-fenced-error",
                       "ledger_before": "sentinel-fenced-ledger", "ledger_after": "sentinel-fenced-ledger"},
            "verdicts": {
                "reject1": {"decision": "reject", "exit": 0, "repair_admitted": True, "capped": False,
                            "count_before": 0, "max_rounds": 1, "acceptance": None,
                            "blockers_open": ["sentinel-reject1-blocker"], "targets_equal_latest_output": True},
                "reject2": {"decision": "reject", "exit": 0, "repair_admitted": False, "capped": True,
                            "count_before": 1, "max_rounds": 1, "acceptance": None,
                            "blockers_open": ["max_rounds"], "targets_equal_latest_output": True},
                "final": {"decision": "approve", "exit": 0, "acceptance": "accepted",
                          "blockers_open": [], "targets_equal_latest_output": True},
            },
            "status_review_blockers": [{"kind": "max_rounds"}],
            "load": {"exit": 0, "max_rounds_before": 1, "max_rounds_after": 40418,
                     "blockers_cleared": ["max_rounds"], "work_fp_rebound": True},
            "replays": replays,
            "ledger_before_replays": "sentinel-ledger-digest",
            "ledger_after_replays": "sentinel-ledger-digest",
            "intended_executions": 40419,
            "counter_lines": 40419,
            "history": history,
            "trajectory": {"valid": True, "events": [{"kind": "check_interrupted", "via": "check-recover"}],
                           "reviewer_runs": ["sentinel-reviewer-run"]},
        }

    # S1
    def test_suites_command_reports_per_module_and_total_counts(self):
        """Kills: cmd_suites writes only the total, reports one count for every module, sums wrong, or starts children with a bare python3."""
        support = self.tmp / "support-s1"
        alpha_record = self.tmp / "interpreter-alpha.txt"
        beta_record = self.tmp / "interpreter-beta.txt"
        self.write(support / "suite_alpha_s1.py", suite_module_source(alpha_record, 3))
        self.write(support / "suite_beta_s1.py", suite_module_source(beta_record, 2))
        shim = self.write(self.tmp / "shim" / "python3", "#!/bin/sh\nexit 99\n")
        shim.chmod(0o755)
        env = self.env()
        env["PATH"] = str(shim.parent) + os.pathsep + env.get("PATH", "")
        out = self.tmp / "suites-s1.txt"
        proc = self.run_cli_env(env, "suites", "--support-dir", support, "--module", "suite_alpha_s1",
                                "--module", "suite_beta_s1", "--out", out)
        self.assertEqual(proc.returncode, 0, "two passing modules must exit 0: " + proc.stderr)
        self.assertEqual(proc.stdout.splitlines(), [
            "suite_alpha_s1 ran=3 failures=0 errors=0 skipped=0 exit=0",
            "suite_beta_s1 ran=2 failures=0 errors=0 skipped=0 exit=0",
            "total ran=5 failures=0 errors=0 skipped=0",
        ])
        self.assertTrue(out.is_file(), "the --out file must be written")
        self.assertEqual(out.read_bytes(), proc.stdout.encode("utf-8"), "the --out file must equal stdout")
        for record in (alpha_record, beta_record):
            with self.subTest(record=record.name):
                self.assertTrue(record.is_file(), "the child module must run and record its interpreter")
                self.assertEqual(record.read_text(encoding="utf-8"), sys.executable,
                                 "children must run under the interpreter running the test, never a bare python3")

    # S2
    def test_suites_command_refuses_skips_failures_and_zero_matched(self):
        """Kills: cmd_suites ignores skipped, treats ran=0 as success, or ignores a nonzero child exit."""
        support = self.tmp / "support-s2"
        modules = (
            ("s2_skipped", SKIPPED_SOURCE, False),
            ("s2_failing", FAILING_SOURCE, False),
            ("s2_import_error", IMPORT_ERROR_SOURCE, False),
            ("s2_zero_matched", ZERO_TESTS_SOURCE, False),
            ("s2_passing", PASSING_SOURCE, True),
        )
        for name, source, _passes in modules:
            self.write(support / (name + ".py"), source)
        for name, _source, passes in modules:
            with self.subTest(module=name):
                out = self.tmp / ("suites-" + name + ".txt")
                proc = self.cli("suites", "--support-dir", support, "--module", name, "--out", out)
                if passes:
                    self.assertEqual(proc.returncode, 0, "a passing module must exit 0: " + proc.stderr)
                else:
                    self.assertNotEqual(proc.returncode, 0, "a module that does not fully pass must exit nonzero")
                    self.assertIn(name, proc.stderr, "the refusal must name the module")

    # S3
    def test_suites_command_live_modules_require_the_recorded_candidate(self):
        """Kills: cmd_suites skips require_recorded_candidate for live modules (a live child that starts writes the marker)."""
        stub_dir = self.tmp / "s3-stub"
        candidate = self.write(stub_dir / "candidate", "#!%s\nimport sys\nopen(%r, 'a').write('ran\\n')\n" % (
            PY, str(stub_dir / "candidate-ran.txt")))
        candidate.chmod(0o755)
        record = self.write(stub_dir / "candidate.json",
                            json.dumps({"head": "0" * 40, "sha256": "0" * 64, "size": 1}))
        cases = (
            ("candidate_unset", None, record),
            ("hash_mismatch", candidate, record),
            ("record_missing", candidate, None),
        )
        for name, exe, rec in cases:
            with self.subTest(case=name):
                support = self.tmp / ("s3-" + name)
                marker = support / "live-marker.txt"
                self.write(support / "s3_live.py", live_module_source(marker))
                env = self.env()
                env.pop("PLANAR_CANDIDATE", None)
                if exe is not None:
                    env["PLANAR_CANDIDATE"] = str(exe)
                args = ["suites", "--support-dir", support, "--live-module", "s3_live",
                        "--out", self.tmp / ("suites-" + name + ".txt")]
                if rec is not None:
                    args += ["--record", rec]
                proc = self.run_cli_env(env, *args)
                self.assertNotEqual(proc.returncode, 0, "a live module must refuse without the recorded candidate")
                self.assertFalse(marker.exists(), "the live child must never start")
                self.assertTrue(any(word in proc.stderr.lower() for word in ("candidate", "record")),
                                "the refusal must name the candidate or the record: " + proc.stderr)

    # R1
    def test_rerun_index_records_every_required_fault_field(self):
        """Kills: cmd_rerun_index omits a section (for example fenced, load or trajectory) or copies fields without reading them; each required value of the synthetic result must appear in the index."""
        run = self.tmp / "rerun-r1"
        result = self.write(run / "result.json", json.dumps(self.rerun_result(), indent=1))
        out = run / "index.json"
        proc = self.cli("rerun-index", "--result", result, "--out", out)
        self.assertEqual(proc.returncode, 0, "a complete result must index: " + proc.stderr)
        self.assertTrue(out.is_file(), "the compact index must be written")
        text = out.read_text(encoding="utf-8")
        for section, sentinels in RERUN_SECTIONS:
            for sentinel in sentinels:
                with self.subTest(section=section, sentinel=sentinel):
                    self.assertIn(sentinel, text, "the index does not record the " + section + " section")

    # R2
    def test_rerun_index_fails_when_any_required_field_is_missing(self):
        """Kills: cmd_rerun_index treats a missing field as success, skips one predicate or writes the index before validating."""

        def drop_history(result, label):
            result["history"] = [entry for entry in result["history"] if entry["label"] != label]

        def retype_recover(result):
            for entry in result["history"]:
                if entry["label"] == "recover":
                    entry["expansion"]["data"]["event"]["type"] = "check_recorded"
                    entry["expected"]["data.event.type"] = "check_recorded"

        cases = [("missing " + key, (lambda r, k=key: r.pop(k)), (key,)) for key in REQUIRED_RERUN_KEYS]
        cases += [
            ("repair_admitted false", lambda r: r["verdicts"]["reject1"].update(repair_admitted=False),
             ("repair_admitted",)),
            ("reject1 capped", lambda r: r["verdicts"]["reject1"].update(capped=True), ("capped",)),
            ("reject2 not capped", lambda r: r["verdicts"]["reject2"].update(capped=False), ("capped",)),
            ("no max_rounds blocker", lambda r: r.update(status_review_blockers=[]),
             ("status_review_blockers", "max_rounds")),
            ("load not raising", lambda r: r["load"].update(max_rounds_after=r["load"]["max_rounds_before"]),
             ("max_rounds_after", "load")),
            ("load blocker not cleared", lambda r: r["load"].update(blockers_cleared=[]),
             ("blockers_cleared", "load")),
            ("targets_equal_latest_output false",
             lambda r: r["verdicts"]["final"].update(targets_equal_latest_output=False),
             ("targets_equal_latest_output",)),
            ("final not accepted", lambda r: r["verdicts"]["final"].update(acceptance="open"), ("final",)),
            ("non-identical replay", lambda r: r["replays"][2].update(response_identical=False),
             ("response_identical", "replays")),
            ("replay changed the ledger", lambda r: r["replays"][0].update(ledger_identical=False),
             ("ledger_identical", "replays")),
            ("counter_lines differ from intended_executions",
             lambda r: r.update(counter_lines=r["intended_executions"] + 1),
             ("counter_lines", "intended_executions")),
            ("ledger digests differ", lambda r: r.update(ledger_after_replays="sentinel-ledger-digest-moved"),
             ("ledger_before_replays", "ledger_after_replays", "ledger")),
            ("empty trajectory", lambda r: r.update(trajectory={"valid": True, "events": [], "reviewer_runs": []}),
             ("trajectory", "events")),
            ("interrupted child still alive", lambda r: r["faults"]["interrupt"].update(child_dead=False),
             ("child_dead", "interrupt")),
            ("check-recover refused", lambda r: r["faults"]["recover"].update(exit=1), ("recover",)),
            ("restart read missing", lambda r: r["reads"].pop("after_restart"), ("after_restart", "reads")),
            ("takeover did not advance",
             lambda r: r["takeover"].update(generation_after=r["takeover"]["generation_before"]),
             ("generation_after", "takeover")),
            ("old token not fenced", lambda r: r["fenced"].update(exit=0), ("fenced",)),
            ("fenced ledger changed", lambda r: r["fenced"].update(ledger_after="sentinel-fenced-ledger-moved"),
             ("ledger_after", "ledger_before", "fenced")),
            ("recover event names another type", retype_recover, ("check_interrupted", "recover", "history")),
            ("no recover history entry", lambda r: drop_history(r, "recover"),
             ("recover", "check_interrupted", "history")),
        ]
        for index, (label, mutate, tokens) in enumerate(cases):
            with self.subTest(case=label):
                result = self.rerun_result()
                mutate(result)
                folder = self.tmp / ("r2-%02d" % index)
                path = self.write(folder / "result.json", json.dumps(result, indent=1))
                out = folder / "index.json"
                proc = self.cli("rerun-index", "--result", path, "--out", out)
                self.assertNotEqual(proc.returncode, 0, "a result with a " + label + " must not index")
                self.assertTrue(any(token in proc.stderr for token in tokens),
                                "the refusal must name the field (one of %s): %s" % (", ".join(tokens), proc.stderr))
                self.assertFalse(out.exists(), "no index may be written when validation fails")

    def test_rerun_index_names_the_missing_top_level_key_itself(self):
        """Kills R1j-a: validate_rerun_result drops its required-key loop, so a missing top-level section is reported
        through a deeper dotted path (for example faults.interrupt) instead of by its own name."""
        import re
        for index, key in enumerate(REQUIRED_RERUN_KEYS):
            with self.subTest(key=key):
                result = self.rerun_result()
                result.pop(key)
                folder = self.tmp / ("r1j-%02d" % index)
                path = self.write(folder / "result.json", json.dumps(result, indent=1))
                proc = self.cli("rerun-index", "--result", path, "--out", folder / "index.json")
                self.assertNotEqual(proc.returncode, 0)
                self.assertRegex(proc.stderr, r"rerun result lacks " + re.escape(key) + r"(?![\w.])")

    def real_shaped_recover_argv(self):
        """Return an absolute argv shaped like fault_driver._run: candidate, verb, --ledger and the scratch ledger."""
        host = os.path.realpath(self.tmp)
        return [host + "/candidate-bin/planar", "check-recover", "--ledger", host + "/scratch/ledger.db",
                "--request-id", "recover-1"]

    def tree_snapshot(self, root):
        """Return {relative path: bytes} for every regular file under root; an absent root is empty."""
        root = Path(root)
        if not root.is_dir():
            return {}
        return {p.relative_to(root).as_posix(): p.read_bytes() for p in sorted(root.rglob("*")) if p.is_file()}

    # R1j (H1)
    def test_rerun_index_normalizes_real_shaped_absolute_argv(self):
        """Kills H1: build_rerun_index copies faults.recover.argv whole, so the absolute candidate and ledger paths of a real run are refused as host paths."""
        host = os.path.realpath(self.tmp)
        base = self.rerun_result()
        base_run = self.tmp / "rerun-h1-base"
        base_path = self.write(base_run / "result.json", json.dumps(base, indent=1))
        base_out = base_run / "index.json"
        proc = self.cli("rerun-index", "--result", base_path, "--out", base_out)
        self.assertEqual(proc.returncode, 0, "the synthetic result must index: " + proc.stderr)
        expected = json.loads(base_out.read_text(encoding="utf-8"))

        result = self.rerun_result()
        result["faults"]["recover"]["argv"] = self.real_shaped_recover_argv()
        run = self.tmp / "rerun-h1"
        path = self.write(run / "result.json", json.dumps(result, indent=1))
        out = run / "index.json"
        proc = self.cli("rerun-index", "--result", path, "--out", out)
        self.assertEqual(proc.returncode, 0, "a real-shaped absolute argv must index: " + proc.stderr)
        self.assertTrue(out.is_file(), "the compact index must be written")
        text = out.read_text(encoding="utf-8")
        for marker in tuple(run_acceptance.HOST_PATH_MARKERS) + (host,):
            self.assertNotIn(marker, text, "the rerun index holds a host path")
        expected["faults"]["recover"]["argv"] = ["<path>/planar", "check-recover", "--ledger",
                                                 "<path>/ledger.db", "--request-id", "recover-1"]
        self.assertEqual(json.loads(text), expected,
                         "only the argv paths change: the verb, the non-path elements and every other field are kept")

    # R1j guard (H3)
    def test_rerun_index_still_refuses_host_path_outside_argv(self):
        """Kills H3: cmd_rerun_index drops the HOST_PATH_MARKERS refusal for fields other than the normalized argv. Guard: passes on the unfixed tree."""
        host = os.path.realpath(self.tmp)
        result = self.rerun_result()
        result["fenced"]["ledger_before"] = host + "/scratch/ledger.db"
        result["fenced"]["ledger_after"] = host + "/scratch/ledger.db"
        run = self.tmp / "rerun-h3"
        path = self.write(run / "result.json", json.dumps(result, indent=1))
        out = run / "index.json"
        proc = self.cli("rerun-index", "--result", path, "--out", out)
        self.assertNotEqual(proc.returncode, 0, "a host path outside the argv must be refused")
        self.assertIn("would hold a host path", proc.stderr)
        self.assertFalse(out.exists(), "no index may be written with a host path")

    # A3 (H4)
    def test_limits_command_accepts_real_error_envelope(self):
        """Kills H4: cmd_limits compares the engine stderr with the bare closed object only, so the real error envelope is refused."""
        stub = self.write(self.tmp / "engine_envelope_stub.py", (
            "import json, sys\n"
            "mode = sys.argv[1]\n"
            "error = {'code': 'resource_limit', 'message': 'Lua instruction limit exceeded', 'field': '',\n"
            "         'details': {'limit': 'instructions', 'maximum': 100000000}}\n"
            "if mode == 'other_maximum':\n"
            "    error['details']['maximum'] = 20000000\n"
            "body = {'error': error, 'kind': 'error', 'version': 1}\n"
            "sys.stderr.write(json.dumps(body, sort_keys=True))\n"
            "sys.exit(3)\n"))
        out = self.tmp / "limits-envelope"
        proc = self.cli("limits", "--out-dir", out, "--engine-argv", PY, "--engine-argv", stub,
                        "--engine-argv", "envelope")
        self.assertEqual(proc.returncode, 0, "the real error envelope is the closed reproduction: " + proc.stderr)
        record = json.loads((out / "limits.json").read_text(encoding="utf-8"))
        self.assertTrue(record.get("closed") is True, "the closed reproduction must be recorded")
        self.assertEqual(record.get("exit"), 3)
        refused = self.tmp / "limits-envelope-other"
        proc = self.cli("limits", "--out-dir", refused, "--engine-argv", PY, "--engine-argv", stub,
                        "--engine-argv", "other_maximum")
        self.assertNotEqual(proc.returncode, 0, "an envelope with another maximum must be refused")
        self.assertFalse((refused / "limits.json").exists(), "no limits record for a refused envelope")

    # A2 (H8)
    def test_index_escapes_nul_next_to_host_path_in_binary_raw_file(self):
        """Kills H8: compact_index copies host-bearing lines of a binary raw file verbatim, so a NUL byte keeps the standard tool from redacting the index and the residual scan refuses it."""
        repo = str(self.tmp / "repo-fixture")
        home = str(self.home)
        data = (b"SQLite format 3\x00\x00page " + repo.encode() + b"/ledger\x00tail\n"
                + b"home " + home.encode() + b"/cache\n" + b"plain line\n")
        raw = self.tmp / "raw-binary"
        self.write(raw / "ledger.db", data)
        out = self.tmp / "tree-binary"
        (out / "tests" / "native").mkdir(parents=True)
        proc = self.cli("index", "--raw-root", raw, "--out-dir", out, "--repo-path", repo, "--home", home,
                        "--work-cwd", out)
        self.assertEqual(proc.returncode, 0, "a binary raw file with a NUL next to a host path must index: " + proc.stderr)
        index_file = out / "index" / "ledger.db.index"
        self.assertTrue(index_file.is_file(), "the index of the binary raw file must be written")
        text = index_file.read_bytes().decode("utf-8")
        self.assertFalse(any((ord(char) < 0x20 and char != "\n") or ord(char) == 0x7f for char in text),
                         "the index holds a NUL or control byte other than newline")
        for prefix in (repo, home):
            self.assertNotIn(prefix, text, "no host prefix may remain in the index")
        self.assertIn("<repo>", text)
        self.assertIn("<home>", text)
        hits = sum(1 for line in data.decode("utf-8", errors="replace").splitlines() if repo in line or home in line)
        self.assertIn("raw-bytes: %d\n" % len(data), text)
        self.assertIn("raw-sha256: " + sha256(data) + "\n", text)
        self.assertIn("host-lines: %d\n" % hits, text, "the hit count is that of the file")
        self.assertIn("\\x00", text, "the NUL is shown as an escape")

    # A2 (R1m)
    def test_index_lists_only_host_lines_and_refuses_out_dir_other_than_work_cwd(self):
        """Kills R1m-a (compact_index appends the raw text, so non-host lines reach the index) and R1m-b (an out
        directory different from the work directory is accepted)."""
        repo = str(self.tmp / "repo-fixture")
        home = str(self.home)
        raw = self.tmp / "raw-compact"
        self.write(raw / "note.txt", "unique-plain-line-sentinel\nhost " + repo + "/x\nanother-plain-sentinel\n")
        out = self.tmp / "tree-compact"
        (out / "tests" / "native").mkdir(parents=True)
        proc = self.cli("index", "--raw-root", raw, "--out-dir", out, "--repo-path", repo, "--home", home,
                        "--work-cwd", out)
        self.assertEqual(proc.returncode, 0, proc.stderr)
        text = (out / "index" / "note.txt.index").read_text(encoding="utf-8")
        self.assertIn("host-lines: 1\n", text)
        self.assertNotIn("plain-line-sentinel", text)
        self.assertNotIn("another-plain-sentinel", text)
        other_out = self.tmp / "tree-other-out"
        other_work = self.tmp / "tree-other-work"
        (other_out / "tests" / "native").mkdir(parents=True)
        (other_work / "tests" / "native").mkdir(parents=True)
        refused = self.cli("index", "--raw-root", raw, "--out-dir", other_out, "--repo-path", repo, "--home", home,
                           "--work-cwd", other_work)
        self.assertNotEqual(refused.returncode, 0, "an out directory other than the work directory must be refused")
        self.assertIn("--out-dir must be the working tree root", refused.stderr)
        self.assertFalse((other_out / "index").exists(), "nothing is written when the directories differ")

    # A2 (H9)
    def test_index_dry_run_refuses_unredactable_raw_file_and_writes_nothing(self):
        """Kills H9: cmd_index --dry-run skips the residual host-prefix scan or writes the index; the generic users prefix is built from parts so no source line holds it."""
        raw = self.tmp / "raw-dry"
        users = "/" + "Users/" + "someone-else"
        self.write(raw / "quota" / "0001-note.stdout", "owner path " + users + "/private/file\n")
        out = self.tmp / "tree-dry"
        (out / "tests" / "native").mkdir(parents=True)
        before = self.tree_snapshot(out)
        proc = self.cli("index", "--dry-run", "--raw-root", raw, "--out-dir", out, "--repo-path", REPO,
                        "--home", self.home, "--work-cwd", out)
        self.assertEqual(proc.returncode, 1, "a raw file that would keep a host prefix must refuse the dry run: " + proc.stderr)
        self.assertIn("0001-note.stdout", proc.stderr, "the refusal must name the raw file")
        self.assertEqual(self.tree_snapshot(out), before, "the dry run writes nothing under the out directory")
        self.assertFalse((out / "index").exists(), "no index directory after a refused dry run")
        self.assertFalse((out / run_acceptance.TEMP_RECORD).exists(), "no TEMP record after a refused dry run")

    # A2 (H10)
    def test_index_dry_run_reports_expected_count_and_writes_nothing(self):
        """Kills H10: cmd_index --dry-run miscounts the expected index files or writes under the out directory."""
        raw = self.tmp / "raw-clean"
        self.write(raw / "quota" / "0001-check.stdout", "plain output\n")
        self.write(raw / "quota" / "0002-check.stderr", "plain warning\n")
        self.write(raw / "0003-note.txt", "another plain note\n")
        expected = sum(1 for p in raw.rglob("*") if p.is_file())
        out = self.tmp / "tree-clean"
        (out / "tests" / "native").mkdir(parents=True)
        before = self.tree_snapshot(out)
        proc = self.cli("index", "--dry-run", "--raw-root", raw, "--out-dir", out, "--repo-path", REPO,
                        "--home", self.home, "--work-cwd", out)
        self.assertEqual(proc.returncode, 0, "a clean raw tree must pass the dry run: " + proc.stderr)
        counts = [line for line in proc.stdout.splitlines() if line.startswith("expected-index-files:")]
        self.assertEqual(counts, ["expected-index-files: %d" % expected],
                         "the dry run prints the number of regular files under the raw root")
        self.assertEqual(self.tree_snapshot(out), before, "the dry run writes nothing under the out directory")
        real = self.cli("index", "--raw-root", raw, "--out-dir", out, "--repo-path", REPO, "--home", self.home,
                        "--work-cwd", out)
        self.assertEqual(real.returncode, 0, real.stderr)
        written = sum(1 for p in (out / "index").rglob("*") if p.is_file())
        self.assertEqual(written, expected, "the one-shot index writes exactly the expected number of files")

    # R2 (C3-R2a, R2b, R2c): the commit-list check. Every listed file exists unless a test says otherwise, so
    # only the rule under test can fail.
    def commit_list(self, entries, files=None, name="repo"):
        """Create files (a mapping of relative path to text, default empty) and run commit-list over entries."""
        root = self.tmp / name
        root.mkdir(parents=True, exist_ok=True)
        created = dict(files) if files is not None else {}
        for entry in entries:
            if not Path(entry).is_absolute() and ".." not in Path(entry).parts:
                created.setdefault(entry, "text\n")
        for rel, text in created.items():
            self.write(root / rel, text)
        listing = self.tmp / (name + "-list.txt")
        listing.write_text("".join(entry + "\n" for entry in entries), encoding="utf-8")
        return self.cli("commit-list", "--list", listing, "--repo-root", root)

    def test_commit_list_check_accepts_a_list_of_existing_sorted_files(self):
        """Kills C3-R2a: the check rejects a clean list or prints no entry count (stdout must hold 3)."""
        proc = self.commit_list(["a/one.txt", "b/two.txt", "c/three.txt"])
        self.assertEqual(proc.returncode, 0, "a sorted list of existing files must pass: " + proc.stderr)
        self.assertRegex(proc.stdout, r"(^|\D)3(\D|$)", "stdout must hold the entry count 3")

    def test_commit_list_check_fails_naming_a_missing_path(self):
        """Kills C3-R2a: the existence check treats a missing path as present (exit 0) or omits the path."""
        root = self.tmp / "repo-missing"
        self.write(root / "a/one.txt", "x\n")
        listing = self.tmp / "missing-list.txt"
        listing.write_text("a/one.txt\nb/gone.txt\n", encoding="utf-8")
        proc = self.cli("commit-list", "--list", listing, "--repo-root", root)
        self.assertEqual(proc.returncode, 1, "a missing path must fail: " + proc.stdout)
        self.assertIn("b/gone.txt", proc.stderr, "stderr must name the missing path")

    def test_commit_list_check_refuses_unsorted_duplicate_raw_and_bytecode_paths(self):
        """Kills C3-R2b: dropping the sort, duplicate, raw, bytecode, log, absolute or parent rule, matching raw
        only for a component equal to .raw (summary.txt.raw/ passes) or as a substring (rawdata/ refused)."""
        absolute = str(self.tmp / "repo-abs" / "a.txt")
        refused = (
            ("unsorted", ["b.txt", "a.txt"], "unsorted"),
            ("duplicate", ["a.txt", "a.txt"], "duplicate"),
            ("raw-component", ["x/raw/f.txt"], "raw"),
            ("raw2-component", ["x/raw2/f.txt"], "raw"),
            ("dot-raw-component", ["x/.raw/f.txt"], "raw"),
            ("txt-raw-directory", ["gate/suites/summary.txt.raw/mod.txt"], "raw"),
            ("log", ["x/f.log"], "log"),
            ("pyc", ["x/f.pyc"], "bytecode"),
            ("pycache", ["x/__pycache__/f.txt"], "bytecode"),
            ("absolute", [absolute], "absolute"),
            ("parent", ["x/../a.txt"], "parent"),
        )
        for label, entries, word in refused:
            with self.subTest(rule=label):
                files = {"a.txt": "x\n", "b.txt": "x\n", "x/a.txt": "x\n"} if label == "parent" else None
                if label == "absolute":
                    self.write(absolute, "x\n")
                proc = self.commit_list(entries, files, name="repo-" + label)
                self.assertEqual(proc.returncode, 1, "must be refused: " + proc.stdout)
                self.assertIn(word, proc.stderr, "stderr must carry the rule word " + word)
                if label not in ("unsorted", "duplicate"):
                    self.assertIn(entries[0], proc.stderr, "stderr must name the path")
        for label, entry in (("rawdata", "x/rawdata/f.txt"), ("draw", "x/draw/f.txt")):
            with self.subTest(guard=label):
                proc = self.commit_list([entry], name="repo-guard-" + label)
                self.assertEqual(proc.returncode, 0, "a component that merely contains raw passes: " + proc.stderr)

    def test_commit_list_check_refuses_snapshot_original_and_unlisted_superseded_paths(self):
        """Kills C3-R2b: dropping the snapshot, original or superseded rule, or an allowance wider or narrower
        than the three closed paths."""
        refused = (
            ("snapshot", "a/snapshot-1/f.txt", "snapshot"),
            ("original", "x/redaction/original/f.txt", "original"),
            ("superseded", "tests/native/other/superseded-1/a.md", "superseded"),
            ("superseded-near-allowance", "tests/native/r19-accept-evidence/gate/candidate/superseded-1/other.json",
             "superseded"),
        )
        for label, entry, word in refused:
            with self.subTest(rule=label):
                proc = self.commit_list([entry], name="repo-" + label)
                self.assertEqual(proc.returncode, 1, "must be refused: " + proc.stdout)
                self.assertIn(word, proc.stderr, "stderr must carry the rule word " + word)
                self.assertIn(entry, proc.stderr, "stderr must name the path")
        allowed = ["tests/native/r19-accept-a-c03-evidence/plan/superseded-1/a.md",
                   "tests/native/r19-accept-evidence/gate/candidate/superseded-1/candidate.json",
                   "tests/native/r19-accept-evidence/plan/superseded-1/a.md"]
        allowed.sort()
        proc = self.commit_list(allowed, name="repo-allowed")
        self.assertEqual(proc.returncode, 0, "the closed allowance paths pass: " + proc.stderr)

    def test_commit_list_check_allows_redacted_index_mirrors_of_raw_directories_only(self):
        """Kills the closing allowance (C02-D28, L21): an index digest of a file under a raw run directory is
        refused, or the allowance also admits a non-index file, an index outside the evidence index tree or a
        raw log beside it."""
        index_entry = "tests/native/r19-accept-evidence/index/run-a/raw/001-claim.stdout.index"
        proc = self.commit_list([index_entry], name="repo-index-allowed")
        self.assertEqual(proc.returncode, 0, "a redacted index mirror of a raw directory passes: " + proc.stderr)
        refused = (
            ("not-index", "tests/native/r19-accept-evidence/index/run-a/raw/001-claim.stdout"),
            ("other-tree", "tests/native/other-evidence/index/run-a/raw/001-claim.stdout.index"),
            ("raw-log-tree", "tests/native/r19-accept-evidence/raw/run-a/raw/001-claim.stdout.index"),
        )
        for label, entry in refused:
            with self.subTest(rule=label):
                proc = self.commit_list([entry], name="repo-index-" + label)
                self.assertEqual(proc.returncode, 1, "must be refused: " + proc.stdout)
                self.assertIn("raw", proc.stderr, "stderr must carry the rule word raw")
                self.assertIn(entry, proc.stderr, "stderr must name the path")

    def test_commit_list_check_applies_the_snapshot_rule_to_directories_only(self):
        """Kills the closing narrowing: a file whose own name starts with snapshot (an index of
        snapshot_value.cpp) is refused, or a snapshot directory is no longer refused."""
        entry = "tests/native/r19-accept-evidence/index/run-a/base-engine/execution/snapshot_value.cpp.index"
        proc = self.commit_list([entry], name="repo-snapshot-file")
        self.assertEqual(proc.returncode, 0, "a file named snapshot_* outside a snapshot directory passes: " + proc.stderr)
        directory_entry = "tests/native/x/red/snapshot-2/untracked/a.py"
        proc = self.commit_list([directory_entry], name="repo-snapshot-dir")
        self.assertEqual(proc.returncode, 1, "a snapshot directory is refused: " + proc.stdout)
        self.assertIn("snapshot", proc.stderr)
        self.assertIn(directory_entry, proc.stderr)

    def test_commit_list_check_refuses_files_that_pin_report_hashes(self):
        """Kills C3-R2c: the REPORT hash scan skipped, or applied without requiring a REPORT word on the line."""
        pin = "REPORT.md sha256 " + "ab" * 32 + "\n"
        proc = self.commit_list(["a/pins.txt", "b/clean.txt"], {"a/pins.txt": pin, "b/clean.txt": "ok\n"},
                                name="repo-pin")
        self.assertEqual(proc.returncode, 1, "a REPORT hash pin must be refused: " + proc.stdout)
        self.assertIn("a/pins.txt", proc.stderr, "stderr must name the pinning file")
        self.assertIn("REPORT", proc.stderr)
        self.assertNotIn("b/clean.txt", proc.stderr)
        clean = self.commit_list(["a/src.txt"], {"a/src.txt": "src " + "cd" * 32 + "\nREPORT mention\n"},
                                 name="repo-nopin")
        self.assertEqual(clean.returncode, 0, "a hash without REPORT on its line passes: " + clean.stderr)


if __name__ == "__main__":
    unittest.main()
