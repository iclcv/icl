#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
# ICL - Image Component Library (https://github.com/iclcv/icl)
# Copyright (C) 2006-2026 Christof Elbrechter
"""
Mechanical migration helper for retiring the `.out()` surface on GUI
components in favour of reading values through `.handle()` names.

Context: `.out("name")` allocates a primitive inside the DataStore and
updates it from the GUI thread on every widget change — racy for
app-thread readers.  After Step A (atomic value caches in every
value-carrying handle), consumers can read the same value via
`gui["handle-name"]`, which routes through the lock-free / mutex-guarded
cache on the handle.  This script rewrites call sites to drop `.out()`
accordingly.

Decision rule per GUI component (one instance — `.handle()` and
`.out()` are method calls on the same chained expression):

    has out("X")  has handle("Y")  gui["X"] reads  gui["Y"] reads   action
    -----------   ---------------  --------------  --------------   ------
    yes           no                    —              —            rename .out("X") → .handle("X")
    yes           yes                   >0              0           drop .handle("Y"), rename .out → .handle("X")
    yes           yes                    0             >0           drop .out("X")
    yes           yes                    0              0           dead handle — drop .handle("Y"), rename .out → .handle("X")
    yes           yes                   >0             >0           AMBIGUOUS → skip + report
    no            yes                   —              —            no change

When a name is dropped, all `gui["dropped-name"]` and `gui("dropped-name")`
reader sites in the same file are rewritten to the surviving name.

Usage:
    ./migrate-out-to-handle.py                 # dry run, whole tree
    ./migrate-out-to-handle.py --apply          # actually rewrite files
    ./migrate-out-to-handle.py --file F.cpp     # scope to one file (dry run)
    ./migrate-out-to-handle.py --file F.cpp --apply

Exit code:
    0 = plan was clean (or applied cleanly)
    1 = ambiguous sites encountered — human needed
"""
from __future__ import annotations
import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path

# Root of the ICL source tree relative to this script (../).
REPO_ROOT = Path(__file__).resolve().parent.parent

# Only rewrite under these paths.  GUI.cpp + ChromaGUI.cpp + CamCfgWidget.cpp
# + Widget.cpp are framework-internal and handled separately (they define
# .out / .handle on the producing side).
SCAN_GLOB = "**/*.cpp"
SKIP_SUBSTRINGS = (
    "/qt/GUI.cpp",
    "/qt/Widget.cpp",
    "/qt/ChromaGUI.cpp",
    "/qt/CamCfgWidget.cpp",
    "/qt/DataStore.cpp",
    "/build/",
    "/builddir/",
    "/.git/",
    "/3rdparty/",
)


# A chain is the segment of source between "<<" or ";" or the start of
# line and the next ";" or "<<" — intuitively, one GUI component
# expression.  We collapse runs of whitespace to a single space so
# multi-line chains become single-line for regex matching.
#
# We don't try to fully parse C++; we just rely on ICL's convention
# that GUI chains are nested inside `gui << Component(...).a().b()...`
# and don't contain arbitrary semicolons/double-angles within.
CHAIN_SPLIT_RE = re.compile(r"<<|;")

OUT_RE = re.compile(r'\.out\(\s*"([^"]+)"\s*\)')
HANDLE_RE = re.compile(r'\.handle\(\s*"([^"]+)"\s*\)')

# Reader patterns observed in the tree:
#   gui["K"]                — operator[]
#   gui("K")                — operator()
#   gui.get<T>("K")         — typed lookup
#   gui.removeCallbacks("K") — callback management
# All of these reference a DataStore entry by its string key, so
# renaming the key has to update them all.  `allocValue<T>("K", ...)`
# is NOT a reader — it's the declaration site — but it's already
# handled by the .out / .handle rewrite path.
READER_RE = re.compile(
    r'\bgui\s*(?:'
    r'[\[(]\s*"([^"]+)"\s*[\])]'          # gui["K"] / gui("K")
    r'|'
    r'\.get\s*<[^<>]*>\s*\(\s*"([^"]+)"\s*\)'   # gui.get<T>("K")
    r'|'
    r'\.removeCallbacks\s*\(\s*"([^"]+)"\s*\)'  # gui.removeCallbacks("K")
    r')'
)


