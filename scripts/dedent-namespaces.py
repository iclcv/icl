#!/usr/bin/env python3
"""Dedent content inside fused namespace icl::X blocks by 2 spaces.

After fusing 'namespace icl { namespace X {' into 'namespace icl::X {',
the content is at 4+ spaces (was 2 for each nesting level). This script
removes exactly 2 leading spaces from every line between the namespace
opening and its closing brace, preserving relative indentation.

Lines NOT dedented:
- Blank lines (no change needed)
- Preprocessor directives (already at column 0)
- Lines with fewer than 2 leading spaces
- Lines outside the namespace block
"""

import re
import sys
from pathlib import Path


def dedent_file(path: Path, dry_run: bool = False) -> bool:
    text = path.read_text()

    # Find all fused namespace openings
    ns_re = re.compile(r'^namespace icl::\w+ \{', re.MULTILINE)
    matches = list(ns_re.finditer(text))
    if not matches:
        return False

    lines = text.split('\n')
    changed = False

    for m in matches:
        # Find which line this match is on
        start_line = text[:m.start()].count('\n')

        # Find the closing brace: walk forward looking for the matching '}'
        # that corresponds to the namespace close comment or is at column 0
        # We identify it as a line matching '} // namespace icl::...' or
        # simply the last '}' at < 2 indent that closes the block.

        # Find the closing line: scan for '} // namespace icl::' pattern
        close_line = None
        for i in range(start_line + 1, len(lines)):
            stripped = lines[i].strip()
            if re.match(r'^\} // namespace icl::', stripped):
                close_line = i
                break
            # Also match bare '}' at column 0 that could be the close
            # (some files don't have the comment)
            if stripped == '}' and not lines[i].startswith(' '):
                # Check if next non-blank line is #endif or EOF
                j = i + 1
                while j < len(lines) and lines[j].strip() == '':
                    j += 1
                if j >= len(lines) or lines[j].strip().startswith('#endif') or lines[j].strip().startswith('#else'):
                    close_line = i
                    break

        if close_line is None:
            continue

        # Dedent lines between start_line+1 and close_line (exclusive)
        for i in range(start_line + 1, close_line):
            line = lines[i]
            # Skip blank lines and preprocessor directives
            if line == '' or line.startswith('#'):
                continue
            # Remove exactly 2 leading spaces if present
            if line.startswith('  '):
                lines[i] = line[2:]
                changed = True

    if changed:
        new_text = '\n'.join(lines)
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
            if dedent_file(path, dry_run=dry_run):
                changed.append(rel)

    action = "Would change" if dry_run else "Changed"
    print(f"{action} {len(changed)} files")

    if dry_run:
        print("\nRe-run without --dry-run to apply changes.")


if __name__ == '__main__':
    main()
