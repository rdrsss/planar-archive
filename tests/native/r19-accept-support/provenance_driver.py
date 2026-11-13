#!/usr/bin/env python3
# R19-ACCEPT-B provenance driver: writes the driver-assertion provenance documents of
# the minimal A34 run (a claim with --provenance, a takeover with --provenance and a
# final takeover without it) through the candidate, then reads each stored record
# back in fresh history processes and checks it against the facts this driver
# recomputes. It reuses fault_driver._Scenario in cmake mode (check_mode "sleep",
# base "plain"), so the baseline is the 58128234 engine tree and the workspace is
# engine_cmake with base label baseline_head_cmake58128234.
#
# Config (run_provenance; the keys of fault_driver.run_scenario):
#   candidate, scratch, raw_root, home, ledger, counter, repo (default the checkout,
#   read only), tamper (dict; provenance_path "missing" points the claim at a file
#   that does not exist), verify (bool, default True; False skips the in-run
#   verify_readback and verify_absent_readback loop). mode, check_mode and base are
#   forced to cmake, sleep, plain.
#
# Result (run_provenance): scenario, mode, takeover_applicable, candidate {sha256,
#   size}, env {HOME, PLANAR_DB} (copied from the environment given to every child,
#   never from config or os.environ), ledger (digest of the ledger bytes), facts
#   {accepted_revision, baseline_ref, baseline_sha256, candidate_sha256,
#   candidate_size}, verified (true only when the in-run read-back loop ran),
#   token_absent (true when scan_for_tokens over the retained provenance files and the
#   retained *-history.stdout files found none of the three bearer tokens), steps
#   [{index, verb, argv, exit}], claim and takeover {request_id, exit, argv,
#   provenance_path, provenance_sha256, document, event_id, run_id, readback}
#   (takeover adds previous_run), nofile {request_id, exit, argv, event_id, run_id,
#   previous_run, readback}. Tokens never appear in the result.
#
# Abort contract: an AcceptError writes raw_root/ABORTED.json (verb, error,
#   ledger_digest) and re-raises. A native refusal of a driver-validated document is
#   a product defect (require_accepted): the run stops, nothing is worked around, and
#   the retained raw files hold the exact argv, stdout and stderr.
import sys

sys.dont_write_bytecode = True

import copy
import hashlib
import json
import os
from pathlib import Path

HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))

import accept_lib  # noqa: E402
import fault_driver  # noqa: E402

UNKNOWN = {"status": "unknown"}
DRIVER_NAME = "r19-accept-support/provenance_driver.py"
TRIAL_ID = "r19-accept-b-a34-minimal"
VIEW_ID = "source"
BASE_LABEL = "baseline_head_cmake58128234"
BASE_REF = "58128234:src/engine"
AUTHORITY = "driver_assertion_unverified"
JOB = fault_driver.JOB
BUILDER = fault_driver.BUILDER
REPLACEMENT = fault_driver.REPLACEMENT
TOP_KEYS = ("version", "driver", "trial", "view", "model", "session", "settings")
SECTIONS = (
    ("driver", ("name", "version")),
    ("trial", ("id",)),
    ("view", ("id",)),
    ("model", ("requested", "actual")),
    ("session", ("requested", "actual")),
)
SETTING_KEYS = ("name", "value")
MAX_SETTINGS = 32
MAX_STRING_BYTES = 1024
NATIVE_KEYS = ("base_label", "generation", "job", "ledger", "preferences", "render_fp", "revision", "role",
               "source", "work_fp")
PREFERENCES = ("model", "session")


# Document shape and leaf rules -------------------------------------------------------------------

def _unknown():
    """Return a fresh copy of the unknown leaf."""
    return dict(UNKNOWN)


def _all_unknown_document():
    """Return the all-unknown driver document with no settings (the shape of a takeover without a file)."""
    return {
        "version": 1,
        "driver": {"name": _unknown(), "version": _unknown()},
        "trial": {"id": _unknown()},
        "view": {"id": _unknown()},
        "model": {"requested": _unknown(), "actual": _unknown()},
        "session": {"requested": _unknown(), "actual": _unknown()},
        "settings": [],
    }


