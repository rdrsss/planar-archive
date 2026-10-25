#!/usr/bin/env python3
"""Redact machine-local absolute paths from committed evidence, reversibly.

Replaces the repository root with ``<repo>`` and the home directory with
``<home>`` in text files, recording each file's original and redacted SHA-256 in
``tests/native/path-redaction.json`` so manifests that pinned original bytes stay
verifiable: ``verify`` restores the original strings in memory and rechecks.

Usage:
  redact-local-paths.py redact --repo-path P --home H [FILE ...]   (default: all files under cwd)
  redact-local-paths.py check  --repo-path P --home H [FILE ...]   (exit 1 if any file leaks P or H)
  redact-local-paths.py verify --repo-path P --home H              (recheck recorded original hashes)
Run from the repository root (or a tree checkout of it). vendor/ and .git/ are skipped.
"""

import argparse
import hashlib
import json
import os
import sys

MANIFEST = "tests/native/path-redaction.json"
REPO_TOKEN = b"<repo>"
HOME_TOKEN = b"<home>"


def candidate_files(explicit):
    if explicit:
        return [f for f in explicit if os.path.isfile(f) and not os.path.islink(f)]
    out = []
    for dirpath, dirnames, filenames in os.walk("."):
        dirnames[:] = [d for d in dirnames if d not in (".git", "vendor")] if dirpath == "." else dirnames
        for name in filenames:
            path = os.path.normpath(os.path.join(dirpath, name))
            if not os.path.islink(path) and path != MANIFEST:
                out.append(path)
    return sorted(out)


def is_text(data):
    return b"\0" not in data[:8192]


def redact_bytes(data, repo, home):
    return data.replace(repo, REPO_TOKEN).replace(home, HOME_TOKEN)


def restore_bytes(data, repo, home):
    return data.replace(HOME_TOKEN, home).replace(REPO_TOKEN, repo)


def sha(data):
    return hashlib.sha256(data).hexdigest()


def load_manifest():
    if os.path.exists(MANIFEST):
        with open(MANIFEST) as handle:
            return json.load(handle)
    return {
        "description": "Machine-local paths redacted from committed files. original_sha256 is the "
        "hash before redaction; restore <repo>/<home> to the original absolute paths to verify.",
        "tokens": {"<repo>": "repository root", "<home>": "home directory"},
        "files": {},
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("mode", choices=["redact", "check", "verify"])
    parser.add_argument("--repo-path", required=True)
    parser.add_argument("--home", required=True)
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()
    repo = args.repo_path.rstrip("/").encode()
    home = args.home.rstrip("/").encode()

    if args.mode == "verify":
        manifest = load_manifest()
        bad = 0
        for path, entry in sorted(manifest["files"].items()):
            if not os.path.exists(path):
                continue
            with open(path, "rb") as handle:
                data = handle.read()
            if sha(data) != entry["redacted_sha256"]:
                continue  # changed since redaction; not this manifest's concern
            if sha(restore_bytes(data, repo, home)) != entry["original_sha256"]:
                print(f"mismatch: {path}")
                bad += 1
        print(f"verified {len(manifest['files']) - bad} entries, {bad} mismatches")
        return 1 if bad else 0

    leaks = []
    manifest = load_manifest()
    changed = 0
    for path in candidate_files(args.files):
        with open(path, "rb") as handle:
            data = handle.read()
        if not is_text(data) or (home not in data and repo not in data):
            continue
        if args.mode == "check":
            leaks.append(path)
            continue
        if REPO_TOKEN in data or HOME_TOKEN in data:
            print(f"refusing {path}: already contains a redaction token", file=sys.stderr)
            return 2
        redacted = redact_bytes(data, repo, home)
        with open(path, "wb") as handle:
            handle.write(redacted)
        manifest["files"][path] = {"original_sha256": sha(data), "redacted_sha256": sha(redacted)}
        changed += 1

    if args.mode == "check":
        for path in leaks:
            print(f"local path in {path}")
        return 1 if leaks else 0
    if changed:
        os.makedirs(os.path.dirname(MANIFEST), exist_ok=True)
        with open(MANIFEST, "w") as handle:
            json.dump(manifest, handle, indent=1, sort_keys=True)
            handle.write("\n")
    print(f"redacted {changed} files")
    return 0


if __name__ == "__main__":
    sys.exit(main())
