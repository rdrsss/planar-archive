#!/usr/bin/env python3
# R19-ACCEPT-B-C01 checkpoint tests: rows C1-C24 of plan/INVARIANTS.md (A03 evidence).
# CheckpointDocTest is offline and checks the checkpoint object, the validator, the
# effect verifier and the manifest. CheckpointLiveTest and CheckpointLiveRefusalTest
# drive one A03 checkpoint scenario through the D28 candidate named by PLANAR_CANDIDATE
# and FAIL, never skip, when it is unset. CheckpointCliTest runs run_acceptance.py
# checkpoint as a subprocess. Failure messages are fixed text or AcceptError text from
# the support tree; raw subprocess output is never echoed, so the red log carries no
# compile or import marker. Each docstring names the mutant that the row must kill;
# only the mutation prober applies mutants, in a scratch copy outside the checkout.
import sys

sys.dont_write_bytecode = True

import copy
import hashlib
import json
import os
from pathlib import Path
import pwd
import re
import shutil
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import accept_lib as lib  # noqa: E402
import checkpoint_driver as ck  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
RUN = HERE / "run_acceptance.py"
PY = sys.executable
BASE_REF = "58128234:src/engine"
STATE = "running"
TOKEN_SHAPE = re.compile(r"[^\s]+\.[0-9]+\.[0-9a-f]{32}")
RUN_TOKEN_PLACEHOLDER = "<run-token>"
CHECK_KEYS = ("work_fp_equal", "state_equal", "render_moved_by_checkpoint1", "render_moved_by_checkpoint2",
              "control_equal", "since_material_empty", "since_has_checkpoint_added_section",
              "since_matches_after2", "token_absent", "verified")
MANIFEST_KEYS = ("version", "facts", "claim", "checkpoint1", "checkpoint2", "previews", "since",
                 "informational", "checks")
PREVIEW_NAMES = ("before", "after1", "after2", "control")
MARK_HOME = "/marker/home-7f3a"
MARK_DB = "/marker/db-7f3a"
MARK_LEDGER = "/marker/ledger-3a.db"
MARK_OBS = "/marker/obs-1c4e.json"
MARK_CANDIDATE = "/marker/candidate-9b2d"
MANIFEST_MARKERS = (MARK_HOME, MARK_DB, MARK_LEDGER, MARK_OBS, MARK_CANDIDATE)
W_OK = "xxh3_128:" + "a" * 32
W_MOVED = "xxh3_128:" + "e" * 32
D_CLAIM = "xxh3_128:" + "1" * 32
D_ONE = "xxh3_128:" + "2" * 32
D_TWO = "xxh3_128:" + "3" * 32
D_OTHER = "xxh3_128:" + "4" * 32
BASE_MD = "## run\n\nrun request: (preview)\n"
CP1_MD = ("## checkpoint\n\nCheckpoint 9 recorded by run 1 at event 9; session note, unverified.\n"
          + ck.TEXT_ONE + "\n")
CP2_MD = ("## checkpoint\n\nCheckpoint 15 recorded by run 1 at event 15; session note, unverified.\n"
          + ck.TEXT_TWO + "\n")
CP_BYTES = len(CP1_MD.encode("utf-8"))
RUN_ROW = {"id": "run", "included": True, "bytes": len(BASE_MD.encode("utf-8"))}
CP1_ROW = {"id": "checkpoint", "included": True, "bytes": CP_BYTES}
CP2_ROW = {"id": "checkpoint", "included": True, "bytes": len(CP2_MD.encode("utf-8"))}
AUTHORITY_ROW = {"id": "authority", "status": "changed", "before_bytes": 48, "after_bytes": 52}
RUN_SINCE_ROW = {"id": "run", "status": "changed", "before_bytes": 30, "after_bytes": 34}
CP_SINCE_ROW = {"id": "checkpoint", "status": "added", "before_bytes": 0, "after_bytes": CP_BYTES}


# Shared helpers -----------------------------------------------------------------------------------

def _expect(test, condition, message):
    """Fail the test with a fixed message unless condition holds; values are never echoed."""
    if not condition:
        test.fail(message)


def _dig(value, *keys):
    """Return value[k1][k2]... or None when any step is missing or not an object."""
    for key in keys:
        if not isinstance(value, dict) or key not in value:
            return None
        value = value[key]
    return value


def _text(value):
    """Return value when it is a string, otherwise the empty string."""
    return value if isinstance(value, str) else ""


def _block(test, result, name):
    """Return one section of a run result, failing with fixed text when it is absent."""
    value = result.get(name) if isinstance(result, dict) else None
    if not isinstance(value, dict):
        test.fail("result lacks the " + name + " section")
    return value


def _read_json(test, path, what):
    """Parse a retained JSON file; a missing or malformed file fails with fixed text."""
    try:
        data = json.loads(Path(path).read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, ValueError):
        data = None
    if data is None:
        test.fail(what + " is missing or is not JSON")
    return data


def _dumps(test, value):
    """Return the JSON text of value, failing with fixed text when it cannot be serialized."""
    try:
        return json.dumps(value, sort_keys=True)
    except (TypeError, ValueError):
        test.fail("the value is not JSON serializable")


def _sha256_file(path):
    """Return the hex sha256 of a file's bytes, read in chunks."""
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _real_home():
    """Return the real home directory from the password database (never from the environment)."""
    return pwd.getpwuid(os.getuid()).pw_dir


def _planar_names(home):
    """Return the sorted top-level entry names of home/.planar, or None when the directory is absent."""
    planar = Path(home) / ".planar"
    if not planar.is_dir():
        return None
    return sorted(os.listdir(planar))


def _names_digest(names):
    """Return the digest of the NUL-joined sorted entry names, or the word absent."""
    if names is None:
        return "absent"
    return hashlib.sha256("\0".join(names).encode("utf-8", "surrogateescape")).hexdigest()


def _under(value, root):
    """Return True when the real path of value lies under root (a realpath)."""
    try:
        Path(os.path.realpath(value)).relative_to(root)
    except ValueError:
        return False
    return True