@dataclass
class Pair:
    """One `.out` / `.handle` pair found in a chain, with source locations."""
    out_name: str | None
    handle_name: str | None
    out_match: re.Match | None
    handle_match: re.Match | None
    # Byte offsets into the *original* (un-flattened) file text so we
    # can rewrite in place.
    out_span: tuple[int, int] | None
    handle_span: tuple[int, int] | None
    chain_span: tuple[int, int]    # for diagnostics


@dataclass
class FilePlan:
    path: Path
    pairs: list[Pair] = field(default_factory=list)
    reader_counts: dict[str, int] = field(default_factory=dict)
    edits: list[tuple[int, int, str]] = field(default_factory=list)
    rename_map: dict[str, str] = field(default_factory=dict)
    ambiguous: list[Pair] = field(default_factory=list)
    skipped: list[tuple[Pair, str]] = field(default_factory=list)


def find_chains(text: str) -> list[tuple[int, int]]:
    """Return (start, end) byte ranges of chain candidates — text between
    `<<` / `;` delimiters.  These are supersets of one GUI component's
    method-call chain; that's fine, our regexes are conservative.
    """
    chains: list[tuple[int, int]] = []
    cursor = 0
    n = len(text)
    for m in CHAIN_SPLIT_RE.finditer(text):
        end = m.start()
        if end > cursor:
            chains.append((cursor, end))
        cursor = m.end()
    if cursor < n:
        chains.append((cursor, n))
    return chains


def collect_pairs(text: str) -> list[Pair]:
    pairs: list[Pair] = []
    for (cstart, cend) in find_chains(text):
        segment = text[cstart:cend]
        outs = list(OUT_RE.finditer(segment))
        handles = list(HANDLE_RE.finditer(segment))
        # We expect at most one .out and one .handle per component chain,
        # but the chain-splitting is line-based so a single logical chain
        # might be split across our candidates OR several components might
        # share a chain (rare).  The common cases are: 0-or-1 .out and
        # 0-or-1 .handle per chain.
        if not outs and not handles:
            continue
        # Pair them up: the 1:1 case is the overwhelming majority.  If
        # multiple per chain, zip in declaration order and treat leftovers
        # as unpaired.
        n = max(len(outs), len(handles))
        for i in range(n):
            out_m = outs[i] if i < len(outs) else None
            hdl_m = handles[i] if i < len(handles) else None
            pairs.append(Pair(
                out_name=out_m.group(1) if out_m else None,
                handle_name=hdl_m.group(1) if hdl_m else None,
                out_match=out_m,
                handle_match=hdl_m,
                out_span=(cstart + out_m.start(), cstart + out_m.end()) if out_m else None,
                handle_span=(cstart + hdl_m.start(), cstart + hdl_m.end()) if hdl_m else None,
                chain_span=(cstart, cend),
            ))
    return pairs


def count_readers(text: str) -> dict[str, int]:
    counts: dict[str, int] = {}
    for m in READER_RE.finditer(text):
        # Only one capture group populates per match (alternation).
        key = next((g for g in m.groups() if g is not None), None)
        if key is None:
            continue
        counts[key] = counts.get(key, 0) + 1
    return counts


def plan_file(path: Path) -> FilePlan:
    plan = FilePlan(path=path)
    text = path.read_text()
    plan.pairs = collect_pairs(text)
    plan.reader_counts = count_readers(text)

    # For each pair, figure out the action.  Edits are expressed as
    # (start, end, replacement) on the ORIGINAL text.
    for p in plan.pairs:
        x = p.out_name
        y = p.handle_name

        if x is None and y is not None:
            # Only .handle — no change.
            continue

        if x is not None and y is None:
            # Only .out("X") — rename to .handle("X").
            assert p.out_span is not None
            plan.edits.append((p.out_span[0], p.out_span[1], f'.handle("{x}")'))
            continue

        # Both present.  Decide by reader counts.
        if x is None or y is None:
            continue  # (defensive; shouldn't happen)

        x_reads = plan.reader_counts.get(x, 0)
        y_reads = plan.reader_counts.get(y, 0)

        if x_reads > 0 and y_reads > 0:
            plan.ambiguous.append(p)
            continue

        if y_reads > 0 and x_reads == 0:
            # Readers use the handle name.  Drop .out("X"); keep .handle("Y").
            assert p.out_span is not None
            plan.edits.append((p.out_span[0], p.out_span[1], ""))
            continue

        # x_reads >= 0 and y_reads == 0  → the handle name "Y" is dead
        # or readers use "X".  Drop .handle("Y") and rename .out("X")
        # to .handle("X").
        assert p.out_span is not None and p.handle_span is not None
        plan.edits.append((p.handle_span[0], p.handle_span[1], ""))
        plan.edits.append((p.out_span[0], p.out_span[1], f'.handle("{x}")'))
        if y != x and y_reads == 0:
            # No reader cleanup needed for Y since it has no readers.
            pass
        if y_reads > 0:
            # (shouldn't reach here given the branch above)
            plan.rename_map[y] = x

    return plan