def build_document(*, driver_version, trial_id, view_id, settings):
    """Return the honest driver_provenance_v1 document; model and session leaves are fresh unknown leaves.

    settings maps name to string value and is emitted sorted by the unsigned bytes of each name.
    """
    ordered = sorted(settings.items(), key=lambda item: item[0].encode("utf-8"))
    return {
        "version": 1,
        "driver": {"name": DRIVER_NAME, "version": driver_version},
        "trial": {"id": trial_id},
        "view": {"id": view_id},
        "model": {"requested": _unknown(), "actual": _unknown()},
        "session": {"requested": _unknown(), "actual": _unknown()},
        "settings": [{"name": name, "value": value} for name, value in ordered],
    }


def _invalid(path, reason):
    """Return the AcceptError for a document rule violation at a dotted path."""
    return accept_lib.AcceptError(f"provenance document: {path}: {reason}")


def _check_string(path, value):
    """Refuse a string leaf that holds NUL or is not 1 to 1024 UTF-8 bytes."""
    if "\x00" in value:
        raise _invalid(path, "must not contain NUL")
    try:
        size = len(value.encode("utf-8"))
    except UnicodeEncodeError as error:
        raise _invalid(path, "must be valid UTF-8") from error
    if size < 1 or size > MAX_STRING_BYTES:
        raise _invalid(path, "must be 1 to 1024 UTF-8 bytes")


def _check_leaf(path, value):
    """Refuse a leaf that is neither a valid string nor exactly the unknown object."""
    if isinstance(value, dict):
        if value == UNKNOWN:
            return
        raise _invalid(path, "must be a string or the unknown object")
    if not isinstance(value, str):
        raise _invalid(path, "must be a string or the unknown object")
    _check_string(path, value)


def validate_document(doc):
    """Raise AcceptError("provenance document: <path>: <reason>") for a rule violation; None when valid.

    Order: the top-level key set, the version, then the leaves in document order (the sections, then the
    settings). A missing key names its own path and an unknown key names itself.
    """
    if not isinstance(doc, dict):
        raise _invalid("document", "must be an object")
    for key in TOP_KEYS:
        if key not in doc:
            raise _invalid(key, "is missing")
    for key in doc:
        if key not in TOP_KEYS:
            raise _invalid(key, "is an unknown key")
    if type(doc["version"]) is not int or doc["version"] != 1:
        raise _invalid("version", "must be 1")
    for section, keys in SECTIONS:
        block = doc[section]
        if not isinstance(block, dict):
            raise _invalid(section, "must be an object")
        for key in keys:
            if key not in block:
                raise _invalid(section + "." + key, "is missing")
        for key in block:
            if key not in keys:
                raise _invalid(section + "." + key, "is an unknown key")
        for key in keys:
            _check_leaf(section + "." + key, block[key])
    settings = doc["settings"]
    if not isinstance(settings, list):
        raise _invalid("settings", "must be a list")
    if len(settings) > MAX_SETTINGS:
        raise _invalid("settings", "has more than %d settings" % MAX_SETTINGS)
    seen = set()
    for index, item in enumerate(settings):
        path = "settings." + str(index)
        if not isinstance(item, dict):
            raise _invalid(path, "must be an object")
        for key in SETTING_KEYS:
            if key not in item:
                raise _invalid(path + "." + key, "is missing")
        for key in item:
            if key not in SETTING_KEYS:
                raise _invalid(path + "." + key, "is an unknown key")
        name = item["name"]
        if not isinstance(name, str):
            raise _invalid(path + ".name", "must be a string")
        _check_string(path + ".name", name)
        _check_leaf(path + ".value", item["value"])
        if name in seen:
            raise _invalid(path, "repeats a settings name")
        seen.add(name)
    return None


def _name_bytes(item):
    """Return the unsigned UTF-8 bytes of a settings item's name (empty when the name is not a string)."""
    name = item.get("name") if isinstance(item, dict) else None
    return name.encode("utf-8", "surrogatepass") if isinstance(name, str) else b""


def normalize_document(doc):
    """Return a deep copy of doc with its settings sorted by unsigned name bytes; the input is not changed."""
    normalized = copy.deepcopy(doc)
    if isinstance(normalized, dict) and isinstance(normalized.get("settings"), list):
        normalized["settings"] = sorted(normalized["settings"], key=_name_bytes)
    return normalized


def _join(path, key):
    """Return the dotted path of key under path (the root has the empty path)."""
    return path + "." + str(key) if path else str(key)