def _config(root, candidate, tamper=None):
    """Return the run config the live rows share: a fresh scratch tree under root for one cmake run."""
    scratch = Path(root) / "run"
    return {
        "candidate": candidate,
        "repo": str(REPO),
        "scratch": str(scratch),
        "raw_root": str(scratch / "raw"),
        "home": str(scratch / "home"),
        "ledger": str(scratch / "ledger.db"),
        "counter": str(scratch / "check-counter.log"),
        "mode": "cmake",
        "check_mode": "sleep",
        "base": "plain",
        "tamper": dict(tamper or {}),
    }


def _cli_env(root):
    """Return a child environment whose HOME and PLANAR_DB are scratch paths under root."""
    home = Path(root) / "home"
    home.mkdir(exist_ok=True)
    return lib.child_env({"HOME": str(home), "PLANAR_DB": str(Path(root) / "unused-legacy.db")})


def _claim_token(raw_root):
    """Return the run token from the single retained *-claim.stdout (key run.token), or the empty string."""
    files = sorted(Path(raw_root).glob("*-claim.stdout"))
    if len(files) != 1:
        return ""
    try:
        data = json.loads(files[0].read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, ValueError):
        return ""
    return _text(_dig(data, "run", "token"))


def _retained(config, pattern):
    """Return the sorted retained raw files of the shared run that match pattern."""
    return sorted(Path(config["raw_root"]).glob(pattern))


def _argv_matches_retained(result_argv, retained):
    """Return True when the retained argv equals the result argv except at the run-token placeholder."""
    if not isinstance(retained, list) or len(retained) != len(result_argv):
        return False
    for mine, kept in zip(result_argv, retained):
        if mine == RUN_TOKEN_PLACEHOLDER:
            if not isinstance(kept, str) or not TOKEN_SHAPE.fullmatch(kept):
                return False
        elif mine != kept:
            return False
    return True


def _fresh(test, config, verb, *args):
    """Run one fresh candidate read of the run ledger with HOME and PLANAR_DB in scratch; return its JSON."""
    scratch = Path(config["scratch"])
    env = lib.child_env({"HOME": config["home"], "PLANAR_DB": str(scratch / "unused-legacy.db")})
    argv = [config["candidate"], verb, "--ledger", os.path.realpath(config["ledger"]), *map(str, args)]
    try:
        proc = subprocess.run(argv, capture_output=True, env=env, cwd=os.path.realpath(config["scratch"]),
                              timeout=600)
    except (OSError, subprocess.SubprocessError):
        test.fail("fresh " + verb + " read could not run")
    if proc.returncode != 0:
        test.fail("fresh %s read exited %d" % (verb, proc.returncode))
    try:
        return json.loads(proc.stdout.decode("utf-8"))
    except (UnicodeDecodeError, ValueError):
        test.fail("fresh " + verb + " read printed no JSON")


def _refusal(label, call, *fragments):
    """Return None when call() raises AcceptError whose text holds every fragment, else a fixed miss description."""
    try:
        call()
    except lib.AcceptError as error:
        text = str(error)
        if all(fragment in text for fragment in fragments):
            return None
        return label + ": refusal lacks an expected clause"
    except Exception:
        return label + ": raised an exception other than AcceptError"
    return label + ": accepted, expected a refusal"


def _accepts(label, call):
    """Return (value, problem): the value of call() when it is accepted, else a fixed problem description."""
    try:
        value = call()
    except lib.AcceptError as error:
        return None, label + " was refused: " + str(error)
    except Exception:
        return None, label + " raised an exception other than AcceptError"
    return value, None


def _canonical_size(checkpoint):
    """Return the canonical JSON size in bytes of a checkpoint object (render-contract.md:219-226)."""
    return len(json.dumps(checkpoint, sort_keys=True, separators=(",", ":"), ensure_ascii=False).encode("utf-8"))


def _checkpoint(**changes):
    """Return a valid checkpoint object with short fields; keyword changes replace fields."""
    checkpoint = {"progress": "progress one", "next": "next one", "unresolved": "unresolved one",
                  "files": ["src/a.c"]}
    checkpoint.update(changes)
    return checkpoint


def _without(key, **changes):
    """Return a valid checkpoint object with one key removed."""
    checkpoint = _checkpoint(**changes)
    del checkpoint[key]
    return checkpoint


def _files(count):
    """Return count distinct short relative source paths."""
    return ["src/f%02d.c" % index for index in range(count)]


def _long_files():
    """Return 32 distinct relative paths long enough that the canonical object exceeds 8,192 bytes."""
    return ["src/module_%02d/%s.c" % (index, "a" * 60) for index in range(32)]


def _preview_block(markdown, render_digest, *, state=STATE, work_fp=W_OK, sections=None, omitted=None):
    """Return one fresh preview block in the shape verify_effect and checkpoint_manifest consume."""
    rows = sections if sections is not None else [RUN_ROW]
    return {"kind": "preview", "work_fp": work_fp, "render_digest": render_digest, "state": state,
            "markdown": markdown, "sections": copy.deepcopy(rows), "omitted": list(omitted or [])}


def _before(**changes):
    """Return the before preview: the run section only, no checkpoint section."""
    block = _preview_block(BASE_MD, D_CLAIM)
    block.update(changes)
    return block


def _after(**changes):
    """Return the after preview: the run section and one checkpoint section, with a moved digest."""
    block = _preview_block(BASE_MD + CP1_MD, D_ONE, sections=[RUN_ROW, CP1_ROW])
    block.update(changes)
    return block


def _since_block(*, render_digest=D_ONE, material=None, sections=None, render_changed=True):
    """Return the whole decoded preview --since JSON: preview_v1 fields plus the since key (R16-2)."""
    rows = sections if sections is not None else [AUTHORITY_ROW, RUN_SINCE_ROW, CP_SINCE_ROW]
    return {"kind": "preview", "work_fp": W_OK, "render_digest": render_digest, "state": STATE,
            "since": {"dispatch": {"id": "1", "run": "1", "kind": "claim", "request_id": "claim-builder-1",
                                   "revision": "1", "work_fp": W_OK, "render_digest": D_CLAIM},
                      "material": list(material or []), "render_changed": render_changed,
                      "sections": copy.deepcopy(rows)}}


