#!/usr/bin/env python3
# R19-ACCEPT-B provenance tests: rows I1-I19, I31, I32 and I41 of
# plan/INVARIANTS.md. ProvenanceDocTest is offline and checks the driver document,
# validator and read-back verifiers. ProvenanceLiveTest and
# ProvenanceLiveRefusalTest drive the D28 candidate named by PLANAR_CANDIDATE and
# FAIL, never skip, when it is unset. ProvenanceCliTest runs run_acceptance.py
# provenance as a subprocess. ProvenanceManifestTest is offline and checks the
# readback_manifest and scan_for_tokens helpers with synthetic results. Failure
# messages are fixed text or AcceptError text from the support tree; raw subprocess
# output is never echoed, so the red log carries no compile or import marker (D20,
# D30). Each docstring names the mutant that the row must kill; only the mutation
# prober applies mutants.
import sys

sys.dont_write_bytecode = True

import copy
import hashlib
import json
import os
from pathlib import Path
import pwd
import shutil
import subprocess
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))

import accept_lib as lib  # noqa: E402
import provenance_driver as pd  # noqa: E402

REPO = Path(__file__).resolve().parents[3]
RUN = HERE / "run_acceptance.py"
PY = sys.executable
DRIVER_NAME = "r19-accept-support/provenance_driver.py"
TRIAL = "r19-accept-b-a34-minimal"
VIEW = "source"
BASE_COMMIT = "58128234"
BASE_REF = "58128234:src/engine"
BASE_LABEL = "baseline_head_cmake58128234"
AUTHORITY = "driver_assertion_unverified"
TOKEN = "tok-5d41402abc4b2a76b9719d911017c592"
TOP_KEYS = ["version", "driver", "trial", "view", "model", "session", "settings"]
NATIVE_KEYS = ["base_label", "generation", "job", "ledger", "preferences", "render_fp", "revision", "role",
               "source", "work_fp"]
NATIVE_EXPECT = {"job": "combined", "role": "builder", "generation": "1", "base_label": BASE_LABEL}
SETTING_VALUES = {
    "accepted_revision": "1",
    "baseline_ref": BASE_REF,
    "baseline_sha256": "a" * 64,
    "candidate_sha256": "b" * 64,
    "candidate_size": "4096",
    "driver_phase": "claim",
}


# Shared helpers -----------------------------------------------------------------------------------

def _unknown():
    """Return a fresh copy of the unknown leaf."""
    return {"status": "unknown"}


def _all_unknown_document():
    """Return the all-unknown driver document with no settings, built by the test (not by the driver)."""
    return {
        "version": 1,
        "driver": {"name": _unknown(), "version": _unknown()},
        "trial": {"id": _unknown()},
        "view": {"id": _unknown()},
        "model": {"requested": _unknown(), "actual": _unknown()},
        "session": {"requested": _unknown(), "actual": _unknown()},
        "settings": [],
    }


def _doc(phase="claim"):
    """Return a contract-shaped honest document written by the test, settings sorted by name bytes."""
    values = dict(SETTING_VALUES, driver_phase=phase)
    return {
        "version": 1,
        "driver": {"name": DRIVER_NAME, "version": "0123456789ab"},
        "trial": {"id": TRIAL},
        "view": {"id": VIEW},
        "model": {"requested": _unknown(), "actual": _unknown()},
        "session": {"requested": _unknown(), "actual": _unknown()},
        "settings": [{"name": name, "value": values[name]} for name in sorted(values)],
    }


def _native(generation="1", revision="1"):
    """Return a native block with the ten keys, preferences unknown and xxh3_128 digests."""
    return {
        "base_label": BASE_LABEL,
        "generation": generation,
        "job": "combined",
        "ledger": "0123456789abcdef0123456789abcdef",
        "preferences": {"model": _unknown(), "session": _unknown()},
        "render_fp": "xxh3_128:" + "11" * 8,
        "revision": revision,
        "role": "builder",
        "source": {"id": "4", "digest": "xxh3_128:" + "22" * 8},
        "work_fp": "xxh3_128:" + "33" * 8,
    }


def _provenance(document, authority=AUTHORITY, driver_id="39", generation="1"):
    """Return a provenance block whose stored document has its settings in reverse order."""
    stored = copy.deepcopy(document)
    stored["settings"] = list(reversed(stored["settings"]))
    return {"driver": {"id": driver_id, "authority": authority, "document": stored},
            "native": _native(generation)}


def _wrap(provenance):
    """Wrap a provenance block as the data.execution.provenance part of a history --event expansion."""
    return {"data": {"execution": {"provenance": provenance}}}


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


def _expect(test, condition, message):
    """Fail the test with a fixed message unless condition holds; values are never echoed."""
    if not condition:
        test.fail(message)


def _must_accept(test, label, call):
    """Return call(); an AcceptError or other exception becomes a test failure naming label."""
    try:
        return call()
    except lib.AcceptError as error:
        refused = str(error)
    except Exception:
        refused = None
    if refused is None:
        test.fail(label + " raised an exception other than AcceptError")
    test.fail(label + " was refused: " + refused)


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


def _setting_names(document):
    """Return the settings names in stored order, or None when the document has no settings list."""
    settings = _dig(document, "settings")
    if not isinstance(settings, list):
        return None
    return [item.get("name") if isinstance(item, dict) else None for item in settings]


def _settings_map(test, document):
    """Return {name: value} for a document's settings, failing with fixed text on a malformed list."""
    settings = _dig(document, "settings")
    if not isinstance(settings, list) or not all(isinstance(item, dict) for item in settings):
        test.fail("document settings are not a list of objects")
    names = [item.get("name") for item in settings]
    if len(set(names)) != len(names):
        test.fail("document settings names are not unique")
    return {item.get("name"): item.get("value") for item in settings}


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
        test.fail("the result is not JSON serializable")


def _sha256_file(path):
    """Return the hex sha256 of a file's bytes, read in chunks."""
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _baseline_digest():
    """Recompute the tree digest of git tree 58128234:src/engine from git alone (the tree_sha256 definition)."""
    prefix = b"src/engine/"
    listing = subprocess.run(["git", "-C", str(REPO), "ls-tree", "-r", "-z", BASE_COMMIT, "src/engine"],
                             capture_output=True, check=True).stdout
    records = []
    for entry in listing.split(b"\0"):
        if not entry:
            continue
        meta, _, path = entry.partition(b"\t")
        fields = meta.split(b" ")
        if len(fields) != 3 or fields[1] != b"blob" or fields[0] not in (b"100644", b"100755"):
            continue
        if not path.startswith(prefix):
            continue
        content = subprocess.run(["git", "-C", str(REPO), "cat-file", "blob", fields[2].decode("ascii")],
                                 capture_output=True, check=True).stdout
        records.append((path[len(prefix):], hashlib.sha256(content).hexdigest()))
    digest = hashlib.sha256()
    for relpath, file_hex in sorted(records):
        digest.update(relpath + b"\0" + file_hex.encode("ascii") + b"\n")
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