def _first_difference(expected, actual, path):
    """Return the dotted path of the first difference between two JSON values, or None when they are equal."""
    if isinstance(expected, dict) and isinstance(actual, dict):
        keys = list(expected) + [key for key in actual if key not in expected]
        for key in keys:
            child = _join(path, key)
            if key not in expected or key not in actual:
                return child
            found = _first_difference(expected[key], actual[key], child)
            if found is not None:
                return found
        return None
    if isinstance(expected, list) and isinstance(actual, list):
        if len(expected) != len(actual):
            return path
        for index, (left, right) in enumerate(zip(expected, actual)):
            found = _first_difference(left, right, _join(path, index))
            if found is not None:
                return found
        return None
    if type(expected) is type(actual) and expected == actual:
        return None
    return path


# Read-back verifiers -----------------------------------------------------------------------------

def _readback_error(clause):
    """Return the AcceptError for a read-back clause that failed."""
    return accept_lib.AcceptError("provenance read-back: " + clause)


def _show(mapping, key):
    """Return a mapping's value for key as text for a message; a missing key reads as missing."""
    if not isinstance(mapping, dict) or key not in mapping:
        return "missing"
    value = mapping[key]
    if isinstance(value, str):
        return value
    return json.dumps(value, sort_keys=True)


def _lookup(value, *keys):
    """Return value[k1][k2]... or None when a step is missing or not an object."""
    for key in keys:
        if not isinstance(value, dict) or key not in value:
            return None
        value = value[key]
    return value


def _provenance_block(expansion):
    """Return data.execution.provenance of a history --event expansion, refusing when it is absent."""
    block = _lookup(expansion, "data", "execution", "provenance")
    if not isinstance(block, dict):
        raise _readback_error("expansion lacks data.execution.provenance")
    return block


def _check_native(expansion, block, *, token, native_expect):
    """Check the native block (exactly ten keys, preferences unknown, native_expect pairs) and the token scan."""
    native = block["native"] if isinstance(block.get("native"), dict) else {}
    for key in NATIVE_KEYS:
        if key not in native:
            raise _readback_error("native lacks " + key)
    extra = sorted(set(native) - set(NATIVE_KEYS))
    if extra:
        raise _readback_error("native has an unexpected key " + extra[0])
    prefs = native["preferences"] if isinstance(native["preferences"], dict) else {}
    for which in PREFERENCES:
        if prefs.get(which) != UNKNOWN:
            raise _readback_error("native.preferences.%s is not unknown" % which)
    for key, expected in native_expect.items():
        if key not in native or native[key] != expected:
            raise _readback_error("native.%s is %s, expected %s" % (key, _show(native, key), expected))
    if not token:
        raise accept_lib.AcceptError("provenance read-back: no token was supplied to scan for")
    if token in json.dumps(expansion, sort_keys=True):
        raise _readback_error("bearer token present")


def verify_readback(expansion, document, *, token, native_expect):
    """Refuse a stored claim or takeover record that differs from document or breaks the read-back rules.

    Clauses in order: authority, driver.id, document equality after normalizing both sides, the native
    block (ten keys and unknown preferences), native_expect pairs, then the bearer token scan of the whole
    expansion. Each refusal starts with 'provenance read-back:'.
    """
    block = _provenance_block(expansion)
    driver = block["driver"] if isinstance(block.get("driver"), dict) else {}
    if "authority" not in driver or driver["authority"] != AUTHORITY:
        raise _readback_error("authority is %s, expected %s" % (_show(driver, "authority"), AUTHORITY))
    if "id" not in driver:
        raise _readback_error("driver.id is missing")
    if driver["id"] == "none":
        raise _readback_error("driver.id is none")
    difference = _first_difference(normalize_document(document), normalize_document(driver.get("document")), "")
    if difference is not None:
        raise _readback_error("document differs at " + (difference or "(document)"))
    _check_native(expansion, block, token=token, native_expect=native_expect)