def _checkpoint_block(text, event, observations_path, request_id):
    """Return one checkpoint result block: the argv with the run-token placeholder, the file and the event."""
    checkpoint = {"progress": text, "next": "next step", "unresolved": "unresolved item",
                  "files": ["value/CMakeLists.txt"]}
    return {
        "request_id": request_id,
        "exit": 0,
        "argv": [MARK_CANDIDATE, "checkpoint", "--ledger", MARK_LEDGER, "--run", RUN_TOKEN_PLACEHOLDER,
                 "--observations", observations_path, "--request-id", request_id, "--actor", "builder-session"],
        "observations_path": observations_path,
        "observations_sha256": "5" * 64,
        "observations": {"version": 1, "checks": [], "findings": [], "checkpoint": checkpoint},
        "event_id": event,
        "text": text,
    }


def _honest_result():
    """Return a synthetic honest run result (every A03 check true) built by the test for the manifest rows."""
    after2 = _preview_block(BASE_MD + CP2_MD, D_TWO, sections=[RUN_ROW, CP2_ROW])
    return {
        "scenario": "a03-checkpoint",
        "mode": "cmake",
        "candidate": {"sha256": "6" * 64, "size": 4096},
        "env": {"HOME": MARK_HOME, "PLANAR_DB": MARK_DB},
        "ledger": "7" * 64,
        "facts": {"accepted_revision": "1", "baseline_ref": BASE_REF, "baseline_sha256": "8" * 64,
                  "candidate_sha256": "6" * 64, "candidate_size": 4096},
        "claim": {"request_id": "claim-builder-1", "run_id": "1", "work_fp": W_OK, "render_digest": D_CLAIM,
                  "state": STATE},
        "before": _preview_block(BASE_MD, D_CLAIM),
        "after1": _preview_block(BASE_MD + CP1_MD, D_ONE, sections=[RUN_ROW, CP1_ROW]),
        "after2": after2,
        "control": copy.deepcopy(after2),
        "checkpoint1": _checkpoint_block(ck.TEXT_ONE, "9", MARK_OBS, "checkpoint-1"),
        "checkpoint2": _checkpoint_block(ck.TEXT_TWO, "15", "/marker/obs-2b7d.json", "checkpoint-2"),
        "since": _since_block(render_digest=D_TWO, sections=[AUTHORITY_ROW, RUN_SINCE_ROW, CP_SINCE_ROW]),
        "verified": True,
        "token_absent": True,
    }


# Offline rows ------------------------------------------------------------------------------------