def _config(root, candidate, tamper=None, verify=None):
    """Return the run config the live rows share: a fresh scratch tree under root for one cmake run.

    verify is set in the config only when given; only ProvenanceLiveTest.setUpClass passes verify=False
    (in-run verification off, so the named assertions of X3-X7 are the detectors). Other callers keep the
    run default (verification on).
    """
    scratch = Path(root) / "run"
    config = {
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
    if verify is not None:
        config["verify"] = verify
    return config


def _reversed_settings(document):
    """Return a deep copy of document with its settings list in reverse order (out of name order)."""
    reordered = copy.deepcopy(document)
    reordered["settings"] = list(reversed(reordered["settings"]))
    return reordered


def _cli_env(root):
    """Return a child environment whose HOME and PLANAR_DB are scratch paths under root."""
    home = Path(root) / "home"
    home.mkdir(exist_ok=True)
    return lib.child_env({"HOME": str(home), "PLANAR_DB": str(Path(root) / "unused-legacy.db")})


def _claim_token(raw_root):
    """Return the claim token from the single retained *-claim.stdout (key run.token), or the empty string."""
    files = sorted(Path(raw_root).glob("*-claim.stdout"))
    if len(files) != 1:
        return ""
    try:
        data = json.loads(files[0].read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, ValueError):
        return ""
    return _text(_dig(data, "run", "token"))


# Offline rows ------------------------------------------------------------------------------------

class ProvenanceDocTest(unittest.TestCase):
    """Offline checks of the driver document, the validator, the normalizer and the read-back verifiers."""

    def test_build_document_has_every_key_and_leaves_model_and_session_unknown(self):
        """Kills: provenance_driver.build_document sets model.actual to the string "unknown" instead of a copy of UNKNOWN."""
        doc = pd.build_document(driver_version="0123456789ab", trial_id=TRIAL, view_id=VIEW,
                                settings={"candidate_size": "4096", "accepted_revision": "1"})
        self.assertEqual(sorted(doc), sorted(TOP_KEYS), "build_document must carry every contract key")
        self.assertEqual(doc["version"], 1, "the document version must be 1")
        self.assertEqual(doc["driver"], {"name": DRIVER_NAME, "version": "0123456789ab"},
                         "driver must carry the driver name and the given version")
        self.assertEqual(doc["trial"], {"id": TRIAL}, "trial must carry the given trial id")
        self.assertEqual(doc["view"], {"id": VIEW}, "view must carry the given view id")
        self.assertEqual(doc["model"], {"requested": _unknown(), "actual": _unknown()},
                         "model leaves must be exactly the unknown object")
        self.assertEqual(doc["session"], {"requested": _unknown(), "actual": _unknown()},
                         "session leaves must be exactly the unknown object")
        self.assertEqual(doc["settings"], [{"name": "accepted_revision", "value": "1"},
                                           {"name": "candidate_size", "value": "4096"}],
                         "settings must be the given pairs sorted by name")

    def test_validate_document_refuses_each_leaf_rule_violation_alone(self):
        """Kills: validate_document deletes the NUL check (second mutant run: > 1024 to > 2048)."""
        cases = []
        doc = _doc()
        doc["trial"]["id"] = ""
        cases.append(("empty leaf", doc, "trial.id"))
        doc = _doc()
        doc["driver"]["version"] = "a" * 1025
        cases.append(("1025-byte leaf", doc, "driver.version"))
        doc = _doc()
        doc["view"]["id"] = "a\x00b"
        cases.append(("NUL in leaf", doc, "view.id"))
        doc = _doc()
        doc["model"]["requested"] = {"status": "known"}
        cases.append(("object other than unknown", doc, "model.requested"))
        doc = _doc()
        del doc["session"]
        cases.append(("missing key", doc, "session"))
        doc = _doc()
        doc["extra"] = "x"
        cases.append(("unknown key", doc, "extra"))
        doc = _doc()
        doc["version"] = 2
        cases.append(("wrong version", doc, "version"))
        doc = _doc()
        doc["settings"] = [{"name": "a", "value": "1"}, {"name": "a", "value": "2"}]
        cases.append(("duplicate settings name", doc, "settings.1"))
        doc = _doc()
        doc["settings"] = [{"name": "n%02d" % index, "value": "1"} for index in range(33)]
        cases.append(("33 settings", doc, "settings"))
        problems = []
        for label, case_doc, path in cases:
            problem = _refusal(label, lambda document=case_doc: pd.validate_document(document),
                               "provenance document: " + path)
            if problem:
                problems.append(problem)
        if problems:
            self.fail("; ".join(problems))

    def test_validate_document_accepts_one_and_1024_byte_leaves_and_unknown_object(self):
        """Kills: validate_document changes > 1024 to >= 1024, so the 1024-byte leaf is refused."""
        one = _doc()
        one["view"]["id"] = "v"
        multi = _doc()
        multi["trial"]["id"] = "é" * 512
        if len(multi["trial"]["id"].encode("utf-8")) != 1024:
            self.fail("the multi-byte fixture must be exactly 1024 UTF-8 bytes")
        unknown = _doc()
        unknown["model"]["requested"] = _unknown()
        many = _doc()
        many["settings"] = [{"name": "n%02d" % index, "value": "1"} for index in range(32)]
        for label, document in (("1-byte leaf", one), ("1024-byte multi-byte leaf", multi),
                                ("unknown-object leaf", unknown), ("32 settings", many)):
            outcome = _must_accept(self, label, lambda document=document: pd.validate_document(document))
            if outcome is not None:
                self.fail(label + " returned a value instead of None")

    def test_normalize_document_sorts_settings_by_unsigned_name_bytes(self):
        """Kills: normalize_document and build_document drop the sorted(...) call."""
        doc = _doc()
        doc["settings"] = [{"name": "é", "value": "1"}, {"name": "B", "value": "2"},
                           {"name": "a", "value": "3"}]
        original = copy.deepcopy(doc)
        normalized = pd.normalize_document(doc)
        self.assertEqual(_setting_names(normalized), ["B", "a", "é"],
                         "settings must be sorted by unsigned name bytes")
        self.assertEqual(doc, original, "normalize_document must not mutate its input")
        self.assertIsNot(normalized, doc, "normalize_document must return a copy")
        built = pd.build_document(driver_version="0123456789ab", trial_id=TRIAL, view_id=VIEW,
                                  settings={"é": "1", "B": "2", "a": "3"})
        self.assertEqual(_setting_names(built), ["B", "a", "é"],
                         "build_document must sort settings by name bytes")

    def test_verify_readback_refuses_wrong_authority(self):
        """Kills: verify_readback deletes the authority comparison."""
        doc = _doc()
        _must_accept(self, "control expansion", lambda: pd.verify_readback(
            _wrap(_provenance(doc)), doc, token=TOKEN, native_expect=NATIVE_EXPECT))
        problems = []
        for label, authority in (("authority none", "none"), ("authority verified", "driver_assertion_verified"),
                                 ("authority empty", "")):
            expansion = _wrap(_provenance(doc, authority=authority))
            problem = _refusal(label, lambda exp=expansion: pd.verify_readback(
                exp, doc, token=TOKEN, native_expect=NATIVE_EXPECT),
                "authority is " + authority, "expected " + AUTHORITY)
            if problem:
                problems.append(problem)
        missing = _wrap(_provenance(doc))
        del missing["data"]["execution"]["provenance"]["driver"]["authority"]
        problem = _refusal("authority missing", lambda exp=missing: pd.verify_readback(
            exp, doc, token=TOKEN, native_expect=NATIVE_EXPECT), "authority is")
        if problem:
            problems.append(problem)
        if problems:
            self.fail("; ".join(problems))

    def test_verify_readback_refuses_document_differing_in_one_leaf(self):
        """Kills: verify_readback compares only document.driver and document.version."""
        doc = _doc()
        _must_accept(self, "control expansion", lambda: pd.verify_readback(
            _wrap(_provenance(doc)), doc, token=TOKEN, native_expect=NATIVE_EXPECT))

        def stored(expansion):
            return expansion["data"]["execution"]["provenance"]["driver"]["document"]

        trial = _wrap(_provenance(doc))
        stored(trial)["trial"]["id"] = "other-trial"
        view = _wrap(_provenance(doc))
        stored(view)["view"]["id"] = "other-view"
        model = _wrap(_provenance(doc))
        stored(model)["model"]["actual"] = "fast"
        setting = _wrap(_provenance(doc))
        for item in stored(setting)["settings"]:
            if item["name"] == "candidate_size":
                item["value"] = "1"
        cases = (("trial.id", trial, "trial.id"), ("view.id", view, "view.id"),
                 ("model.actual", model, "model.actual"), ("settings value", setting, "settings"))
        problems = []
        for label, expansion, path in cases:
            problem = _refusal(label, lambda exp=expansion: pd.verify_readback(
                exp, doc, token=TOKEN, native_expect=NATIVE_EXPECT), "document differs at " + path)
            if problem:
                problems.append(problem)
        if problems:
            self.fail("; ".join(problems))

    def test_verify_readback_refuses_missing_native_key_and_known_preferences(self):
        """Kills: verify_readback skips the preferences check and the key-presence loop."""
        doc = _doc()
        _must_accept(self, "control expansion", lambda: pd.verify_readback(
            _wrap(_provenance(doc)), doc, token=TOKEN, native_expect=NATIVE_EXPECT))
        problems = []
        for key in NATIVE_KEYS:
            expansion = _wrap(_provenance(doc))
            del expansion["data"]["execution"]["provenance"]["native"][key]
            problem = _refusal("native without " + key, lambda exp=expansion: pd.verify_readback(
                exp, doc, token=TOKEN, native_expect=NATIVE_EXPECT), "native lacks " + key)
            if problem:
                problems.append(problem)
        for which in ("model", "session"):
            expansion = _wrap(_provenance(doc))
            expansion["data"]["execution"]["provenance"]["native"]["preferences"][which] = "fast"
            problem = _refusal("known " + which + " preference", lambda exp=expansion: pd.verify_readback(
                exp, doc, token=TOKEN, native_expect=NATIVE_EXPECT),
                "native.preferences." + which + " is not unknown")
            if problem:
                problems.append(problem)
        if problems:
            self.fail("; ".join(problems))

    def test_verify_readback_refuses_bearer_token_anywhere(self):
        """Kills: verify_readback deletes the serialized-token scan."""
        doc = _doc()
        leaked = copy.deepcopy(doc)
        leaked["settings"].append({"name": "note", "value": TOKEN})
        _must_accept(self, "control expansion without the token", lambda: pd.verify_readback(
            _wrap(_provenance(doc)), doc, token=TOKEN, native_expect=NATIVE_EXPECT))
        expansion = _wrap(_provenance(leaked))
        problem = _refusal("token in a settings value", lambda: pd.verify_readback(
            expansion, leaked, token=TOKEN, native_expect=NATIVE_EXPECT), "bearer token present")
        if problem:
            self.fail(problem)

    def test_verify_readback_accepts_exact_expansion(self):
        """Kills X1 (I9): verify_readback compares the un-normalized supplied document (the unsorted-supplied
        control below is then refused, since its settings are out of name order and the stored ones are sorted).
        Kills X2 (I9): verify_readback compares the stored document without normalize_document (the
        reverse-stored control is then refused against the sorted supplied document)."""
        doc = _doc()
        expansion = _wrap(_provenance(doc))
        _must_accept(self, "real-shaped expansion (settings stored in reverse order)",
                     lambda: pd.verify_readback(expansion, doc, token=TOKEN, native_expect=NATIVE_EXPECT))
        sorted_store = _wrap({"driver": {"id": "39", "authority": AUTHORITY, "document": copy.deepcopy(doc)},
                              "native": _native()})
        unsorted_supplied = _reversed_settings(doc)
        _must_accept(self, "supplied document with settings out of name order (stored sorted)",
                     lambda: pd.verify_readback(sorted_store, unsorted_supplied, token=TOKEN,
                                                native_expect=NATIVE_EXPECT))

    def test_refused_claim_is_reported_as_product_defect(self):
        """Kills: require_accepted returns None regardless of the exit status."""
        refused = {"exit": 2, "stderr": json.dumps({"error": {"code": "schema", "message": "refused"}})}
        unparsed = {"exit": 2, "stderr": "not json"}
        problems = []
        problem = _refusal("claim refused with a JSON code", lambda: pd.require_accepted(refused, "claim"),
                           "product defect", "claim", "exit 2", "code schema")
        if problem:
            problems.append(problem)
        problem = _refusal("claim refused with unparsed stderr", lambda: pd.require_accepted(unparsed, "claim"),
                           "product defect", "claim", "exit 2", "code unparsed")
        if problem:
            problems.append(problem)
        if problems:
            self.fail("; ".join(problems))
        outcome = _must_accept(self, "exit 0 step", lambda: pd.require_accepted({"exit": 0, "stderr": ""},
                                                                                "claim"))
        if outcome is not None:
            self.fail("require_accepted did not return None for exit 0")

    def test_verify_absent_readback_accepts_none_driver_and_refuses_stored_record_or_known_leaf(self):
        """Kills: verify_absent_readback deletes the authority none comparison."""
        absent = {
            "driver": {"id": "none", "authority": "none", "document": _all_unknown_document()},
            "native": _native(generation="3"),
        }
        expected_native = dict(NATIVE_EXPECT, generation="3")
        _must_accept(self, "control no-file read-back", lambda: pd.verify_absent_readback(
            _wrap(copy.deepcopy(absent)), token=TOKEN, native_expect=expected_native))
        problems = []
        stored = _wrap(copy.deepcopy(absent))
        stored["data"]["execution"]["provenance"]["driver"]["authority"] = AUTHORITY
        problem = _refusal("stored record (authority)", lambda: pd.verify_absent_readback(
            stored, token=TOKEN, native_expect=expected_native),
            "authority is " + AUTHORITY)
        if problem:
            problems.append(problem)
        known = _wrap(copy.deepcopy(absent))
        known["data"]["execution"]["provenance"]["driver"]["document"]["trial"]["id"] = TRIAL
        problem = _refusal("known leaf", lambda: pd.verify_absent_readback(
            known, token=TOKEN, native_expect=expected_native), "document differs at trial.id")
        if problem:
            problems.append(problem)
        settings = _wrap(copy.deepcopy(absent))
        settings["data"]["execution"]["provenance"]["driver"]["document"]["settings"] = [
            {"name": "driver_phase", "value": "takeover"}]
        problem = _refusal("non-empty settings", lambda: pd.verify_absent_readback(
            settings, token=TOKEN, native_expect=expected_native), "document differs at settings")
        if problem:
            problems.append(problem)
        leaked = _wrap(copy.deepcopy(absent))
        leaked["data"]["token_echo"] = TOKEN
        problem = _refusal("token present", lambda: pd.verify_absent_readback(
            leaked, token=TOKEN, native_expect=expected_native), "bearer token present")
        if problem:
            problems.append(problem)
        if problems:
            self.fail("; ".join(problems))


# Live rows ---------------------------------------------------------------------------------------

class ProvenanceLiveTest(unittest.TestCase):
    """Live rows sharing one run of the claim, the takeover and the no-file takeover through the candidate."""

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
            cls.root = os.path.realpath(tempfile.mkdtemp(prefix="r19-provenance-live-"))
            cls.config = _config(cls.root, candidate, verify=False)
            cls.real_home = _real_home()
            cls.names_before = _planar_names(cls.real_home)
            try:
                cls.result = pd.run_provenance(cls.config)
            finally:
                cls.names_after = _planar_names(cls.real_home)
            if not isinstance(cls.result, dict):
                cls.error = "provenance run returned no result object"
        except Exception as error:
            cls.error = "provenance run did not complete: " + (str(error) or type(error).__name__)

    @classmethod
    def tearDownClass(cls):
        """Remove the shared scratch tree."""
        if cls.root:
            shutil.rmtree(cls.root, ignore_errors=True)

    # Fresh reads and recomputed facts ------------------------------------------------------------

    def _fresh(self, verb, *args):
        """Run one fresh candidate read of the run ledger with HOME and PLANAR_DB in scratch; return its JSON."""
        config = self.config
        scratch = Path(config["scratch"])
        env = lib.child_env({"HOME": config["home"], "PLANAR_DB": str(scratch / "unused-legacy.db")})
        argv = [config["candidate"], verb, "--ledger", os.path.realpath(config["ledger"]), *map(str, args)]
        try:
            proc = subprocess.run(argv, capture_output=True, env=env, cwd=os.path.realpath(config["scratch"]),
                                  timeout=600)
        except (OSError, subprocess.SubprocessError):
            self.fail("fresh " + verb + " read could not run")
        if proc.returncode != 0:
            self.fail("fresh " + verb + " read exited %d" % proc.returncode)
        try:
            return json.loads(proc.stdout.decode("utf-8"))
        except (UnicodeDecodeError, ValueError):
            self.fail("fresh " + verb + " read printed no JSON")

    def _status(self):
        """Return the fresh status read, which must carry active_revision and ledger."""
        status = self._fresh("status")
        _expect(self, isinstance(status, dict) and "active_revision" in status and "ledger" in status,
                "status read lacks active_revision or ledger")
        return status

    def _event(self, kind, request_id):
        """Return the one history event of this type and request id, found in a fresh history read."""
        events = _dig(self._fresh("history", "--limit", "100"), "data", "events")
        if not isinstance(events, list):
            self.fail("history read lacks data.events")
        matches = [event for event in events
                   if isinstance(event, dict) and event.get("type") == kind and event.get("request_id") == request_id]
        if len(matches) != 1:
            self.fail("expected exactly one %s event for %s" % (kind, request_id))
        return matches[0]

    def _expansion(self, kind, request_id):
        """Return (sequence, provenance block, full expansion) from a fresh history --event read."""
        event = self._event(kind, request_id)
        full = self._fresh("history", "--event", str(event.get("sequence")))
        provenance = _dig(full, "data", "execution", "provenance")
        if not isinstance(provenance, dict):
            self.fail("history --event lacks data.execution.provenance for " + kind)
        return event.get("sequence"), provenance, full

    def _section(self, name):
        """Return one section of the shared result, failing with fixed text when it is absent."""
        return _section(self, self.result, name)

    def _retained(self, pattern):
        """Return the sorted retained raw files of the shared run that match pattern."""
        return sorted(Path(self.config["raw_root"]).glob(pattern))

    def _candidate_sha(self):
        """Return the sha256 of the candidate bytes, recomputed by the test."""
        try:
            return _sha256_file(self.config["candidate"])
        except OSError:
            self.fail("the candidate binary cannot be read for its digest")

    def _candidate_size(self):
        """Return the size of the candidate binary, recomputed by the test."""
        try:
            return os.stat(self.config["candidate"]).st_size
        except OSError:
            self.fail("the candidate binary cannot be stat'ed for its size")

    def _baseline_sha(self):
        """Return the recomputed baseline digest of git tree 58128234:src/engine."""
        try:
            return _baseline_digest()
        except (OSError, subprocess.CalledProcessError):
            self.fail("git could not read tree 58128234:src/engine for the baseline digest")

    def _driver_version(self):
        """Return the first 12 hex digits of the sha256 of provenance_driver.py bytes, recomputed by the test."""
        return hashlib.sha256((HERE / "provenance_driver.py").read_bytes()).hexdigest()[:12]

    def _check_native(self, native, generation, status):
        """Check the ten native keys and the known native values for one run."""
        _expect(self, isinstance(native, dict) and sorted(native) == sorted(NATIVE_KEYS),
                "native block does not carry exactly the ten keys")
        _expect(self, native["ledger"] == _text(status["ledger"]) and native["ledger"],
                "native ledger differs from status ledger")
        _expect(self, native["revision"] == str(status["active_revision"]),
                "native revision differs from status active_revision")
        _expect(self, native["job"] == "combined", "native job is not combined")
        _expect(self, native["role"] == "builder", "native role is not builder")
        _expect(self, native["generation"] == generation, "native generation differs from the expected generation")
        _expect(self, native["base_label"] == BASE_LABEL, "native base_label differs from the R19-CMAKE baseline")
        _expect(self, _text(_dig(native, "source", "digest")).startswith("xxh3_128:"),
                "native source digest is not an xxh3_128 digest")
        _expect(self, _text(native["render_fp"]).startswith("xxh3_128:"), "native render_fp is not an xxh3_128 digest")
        _expect(self, _text(native["work_fp"]).startswith("xxh3_128:"), "native work_fp is not an xxh3_128 digest")
        _expect(self, native["preferences"] == {"model": _unknown(), "session": _unknown()},
                "native preferences are not unknown")

    # Rows ----------------------------------------------------------------------------------------

    def test_claim_argv_passes_provenance_file_holding_the_honest_document(self):
        """Kills: run_provenance drops --provenance <path> from the claim argv."""
        if self.error:
            self.fail(self.error)
        claim = self._section("claim")
        takeover = self._section("takeover")
        nofile = self._section("nofile")
        for name, info in (("claim", claim), ("takeover", takeover)):
            argv = info.get("argv")
            _expect(self, isinstance(argv, list) and "--provenance" in argv, name + " argv lacks --provenance")
            index = argv.index("--provenance")
            path = info.get("provenance_path")
            _expect(self, index + 1 < len(argv) and argv[index + 1] == path,
                    name + " argv does not pass provenance_path after --provenance")
            _expect(self, isinstance(path, str) and Path(path).is_file(), name + " provenance file is missing")
            raw_bytes = Path(path).read_bytes()
            _expect(self, hashlib.sha256(raw_bytes).hexdigest() == info.get("provenance_sha256"),
                    name + " provenance_sha256 does not match the file bytes")
            written = _read_json(self, path, name + " provenance file")
            _expect(self, written == info.get("document"),
                    name + " provenance file does not hold the driver-validated document")
            _expect(self, _settings_map(self, written).get("driver_phase") == name,
                    name + " provenance file does not record its driver phase")
        retained = self._retained("*-claim.argv.json")
        _expect(self, len(retained) == 1, "expected exactly one retained claim argv file")
        retained_argv = _read_json(self, retained[0], "retained claim argv")
        _expect(self, retained_argv == claim.get("argv"), "retained claim argv differs from the result argv")
        nofile_argv = nofile.get("argv")
        _expect(self, isinstance(nofile_argv, list) and "--provenance" not in nofile_argv,
                "the no-file takeover argv carries --provenance")
        for name, info in (("claim", claim), ("takeover", takeover), ("nofile", nofile)):
            _expect(self, info.get("exit") == 0, name + " did not exit 0")

    def test_history_readback_shows_supplied_leaves_and_unknown_leaves(self):
        """Kills: run_provenance builds the claim document with every leaf UNKNOWN."""
        if self.error:
            self.fail(self.error)
        status = self._status()
        _seq, provenance, _full = self._expansion("run_claimed", "claim-builder-1")
        document = _dig(provenance, "driver", "document")
        _expect(self, _dig(provenance, "driver", "authority") == AUTHORITY,
                "claim authority is not driver_assertion_unverified")
        _expect(self, _dig(document, "driver", "name") == DRIVER_NAME, "claim driver name differs")
        _expect(self, _dig(document, "trial", "id") == TRIAL, "claim trial id differs")
        _expect(self, _dig(document, "view", "id") == VIEW, "claim view id differs")
        _expect(self, _dig(document, "model") == {"requested": _unknown(), "actual": _unknown()},
                "claim model leaves are not unknown")
        _expect(self, _dig(document, "session") == {"requested": _unknown(), "actual": _unknown()},
                "claim session leaves are not unknown")
        settings = _settings_map(self, document)
        _expect(self, settings.get("driver_phase") == "claim", "claim driver_phase is not claim")
        _expect(self, settings.get("baseline_ref") == BASE_REF, "claim baseline_ref differs")
        _expect(self, settings.get("candidate_sha256") == self._candidate_sha(),
                "claim candidate_sha256 differs from the recomputed candidate digest")
        _expect(self, settings.get("baseline_sha256") == self._baseline_sha(),
                "claim baseline_sha256 differs from the recomputed baseline digest")
        _expect(self, settings.get("accepted_revision") == str(status["active_revision"]),
                "claim accepted_revision differs from status active_revision")

    def test_history_readback_shows_native_derived_block(self):
        """Kills: run_provenance reads the job_activated event instead of run_claimed for claim.readback.

        Kills X3 (I13, named assertion): _ProvenanceScenario.run sets claim_prov = takeover_prov, so the result
        claim read-back differs from the independent fresh run_claimed read (named message below)."""
        if self.error:
            self.fail(self.error)
        status = self._status()
        _seq, provenance, _full = self._expansion("run_claimed", "claim-builder-1")
        self._check_native(_dig(provenance, "native"), "1", status)
        _expect(self, self._section("claim").get("readback") == provenance,
                "result claim readback differs from the independent history read")
        _expect(self, _settings_map(self, self._section("claim").get("document")).get("driver_phase") == "claim",
                "result claim document driver_phase is not claim")

    def test_takeover_stores_its_own_provenance_record(self):
        """Kills: run_provenance omits --provenance from the takeover argv or reuses the claim file."""
        if self.error:
            self.fail(self.error)
        status = self._status()
        _claim_seq, claim_provenance, claim_full = self._expansion("run_claimed", "claim-builder-1")
        claim_run = str(_dig(claim_full, "data", "execution", "run", "id"))
        _seq, takeover_provenance, takeover_full = self._expansion("run_taken_over", "takeover-1")
        _expect(self, _dig(takeover_provenance, "driver", "authority") == AUTHORITY,
                "takeover authority is not driver_assertion_unverified")
        takeover_document = _dig(takeover_provenance, "driver", "document")
        claim_document = _dig(claim_provenance, "driver", "document")
        _expect(self, _settings_map(self, takeover_document).get("driver_phase") == "takeover",
                "takeover driver_phase is not takeover")
        _expect(self, _dig(takeover_document, "trial", "id") == _dig(claim_document, "trial", "id"),
                "takeover trial id differs from the claim trial id")
        _expect(self, _dig(takeover_provenance, "driver", "id") != _dig(claim_provenance, "driver", "id"),
                "takeover driver id equals the claim driver id")
        self._check_native(_dig(takeover_provenance, "native"), "2", status)
        _expect(self, str(_dig(takeover_full, "data", "execution", "previous_run")) == claim_run,
                "takeover previous_run is not the claim run id")
        _expect(self, self._section("takeover").get("readback") == takeover_provenance,
                "result takeover readback differs from the independent history read")
        _expect(self, str(self._section("takeover").get("previous_run")) == claim_run,
                "result takeover previous_run is not the claim run id")

    def test_token_never_appears_in_provenance_files_or_readbacks(self):
        """Kills X6 (I15, named assertion): the takeover document build adds the setting session_token equal to
        the claim run token, so the takeover provenance file holds the claim token (named message below).
        Also requires the live result to report token_absent true (the scan of I1)."""
        if self.error:
            self.fail(self.error)
        token = _claim_token(self.config["raw_root"])
        _expect(self, len(token) >= 16, "the retained claim stdout carries no claim token to check against")
        for section in ("claim", "takeover"):
            path = self._section(section).get("provenance_path")
            _expect(self, isinstance(path, str) and Path(path).is_file(), section + " provenance file is missing")
            _expect(self, token.encode("utf-8") not in Path(path).read_bytes(),
                    section + " provenance file holds the claim token")
        histories = self._retained("*-history.stdout")
        _expect(self, len(histories) > 0, "no retained history stdout exists to scan")
        for path in histories:
            _expect(self, token.encode("utf-8") not in path.read_bytes(),
                    "a retained history stdout holds the claim token")
        _expect(self, token not in _dumps(self, self.result), "the result holds the claim token")
        _expect(self, self.result.get("token_absent") is True,
                "the live result does not report token_absent true")

    def test_run_uses_scratch_home_and_leaves_real_home_untouched(self):
        """Kills: run_provenance sets scenario.env = dict(os.environ) after prepare() (D36, D37)."""
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
        real = os.path.realpath(self.real_home)
        _expect(self, os.path.realpath(env["HOME"]) != real, "the result env HOME is the real home")
        if _names_digest(self.names_before) != _names_digest(self.names_after):
            before = set(self.names_before or [])
            after = set(self.names_after or [])
            added = ", ".join(sorted(after - before)) or "none"
            removed = ", ".join(sorted(before - after)) or "none"
            self.fail("real home .planar top-level entry names changed during the run (added: %s; removed: %s); "
                      "a foreign process may be the cause" % (added, removed))

    def test_takeover_without_provenance_reads_back_all_unknown_driver(self):
        """Kills: run_provenance passes the takeover provenance file on the final takeover step."""
        if self.error:
            self.fail(self.error)
        status = self._status()
        _seq1, _provenance1, takeover1_full = self._expansion("run_taken_over", "takeover-1")
        takeover1_run = str(_dig(takeover1_full, "data", "execution", "run", "id"))
        _seq, provenance, full = self._expansion("run_taken_over", "takeover-2")
        _expect(self, _dig(provenance, "driver", "id") == "none", "no-file takeover driver id is not none")
        _expect(self, _dig(provenance, "driver", "authority") == "none", "no-file takeover authority is not none")
        _expect(self, _dig(provenance, "driver", "document") == _all_unknown_document(),
                "no-file takeover document is not the all-unknown document with no settings")
        self._check_native(_dig(provenance, "native"), "3", status)
        _expect(self, str(_dig(full, "data", "execution", "previous_run")) == takeover1_run,
                "no-file takeover previous_run is not the first takeover run id")
        nofile = self._section("nofile")
        _expect(self, nofile.get("readback") == provenance, "result nofile readback differs from the independent read")
        _expect(self, isinstance(nofile.get("argv"), list) and "--provenance" not in nofile["argv"],
                "the no-file takeover argv carries --provenance")

    def test_documents_carry_recomputed_driver_version_and_candidate_size(self):
        """Kills: run_provenance passes the constant "v1" as driver_version, or candidate_size from len(path)."""
        if self.error:
            self.fail(self.error)
        version = self._driver_version()
        size = self._candidate_size()
        _expect(self, _dig(self.result, "candidate", "size") == size,
                "result candidate size differs from the binary size")
        for kind, request_id in (("run_claimed", "claim-builder-1"), ("run_taken_over", "takeover-1")):
            _seq, provenance, _full = self._expansion(kind, request_id)
            document = _dig(provenance, "driver", "document")
            _expect(self, _dig(document, "driver", "version") == version,
                    kind + " driver.version differs from the recomputed driver digest")
            _expect(self, _settings_map(self, document).get("candidate_size") == str(size),
                    kind + " candidate_size differs from the binary size")

    def test_live_fixture_runs_with_verification_off_and_reports_it(self):
        """Kills V1 (E24): run_provenance reports "verified": True constantly. The shared fixture runs with
        in-run verification off (config verify False), so the result must say verified is False."""
        if self.error:
            self.fail(self.error)
        _expect(self, self.result.get("verified") is False,
                "the live fixture result does not report verification off (verified is not false)")


def _section(test, result, name):
    """Return one section of a run result, failing with fixed text when it is absent."""
    value = result.get(name) if isinstance(result, dict) else None
    if not isinstance(value, dict):
        test.fail("result lacks the " + name + " section")
    return value


class ProvenanceLiveRefusalTest(unittest.TestCase):
    """Live refusal row: a claim whose provenance file is missing is a product defect, and the run aborts."""

    def setUp(self):
        """Require the candidate and a fresh scratch root for this run."""
        self.candidate = os.environ.get("PLANAR_CANDIDATE", "")
        if not self.candidate:
            self.fail("PLANAR_CANDIDATE must name the D28 candidate binary; live rows fail, never skip")
        self.root = os.path.realpath(tempfile.mkdtemp(prefix="r19-provenance-refusal-"))
        self.addCleanup(shutil.rmtree, self.root, True)

    def test_run_provenance_aborts_as_product_defect_when_claim_is_refused(self):
        """Kills: run_provenance deletes the require_accepted(step, claim) call."""
        config = _config(self.root, self.candidate, tamper={"provenance_path": "missing"})
        message = ""
        try:
            pd.run_provenance(config)
        except lib.AcceptError as error:
            message = str(error)
        except Exception:
            self.fail("run_provenance raised an exception other than AcceptError")
        else:
            self.fail("run_provenance accepted a claim whose provenance file is missing")
        _expect(self, "product defect" in message and "claim" in message,
                "the refused claim is not reported as a product defect naming claim")
        raw = Path(config["raw_root"])
        _expect(self, (raw / "ABORTED.json").is_file(), "ABORTED.json is missing from raw_root")
        _expect(self, not list(raw.glob("*-takeover.argv.json")),
                "a takeover argv file exists after the claim was refused")


# CLI rows ----------------------------------------------------------------------------------------

class ProvenanceCliTest(unittest.TestCase):
    """CLI rows: the provenance command verifies the candidate, then writes result.json with both read-backs."""

    def setUp(self):
        """Create a per-test temporary root for the subprocess cases."""
        self.tmp = os.path.realpath(tempfile.mkdtemp(prefix="r19-provenance-cli-"))
        self.addCleanup(shutil.rmtree, self.tmp, True)

    def test_provenance_command_aborts_before_first_action_on_candidate_mismatch(self):
        """Kills: cmd_provenance calls provenance_driver.run_provenance directly instead of via run_live."""
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
        argv = [PY, "-B", str(RUN), "provenance", "--candidate", str(stub), "--record", str(record),
                "--repo-path", str(REPO), "--raw-root", str(raw)]
        try:
            proc = subprocess.run(argv, env=_cli_env(tmp), cwd=str(tmp), capture_output=True, text=True,
                                  timeout=600)
        except (OSError, subprocess.SubprocessError):
            self.fail("the provenance command could not be started")
        if proc.returncode == 0:
            self.fail("provenance exited 0 for a candidate that differs from the recorded candidate")
        if sentinel.exists():
            self.fail("the stub candidate ran before the recorded-candidate check")
        if list(raw.rglob("*.argv.json")):
            self.fail("an action argv file exists after a candidate mismatch")
        if "does not match the recorded candidate" not in proc.stderr:
            self.fail("provenance did not report the recorded-candidate mismatch (exit %d)" % proc.returncode)

    def test_provenance_command_writes_result_json_with_both_readbacks(self):
        """Kills M1 (in-run verification on): cmd_provenance sets config verify False, so result.json lacks
        "verified": true. Kills M2 (a34-readback.json): cmd_provenance skips writing the path-free manifest
        beside result.json, or writes one that differs from readback_manifest(result) or holds a path.
        Kills the earlier mutant: cmd_provenance writes the result without the takeover section."""
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
        argv = [PY, "-B", str(RUN), "provenance", "--candidate", candidate, "--record", str(record_path),
                "--repo-path", str(REPO), "--raw-root", str(raw), "--run-name", "a34-cli"]
        try:
            proc = subprocess.run(argv, env=_cli_env(tmp), cwd=str(tmp), capture_output=True, text=True,
                                  timeout=1800)
        except (OSError, subprocess.SubprocessError):
            self.fail("the provenance command could not be run to completion")
        if proc.returncode != 0:
            self.fail("provenance exited %d, expected 0" % proc.returncode)
        lines = [line for line in proc.stdout.splitlines() if line.strip()]
        run_dir = Path(lines[-1]) if lines else None
        if run_dir is None or not run_dir.is_dir():
            self.fail("provenance did not print an existing run directory as its last line")
        if not (run_dir / "result.json").is_file():
            self.fail("the run directory lacks result.json")
        if not (run_dir / "scratch" / "ledger.db").is_file():
            self.fail("the run directory lacks scratch/ledger.db")
        result = _read_json(self, run_dir / "result.json", "result.json")
        token = _claim_token(run_dir / "raw")
        if len(token) < 16:
            self.fail("the retained claim stdout carries no claim token to check against")
        native_generations = {"claim": "1", "takeover": "2"}
        for section in ("claim", "takeover"):
            info = _section(self, result, section)
            readback = info.get("readback")
            if _dig(readback, "driver", "authority") != AUTHORITY:
                self.fail(section + " read-back authority is not driver_assertion_unverified")
            try:
                pd.verify_readback(_wrap(readback), info.get("document"), token=token,
                                   native_expect=dict(NATIVE_EXPECT, generation=native_generations[section]))
            except lib.AcceptError as error:
                self.fail(section + " read-back refused: " + str(error))
            except Exception:
                self.fail(section + " read-back verifier raised an exception other than AcceptError")
        nofile = _section(self, result, "nofile")
        try:
            pd.verify_absent_readback(_wrap(nofile.get("readback")), token=token,
                                      native_expect=dict(NATIVE_EXPECT, generation="3"))
        except lib.AcceptError as error:
            self.fail("no-file read-back refused: " + str(error))
        except Exception:
            self.fail("no-file read-back verifier raised an exception other than AcceptError")
        if result.get("verified") is not True:
            self.fail("the result does not report in-run verification on (verified is not true)")
        readback_path = run_dir / "a34-readback.json"
        if not readback_path.is_file():
            self.fail("the run directory lacks a34-readback.json")
        manifest = _read_json(self, readback_path, "a34-readback.json")
        if not isinstance(manifest, dict) or manifest.get("version") != 1:
            self.fail("a34-readback.json does not carry version 1")
        if manifest != pd.readback_manifest(result):
            self.fail("a34-readback.json differs from readback_manifest(result)")
        raw_text = readback_path.read_bytes()
        scratch_home = run_dir / "scratch" / "home"
        forbidden = (("run directory", str(run_dir)), ("run directory", os.path.realpath(run_dir)),
                     ("scratch home", str(scratch_home)), ("scratch home", os.path.realpath(scratch_home)),
                     ("candidate", candidate), ("candidate", os.path.realpath(candidate)))
        for label, value in forbidden:
            if value.encode("utf-8") in raw_text:
                self.fail("a34-readback.json holds a host path (" + label + ")")


# Offline manifest rows ---------------------------------------------------------------------------

MANIFEST_KEYS = ["checks", "claim", "facts", "nofile", "takeover", "version"]


def _synthetic_result():
    """Return a run result the test builds itself, with distinct values per field and no driver output."""
    claim_doc = _doc("claim")
    takeover_doc = _doc("takeover")
    return {
        "scenario": "a34-provenance",
        "mode": "cmake",
        "takeover_applicable": True,
        "candidate": {"sha256": "c" * 64, "size": 3141592},
        "env": {"HOME": "/scratch/run/home", "PLANAR_DB": "/scratch/run/unused-legacy.db"},
        "ledger": "5" * 32,
        "verified": True,
        "token_absent": True,
        "facts": {"accepted_revision": "7", "baseline_ref": BASE_REF, "baseline_sha256": "a" * 64,
                  "candidate_sha256": "c" * 64, "candidate_size": 3141592},
        "steps": [{"index": 0, "verb": "claim", "argv": ["claim", "--request-id", "claim-builder-1"], "exit": 0}],
        "claim": {"request_id": "claim-builder-1", "exit": 0,
                  "argv": ["claim", "--provenance", "/scratch/provenance-claim.json"],
                  "provenance_path": "/scratch/provenance-claim.json", "provenance_sha256": "e" * 64,
                  "document": claim_doc, "event_id": "11", "run_id": "21",
                  "readback": _provenance(claim_doc, driver_id="31", generation="1")},
        "takeover": {"request_id": "takeover-1", "exit": 0,
                     "argv": ["takeover", "--provenance", "/scratch/provenance-takeover.json"],
                     "provenance_path": "/scratch/provenance-takeover.json", "provenance_sha256": "f" * 64,
                     "document": takeover_doc, "event_id": "12", "run_id": "22", "previous_run": "21",
                     "readback": _provenance(takeover_doc, driver_id="32", generation="2")},
        "nofile": {"request_id": "takeover-2", "exit": 0, "argv": ["takeover", "--request-id", "takeover-2"],
                   "event_id": "13", "run_id": "23", "previous_run": "22",
                   "readback": {"driver": {"id": "none", "authority": "none", "document": _all_unknown_document()},
                                "native": _native(generation="3")}},
    }


class ProvenanceManifestTest(unittest.TestCase):
    """Offline rows for readback_manifest and scan_for_tokens; each test has its own temporary root."""

    def setUp(self):
        """Create a per-test temporary root for the file-based scan row."""
        self.root = os.path.realpath(tempfile.mkdtemp(prefix="r19-provenance-manifest-"))
        self.addCleanup(shutil.rmtree, self.root, True)

    def test_readback_manifest_has_exact_keys_and_values_from_result(self):
        """Kills M3 (readback_manifest omits the nofile block): the manifest must carry exactly the keys
        version, facts, claim, takeover, nofile and checks, and each block must equal the result's values.
        The driver version is read from the claim document's driver.version (the only source in the result)."""
        result = _synthetic_result()
        manifest = pd.readback_manifest(result)
        _expect(self, isinstance(manifest, dict) and sorted(manifest) == MANIFEST_KEYS,
                "readback_manifest does not carry exactly the keys version, facts, claim, takeover, nofile, checks")
        _expect(self, type(manifest["version"]) is int and manifest["version"] == 1,
                "readback_manifest version is not the integer 1")
        facts = result["facts"]
        expected_facts = {"accepted_revision": facts["accepted_revision"], "baseline_ref": facts["baseline_ref"],
                          "baseline_sha256": facts["baseline_sha256"],
                          "candidate_sha256": facts["candidate_sha256"],
                          "candidate_size": facts["candidate_size"],
                          "driver_version": result["claim"]["document"]["driver"]["version"]}
        _expect(self, manifest["facts"] == expected_facts, "readback_manifest facts differ from the result facts")
        for section in ("claim", "takeover", "nofile"):
            info = result[section]
            expected = {"event_id": info["event_id"], "run_id": info["run_id"], "provenance": info["readback"]}
            _expect(self, manifest[section] == expected,
                    "readback_manifest " + section + " block differs from the result event, run or read-back")

    def test_readback_manifest_holds_no_path_argv_or_token(self):
        """Kills M4 (readback_manifest copies result env into checks): marker values for HOME, PLANAR_DB, the
        ledger path, provenance_path, an argv element and a bearer token are placed in the result; none may
        appear in the manifest. The positive key check comes first so the manifest is not vacuously empty."""
        result = _synthetic_result()
        result["env"] = {"HOME": "/MARK-home-6a1f/run/home", "PLANAR_DB": "/MARK-db-6a1f/unused-legacy.db"}
        result["ledger_path"] = "/MARK-ledger-6a1f/run/ledger.db"
        result["claim"]["provenance_path"] = "/MARK-prov-6a1f/provenance-claim.json"
        result["claim"]["argv"] = ["claim", "--note", "MARK-argv-6a1f"]
        result["steps"][0]["argv"] = ["claim", "--token", "tok-MARK-6a1f"]
        manifest = pd.readback_manifest(result)
        _expect(self, isinstance(manifest, dict) and sorted(manifest) == MANIFEST_KEYS,
                "readback_manifest does not carry exactly the keys version, facts, claim, takeover, nofile, checks")
        text = json.dumps(manifest, sort_keys=True)
        _expect(self, "MARK-home-6a1f" not in text and "MARK-db-6a1f" not in text,
                "readback_manifest holds the scratch HOME or PLANAR_DB")
        _expect(self, "MARK-ledger-6a1f" not in text, "readback_manifest holds the ledger path")
        _expect(self, "MARK-prov-6a1f" not in text, "readback_manifest holds a provenance_path")
        _expect(self, "MARK-argv-6a1f" not in text, "readback_manifest holds an argv element")
        _expect(self, "MARK-token-6a1f" not in text, "readback_manifest holds a bearer token")

    def test_readback_manifest_recomputes_each_check_from_the_blocks(self):
        """Kills M5 (readback_manifest sets generation_expected to the constant True): the takeover native
        generation is 9, the nofile preferences are not unknown, the claim native lacks render_fp and the
        stored claim document differs from the supplied one. Exactly those four checks are False; the rest,
        including verified and token_absent_in_run_dir, are True."""
        result = _synthetic_result()
        del result["claim"]["readback"]["native"]["render_fp"]
        result["takeover"]["readback"]["native"]["generation"] = "9"
        result["nofile"]["readback"]["native"]["preferences"]["model"] = "fast"
        result["claim"]["readback"]["driver"]["document"]["settings"] = [
            dict(item, value="1") if item["name"] == "candidate_size" else item
            for item in result["claim"]["readback"]["driver"]["document"]["settings"]]
        manifest = pd.readback_manifest(result)
        expected = {
            "token_absent_in_run_dir": True,
            "verified": True,
            "claim": {"native_ten_keys": False, "preferences_unknown": True, "generation_expected": True,
                      "document_matches_supplied": False},
            "takeover": {"native_ten_keys": True, "preferences_unknown": True, "generation_expected": False,
                         "document_matches_supplied": True},
            "nofile": {"native_ten_keys": True, "preferences_unknown": False, "generation_expected": True},
        }
        _expect(self, isinstance(manifest, dict) and manifest.get("checks") == expected,
                "readback_manifest checks differ from the recomputed per-block checks")

    def test_scan_for_tokens_finds_a_token_in_any_listed_file(self):
        """Kills M6 (scan_for_tokens returns True without reading the files): the token is found in a listed
        file at the middle and at the last position, a missing path is skipped, clean files return True and
        empty tokens are refused with the fixed message."""
        root = Path(self.root)
        clean_a = root / "a.txt"
        clean_a.write_text("nothing to see here\n", encoding="utf-8")
        holder = root / "b.txt"
        holder.write_text("prefix " + TOKEN + " suffix\n", encoding="utf-8")
        clean_c = root / "c.txt"
        clean_c.write_text("more clean text\n", encoding="utf-8")
        missing = root / "missing.txt"
        _expect(self, pd.scan_for_tokens([clean_a, holder, clean_c], [TOKEN]) is False,
                "scan_for_tokens did not report the token in a listed file")
        _expect(self, pd.scan_for_tokens([clean_a, clean_c, holder], [TOKEN]) is False,
                "scan_for_tokens did not report the token in the last listed file")
        _expect(self, pd.scan_for_tokens([clean_a, missing, clean_c], [TOKEN]) is True,
                "scan_for_tokens did not return True for clean files with a missing path")
        problem = _refusal("empty token list", lambda: pd.scan_for_tokens([clean_a], []),
                           "provenance read-back: no token was supplied to scan for")
        if problem:
            self.fail(problem)


if __name__ == "__main__":
    unittest.main()