def verify_absent_readback(expansion, *, token, native_expect):
    """Accept only the all-unknown none driver shape of a takeover that supplied no file.

    Clauses in order: authority must be none, driver.id must be none, the document must be exactly the
    all-unknown document with no settings, then the same native, preferences, native_expect and token
    clauses as verify_readback.
    """
    block = _provenance_block(expansion)
    driver = block["driver"] if isinstance(block.get("driver"), dict) else {}
    if "authority" not in driver or driver["authority"] != "none":
        raise _readback_error("authority is %s, expected none" % _show(driver, "authority"))
    if "id" not in driver or driver["id"] != "none":
        raise _readback_error("driver.id is %s, expected none" % _show(driver, "id"))
    difference = _first_difference(_all_unknown_document(), normalize_document(driver.get("document")), "")
    if difference is not None:
        raise _readback_error("document differs at " + (difference or "(document)"))
    _check_native(expansion, block, token=token, native_expect=native_expect)


def require_accepted(step, verb):
    """Return None for exit 0; otherwise raise the product defect for a refused driver-validated document.

    The code is error.code of the JSON stderr, or 'unparsed' when stderr is not that JSON.
    """
    if step["exit"] == 0:
        return None
    code = "unparsed"
    try:
        value = json.loads(step["stderr"])["error"]["code"]
        if isinstance(value, str) and value:
            code = value
    except (TypeError, ValueError, KeyError):
        pass
    raise accept_lib.AcceptError(
        f"product defect: {verb} refused a contract-valid provenance document (exit {step['exit']}, code {code})")


# Recomputed facts --------------------------------------------------------------------------------

def _file_sha256(path):
    """Return the hex sha256 of a file's bytes, read in chunks."""
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def tree_sha256(root):
    """Return the hex sha256 over the regular files of root: relpath NUL file-sha256 newline, in relpath byte order."""
    base = Path(root)
    records = []
    for dirpath, _dirnames, filenames in os.walk(base):
        for name in filenames:
            path = Path(dirpath) / name
            if path.is_symlink() or not path.is_file():
                continue
            relpath = path.relative_to(base).as_posix().encode("utf-8")
            records.append((relpath, _file_sha256(path)))
    digest = hashlib.sha256()
    for relpath, file_hex in sorted(records):
        digest.update(relpath + b"\0" + file_hex.encode("ascii") + b"\n")
    return digest.hexdigest()


def _driver_version():
    """Return the first 12 hex digits of the sha256 of this driver file's bytes."""
    return hashlib.sha256(Path(__file__).resolve().read_bytes()).hexdigest()[:12]


def _json_field(raw, *keys):
    """Return the field at keys of a JSON response, refusing with fixed text when it is absent."""
    try:
        value = json.loads(raw)
        for key in keys:
            value = value[key]
    except (TypeError, ValueError, KeyError, IndexError) as error:
        raise accept_lib.AcceptError("response lacks " + ".".join(keys)) from error
    return value


def _find_event(events, kind, request_id):
    """Return the one history event of this type and request id; refuse when the count is not one."""
    matches = [event for event in events
               if isinstance(event, dict) and event.get("type") == kind and event.get("request_id") == request_id]
    if len(matches) != 1:
        raise accept_lib.AcceptError("expected one %s event for %s, found %d" % (kind, request_id, len(matches)))
    return matches[0]


def _sequence(event):
    """Return a history event sequence (a canonical decimal string at the JSON boundary) as an int."""
    sequence = event.get("sequence")
    if (type(sequence) is not str or not sequence.isascii() or not sequence.isdigit()
            or sequence[0] == "0" or int(sequence) > 9223372036854775807):
        raise accept_lib.AcceptError("history event lacks a positive sequence")
    return int(sequence)


def _previous_run(expansion):
    """Return data.execution.previous_run as text, or None when absent."""
    value = _lookup(expansion, "data", "execution", "previous_run")
    return None if value is None else str(value)


def _provenance_of(expansion, label):
    """Return the provenance block of a history --event expansion, refusing when it is absent."""
    block = _lookup(expansion, "data", "execution", "provenance")
    if not isinstance(block, dict):
        raise accept_lib.AcceptError("history --event for " + label + " lacks data.execution.provenance")
    return block


# Scenario ----------------------------------------------------------------------------------------

def _document(driver_version, facts, phase):
    """Return the honest document of one phase, built from the recomputed facts."""
    return build_document(
        driver_version=driver_version, trial_id=TRIAL_ID, view_id=VIEW_ID,
        settings={
            "accepted_revision": facts["accepted_revision"],
            "baseline_ref": facts["baseline_ref"],
            "baseline_sha256": facts["baseline_sha256"],
            "candidate_sha256": facts["candidate_sha256"],
            "candidate_size": str(facts["candidate_size"]),
            "driver_phase": phase,
        })


