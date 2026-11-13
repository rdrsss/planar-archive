#!/usr/bin/env python3
# R19-ACCEPT-B-C01 checkpoint driver (package I2): runs the A03 evidence scenario through
# the candidate and checks that each checkpoint moves only the render digest. The scenario
# is init, load, activate, claim builder, a fresh before preview, checkpoint #1 (text T1),
# after1, checkpoint #2 (text T2), after2, preview --since the claim render digest, and a
# control preview with no change in between. It reuses fault_driver._Scenario in cmake
# mode (check_mode "sleep", base "plain").
# Contract: docs/vernacular/execution-contract-r16.md R16-0, R16-2, R16-3 and
# docs/vernacular/render-contract.md (checkpoint object and budget); DECISIONS E10, E11,
# E12, E23 and E24 of plan/DECISIONS.md.
#
# Config (run_checkpoint; the keys of fault_driver._Scenario):
#   candidate, scratch, raw_root, home, ledger, counter, repo (default the checkout, read
#   only), verify (default True: run the in-run effect checks and report verified true;
#   False skips them and reports verified false), tamper (dict; observations_path "missing"
#   points checkpoint #1 at a file that does not exist). mode, check_mode and base are forced
#   to cmake, sleep and plain.
#
# Result (run_checkpoint): scenario, mode, verified, token_absent, candidate {sha256, size},
#   env {HOME, PLANAR_DB} (copied from the environment given to every child), ledger (digest),
#   facts, claim {request_id, run_id, exit, argv, work_fp, render_digest, state}, before,
#   after1, after2, control (preview blocks {kind, work_fp, render_digest, state, markdown,
#   sections, omitted}), checkpoint1 and checkpoint2 {request_id, exit, argv, observations_path,
#   observations_sha256, observations, event_id, text}, since (the decoded preview --since
#   JSON), steps [{index, verb, argv, exit}]. The run token appears only in retained raw files
#   (the claim stdout and the checkpoint argv files): every argv field carries <run-token>.
#
# Abort contract: an AcceptError writes raw_root/ABORTED.json (verb, error, ledger_digest) and
#   re-raises. A refused checkpoint or claim is a product defect (_require_accepted): the run
#   stops and nothing is worked around.
#
# Manifest (checkpoint_manifest): path-free; exact keys version, facts, claim, checkpoint1,
#   checkpoint2, previews, since, informational, checks. checks is exactly the ten booleans of
#   the A03 acceptance condition; informational holds only the budget-omission flag, which is a
#   fixture limit (E23) and never a check. A03 is met only when all ten checks are true and the
#   informational flag is false.
import sys

sys.dont_write_bytecode = True

import hashlib
import json
import re
from pathlib import Path

HERE = Path(__file__).resolve().parent
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))

import accept_lib  # noqa: E402
import fault_driver  # noqa: E402
import provenance_driver  # noqa: E402

TEXT_ONE = "checkpoint text one: parser progress recorded by the first checkpoint"
TEXT_TWO = "checkpoint text two: parser progress recorded by the second checkpoint"

SCENARIO = "a03-checkpoint"
BASE_REF = "58128234:src/engine"
BUILDER = fault_driver.BUILDER
JOB = fault_driver.JOB
NEXT_STEP = "next step"
UNRESOLVED = "unresolved item"
UNIT_FILE = "value/CMakeLists.txt"
CHECKPOINT_KEYS = ("progress", "next", "unresolved", "files")
TEXT_KEYS = ("progress", "next", "unresolved")
MAX_TEXT_BYTES = 2048
MAX_FILES = 32
MAX_CANONICAL_BYTES = 8192
RUN_TOKEN_PLACEHOLDER = "<run-token>"
CHECKPOINT_SECTION = re.compile(r"(?:^|\n)## checkpoint\n")
PREVIEW_NAMES = ("before", "after1", "after2", "control")
CLAIM_KEYS = ("request_id", "run_id", "work_fp", "render_digest", "state")
CHECKPOINT_SUMMARY_KEYS = ("request_id", "exit", "event_id", "text")
CHECK_NAMES = ("work_fp_equal", "state_equal", "render_moved_by_checkpoint1", "render_moved_by_checkpoint2",
               "control_equal", "since_material_empty", "since_has_checkpoint_added_section",
               "since_matches_after2", "token_absent", "verified")


