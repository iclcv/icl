#!/usr/bin/env python3
"""Fix orphaned closing braces left by fuse-namespaces.py.

When the opening was fused (namespace icl { namespace X { -> namespace icl::X {)
but the closing was not (two braces remain), this script removes the extra
closing brace that corresponded to the old 'namespace icl {'.

Strategy: for each excess closing brace, find the last line at column 0
that is just '}' (with optional comment like '// namespace icl') and remove it.
"""

import re
from pathlib import Path


def fix_file(path: Path) -> bool:
    text = path.read_text()

    # Only process files that had namespace fusion applied
    if not re.search(r'^namespace icl::\w+ \{', text, re.MULTILINE):
        return False

    opens = text.count('{')
    closes = text.count('}')
    excess = closes - opens

    if excess <= 0:
        return False

    lines = text.split('\n')
    original_lines = list(lines)

    # For each excess brace, find and remove the last line at column 0
    # that is just '}' with optional whitespace/comment
    # These are the orphaned outer namespace closes.
    # Work backwards from end of file.
    for _ in range(excess):
        # Scan from end, find last bare '}' at column 0
        for i in range(len(lines) - 1, -1, -1):
            stripped = lines[i].strip()
            # Match: bare }, }// comment, } // comment
            if (stripped == '}' or
                re.match(r'^\}(\s*//.*)?$', stripped)):
                # Make sure this is at column 0 (not indented) — it's the outer namespace
                if lines[i].lstrip() == stripped and not lines[i].startswith(' ') and not lines[i].startswith('\t'):
                    lines.pop(i)
                    break

    new_text = '\n'.join(lines)
    if new_text != text:
        path.write_text(new_text)
        return True
    return False


def main():
    root = Path(__file__).resolve().parent.parent
    fixed = []
    still_broken = []

    for ext in ('*.h', '*.cpp'):
        for path in sorted(root.rglob(ext)):
            rel = str(path.relative_to(root))
            if any(p in rel for p in ['build/', '3rdparty/', '.git/', 'outputs/']):
                continue
            if fix_file(path):
                fixed.append(rel)
                # Re-check
                text = path.read_text()
                excess = text.count('}') - text.count('{')
                if excess > 0:
                    still_broken.append((rel, excess))

    print(f"Fixed {len(fixed)} files")
    if still_broken:
        print(f"\nStill have extra braces ({len(still_broken)} files):")
        for f, n in still_broken:
            print(f"  {f}: +{n}")


if __name__ == '__main__':
    main()