def _write_document(document, path):
    """Validate the document, write its JSON bytes to path and return the sha256 of those bytes."""
    validate_document(document)
    data = (json.dumps(document, indent=1) + "\n").encode("utf-8")
    path.write_bytes(data)
    return hashlib.sha256(data).hexdigest()


class _ProvenanceScenario(fault_driver._Scenario):
    """The minimal A34 scenario: init, load, activate, claim, takeover, no-file takeover, then fresh reads."""

    def run(self):
        """Run the scenario through the candidate and return the result; every read is a fresh process."""
        self.prepare()
        self.current = "init"
        init_step, _init_out = self._run("init", ["--workspace-id", self.workspace_id])
        if init_step["exit"] != 0:
            raise accept_lib.AcceptError("init was refused")
        (self.workspace / "combined.lua").write_bytes(self.job_max1)
        self.current = "load"
        load_step, load_out = self._run("load", ["--workspace", str(self.workspace / "workspace.lua"),
                                                 "--registry", str(self.registry_path),
                                                 "--expect-revision", "none",
                                                 "--request-id", "load-1", "--actor", fault_driver.DRIVER],
                                        request_id="load-1")
        if load_step["exit"] != 0:
            raise accept_lib.AcceptError("initial load was refused")
        revision = _json_field(load_out, "revision")
        self.current = "activate"
        activate_step, _activate_out = self._run(
            "activate", ["--job", JOB, "--expect-revision", str(revision), "--expect-job", "NONE",
                         "--request-id", "activate-1", "--actor", fault_driver.DRIVER],
            traced=True, request_id="activate-1")
        if activate_step["exit"] != 0:
            raise accept_lib.AcceptError("activate was refused")

        candidate_sha = _file_sha256(self.candidate)
        candidate_size = os.stat(self.candidate).st_size
        facts = {
            "accepted_revision": str(revision),
            "baseline_ref": BASE_REF,
            "baseline_sha256": tree_sha256(self.base),
            "candidate_sha256": candidate_sha,
            "candidate_size": candidate_size,
        }
        driver_version = _driver_version()

        claim_doc = _document(driver_version, facts, "claim")
        claim_file = self.scratch / "provenance-claim.json"
        claim_sha = _write_document(claim_doc, claim_file)
        if self.tamper.get("provenance_path") == "missing":
            claim_arg = str(self.scratch / "does-not-exist.json")
        else:
            claim_arg = str(claim_file)
        self.current = "claim"
        claim_step, claim = self.act(
            "claim", ["--job", JOB, "--role", "builder", "--request-id", "claim-builder-1",
                      "--actor", BUILDER, "--provenance", claim_arg],
            point="claim_builder_1", traced=True, request_id="claim-builder-1")
        require_accepted(claim_step, "claim")
        claim_run = claim["run"]
        claim_run_id = str(claim_run["id"])

        takeover_doc = _document(driver_version, facts, "takeover")
        takeover_file = self.scratch / "provenance-takeover.json"
        takeover_sha = _write_document(takeover_doc, takeover_file)
        self.current = "takeover"
        takeover_step, taken = self.act(
            "takeover", ["--job", JOB, "--role", "builder", "--request-id", "takeover-1",
                         "--actor", REPLACEMENT, "--run-id", claim_run_id, "--provenance", str(takeover_file)],
            point="takeover", traced=True, request_id="takeover-1")
        require_accepted(takeover_step, "takeover")
        taken_run = taken["run"]
        taken_run_id = str(taken_run["id"])

        # The no-file control: the guidance check runs first, so a takeover refused as an action (not by the
        # candidate) is reported as a product defect with no argv file, as the D34 defect protocol requires.
        self.current = "takeover"
        try:
            self.guided("takeover", "takeover")
        except accept_lib.AcceptError as error:
            raise accept_lib.AcceptError(
                "product defect: a takeover after a takeover is not a legal action in the captured guidance ("
                + str(error) + ")") from error
        nofile_step, nofile_out = self._run(
            "takeover", ["--job", JOB, "--role", "builder", "--request-id", "takeover-2",
                         "--actor", BUILDER, "--run-id", taken_run_id],
            traced=True, request_id="takeover-2")
        require_accepted(nofile_step, "takeover")
        nofile_run = json.loads(nofile_out)["run"]
        nofile_run_id = str(nofile_run["id"])

        self.current = "history"
        events = _lookup(self.read_json("history", ["--limit", "100"]), "data", "events")
        if not isinstance(events, list):
            raise accept_lib.AcceptError("history read lacks data.events")
        claim_event = _find_event(events, "run_claimed", "claim-builder-1")
        takeover_event = _find_event(events, "run_taken_over", "takeover-1")
        nofile_event = _find_event(events, "run_taken_over", "takeover-2")
        claim_seq = _sequence(claim_event)
        takeover_seq = _sequence(takeover_event)
        nofile_seq = _sequence(nofile_event)
        claim_full = self.read_json("history", ["--event", str(claim_seq)])
        takeover_full = self.read_json("history", ["--event", str(takeover_seq)])
        nofile_full = self.read_json("history", ["--event", str(nofile_seq)])
        claim_prov = _provenance_of(claim_full, "claim")
        takeover_prov = _provenance_of(takeover_full, "takeover")
        nofile_prov = _provenance_of(nofile_full, "nofile")

        status = self.status()
        ledger = _lookup(status, "ledger")
        active = _lookup(status, "active_revision")
        if not isinstance(ledger, str) or not ledger or active is None:
            raise accept_lib.AcceptError("status read lacks ledger or active_revision")

        def expect(generation):
            """Return the native values one run's read-back must show."""
            return {"job": JOB, "role": "builder", "generation": generation, "base_label": BASE_LABEL,
                    "ledger": ledger, "revision": str(active)}

        tokens = [str(claim_run["token"]), str(taken_run["token"]), str(nofile_run["token"])]
        verify = self.cfg["verify"]
        if verify:
            for token in tokens:
                verify_readback(claim_full, claim_doc, token=token, native_expect=expect("1"))
                verify_readback(takeover_full, takeover_doc, token=token, native_expect=expect("2"))
                verify_absent_readback(nofile_full, token=token, native_expect=expect("3"))
        histories = sorted(self.raw.glob("*-history.stdout"))
        token_absent = scan_for_tokens([claim_file, takeover_file, *histories], tokens)

        return {
            "scenario": "a34-provenance",
            "mode": "cmake",
            "takeover_applicable": True,
            "candidate": {"sha256": candidate_sha, "size": candidate_size},
            "env": {"HOME": self.env["HOME"], "PLANAR_DB": self.env["PLANAR_DB"]},
            "ledger": self.ledger_digest(),
            "facts": facts,
            "verified": verify,
            "token_absent": token_absent,
            "steps":[{"index": step["index"], "verb": step["verb"], "argv": step["argv"], "exit": step["exit"]}
                      for step in self.steps],
            "claim": {"request_id": "claim-builder-1", "exit": claim_step["exit"], "argv": claim_step["argv"],
                      "provenance_path": claim_arg, "provenance_sha256": claim_sha, "document": claim_doc,
                      "event_id": str(claim_seq), "run_id": claim_run_id, "readback": claim_prov},
            "takeover": {"request_id": "takeover-1", "exit": takeover_step["exit"],
                         "argv": takeover_step["argv"], "provenance_path": str(takeover_file),
                         "provenance_sha256": takeover_sha, "document": takeover_doc,
                         "event_id": str(takeover_seq), "run_id": taken_run_id,
                         "previous_run": _previous_run(takeover_full), "readback": takeover_prov},
            "nofile": {"request_id": "takeover-2", "exit": nofile_step["exit"], "argv": nofile_step["argv"],
                       "event_id": str(nofile_seq), "run_id": nofile_run_id,
                       "previous_run": _previous_run(nofile_full), "readback": nofile_prov},
        }