def apply_plan(plan: FilePlan) -> str:
    """Return the rewritten file text (does not write to disk)."""
    text = plan.path.read_text()

    # Apply edits from the end so byte offsets remain valid.
    edits = sorted(plan.edits, key=lambda e: e[0], reverse=True)
    for (start, end, repl) in edits:
        text = text[:start] + repl + text[end:]

    # Reader rewrites (if any).  Same three forms we count as readers.
    for old, new in plan.rename_map.items():
        old_q = re.escape(old)
        # gui["K"] / gui("K")
        text = re.sub(
            r'(\bgui\s*[\[(]\s*")' + old_q + r'("\s*[\])])',
            lambda m, n=new: m.group(1) + n + m.group(2),
            text,
        )
        # gui.get<T>("K")
        text = re.sub(
            r'(\bgui\s*\.get\s*<[^<>]*>\s*\(\s*")' + old_q + r'("\s*\))',
            lambda m, n=new: m.group(1) + n + m.group(2),
            text,
        )
        # gui.removeCallbacks("K")
        text = re.sub(
            r'(\bgui\s*\.removeCallbacks\s*\(\s*")' + old_q + r'("\s*\))',
            lambda m, n=new: m.group(1) + n + m.group(2),
            text,
        )

    return text


def describe_pair(p: Pair, counts: dict[str, int]) -> str:
    x = p.out_name
    y = p.handle_name
    xr = counts.get(x, 0) if x else 0
    yr = counts.get(y, 0) if y else 0
    parts = []
    if x is not None:
        parts.append(f'out="{x}"({xr})')
    if y is not None:
        parts.append(f'handle="{y}"({yr})')
    return " ".join(parts)


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--apply", action="store_true", help="rewrite files in place")
    ap.add_argument("--file", type=Path, help="scope to a single file")
    ap.add_argument("--verbose", "-v", action="store_true", help="print per-pair decisions")
    args = ap.parse_args()

    if args.file:
        files = [args.file.resolve()]
    else:
        files = []
        for p in REPO_ROOT.rglob(SCAN_GLOB):
            sp = str(p)
            if any(skip in sp for skip in SKIP_SUBSTRINGS):
                continue
            files.append(p)
        files.sort()

    total_edits = 0
    total_ambiguous = 0
    files_changed = 0

    for f in files:
        try:
            plan = plan_file(f)
        except Exception as e:
            print(f"SKIP {f}: {e}", file=sys.stderr)
            continue

        if not plan.edits and not plan.ambiguous:
            continue

        rel = f.relative_to(REPO_ROOT) if f.is_relative_to(REPO_ROOT) else f
        files_changed += 1
        total_edits += len(plan.edits)
        total_ambiguous += len(plan.ambiguous)

        if plan.edits or plan.ambiguous:
            print(f"\n--- {rel}")
        if args.verbose or not args.apply:
            for p in plan.pairs:
                if p.out_name is None:
                    continue  # no-change pair
                desc = describe_pair(p, plan.reader_counts)
                if p in plan.ambiguous:
                    print(f"  AMBIGUOUS  {desc}  — skip, human needed")
                else:
                    print(f"  plan       {desc}")

        if args.apply and plan.edits:
            new_text = apply_plan(plan)
            f.write_text(new_text)

    print(f"\nfiles touched: {files_changed}")
    print(f"edits planned: {total_edits}")
    print(f"ambiguous:     {total_ambiguous}")
    return 1 if total_ambiguous else 0


if __name__ == "__main__":
    sys.exit(main())
