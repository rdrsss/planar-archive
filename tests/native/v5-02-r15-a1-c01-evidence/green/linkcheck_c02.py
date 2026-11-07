#!/usr/bin/env python3
"""Check relative Markdown links and their heading anchors in the named files.

Usage: python3 -I linkcheck.py <repo root> <file relative to repo root> ...

Each inline link target that is not an absolute URL or mailto: is percent-decoded, its
path part is resolved against the file's directory and must exist, and a #fragment must
match a GitHub-style heading slug of the target file (duplicates get -1, -2 suffixes).
Fenced code blocks and inline code spans are skipped. Prints one line per link and a summary; exits 1 when any
link is broken. Nothing is written to disk.
"""
import re
import sys
import unicodedata
from pathlib import Path
from urllib.parse import unquote

LINK = re.compile(r"\[[^\]]*\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")
HEADING = re.compile(r"^(#{1,6})\s+(.*?)\s*#*\s*$")
FENCE = re.compile(r"^\s*(```|~~~)")
CODE_SPAN = re.compile(r"`[^`]*`")


def visible_lines(text):
    """Yield (line number, line) outside fenced code blocks."""
    fenced = False
    for number, line in enumerate(text.splitlines(), 1):
        if FENCE.match(line):
            fenced = not fenced
            continue
        if not fenced:
            yield number, line


def slug(heading):
    text = re.sub(r"`", "", heading).strip().lower()
    text = unicodedata.normalize("NFKC", text)
    text = re.sub(r"[^\w\- ]", "", text)
    return text.replace(" ", "-")


def anchors_of(path):
    seen = {}
    result = set()
    for _, line in visible_lines(path.read_text(encoding="utf-8")):
        match = HEADING.match(line)
        if not match:
            continue
        base = slug(match.group(2))
        count = seen.get(base, 0)
        result.add(base if count == 0 else f"{base}-{count}")
        seen[base] = count + 1
    return result


def check_file(root, rel, counts, output):
    source = root / rel
    text = source.read_text(encoding="utf-8")
    for number, line in visible_lines(text):
        line = CODE_SPAN.sub("", line)
        for match in LINK.finditer(line):
            target = match.group(1)
            if re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", target):
                continue
            path_part, _, fragment = target.partition("#")
            path_part = unquote(path_part)
            fragment = unquote(fragment)
            destination = source if path_part == "" else (source.parent / path_part).resolve()
            counts["links"] += 1
            if not destination.exists():
                counts["broken"] += 1
                output.append(f"BROKEN {rel}:{number} -> {target} (no such file)")
                continue
            if fragment:
                if destination.suffix.lower() != ".md":
                    counts["broken"] += 1
                    output.append(f"BROKEN {rel}:{number} -> {target} (fragment on non-markdown)")
                    continue
                if fragment not in anchors_of(destination):
                    counts["broken"] += 1
                    output.append(f"BROKEN {rel}:{number} -> {target} (no such anchor)")
                    continue
            counts["ok"] += 1
            output.append(f"OK {rel}:{number} -> {target}")


def main(argv):
    if len(argv) < 3:
        print("usage: linkcheck.py <repo root> <file> ...", file=sys.stderr)
        return 2
    root = Path(argv[1]).resolve()
    counts = {"links": 0, "ok": 0, "broken": 0}
    output = []
    for rel in argv[2:]:
        check_file(root, rel, counts, output)
    for line in output:
        print(line)
    print(f"files checked: {len(argv) - 2}")
    print(f"links checked: {counts['links']}; ok: {counts['ok']}; broken: {counts['broken']}")
    return 1 if counts["broken"] else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