def run_provenance(config):
    """Run the claim, the takeover and the no-file takeover with provenance files; return the result.

    An AcceptError writes raw_root/ABORTED.json and is re-raised. A refused driver-validated document raises
    the product defect message from require_accepted. config["verify"] (default True) selects whether the in-run
    read-back loop runs; a non-boolean value is refused before any action.
    """
    verify = config.get("verify", True)
    if not isinstance(verify, bool):
        raise accept_lib.AcceptError("provenance run: verify must be a boolean")
    scenario = _ProvenanceScenario(dict(config, mode="cmake", check_mode="sleep", base="plain", verify=verify))
    try:
        return scenario.run()
    except accept_lib.AcceptError as error:
        scenario.raw.mkdir(parents=True, exist_ok=True)
        aborted = {"verb": scenario.current, "error": str(error),
                   "ledger_digest": scenario.ledger_digest() if scenario.ledger.exists() else None}
        (scenario.raw / "ABORTED.json").write_text(json.dumps(aborted, indent=1) + "\n", encoding="utf-8")
        raise


def scan_for_tokens(paths, tokens):
    """Return True when no token occurs as UTF-8 bytes in any listed path, otherwise False.

    A path that is missing or is not a regular file is skipped. An empty token list, or an empty token
    string, is refused with the fixed read-back text, since an empty token would occur in every file.
    A listed file that cannot be read is refused with fixed text; the path is never echoed.
    """
    needles = [token.encode("utf-8") for token in tokens]
    if not needles or any(not needle for needle in needles):
        raise _readback_error("no token was supplied to scan for")
    for path in paths:
        target = Path(os.fspath(path))
        if not target.is_file():
            continue
        try:
            data = target.read_bytes()
        except OSError as error:
            raise _readback_error("a listed file cannot be read for the token scan") from error
        for needle in needles:
            if needle in data:
                return False
    return True