class CheckpointDocTest(unittest.TestCase):
    """Offline checks of the checkpoint object, its validator, the effect verifier and the A03 manifest."""

    def test_build_checkpoint_and_observations_have_exact_shape(self):
        """Kills: build_checkpoint swaps the progress and next values (the next_step argument maps to next)."""
        checkpoint = ck.build_checkpoint(progress="progress text", next_step="next text",
                                         unresolved="unresolved text", files=["src/a.c"])
        expected = {"progress": "progress text", "next": "next text", "unresolved": "unresolved text",
                    "files": ["src/a.c"]}
        _expect(self, checkpoint == expected,
                "build_checkpoint does not return exactly progress, next, unresolved and files, "
                "with next_step mapped to next")
        observations = ck.build_observations(expected)
        _expect(self, observations == {"version": 1, "checks": [], "findings": [], "checkpoint": expected},
                "build_observations does not return the observations root holding the checkpoint")

    def test_validate_checkpoint_refuses_each_violation_alone_and_accepts_bounds(self):
        """Kills: validate_checkpoint deletes the 2,048-byte bound (second probe: refuses empty strings; third: refuses an empty files list; also deletes the empty or NUL files-path check)."""
        over = _long_files()
        over_cp = _checkpoint(progress="p" * 2048, next="n" * 2048, unresolved="u" * 2048, files=over)
        problems = []
        if _canonical_size(over_cp) <= 8192:
            problems.append("the over-bound fixture is not over 8192 canonical bytes")
        refusals = (
            ("progress 2049 bytes", _checkpoint(progress="p" * 2049),
             ("checkpoint: progress", "must be at most 2048 UTF-8 bytes")),
            ("next with NUL", _checkpoint(next="n\x00x"), ("checkpoint: next", "must not contain NUL")),
            ("unresolved missing", _without("unresolved"), ("checkpoint: unresolved", "is missing")),
            ("next missing", _without("next"), ("checkpoint: next", "is missing")),
            ("unknown key", _checkpoint(extra="x"), ("checkpoint: extra", "is an unknown key")),
            ("progress not a string", _checkpoint(progress=7), ("checkpoint: progress",)),
            ("files not a list", _checkpoint(files="src/a.c"), ("checkpoint: files",)),
            ("33 files", _checkpoint(files=_files(33)),
             ("checkpoint: files", "must be a list of at most 32 distinct relative paths")),
            ("duplicate path", _checkpoint(files=["src/a.c", "src/a.c"]), ("checkpoint: files",)),
            ("empty path", _checkpoint(files=[""]), ("checkpoint: files.0", "non-empty")),
            ("path with NUL", _checkpoint(files=["a\x00b"]), ("checkpoint: files.0", "NUL")),
            ("absolute path", _checkpoint(files=["/etc/hosts"]), ("checkpoint: files",)),
            ("parent path", _checkpoint(files=["src/../a.c"]), ("checkpoint: files",)),
            ("over 8,192 canonical bytes", over_cp, ("canonical JSON exceeds 8192 bytes",)),
        )
        for label, checkpoint, fragments in refusals:
            problem = _refusal(label, lambda cp=checkpoint: ck.validate_checkpoint(cp), *fragments)
            if problem:
                problems.append(problem)
        accepted = (
            ("2,048-byte progress", _checkpoint(progress="p" * 2048)),
            ("empty strings", _checkpoint(progress="", next="", unresolved="", files=[])),
            ("empty files list", _checkpoint(files=[])),
            ("empty progress only", _checkpoint(progress="")),
            ("exactly 32 files", _checkpoint(files=_files(32))),
        )
        for label, checkpoint in accepted:
            _value, problem = _accepts(label, lambda cp=checkpoint: ck.validate_checkpoint(cp))
            if problem:
                problems.append(problem)
        if problems:
            self.fail("; ".join(problems))

    def test_verify_effect_accepts_a_render_only_change(self):
        """Kills: verify_effect always raises (the render-only change must be accepted with or without since)."""
        problems = []
        value, problem = _accepts("render-only change with since",
                                  lambda: ck.verify_effect(_before(), _after(), _since_block()))
        if problem:
            problems.append(problem)
        elif value is not None:
            problems.append("render-only change with since did not return None")
        value, problem = _accepts("render-only change without since",
                                  lambda: ck.verify_effect(_before(), _after(), None))
        if problem:
            problems.append(problem)
        elif value is not None:
            problems.append("render-only change without since did not return None")
        if problems:
            self.fail("; ".join(problems))

    def test_verify_effect_refuses_each_clause_alone(self):
        """Kills: verify_effect deletes the work_fp check (C4), the state check (C5), the digest check (C6), the section check (C7), or the since material, checkpoint-row, since-digest or render_changed-as-evidence check (C8)."""
        cases = (
            ("work_fp moved", _before(), _after(work_fp=W_MOVED), None,
             ("checkpoint effect: work_fp changed",)),
            ("state changed", _before(), _after(state="idle"), None,
             ("checkpoint effect: state changed",)),
            ("render digest unchanged", _before(), _after(render_digest=D_CLAIM), None,
             ("checkpoint effect: render digest did not change",)),
            ("no checkpoint section", _before(), _after(markdown=BASE_MD, sections=[RUN_ROW], render_digest=D_ONE),
             None, ("checkpoint effect: packet lacks a checkpoint section",)),
            ("since reports material changes", _before(), _after(), _since_block(material=["state"]),
             ("checkpoint effect: since reports material changes",)),
            ("since without a checkpoint row (render_changed true alone)", _before(), _after(),
             _since_block(sections=[AUTHORITY_ROW, RUN_SINCE_ROW], render_changed=True),
             ("checkpoint effect: since lacks an added checkpoint section",)),
            ("since checkpoint row changed", _before(), _after(),
             _since_block(sections=[AUTHORITY_ROW, RUN_SINCE_ROW,
                                    {"id": "checkpoint", "status": "changed", "before_bytes": 12,
                                     "after_bytes": CP_BYTES}]),
             ("checkpoint effect: since lacks an added checkpoint section",)),
            ("since checkpoint row added with zero after bytes", _before(), _after(),
             _since_block(sections=[AUTHORITY_ROW, RUN_SINCE_ROW,
                                    {"id": "checkpoint", "status": "added", "before_bytes": 0,
                                     "after_bytes": 0}]),
             ("checkpoint effect: since lacks an added checkpoint section",)),
            ("since digest differs from after", _before(), _after(), _since_block(render_digest=D_OTHER),
             ("checkpoint effect: since does not match the after packet",)),
        )
        problems = []
        for label, before, after, since, fragments in cases:
            problem = _refusal(label, lambda b=before, a=after, s=since: ck.verify_effect(b, a, s), *fragments)
            if problem:
                problems.append(problem)
        if problems:
            self.fail("; ".join(problems))

    def test_verify_effect_classifies_budget_omission_as_fixture_limit(self):
        """Kills: verify_effect deletes the budget-omission branch, so the absence is always filed as the product defect."""
        omitted = _after(markdown=BASE_MD, sections=[RUN_ROW, {"id": "checkpoint", "included": False, "bytes": 0}],
                         omitted=["checkpoint"])
        unexplained = _after(markdown=BASE_MD, sections=[RUN_ROW], omitted=[])
        problems = []
        problem = _refusal("budget omission", lambda: ck.verify_effect(_before(), omitted, None),
                           "checkpoint fixture limit:", "checkpoint section omitted by budget")
        if problem:
            problems.append(problem)
        problem = _refusal("absence without explanation", lambda: ck.verify_effect(_before(), unexplained, None),
                           "checkpoint effect: packet lacks a checkpoint section")
        if problem:
            problems.append(problem)
        if problems:
            self.fail("; ".join(problems))

    def test_checkpoint_manifest_is_path_free_and_recomputes_its_checks(self):
        """Kills: checkpoint_manifest copies result env into checks (second: work_fp_equal constant True; third: render_changed in checks; fourth: the budget flag moved into checks; also a constant True for verified, token_absent, state_equal, render_moved_by_checkpoint1/2, control_equal, since_material_empty, since_has_checkpoint_added_section or since_matches_after2)."""
        honest = _honest_result()
        manifest = ck.checkpoint_manifest(honest)
        _expect(self, isinstance(manifest, dict) and sorted(manifest) == sorted(MANIFEST_KEYS),
                "the manifest does not carry exactly the keys version, facts, claim, checkpoint1, "
                "checkpoint2, previews, since, informational and checks")
        _expect(self, manifest.get("version") == 1, "the manifest version is not 1")
        checks = manifest.get("checks")
        _expect(self, isinstance(checks, dict) and sorted(checks) == sorted(CHECK_KEYS),
                "manifest checks is not exactly the ten booleans of the A03 acceptance condition")
        _expect(self, all(checks.get(key) is True for key in CHECK_KEYS),
                "the honest synthetic result does not give ten true checks")
        _expect(self, manifest.get("informational") == {"checkpoint_section_omitted_by_budget": False},
                "informational is not exactly the budget flag, false for an honest result")
        previews = manifest.get("previews")
        _expect(self, isinstance(previews, dict) and sorted(previews) == sorted(PREVIEW_NAMES)
                and all(isinstance(previews[name], dict)
                        and sorted(previews[name]) == sorted(("work_fp", "render_digest", "state"))
                        for name in PREVIEW_NAMES),
                "previews does not hold before, after1, after2 and control with work_fp, render_digest and state")
        since = manifest.get("since")
        _expect(self, isinstance(since, dict)
                and sorted(since) == sorted(("dispatch", "material", "render_changed", "sections", "render_digest")),
                "since does not hold exactly dispatch, material, render_changed, sections and render_digest")
        _expect(self, manifest.get("facts") == honest["facts"], "the manifest facts differ from the result facts")
        text = _dumps(self, manifest)
        for marker in MANIFEST_MARKERS:
            _expect(self, marker not in text, "the manifest holds a host path, argv or environment value")
        for name in ("checkpoint1", "checkpoint2"):
            block = manifest.get(name)
            _expect(self, isinstance(block, dict) and not ({"argv", "observations_path"} & set(block)),
                    name + " in the manifest holds argv or a host path")
        moved = _honest_result()
        moved["after2"]["work_fp"] = W_MOVED
        moved_manifest = ck.checkpoint_manifest(moved)
        _expect(self, _dig(moved_manifest, "checks", "work_fp_equal") is False,
                "a moved work_fp does not make work_fp_equal false")
        flagged_source = _honest_result()
        flagged_source["after2"]["sections"] = [RUN_ROW, {"id": "checkpoint", "included": False, "bytes": 0}]
        flagged_source["after2"]["omitted"] = ["checkpoint"]
        flagged = ck.checkpoint_manifest(flagged_source)
        _expect(self, _dig(flagged, "informational", "checkpoint_section_omitted_by_budget") is True,
                "a budget omission does not set the informational flag")
        _expect(self, _dig(flagged, "checks") == checks, "a budget omission changes the ten checks")
        # One negative case per check: each recomputed check must turn false when its own block is
        # broken, so a constant True for any of them is refused.
        def _broken(mutate):
            source = _honest_result()
            mutate(source)
            return ck.checkpoint_manifest(source)

        def _later_state(source):
            source["after1"]["state"] = "moved-state"

        def _same_render_one(source):
            source["after1"]["render_digest"] = source["before"]["render_digest"]

        def _same_render_two(source):
            source["after2"]["render_digest"] = source["after1"]["render_digest"]

        def _other_control(source):
            source["control"]["markdown"] = source["control"]["markdown"] + "\nextra"
            source["control"]["render_digest"] = D_ONE

        def _material(source):
            source["since"]["since"]["material"] = ["value/CMakeLists.txt"]

        def _no_row(source):
            source["since"]["since"]["sections"] = [AUTHORITY_ROW, RUN_SINCE_ROW]

        def _other_since_digest(source):
            source["since"]["render_digest"] = D_ONE

        def _leaked(source):
            source["token_absent"] = False

        def _unverified(source):
            source["verified"] = False

        negatives = (
            ("state_equal", _later_state),
            ("render_moved_by_checkpoint1", _same_render_one),
            ("render_moved_by_checkpoint2", _same_render_two),
            ("control_equal", _other_control),
            ("since_material_empty", _material),
            ("since_has_checkpoint_added_section", _no_row),
            ("since_matches_after2", _other_since_digest),
            ("token_absent", _leaked),
            ("verified", _unverified),
        )
        for key, mutate in negatives:
            _expect(self, _dig(_broken(mutate), "checks", key) is False,
                    "a broken " + key + " input does not make the " + key + " check false")


