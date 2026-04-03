#!/usr/bin/env python3
"""Fuse 'namespace icl { namespace X {' into 'namespace icl::X {' (C++17).

Strategy:
1. Fuse the opening: 'namespace icl {\\n  namespace X {' -> 'namespace icl::X {'
2. Remove the outer closing: the LAST line at column 0 that is just '}'
   (with optional comment mentioning 'icl' or 'namespace').
3. Update the inner closing comment to mention the fused namespace.

The outer '}' is reliably identifiable because ICL files always indent
the inner namespace content, so the inner close is indented (e.g. '  }')
and the outer close is at column 0.
"""

import re
import sys
from pathlib import Path

# Match multi-line namespace opening ONLY.
# The inner namespace line must end with just '{' (with optional whitespace),
# NOT a one-liner like 'namespace X { class Foo; }'.
OPEN_RE = re.compile(
    r'^(namespace icl)\s*\{\s*\n(\s*)(namespace (\w+))\s*\{\s*$',
    re.MULTILINE
)


def fuse_file(path: Path, dry_run: bool = False) -> bool:
    text = path.read_text()

    m = OPEN_RE.search(text)
    if not m:
        return False

    ns_name = m.group(4)

    lines = text.split('\n')

    # Step 1: Find the last line at column 0 that is just '}' (with optional comment).
    # Walk backwards, skipping blank lines, #endif, /** \endcond */.
    outer_close_idx = None
    for i in range(len(lines) - 1, -1, -1):
        stripped = lines[i].strip()
        if stripped == '' or stripped.startswith('#endif') or stripped == '/** \\endcond */':
            continue
        # Check: starts at column 0, is just '}' with optional comment
        if (not lines[i][0:1].isspace() and
            re.match(r'^\}(\s*//.*)?$', stripped)):
            outer_close_idx = i
            break
        else:
            # Hit real code that isn't the outer close
            break

    if outer_close_idx is None:
        return False

    # Step 2: Update the inner close comment (line before the outer close).
    # Walk backwards from outer_close_idx to find the inner close.
    inner_close_idx = None
    for j in range(outer_close_idx - 1, -1, -1):
        stripped = lines[j].strip()
        if stripped == '':
            continue
        if stripped.startswith('}'):
            inner_close_idx = j
            break
        else:
            break

    if inner_close_idx is not None:
        old_inner = lines[inner_close_idx]
        # Update namespace comment if present
        new_inner = re.sub(
            r'\}\s*//\s*(?:end of\s+)?(?:namespace\s+)?' + re.escape(ns_name) + r'\s*$',
            '} // namespace icl::' + ns_name,
            old_inner
        )
        lines[inner_close_idx] = new_inner

    # Step 3: Remove the outer close line and any blank lines between
    # inner close and outer close.
    # First remove outer close
    del lines[outer_close_idx]
    # Then remove trailing blank lines between inner close and where outer was
    while (inner_close_idx is not None and
           inner_close_idx + 1 < len(lines) and
           lines[inner_close_idx + 1].strip() == ''):
        # Only remove if the next non-blank is #endif or EOF
        next_non_blank = inner_close_idx + 1
        while next_non_blank < len(lines) and lines[next_non_blank].strip() == '':
            next_non_blank += 1
        if next_non_blank >= len(lines) or lines[next_non_blank].strip().startswith('#endif'):
            del lines[inner_close_idx + 1]
        else:
            break

    # Step 4: Fuse the opening
    new_text = '\n'.join(lines)
    new_text = OPEN_RE.sub(
        lambda m: f'namespace icl::{m.group(4)} {{',
        new_text,
        count=1
    )

    if new_text != text:
        if not dry_run:
            path.write_text(new_text)
        return True
    return False


def main():
    dry_run = '--dry-run' in sys.argv
    root = Path(__file__).resolve().parent.parent

    changed = []
    for ext in ('*.h', '*.cpp'):
        for path in sorted(root.rglob(ext)):
            rel = path.relative_to(root)
            parts = rel.parts
            if any(p in ('build', '3rdparty', '.git', 'outputs') for p in parts):
                continue
            if fuse_file(path, dry_run=dry_run):
                changed.append(rel)

    action = "Would change" if dry_run else "Changed"
    print(f"{action} {len(changed)} files")

    if dry_run:
        print("\nRe-run without --dry-run to apply changes.")


if __name__ == '__main__':
    main()