# Checkpoint object and validation (render-contract.md, checkpoint) -------------------------------

def build_checkpoint(*, progress, next_step, unresolved, files):
    """Return the checkpoint object with exactly the keys progress, next, unresolved and files.

    The argument next_step is stored under the key next. The object is not validated here; see
    validate_checkpoint.
    """
    return {"progress": progress, "next": next_step, "unresolved": unresolved, "files": list(files)}


def build_observations(checkpoint):
    """Return the observations root {version 1, checks [], findings [], checkpoint} for one checkpoint."""
    return {"version": 1, "checks": [], "findings": [], "checkpoint": checkpoint}


def _refuse(path, reason):
    """Return the AcceptError for one violation at a dotted path under the checkpoint object."""
    return accept_lib.AcceptError("checkpoint: " + path + ": " + reason)


def _check_text(key, value):
    """Refuse a text field that is not a string, holds NUL, or is over 2,048 UTF-8 bytes; empty is allowed."""
    if not isinstance(value, str):
        raise _refuse(key, "must be a string")
    if "\x00" in value:
        raise _refuse(key, "must not contain NUL")
    try:
        size = len(value.encode("utf-8"))
    except UnicodeEncodeError as error:
        raise _refuse(key, "must be valid UTF-8") from error
    if size > MAX_TEXT_BYTES:
        raise _refuse(key, "must be at most 2048 UTF-8 bytes")


def _check_files(files):
    """Refuse a files value that is not a list of at most 32 distinct relative paths; an empty list is allowed."""
    if not isinstance(files, list) or len(files) > MAX_FILES:
        raise _refuse("files", "must be a list of at most 32 distinct relative paths")
    seen = set()
    for index, path in enumerate(files):
        label = "files." + str(index)
        if not isinstance(path, str):
            raise _refuse(label, "must be a string")
        if not path or "\x00" in path:
            raise _refuse(label, "must be a non-empty string without NUL")
        if path.startswith("/"):
            raise _refuse(label, "must be relative")
        if ".." in path.split("/"):
            raise _refuse(label, "must not contain a '..' component")
        if path in seen:
            raise _refuse(label, "repeats a path")
        seen.add(path)


def validate_checkpoint(checkpoint):
    """Raise AcceptError("checkpoint: <path>: <reason>") for one violation; return None when valid.

    Empty strings, an empty files list and the bounds themselves (2,048 bytes per string, 32 files,
    8,192 canonical bytes) are accepted. Each violation is refused alone, in this order: object
    shape, missing keys, unknown keys, the three text fields, the files list, then the canonical size.
    """
    if not isinstance(checkpoint, dict):
        raise accept_lib.AcceptError("checkpoint: must be an object")
    for key in CHECKPOINT_KEYS:
        if key not in checkpoint:
            raise _refuse(key, "is missing")
    for key in checkpoint:
        if key not in CHECKPOINT_KEYS:
            raise _refuse(key, "is an unknown key")
    for key in TEXT_KEYS:
        _check_text(key, checkpoint[key])
    _check_files(checkpoint["files"])
    canonical = json.dumps(checkpoint, sort_keys=True, separators=(",", ":"), ensure_ascii=False)
    if len(canonical.encode("utf-8")) > MAX_CANONICAL_BYTES:
        raise accept_lib.AcceptError("checkpoint: canonical JSON exceeds 8192 bytes")
    return None


# Effect verification (E10, E23; contract R16-2 and R16-3) ----------------------------------------

def _effect(reason):
    """Return the AcceptError for one failed effect clause."""
    return accept_lib.AcceptError("checkpoint effect: " + reason)