# Live rows ---------------------------------------------------------------------------------------

class CheckpointLiveTest(unittest.TestCase):
    """Live rows sharing one A03 run: claim, before, checkpoint #1, after1, checkpoint #2, after2, since, control."""

    error = ""
    result = None
    config = None
    root = ""
    real_home = ""
    names_before = None
    names_after = None

    @classmethod
    def setUpClass(cls):
        """Run the candidate once for every live row; any failure is stored in cls.error, never raised."""
        cls.error = ""
        cls.result = None
        cls.config = None
        cls.root = ""
        candidate = os.environ.get("PLANAR_CANDIDATE", "")
        if not candidate:
            cls.error = "PLANAR_CANDIDATE must name the D28 candidate binary; live rows fail, never skip"
            return
        try:
            cls.root = os.path.realpath(tempfile.mkdtemp(prefix="r19-checkpoint-live-"))
            cls.config = dict(_config(cls.root, candidate), verify=False)
            cls.real_home = _real_home()
            cls.names_before = _planar_names(cls.real_home)
            try:
                cls.result = ck.run_checkpoint(cls.config)
            finally:
                cls.names_after = _planar_names(cls.real_home)
            if not isinstance(cls.result, dict):
                cls.error = "checkpoint run returned no result object"
        except Exception as error:
            cls.error = "checkpoint run did not complete: " + (str(error) or type(error).__name__)

    @classmethod
    def tearDownClass(cls):
        """Remove the shared scratch tree."""
        if cls.root:
            shutil.rmtree(cls.root, ignore_errors=True)

    def _status_builder_state(self):
        """Return the builder role state from a fresh status read."""
        status = _fresh(self, self.config, "status")
        _expect(self, isinstance(status, dict) and "active_revision" in status and "ledger" in status,
                "status read lacks active_revision or ledger")
        rows = [row for row in _dig(status, "roles") or [] if isinstance(row, dict) and row.get("role") == "builder"]
        _expect(self, len(rows) == 1, "the fresh status lacks exactly one builder role")
        return rows[0].get("state")

    def _fresh_preview(self):
        """Return one fresh preview read of the builder role."""
        return _fresh(self, self.config, "preview", "--role", "builder")

    # Rows ----------------------------------------------------------------------------------------

    def test_checkpoint_argv_passes_observations_file_holding_the_honest_checkpoint(self):
        """Kills: run_checkpoint writes a valid but different checkpoint (text T2 or an empty files list) into checkpoint #1's observations file while result checkpoint1 keeps T1 (named assertion; the run completes)."""
        if self.error:
            self.fail(self.error)
        first = _block(self, self.result, "checkpoint1")
        argv = first.get("argv")
        _expect(self, isinstance(argv, list) and "--observations" in argv
                and argv.index("--observations") + 1 < len(argv),
                "checkpoint #1 argv does not pass --observations")
        _expect(self, argv[argv.index("--observations") + 1] == first.get("observations_path"),
                "checkpoint #1 argv does not pass observations_path after --observations")
        retained = _retained(self.config, "*-checkpoint.argv.json")
        _expect(self, len(retained) == 2, "expected exactly two retained checkpoint argv files")
        retained_argv = _read_json(self, retained[0], "retained checkpoint #1 argv")
        _expect(self, _argv_matches_retained(argv, retained_argv),
                "retained checkpoint #1 argv differs from the result argv beyond the run token")
        path = retained_argv[retained_argv.index("--observations") + 1]
        written = _read_json(self, path, "observations file of checkpoint #1")
        recorded = _dig(first, "observations", "checkpoint")
        _expect(self, isinstance(written, dict) and written.get("checkpoint") == recorded,
                "the observations file of checkpoint #1 holds a checkpoint that differs from the recorded one")
        _expect(self, _dig(written, "checkpoint", "progress") == ck.TEXT_ONE,
                "the observations file of checkpoint #1 does not hold the supplied text one")
        _expect(self, first.get("text") == ck.TEXT_ONE, "checkpoint #1 text is not the supplied text one")
        _expect(self, written.get("version") == 1 and written.get("checks") == [] and written.get("findings") == [],
                "the observations root of checkpoint #1 is not version 1 with empty checks and findings")
        _expect(self, _sha256_file(path) == first.get("observations_sha256"),
                "the observations digest of checkpoint #1 does not match the file bytes")

    def test_checkpoint_event_is_recorded_and_read_back_by_history(self):
        """Kills: run_checkpoint takes event_id for checkpoint #1 from checkpoint.id instead of checkpoint.event (named assertion; the run completes)."""
        if self.error:
            self.fail(self.error)
        first = _block(self, self.result, "checkpoint1")
        listing = _fresh(self, self.config, "history", "--limit", "100")
        events = _dig(listing, "data", "events")
        if not isinstance(events, list):
            self.fail("the fresh history read lacks data.events")
        recorded = [event for event in events
                    if isinstance(event, dict) and event.get("type") == "checkpoint_recorded"]
        found = []
        for event in recorded:
            full = _fresh(self, self.config, "history", "--event", str(event.get("sequence")))
            if _dig(full, "data", "execution", "checkpoint", "value", "progress") == ck.TEXT_ONE:
                found.append(str(event.get("sequence")))
        _expect(self, len(found) == 1,
                "expected exactly one checkpoint_recorded event holding the supplied checkpoint text")
        _expect(self, found[0] == str(first.get("event_id")),
                "the recorded event of checkpoint #1 differs from the event id the run reports")
        full = _fresh(self, self.config, "history", "--event", found[0])
        _expect(self, _dig(full, "data", "event", "type") == "checkpoint_recorded",
                "the fresh --event read is not a checkpoint_recorded event")
        _expect(self, _dig(full, "data", "execution", "checkpoint", "value", "progress") == ck.TEXT_ONE,
                "the fresh --event read does not show the supplied checkpoint text")

    def test_checkpoint_moves_only_the_render_digest(self):
        """Kills: run_checkpoint takes the before snapshot after checkpoint #1 (C12), or takes after1 before checkpoint #1 (C13); each named assertion is reached because the run completes."""
        if self.error:
            self.fail(self.error)
        before = _block(self, self.result, "before")
        after1 = _block(self, self.result, "after1")
        after2 = _block(self, self.result, "after2")
        claim = _block(self, self.result, "claim")
        for name, block in (("before", before), ("after1", after1), ("after2", after2)):
            _expect(self, block.get("kind") == "preview", name + " is not a fresh preview")
        builder_state = self._status_builder_state()
        _expect(self, before.get("work_fp") == claim.get("work_fp") == after1.get("work_fp") == after2.get("work_fp"),
                "work_fp is not equal across the claim and the three previews")
        _expect(self, before.get("state") == after1.get("state") == after2.get("state") == claim.get("state")
                == builder_state,
                "builder state is not equal across the claim, the previews and the fresh status")
        _expect(self, "## checkpoint" not in _text(before.get("markdown")),
                "the before packet already has a checkpoint section")
        _expect(self, before.get("render_digest") != after1.get("render_digest"),
                "the first checkpoint did not move the render digest")
        _expect(self, after1.get("render_digest") != after2.get("render_digest"),
                "the second checkpoint did not move the render digest")
        _expect(self, "## checkpoint" in _text(after1.get("markdown")), "after1 has no checkpoint section")
        _expect(self, "## checkpoint" in _text(after2.get("markdown")), "after2 has no checkpoint section")

    def test_preview_since_reports_a_render_only_change(self):
        """Kills: run_checkpoint takes the since read before checkpoint #2, so it no longer equals a fresh read taken after the run (named assertion)."""
        if self.error:
            self.fail(self.error)
        claim = _block(self, self.result, "claim")
        digest = claim.get("render_digest")
        _expect(self, isinstance(digest, str) and digest.startswith("xxh3_128:"),
                "the claim render digest is missing")
        since = _block(self, self.result, "since")
        fresh = _fresh(self, self.config, "preview", "--role", "builder", "--since", digest)
        _expect(self, fresh == since, "result since differs from a fresh preview --since read of the claim digest")
        after2 = _block(self, self.result, "after2")
        control = _block(self, self.result, "control")
        _expect(self, fresh.get("render_digest") == after2.get("render_digest") == control.get("render_digest"),
                "the since render digest is not the render digest of after2 and the control")
        body = _dig(fresh, "since")
        _expect(self, isinstance(body, dict), "the fresh preview lacks the since key")
        _expect(self, _dig(body, "dispatch", "render_digest") == digest,
                "since dispatch is not the claim render digest")
        _expect(self, body.get("material") == [], "since reports material changes; the work fingerprint moved")
        rows = body.get("sections")
        _expect(self, isinstance(rows, list) and all(isinstance(row, dict) for row in rows),
                "since sections are not a list of objects")
        pairs = sorted((row.get("id"), row.get("status")) for row in rows)
        _expect(self, pairs == sorted([("authority", "changed"), ("run", "changed"), ("checkpoint", "added")]),
                "since sections are not exactly authority changed, run changed and checkpoint added")
        checkpoint_rows = [row for row in rows if row.get("id") == "checkpoint"]
        _expect(self, len(checkpoint_rows) == 1 and checkpoint_rows[0].get("before_bytes") == 0
                and isinstance(checkpoint_rows[0].get("after_bytes"), int)
                and checkpoint_rows[0].get("after_bytes") > 0,
                "the checkpoint since row is not added from zero bytes to a positive size")

    def test_second_checkpoint_with_different_text_moves_render_again(self):
        """Kills: run_checkpoint writes the first checkpoint's text again as checkpoint #2 (named assertion; the run completes)."""
        if self.error:
            self.fail(self.error)
        first = _block(self, self.result, "checkpoint1")
        second = _block(self, self.result, "checkpoint2")
        _expect(self, ck.TEXT_ONE != ck.TEXT_TWO, "the two supplied checkpoint texts are not different")
        _expect(self, first.get("text") == ck.TEXT_ONE, "checkpoint #1 text is not the supplied text one")
        _expect(self, second.get("text") == ck.TEXT_TWO, "checkpoint #2 text is not the supplied text two")
        after1 = _block(self, self.result, "after1")
        after2 = _block(self, self.result, "after2")
        _expect(self, ck.TEXT_TWO in _text(after2.get("markdown")),
                "after2 does not carry checkpoint #2's text")
        _expect(self, ck.TEXT_ONE not in _text(after2.get("markdown")),
                "after2 still carries checkpoint #1's text")
        _expect(self, ck.TEXT_ONE in _text(after1.get("markdown")), "after1 does not carry checkpoint #1's text")
        full = _fresh(self, self.config, "history", "--event", str(second.get("event_id")))
        _expect(self, _dig(full, "data", "execution", "checkpoint", "value", "progress") == ck.TEXT_TWO,
                "the fresh history --event of checkpoint #2 does not show the second text")
        _expect(self, after1.get("render_digest") != after2.get("render_digest"),
                "the second checkpoint did not move the render digest again")
        _expect(self, after1.get("work_fp") == after2.get("work_fp") and after1.get("state") == after2.get("state"),
                "work_fp or state moved between after1 and after2")

    def test_unchanged_state_gives_byte_equal_previews(self):
        """Kills: run_checkpoint takes the control preview after a third checkpoint (named assertion: the after2 versus control comparison and the fresh-read comparison)."""
        if self.error:
            self.fail(self.error)
        after2 = _block(self, self.result, "after2")
        control = _block(self, self.result, "control")
        _expect(self, after2.get("markdown") == control.get("markdown"),
                "the control preview markdown differs from after2 with no change in between")
        _expect(self, after2.get("render_digest") == control.get("render_digest"),
                "the control preview render digest differs from after2 with no change in between")
        fresh = self._fresh_preview()
        _expect(self, _dig(fresh, "packet", "markdown") == control.get("markdown"),
                "a fresh preview markdown differs from the recorded control preview")
        _expect(self, fresh.get("render_digest") == control.get("render_digest"),
                "a fresh preview render digest differs from the recorded control preview")

    def test_run_uses_scratch_home_and_leaves_real_home_untouched(self):
        """Kills: run_checkpoint sets scenario.env = dict(os.environ) after prepare() (named assertion: the env record is the one given to children)."""
        if self.error:
            self.fail(self.error)
        env = self.result.get("env")
        _expect(self, isinstance(env, dict), "the result lacks its env record")
        scratch = Path(os.path.realpath(self.config["scratch"]))
        for key in ("HOME", "PLANAR_DB"):
            _expect(self, isinstance(env.get(key), str) and env.get(key) != "",
                    key + " is missing from the result env record")
            _expect(self, _under(env[key], scratch), key + " does not resolve under the run scratch root")
        _expect(self, not os.path.exists(env["PLANAR_DB"]), "PLANAR_DB exists after the run")
        _expect(self, os.path.realpath(env["HOME"]) != os.path.realpath(self.real_home),
                "the result env HOME is the real home")
        if _names_digest(self.names_before) != _names_digest(self.names_after):
            before = set(self.names_before or [])
            after = set(self.names_after or [])
            added = ", ".join(sorted(after - before)) or "none"
            removed = ", ".join(sorted(before - after)) or "none"
            self.fail("real home .planar top-level entry names changed during the run (added: %s; removed: %s); "
                      "a foreign process may be the cause" % (added, removed))

    def test_run_token_never_appears_in_result_or_manifest_files(self):
        """Kills: run_checkpoint stores the unredacted argv (real token) in checkpoint1['argv'] (named assertion)."""
        if self.error:
            self.fail(self.error)
        token = _claim_token(self.config["raw_root"])
        _expect(self, len(token) >= 16, "the retained claim stdout carries no run token to check against")
        carriers = _retained(self.config, "*-checkpoint.argv.json")
        _expect(self, any(token.encode("utf-8") in path.read_bytes() for path in carriers),
                "the retained checkpoint argv files do not hold the run token; the scan has no target")
        for name in ("checkpoint1", "checkpoint2"):
            path = _block(self, self.result, name).get("observations_path")
            _expect(self, isinstance(path, str) and Path(path).is_file(), name + " observations file is missing")
            _expect(self, token.encode("utf-8") not in Path(path).read_bytes(),
                    name + " observations file holds the run token")
        histories = _retained(self.config, "*-history.stdout")
        _expect(self, len(histories) > 0, "no retained history stdout exists to scan")
        for path in histories:
            _expect(self, token.encode("utf-8") not in path.read_bytes(),
                    "a retained history stdout holds the run token")
        _expect(self, token not in _dumps(self, self.result), "the result holds the run token")
        _expect(self, token not in _dumps(self, ck.checkpoint_manifest(self.result)),
                "the manifest holds the run token")
        _expect(self, self.result.get("token_absent") is True, "the result does not report token_absent true")

    def test_live_fixture_reports_verification_off(self):
        """Kills: run_checkpoint reports "verified": True constantly (the shared fixture runs with verification off)."""
        if self.error:
            self.fail(self.error)
        _expect(self, self.result.get("verified") is False, "the live fixture did not report verification off")


