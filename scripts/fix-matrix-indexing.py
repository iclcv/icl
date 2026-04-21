#!/usr/bin/env python3
"""Fix FixedMatrix/DynMatrix operator() calls after removing operator()(col,row).

Reads compiler error output from stdin, finds the call site, swaps arguments,
and replaces var(a, b) with var.index_yx(b, a).

Usage:
  # Dry run (shows proposed changes):
  meson compile -C build -j1 2>&1 | python3 scripts/fix-matrix-indexing.py

  # Apply fixes:
  meson compile -C build -j1 2>&1 | python3 scripts/fix-matrix-indexing.py --apply

  # Process a single error line:
  echo 'icl/geom/Camera.cpp:123:15: error: ...' | python3 scripts/fix-matrix-indexing.py
"""

import sys
import re
import os

apply_fixes = '--apply' in sys.argv

# Parse clang/gcc error lines: file:line:col: error: ...
# Match "does not provide a call operator" and "no matching function" (cascade from templates)
error_re = re.compile(r'^(.*?):(\d+):(\d+): error:.*(?:does not provide a call operator|no member named \'operator\(\)\'|no matching function)')

fixes = {}  # file -> [(line_no, old_line, new_line)]

def find_call_and_swap(line, col):
    """Find the (arg1, arg2) call near `col` and return the fixed line."""
    # Walk backwards from col to find the variable name and the '('
    # col is 1-based in compiler output
    idx = col - 1

    # The error typically points to the variable or the '('
    # Find the opening '(' at or after idx
    paren_start = line.find('(', max(0, idx - 1))
    if paren_start < 0:
        return None

    # Walk backwards to find the start of the variable/expression
    # The character before '(' should be the end of the variable name
    var_end = paren_start
    var_start = var_end - 1
    while var_start >= 0 and (line[var_start].isalnum() or line[var_start] in '_]>'):
        # Handle array subscripts and template args
        if line[var_start] == ']':
            depth = 1
            var_start -= 1
            while var_start >= 0 and depth > 0:
                if line[var_start] == ']': depth += 1
                if line[var_start] == '[': depth -= 1
                var_start -= 1
            continue
        if line[var_start] == '>':
            depth = 1
            var_start -= 1
            while var_start >= 0 and depth > 0:
                if line[var_start] == '>': depth += 1
                if line[var_start] == '<': depth -= 1
                var_start -= 1
            continue
        var_start -= 1
    var_start += 1

    # Extract the two arguments, handling nested parens
    depth = 0
    i = paren_start
    arg_start = paren_start + 1
    comma_pos = -1
    paren_end = -1

    for i in range(paren_start, len(line)):
        c = line[i]
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                paren_end = i
                break
        elif c == ',' and depth == 1:
            if comma_pos < 0:
                comma_pos = i

    if paren_end < 0 or comma_pos < 0:
        return None

    arg1 = line[arg_start:comma_pos].strip()
    arg2 = line[comma_pos + 1:paren_end].strip()
    var = line[var_start:var_end]

    if not var or not arg1 or not arg2:
        return None

    # Check for third argument (skip — not a matrix 2-arg call)
    remaining = line[comma_pos + 1:paren_end]
    if ',' in remaining:
        # Count commas at depth 0
        d = 0
        for c in remaining:
            if c == '(': d += 1
            elif c == ')': d -= 1
            elif c == ',' and d == 0:
                return None  # 3+ args, not a matrix call

    # Build replacement: var(a, b) -> var.index_yx(b, a)
    old = line[var_start:paren_end + 1]
    new = f"{var}.index_yx({arg2}, {arg1})"

    new_line = line[:var_start] + new + line[paren_end + 1:]
    return (old, new, new_line)


seen = set()
for raw_line in sys.stdin:
    m = error_re.match(raw_line.strip())
    if not m:
        continue

    filepath = m.group(1)
    line_no = int(m.group(2))
    col_no = int(m.group(3))

    # Skip duplicates
    key = (filepath, line_no, col_no)
    if key in seen:
        continue
    seen.add(key)

    # Resolve path — compiler outputs paths relative to build dir (../icl/...)
    if not os.path.isfile(filepath):
        for prefix in ['', '../', 'build/../']:
            candidate = prefix + filepath
            if os.path.isfile(candidate):
                filepath = candidate
                break
            # Also try stripping leading ../
            stripped = re.sub(r'^(\.\./)+', '', filepath)
            if os.path.isfile(prefix + stripped):
                filepath = prefix + stripped
                break
        else:
            print(f"  SKIP (file not found): {filepath}:{line_no}:{col_no}", file=sys.stderr)
            continue

    # Read the line
    try:
        with open(filepath) as f:
            lines = f.readlines()
        if line_no > len(lines):
            continue
        src_line = lines[line_no - 1]
    except Exception as e:
        print(f"  SKIP (read error): {filepath}:{line_no} — {e}", file=sys.stderr)
        continue

    result = find_call_and_swap(src_line, col_no)
    if result is None:
        print(f"  MANUAL: {filepath}:{line_no}:{col_no}", file=sys.stderr)
        print(f"          {src_line.rstrip()}", file=sys.stderr)
        continue

    old, new, new_line = result
    print(f"  {filepath}:{line_no}:  {old}  ->  {new}")

    if apply_fixes:
        if filepath not in fixes:
            fixes[filepath] = []
        fixes[filepath].append((line_no, src_line, new_line))

# Apply fixes
if apply_fixes:
    for filepath, file_fixes in fixes.items():
        with open(filepath) as f:
            lines = f.readlines()
        # Apply in reverse order so line numbers stay valid
        for line_no, old_line, new_line in sorted(file_fixes, key=lambda x: -x[0]):
            if lines[line_no - 1] == old_line:
                lines[line_no - 1] = new_line
            else:
                print(f"  WARNING: line changed, skipping {filepath}:{line_no}", file=sys.stderr)
        with open(filepath, 'w') as f:
            f.writelines(lines)
        print(f"  Applied {len(file_fixes)} fix(es) to {filepath}")

print(f"\nTotal: {len(seen)} error sites found", file=sys.stderr)