def _has_checkpoint(markdown):
    """Return True when the Markdown holds a ## checkpoint section heading."""
    return isinstance(markdown, str) and CHECKPOINT_SECTION.search(markdown) is not None


def _is_count(value):
    """Return True for a non-negative integer that is not a boolean."""
    return isinstance(value, int) and not isinstance(value, bool) and value >= 0


def _budget_omitted(block):
    """Return True when the packet's checkpoint section was dropped whole by the render budget (E23)."""
    rows = block.get("sections") or []
    omitted = block.get("omitted") or []
    dropped = any(isinstance(row, dict) and row.get("id") == "checkpoint" and row.get("included") is False
                  for row in rows)
    return dropped and "checkpoint" in omitted


def _checkpoint_row_added(rows):
    """Return True when the since sections hold a checkpoint row added from zero bytes to a positive size."""
    for row in rows:
        if (isinstance(row, dict) and row.get("id") == "checkpoint" and row.get("status") == "added"
                and _is_count(row.get("before_bytes")) and row.get("before_bytes") == 0
                and _is_count(row.get("after_bytes")) and row.get("after_bytes") > 0):
            return True
    return False


def verify_effect(before, after, since=None):
    """Check one checkpoint's render-only effect between two previews; return None when every clause holds.

    Clauses in order: work_fp equal, state equal, render digest different, the after packet has a
    ## checkpoint section (or a budget omission, which raises the fixture-limit message instead), then,
    when since is given, since material is empty, since has a checkpoint row added from zero bytes,
    and since's top-level render digest equals the after packet's. render_changed is not evidence.

    Raises AcceptError: 'checkpoint effect: <reason>' for a product clause, or 'checkpoint fixture
    limit:' when the budget dropped the checkpoint section.
    """
    if before["work_fp"] != after["work_fp"]:
        raise _effect("work_fp changed")
    if before["state"] != after["state"]:
        raise _effect("state changed")
    if before["render_digest"] == after["render_digest"]:
        raise _effect("render digest did not change")
    if not _has_checkpoint(after["markdown"]):
        if _budget_omitted(after):
            raise accept_lib.AcceptError(
                "checkpoint fixture limit: checkpoint section omitted by budget; the fixture must be smaller "
                "and A03 is not met")
        raise _effect("packet lacks a checkpoint section")
    if since is None:
        return None
    body = since["since"]
    if body.get("material") != []:
        raise _effect("since reports material changes")
    if not _checkpoint_row_added(body.get("sections") or []):
        raise _effect("since lacks an added checkpoint section")
    if since.get("render_digest") != after["render_digest"]:
        raise _effect("since does not match the after packet")
    return None


# Manifest (A03 acceptance condition) -------------------------------------------------------------

def _all_equal(*values):
    """Return True when every value equals the first."""
    return all(value == values[0] for value in values)


