#!/usr/bin/env python3
"""Second-pass fixer for matrix operator() -> index_yx migration.

Handles patterns the first script missed:
- Multiple calls per line (already half-fixed)
- (*this)(a,b) and (*ptr)(a,b) dereference patterns
- var.operator ()(a,b) explicit call syntax
- Simple var(a,b) where the first script's column-based approach failed

Reads compiler error output, extracts file:line pairs, then does a global
regex replacement on those lines for ALL operator()-style matrix calls.

Usage:
  ninja -C build -j16 -k0 2>&1 | python3 scripts/fix-matrix-indexing2.py
  ninja -C build -j16 -k0 2>&1 | python3 scripts/fix-matrix-indexing2.py --apply
"""

import sys
import re
import os

apply_fixes = '--apply' in sys.argv

# Parse compiler error lines
error_re = re.compile(
    r'^(.*?):(\d+):\d+: error:.*(?:does not provide a call operator|no member named .operator\(\).|no matching function)'
)

# Collect unique file:line pairs
error_lines = {}  # file -> set of line numbers
for raw_line in sys.stdin:
    m = error_re.match(raw_line.strip())
    if not m:
        continue
    filepath = m.group(1)
    line_no = int(m.group(2))

    # Resolve path
    if not os.path.isfile(filepath):
        stripped = re.sub(r'^(\.\./)+', '', filepath)
        for prefix in ['', '../', 'build/../']:
            for candidate in [prefix + filepath, prefix + stripped]:
                if os.path.isfile(candidate):
                    filepath = candidate
                    break
            else:
                continue
            break
        else:
            continue

    if filepath not in error_lines:
        error_lines[filepath] = set()
    error_lines[filepath].add(line_no)

# Patterns to fix (order matters — more specific first)

def balanced_paren_arg(s, start):
    """Extract content of balanced parens starting at s[start]='(', return (content, end_pos)."""
    if start >= len(s) or s[start] != '(':
        return None
    depth = 0
    for i in range(start, len(s)):
        if s[i] == '(':
            depth += 1
        elif s[i] == ')':
            depth -= 1
            if depth == 0:
                return (s[start+1:i], i)
    return None

def split_two_args(content):
    """Split 'arg1, arg2' respecting nested parens. Returns (a1, a2) or None if not exactly 2 args."""
    depth = 0
    for i, c in enumerate(content):
        if c in '([<':
            depth += 1
        elif c in ')]>':
            depth -= 1
        elif c == ',' and depth == 0:
            # Check no more commas at depth 0
            rest = content[i+1:]
            d2 = 0
            for c2 in rest:
                if c2 in '([<':
                    d2 += 1
                elif c2 in ')]>':
                    d2 -= 1
                elif c2 == ',' and d2 == 0:
                    return None  # 3+ args
            return (content[:i].strip(), content[i+1:].strip())
    return None  # 0 or 1 arg