class CheckpointLiveRefusalTest(unittest.TestCase):
    """Live refusal row: a checkpoint whose observations file is missing is a product defect, and the run aborts."""

    def setUp(self):
        """Require the candidate and a fresh scratch root for this run."""
        self.candidate = os.environ.get("PLANAR_CANDIDATE", "")
        if not self.candidate:
            self.fail("PLANAR_CANDIDATE must name the D28 candidate binary; live rows fail, never skip")
        self.root = os.path.realpath(tempfile.mkdtemp(prefix="r19-checkpoint-refusal-"))
        self.addCleanup(shutil.rmtree, self.root, True)

    def test_run_checkpoint_aborts_as_product_defect_when_checkpoint_is_refused(self):
        """Kills: run_checkpoint deletes the require_accepted(step, checkpoint) call (the run must abort; no second checkpoint argv file)."""
        config = _config(self.root, self.candidate, tamper={"observations_path": "missing"})
        message = ""
        try:
            ck.run_checkpoint(config)
        except lib.AcceptError as error:
            message = str(error)
        except Exception:
            self.fail("run_checkpoint raised an exception other than AcceptError")
        else:
            self.fail("run_checkpoint accepted a checkpoint whose observations file is missing")
        _expect(self, "product defect" in message and "checkpoint" in message,
                "the refused checkpoint is not reported as a product defect naming checkpoint")
        raw = Path(config["raw_root"])
        _expect(self, (raw / "ABORTED.json").is_file(), "ABORTED.json is missing from raw_root")
        _expect(self, len(list(raw.glob("*-checkpoint.argv.json"))) < 2,
                "a second checkpoint argv file exists after the refused checkpoint")