def checkpoint_manifest(result):
    """Return the path-free A03 manifest recomputed from the result blocks.

    checks is exactly the ten booleans of the A03 acceptance condition; each is recomputed from the
    blocks and none reads the sections rows, so the budget flag never changes them. informational is
    exactly {checkpoint_section_omitted_by_budget}: true when either after packet dropped its checkpoint
    section by budget (a fixture limit, E23). The A03 acceptance condition is all ten checks true AND
    the informational flag false.
    """
    before = result["before"]
    after1 = result["after1"]
    after2 = result["after2"]
    control = result["control"]
    claim = result["claim"]
    since = result["since"]
    body = since["since"]
    checkpoint1 = result["checkpoint1"]
    checkpoint2 = result["checkpoint2"]
    checks = {
        "work_fp_equal": _all_equal(claim["work_fp"], before["work_fp"], after1["work_fp"], after2["work_fp"],
                                    control["work_fp"]),
        "state_equal": _all_equal(claim["state"], before["state"], after1["state"], after2["state"],
                                  control["state"]),
        "render_moved_by_checkpoint1": (before["render_digest"] != after1["render_digest"]
                                        and _has_checkpoint(after1["markdown"])
                                        and checkpoint1["text"] in after1["markdown"]),
        "render_moved_by_checkpoint2": (after1["render_digest"] != after2["render_digest"]
                                        and _has_checkpoint(after2["markdown"])
                                        and checkpoint2["text"] in after2["markdown"]),
        "control_equal": (control["markdown"] == after2["markdown"]
                          and control["render_digest"] == after2["render_digest"]),
        "since_material_empty": body.get("material") == [],
        "since_has_checkpoint_added_section": _checkpoint_row_added(body.get("sections") or []),
        "since_matches_after2": since.get("render_digest") == after2["render_digest"],
        "token_absent": result.get("token_absent") is True,
        "verified": result.get("verified") is True,
    }
    omitted = _budget_omitted(after1) or _budget_omitted(after2)
    return {
        "version": 1,
        "facts": dict(result["facts"]),
        "claim": {key: claim[key] for key in CLAIM_KEYS},
        "checkpoint1": {key: checkpoint1[key] for key in CHECKPOINT_SUMMARY_KEYS},
        "checkpoint2": {key: checkpoint2[key] for key in CHECKPOINT_SUMMARY_KEYS},
        "previews": {name: {key: result[name][key] for key in ("work_fp", "render_digest", "state")}
                     for name in PREVIEW_NAMES},
        "since": {"dispatch": body["dispatch"], "material": body["material"],
                  "render_changed": body["render_changed"], "sections": body["sections"],
                  "render_digest": since["render_digest"]},
        "informational": {"checkpoint_section_omitted_by_budget": omitted},
        "checks": {name: checks[name] for name in CHECK_NAMES},
    }


# Scenario ----------------------------------------------------------------------------------------