def fix_line(line):
    """Apply all matrix operator() -> index_yx fixes to a single line. Returns (new_line, count)."""
    count = 0

    # Pattern 1: explicit .operator ()(arg1, arg2)
    # e.g., dst.operator ()(x,y) -> dst.index_yx(y, x)
    def replace_explicit_op(m):
        nonlocal count
        var = m.group(1)
        content = m.group(2)
        args = split_two_args(content)
        if args is None:
            return m.group(0)  # not 2-arg, skip
        count += 1
        return f"{var}.index_yx({args[1]}, {args[0]})"

    line = re.sub(
        r'(\w[\w\[\]]*)\s*\.operator\s*\(\)\s*\(\s*([^)]*(?:\([^)]*\)[^)]*)*)\s*\)',
        replace_explicit_op,
        line
    )

    # Pattern 2: (*expr)(arg1, arg2)
    # e.g., (*this)(i,j) -> (*this).index_yx(j, i)
    # e.g., (*data->m_collisionMatrix)(col,row) -> (*data->m_collisionMatrix).index_yx(row, col)
    def replace_deref(m):
        nonlocal count
        deref_expr = m.group(1)  # includes (*...)
        paren_content = m.group(2)
        args = split_two_args(paren_content)
        if args is None:
            return m.group(0)
        count += 1
        return f"{deref_expr}.index_yx({args[1]}, {args[0]})"

    line = re.sub(
        r'(\(\*[^)]+\))\s*\(\s*([^)]*(?:\([^)]*\)[^)]*)*)\s*\)',
        replace_deref,
        line
    )

    # Pattern 3: regex-based inside-out approach
    # Match innermost var(arg1, arg2) where args contain no commas or unbalanced parens.
    # This naturally handles nesting: inner calls get matched before outer ones.
    # Apply iteratively until stable.

    skip_names = {'if', 'for', 'while', 'switch', 'return', 'sizeof', 'static_cast',
                  'dynamic_cast', 'reinterpret_cast', 'const_cast', 'throw', 'new',
                  'delete', 'case', 'catch', 'index_yx', 'at', 'sqr', 'fabs', 'asin',
                  'acos', 'atan2', 'sqrt', 'pow', 'log', 'exp', 'sin', 'cos', 'tan',
                  'ceil', 'floor', 'round', 'iclMin', 'iclMax', 'abs', 'fmod',
                  'std', 'max', 'min', 'swap', 'pair', 'make_pair',
                  'FixedMatrix', 'FixedColVector', 'FixedRowVector',
                  'DynMatrix', 'DynColVector', 'DynRowVector',
                  'Vec', 'Vec3', 'V3', 'Mat', 'Mat3',
                  'Point32f', 'Point', 'Size', 'Rect',
                  'ERROR_LOG', 'ICL_TEST_NEAR', 'ICL_TEST_EQ',
                  'push_back', 'emplace_back', 'insert', 'erase',
                  'decompose_QR', 'decompose_LU', 'decompose_RQ',
                  }

    # Matches: identifier(arg1, arg2) where args have no parens or commas
    inner_call_re = re.compile(r'\b(\w+)\(([^(),]+),\s*([^(),]+)\)')

    def replace_inner(m):
        nonlocal count
        name = m.group(1)
        arg1 = m.group(2).strip()
        arg2 = m.group(3).strip()
        if name in skip_names or name.startswith('_mm_') or name == 'index_yx':
            return m.group(0)
        count += 1
        return f"{name}.index_yx({arg2}, {arg1})"

    # Pattern 3a: chained call: expr()(arg1, arg2) -> expr().index_yx(arg2, arg1)
    # e.g., getParams()(x,y) -> getParams().index_yx(y, x)
    # e.g., getTransformation()(3,2) -> getTransformation().index_yx(2, 3)
    chained_re = re.compile(r'(\w+\(\))\(([^(),]+),\s*([^(),]+)\)')
    def replace_chained(m):
        nonlocal count
        expr = m.group(1)
        arg1 = m.group(2).strip()
        arg2 = m.group(3).strip()
        count += 1
        return f"{expr}.index_yx({arg2}, {arg1})"

    line = chained_re.sub(replace_chained, line)

    # Apply inner_call_re iteratively until no more changes
    for _ in range(10):  # safety limit
        new_line = inner_call_re.sub(replace_inner, line)
        if new_line == line:
            break
        line = new_line

    return (line, count)


# Process files
total_fixes = 0
total_manual = 0

for filepath, line_nos in sorted(error_lines.items()):
    try:
        with open(filepath) as f:
            lines = f.readlines()
    except Exception as e:
        print(f"  SKIP (read error): {filepath} — {e}", file=sys.stderr)
        continue

    file_fixes = 0
    for line_no in sorted(line_nos):
        if line_no > len(lines):
            continue
        orig = lines[line_no - 1]
        fixed, n = fix_line(orig)
        if n > 0 and fixed != orig:
            print(f"  {filepath}:{line_no}:  {orig.strip()}")
            print(f"    -> {fixed.strip()}")
            if apply_fixes:
                lines[line_no - 1] = fixed
            file_fixes += n
            total_fixes += n
        elif orig.strip() and 'index_yx' not in orig:
            # Still has unfixed calls
            print(f"  MANUAL: {filepath}:{line_no}: {orig.strip()}", file=sys.stderr)
            total_manual += 1

    if apply_fixes and file_fixes > 0:
        with open(filepath, 'w') as f:
            f.writelines(lines)
        print(f"  Applied {file_fixes} fix(es) to {filepath}")

print(f"\nTotal: {total_fixes} fixes, {total_manual} manual", file=sys.stderr)