# CLI rows ----------------------------------------------------------------------------------------

class CheckpointCliTest(unittest.TestCase):
    """CLI rows: the checkpoint command verifies the candidate, then writes result.json and a03-readback.json."""

    def setUp(self):
        """Create a per-test temporary root for the subprocess cases."""
        self.tmp = os.path.realpath(tempfile.mkdtemp(prefix="r19-checkpoint-cli-"))
        self.addCleanup(shutil.rmtree, self.tmp, True)

    def test_checkpoint_command_aborts_before_first_action_on_candidate_mismatch(self):
        """Kills: cmd_checkpoint calls checkpoint_driver.run_checkpoint directly instead of through run_live (the candidate is checked first)."""
        tmp = Path(self.tmp)
        stub_dir = tmp / "stub"
        stub_dir.mkdir()
        sentinel = stub_dir / "sentinel.txt"
        stub = stub_dir / "candidate"
        stub.write_text("#!%s\nimport sys\nopen(%r, 'a').write('ran\\n')\n" % (PY, str(sentinel)))
        stub.chmod(0o755)
        record = stub_dir / "candidate.json"
        record.write_text(json.dumps({"head": "0" * 40, "sha256": "0" * 64, "size": 1}))
        raw = tmp / "raw"
        raw.mkdir()
        argv = [PY, "-B", str(RUN), "checkpoint", "--candidate", str(stub), "--record", str(record),
                "--repo-path", str(REPO), "--raw-root", str(raw)]
        try:
            proc = subprocess.run(argv, env=_cli_env(tmp), cwd=str(tmp), capture_output=True, text=True,
                                  timeout=600)
        except (OSError, subprocess.SubprocessError):
            self.fail("the checkpoint command could not be started")
        if proc.returncode == 0:
            self.fail("checkpoint exited 0 for a candidate that differs from the recorded candidate")
        if sentinel.exists():
            self.fail("the stub candidate ran before the recorded-candidate check")
        if list(raw.rglob("*.argv.json")):
            self.fail("an action argv file exists after a candidate mismatch")
        if "does not match the recorded candidate" not in proc.stderr:
            self.fail("checkpoint did not report the recorded-candidate mismatch (exit %d)" % proc.returncode)

    def test_checkpoint_command_writes_result_and_path_free_readback(self):
        """Kills: cmd_checkpoint skips writing a03-readback.json (second probe: verify forced False, so verified is not true)."""
        candidate = os.environ.get("PLANAR_CANDIDATE", "")
        if not candidate:
            self.fail("PLANAR_CANDIDATE must name the D28 candidate binary; live rows fail, never skip")
        tmp = Path(self.tmp)
        try:
            head = subprocess.run(["git", "-C", str(REPO), "rev-parse", "HEAD"], capture_output=True, text=True,
                                  check=True).stdout.strip()
            record = lib.candidate_record(candidate, REPO, head)
        except Exception:
            self.fail("the candidate record could not be made from the checkout HEAD")
        record_path = tmp / "candidate-record.json"
        record_path.write_text(json.dumps(record), encoding="utf-8")
        raw = tmp / "raw"
        raw.mkdir()
        argv = [PY, "-B", str(RUN), "checkpoint", "--candidate", candidate, "--record", str(record_path),
                "--repo-path", str(REPO), "--raw-root", str(raw), "--run-name", "a03-cli"]
        try:
            proc = subprocess.run(argv, env=_cli_env(tmp), cwd=str(tmp), capture_output=True, text=True,
                                  timeout=1800)
        except (OSError, subprocess.SubprocessError):
            self.fail("the checkpoint command could not be run to completion")
        if proc.returncode != 0:
            self.fail("checkpoint exited %d, expected 0" % proc.returncode)
        lines = [line for line in proc.stdout.splitlines() if line.strip()]
        run_dir = Path(lines[-1]) if lines else None
        if run_dir is None or not run_dir.is_dir():
            self.fail("checkpoint did not print an existing run directory as its last line")
        if not (run_dir / "result.json").is_file():
            self.fail("the run directory lacks result.json")
        if not (run_dir / "scratch" / "ledger.db").is_file():
            self.fail("the run directory lacks scratch/ledger.db")
        result = _read_json(self, run_dir / "result.json", "result.json")
        if result.get("verified") is not True:
            self.fail("the checkpoint result does not report in-run verification on")
        readback_path = run_dir / "a03-readback.json"
        if not readback_path.is_file():
            self.fail("the run directory lacks a03-readback.json")
        manifest = _read_json(self, readback_path, "a03-readback.json")
        text = readback_path.read_text(encoding="utf-8")
        markers = {str(run_dir), os.path.realpath(str(run_dir)), str(tmp), os.path.realpath(str(tmp)),
                   candidate, os.path.realpath(candidate), os.path.realpath(str(tmp / "home"))}
        for marker in markers:
            _expect(self, marker not in text, "a03-readback.json holds a host path")
        _expect(self, manifest == ck.checkpoint_manifest(result),
                "a03-readback.json differs from checkpoint_manifest(result)")
        checks = _dig(manifest, "checks")
        _expect(self, isinstance(checks, dict) and sorted(checks) == sorted(CHECK_KEYS),
                "the readback checks are not exactly the ten A03 booleans")
        _expect(self, all(checks.get(key) is True for key in CHECK_KEYS),
                "not all ten A03 checks are true in the readback")
        _expect(self, _dig(manifest, "informational", "checkpoint_section_omitted_by_budget") is False,
                "the readback reports a budget omission, which is a fixture limit and not met")


if __name__ == "__main__":
    unittest.main()