def _file_sha256(path):
    """Return the hex sha256 of a file's bytes, read in chunks."""
    digest = hashlib.sha256()
    with open(path, "rb") as stream:
        for chunk in iter(lambda: stream.read(1 << 20), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _required(value, *keys):
    """Return value[k1][k2]... from a response, refusing with fixed text when a step is missing."""
    for key in keys:
        if not isinstance(value, dict) or key not in value:
            raise accept_lib.AcceptError("response lacks " + ".".join(keys))
        value = value[key]
    return value


def _preview_block(preview):
    """Return one preview block (the fields the effect checks and the manifest read) from a fresh preview read."""
    packet = _required(preview, "packet")
    manifest = _required(preview, "manifest")
    return {"kind": _required(preview, "kind"), "work_fp": _required(preview, "work_fp"),
            "render_digest": _required(preview, "render_digest"), "state": _required(preview, "state"),
            "markdown": _required(packet, "markdown"), "sections": _required(manifest, "sections"),
            "omitted": _required(manifest, "omitted")}


def _checkpoint_event_listed(history, block):
    """Refuse unless history lists exactly one checkpoint_recorded event whose sequence is the block's event id."""
    events = _required(history, "data", "events")
    if not isinstance(events, list):
        raise accept_lib.AcceptError("history read lacks data.events")
    matches = [event for event in events if isinstance(event, dict) and event.get("type") == "checkpoint_recorded"
               and str(event.get("sequence")) == block["event_id"]]
    if len(matches) != 1:
        raise accept_lib.AcceptError("history does not list the checkpoint_recorded event of " + block["request_id"])


def _control_matches(after, control):
    """Refuse when no-change previews differ in markdown or render digest."""
    if after["markdown"] != control["markdown"] or after["render_digest"] != control["render_digest"]:
        raise _effect("control preview differs from after2 with no change in between")


class _CheckpointScenario(fault_driver._Scenario):
    """The A03 scenario: init, load, activate, claim, then checkpoints and fresh preview reads."""

    token = ""

    def _redact(self, argv):
        """Return argv with the run token replaced by the placeholder (the token is kept only in raw files)."""
        if not self.token:
            return [str(arg) for arg in argv]
        return [str(arg).replace(self.token, RUN_TOKEN_PLACEHOLDER) for arg in argv]

    def _builder_state(self):
        """Return the builder role state from a fresh status read."""
        status = self.status()
        rows = [row for row in status.get("roles") or [] if isinstance(row, dict) and row.get("role") == "builder"]
        if len(rows) != 1 or not isinstance(rows[0].get("state"), str):
            raise accept_lib.AcceptError("the fresh status lacks exactly one builder role state")
        return rows[0]["state"]

    def _preview(self):
        """Return one fresh preview read of the builder role as a preview block."""
        return _preview_block(self.read_json("preview", ["--role", "builder"]))

    def _checkpoint(self, index, text):
        """Record one checkpoint through the candidate and return its result block.

        The observations file holds the honest checkpoint (validated first); the argv passes it by path and the
        run token through --run. A refused checkpoint is a product defect (_require_accepted).
        """
        checkpoint = build_checkpoint(progress=text, next_step=NEXT_STEP, unresolved=UNRESOLVED,
                                      files=[UNIT_FILE])
        validate_checkpoint(checkpoint)
        observations = build_observations(checkpoint)
        point = "checkpoint_%d" % index
        request_id = "checkpoint-%d" % index
        self.current = "checkpoint"
        try:
            self.guided("checkpoint", point)
        except accept_lib.AcceptError as error:
            raise accept_lib.AcceptError(
                "product defect: checkpoint is not a legal action in the captured guidance (" + str(error) + ")"
            ) from error
        if self.tamper.get("observations_path") == "missing":
            path = self.scratch / ("does-not-exist-%d.json" % index)
            digest = None
        else:
            path = self.raw / ("%03d-checkpoint-%d.observations.json" % (self._number(), index))
            data = (json.dumps(observations, indent=1) + "\n").encode("utf-8")
            path.write_bytes(data)
            digest = hashlib.sha256(data).hexdigest()
        args = ["--run", self.token, "--observations", str(path), "--request-id", request_id, "--actor", BUILDER]
        step, out = self._run("checkpoint", args, traced=True, request_id=request_id)
        _require_accepted(step, "checkpoint")
        event = _required(json.loads(out), "checkpoint", "event")
        return {"request_id": request_id, "exit": step["exit"], "argv": self._redact(step["argv"]),
                "observations_path": str(path), "observations_sha256": digest, "observations": observations,
                "event_id": str(event), "text": text}

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
        revision = _required(json.loads(load_out), "revision")
        self.current = "activate"
        activate_step, _activate_out = self._run(
            "activate", ["--job", JOB, "--expect-revision", str(revision), "--expect-job", "NONE",
                         "--request-id", "activate-1", "--actor", fault_driver.DRIVER],
            traced=True, request_id="activate-1")
        if activate_step["exit"] != 0:
            raise accept_lib.AcceptError("activate was refused")

        candidate_sha = _file_sha256(self.candidate)
        candidate_size = Path(self.candidate).stat().st_size
        facts = {
            "accepted_revision": str(revision),
            "baseline_ref": BASE_REF,
            "baseline_sha256": provenance_driver.tree_sha256(self.base),
            "candidate_sha256": candidate_sha,
            "candidate_size": candidate_size,
        }

        self.current = "claim"
        claim_step, claim_out = self.act("claim", ["--job", JOB, "--role", "builder",
                                                   "--request-id", "claim-builder-1", "--actor", BUILDER],
                                         point="claim_builder_1", traced=True, request_id="claim-builder-1")
        _require_accepted(claim_step, "claim")
        self.token = str(_required(claim_out, "run", "token"))
        claim_run_id = str(_required(claim_out, "run", "id"))
        claim_dispatch = _required(claim_out, "dispatch")
        claim_block = {
            "request_id": "claim-builder-1", "run_id": claim_run_id, "exit": claim_step["exit"],
            "argv": self._redact(claim_step["argv"]),
            "work_fp": str(_required(claim_dispatch, "work_fp")),
            "render_digest": str(_required(claim_dispatch, "render_digest")),
            "state": self._builder_state(),
        }

        self.current = "before"
        before = self._preview()
        checkpoint1 = self._checkpoint(1, TEXT_ONE)
        after1 = self._preview()
        checkpoint2 = self._checkpoint(2, TEXT_TWO)
        after2 = self._preview()
        self.current = "since"
        since = self.read_json("preview", ["--role", "builder", "--since", claim_block["render_digest"]])
        self.current = "control"
        control = self._preview()
        self.current = "history"
        history = self.read_json("history", ["--limit", "100"])

        verify = bool(self.cfg.get("verify", True))
        if verify:
            verify_effect(before, after1, None)
            verify_effect(after1, after2, None)
            verify_effect(before, after2, since)
            _control_matches(after2, control)
            for block in (checkpoint1, checkpoint2):
                on_disk = json.loads(Path(block["observations_path"]).read_bytes())
                if on_disk != block["observations"]:
                    raise accept_lib.AcceptError("checkpoint observations file differs from the recorded observations")
                _checkpoint_event_listed(history, block)

        result = {
            "scenario": SCENARIO,
            "mode": "cmake",
            "verified": verify,
            "token_absent": False,
            "candidate": {"sha256": candidate_sha, "size": candidate_size},
            "env": {"HOME": self.env["HOME"], "PLANAR_DB": self.env["PLANAR_DB"]},
            "ledger": self.ledger_digest(),
            "facts": facts,
            "claim": claim_block,
            "before": before,
            "after1": after1,
            "checkpoint1": checkpoint1,
            "after2": after2,
            "checkpoint2": checkpoint2,
            "since": since,
            "control": control,
            "steps": [{"index": step["index"], "verb": step["verb"], "argv": self._redact(step["argv"]),
                       "exit": step["exit"]} for step in self.steps],
        }
        result["token_absent"] = self._token_absent(result)
        if verify and not result["token_absent"]:
            raise accept_lib.AcceptError("product defect: the run token appears outside the retained argv file")
        return result

    def _token_absent(self, result):
        """Return True when the token occurs in no result field, observations file or retained stdout or stderr.

        The claim stdout is excluded: it carries the token by design and is retained raw evidence.
        """
        if not self.token:
            return False
        needle = self.token.encode("utf-8")
        texts = [json.dumps(result, sort_keys=True).encode("utf-8")]
        for block in (result["checkpoint1"], result["checkpoint2"]):
            texts.append(Path(block["observations_path"]).read_bytes() if Path(block["observations_path"]).is_file()
                         else b"")
        for path in sorted(self.raw.glob("*.stdout")) + sorted(self.raw.glob("*.stderr")):
            if path.name.endswith("-claim.stdout"):
                continue
            texts.append(path.read_bytes())
        return all(needle not in text for text in texts)


def _require_accepted(step, verb):
    """Return None for exit 0; otherwise raise the product defect for a refused driver-validated verb."""
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
        f"product defect: {verb} refused a driver-validated request (exit {step['exit']}, code {code})")


def run_checkpoint(config):
    """Run the claim, two checkpoints and their previews, a since read and a control; return the result.

    An AcceptError writes raw_root/ABORTED.json and is re-raised. A refused checkpoint or claim raises the
    product defect message from _require_accepted.
    """
    scenario = _CheckpointScenario(dict(config, mode="cmake", check_mode="sleep", base="plain"))
    try:
        return scenario.run()
    except accept_lib.AcceptError as error:
        scenario.raw.mkdir(parents=True, exist_ok=True)
        aborted = {"verb": scenario.current, "error": str(error),
                   "ledger_digest": scenario.ledger_digest() if scenario.ledger.exists() else None}
        (scenario.raw / "ABORTED.json").write_text(json.dumps(aborted, indent=1) + "\n", encoding="utf-8")
        raise


if __name__ == "__main__":
    sys.exit(0)