def _native_checks(native, generation):
    """Return the recomputed native checks of one read-back: ten keys, unknown preferences, expected generation."""
    if not isinstance(native, dict):
        return {"native_ten_keys": False, "preferences_unknown": False, "generation_expected": False}
    preferences = native.get("preferences")
    return {
        "native_ten_keys": sorted(native) == sorted(NATIVE_KEYS),
        "preferences_unknown": isinstance(preferences, dict)
        and all(preferences.get(which) == UNKNOWN for which in PREFERENCES),
        "generation_expected": native.get("generation") == generation,
    }


def readback_manifest(result):
    """Return the path-free A34 read-back manifest of a run result, with its checks recomputed from the blocks.

    The manifest has exactly the keys version (1), facts, claim, takeover, nofile and checks. facts holds the
    accepted revision, baseline, candidate digest and size, and the driver version taken from the claim
    document. claim, takeover and nofile hold event_id, run_id and the stored read-back block (provenance).
    checks holds token_absent_in_run_dir and verified from the result, and for each block the booleans
    native_ten_keys, preferences_unknown and generation_expected ("1", "2", "3"); claim and takeover add
    document_matches_supplied (the stored document equals the supplied one after normalization). The manifest
    never holds the env, provenance_path, argv, a host path or a token.
    """
    facts = result["facts"]
    checks = {"token_absent_in_run_dir": result.get("token_absent") is True,
              "verified": result.get("verified") is True}
    expected_generation = {"claim": "1", "takeover": "2", "nofile": "3"}
    blocks = {}
    for section in ("claim", "takeover", "nofile"):
        info = result[section]
        readback = info["readback"]
        driver = readback.get("driver") if isinstance(readback, dict) else None
        block_checks = _native_checks(readback.get("native") if isinstance(readback, dict) else None,
                                      expected_generation[section])
        if section != "nofile":
            stored = driver.get("document") if isinstance(driver, dict) else None
            block_checks["document_matches_supplied"] = _first_difference(
                normalize_document(info["document"]), normalize_document(stored), "") is None
        checks[section] = block_checks
        blocks[section] = {"event_id": info["event_id"], "run_id": info["run_id"],
                           "provenance": copy.deepcopy(readback)}
    return {
        "version": 1,
        "facts": {
            "accepted_revision": facts["accepted_revision"],
            "baseline_ref": facts["baseline_ref"],
            "baseline_sha256": facts["baseline_sha256"],
            "candidate_sha256": facts["candidate_sha256"],
            "candidate_size": facts["candidate_size"],
            "driver_version": result["claim"]["document"]["driver"]["version"],
        },
        "claim": blocks["claim"],
        "takeover": blocks["takeover"],
        "nofile": blocks["nofile"],
        "checks": checks,
    }


if __name__ == "__main__":
    sys.exit(0)
