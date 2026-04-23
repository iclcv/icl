#!/usr/bin/env python3
"""
Rewrite legacy string-taking addProperty() calls to the typed prop:: overload.

Usage:
    scripts/migrate-addProperty.py <file> [<file> ...]

Handles the canonical patterns (see grep survey in step-6 planning):

    addProperty("N","flag","",v)               -> prop::Flag{}, v
    addProperty("N","command","")              -> prop::Command{}
    addProperty("N","info","",v)               -> prop::Info{}, v
    addProperty("N","string","M",v)            -> prop::Text{.maxLength=M}, v
    addProperty("N","menu","a,b,c",v)          -> prop::Menu{"a","b","c"}, v
    addProperty("N","range","[A,B]",v)         -> prop::Range{.min=A, .max=B}, v          (slider)
    addProperty("N","range","[A,B]:S",v)       -> prop::Range{.min=A, .max=B, .step=S}, v
    addProperty("N","range:slider", ... )      -> same as "range"
    addProperty("N","range:spinbox","[A,B]",v) -> prop::Range{.min=A, .max=B, .ui=UI::Spinbox}, v
    addProperty("N","float","[A,B]",v)         -> prop::Range{.min=A, .max=B}, v
    addProperty("N","int","[A,B]",v)           -> prop::Range{.min=A, .max=B, .ui=UI::Spinbox}, v

Numeric type inference for Range: if any of min/max/step contains a decimal point
or `e`/`E` exponent, emit `f`-suffixed float literals so CTAD picks Range<float>.
Otherwise emit unsuffixed ints so CTAD picks Range<int>.

After rewriting, ensures `#include <icl/utils/prop/Constraints.h>` is present.
Unrecognized patterns (computed info strings, value-list, Point32f, multi-line
calls, etc.) are left untouched — compiler will flag them after the rewrite.

Reports per-file counts and a summary.
"""
import re
import sys
from pathlib import Path

INCLUDE_LINE = '#include <icl/utils/prop/Constraints.h>'

# Qualified emission — avoids collisions with Configurable::prop() member
# and works from any file whose source namespace is inside icl::
# (utils is reachable as `icl::utils`).  File-scope alias is not used
# because it clashes with the member function of the same name in every
# Configurable subclass.
Q = 'utils::prop::'

# Match floats vs ints inside Range min/max/step values.  Accepts leading - and
# scientific notation.  "looks float" iff contains . or e/E.
def is_float(s: str) -> bool:
    s = s.strip()
    return '.' in s or 'e' in s or 'E' in s

def num_literal(s: str, force_float: bool) -> str:
    """Normalize a numeric token for emission as a C++ literal."""
    s = s.strip()
    if force_float and not s.endswith('f'):
        if is_float(s):
            return s + 'f'
        else:
            return s + '.f'
    return s

def normalize_range(min_s: str, max_s: str, step_s: str | None):
    """Return (min, max, step_or_None) as C++ literals, consistent in type."""
    # Any float triggers float-typed emission for the whole set.
    force_float = is_float(min_s) or is_float(max_s) or (step_s is not None and is_float(step_s))
    m = num_literal(min_s, force_float)
    M = num_literal(max_s, force_float)
    S = num_literal(step_s, force_float) if step_s is not None else None
    return m, M, S

# Character class of an argument terminator: ) , ; \n \s
# We need to match arbitrary balanced tokens as value args — but regex can't do
# full balancing.  Pragmatic: match up through the closing ) of addProperty using
# a heuristic that counts nested parens in the rest of the call.
# Simpler path: rewrite just the type+info+opening-args slice.  The value + trailing
# args keep their original text.

# Patterns operate on the slice starting at `"type"` through the comma after
# `"info"`, leaving any downstream arg text (value, volatileness, tooltip) untouched.

def rewrite_line(line: str, counts: dict) -> str:
    orig = line

    # Skip adaptProperty() calls: they use the same type/info grammar
    # as addProperty but take 4 strings (no typed-constraint overload),
    # so rewriting breaks the build.
    if 'adaptProperty(' in line:
        return line

    # flag: "flag", ""
    line = re.sub(
        r'"flag"\s*,\s*""\s*,\s*',
        lambda m: (counts.__setitem__('flag', counts.get('flag', 0) + 1), f'{Q}Flag{{}}, ')[1],
        line
    )
    # command: handle both legacy forms:
    #   "command","",""[, vol, tip]  — info=="" + explicit value==""
    #      → `Command{}, {}` (explicit monostate initial so trailing
    #         positional args line up with addProperty's signature)
    #   "command",""[, vol, tip]     — info=="" alone (value defaulted)
    #      → `Command{}` (template default initial picks up)
    line = re.sub(
        r'"command"\s*,\s*""\s*,\s*""\s*(?=,)',
        lambda m: (counts.__setitem__('command', counts.get('command', 0) + 1), f'{Q}Command{{}}, {{}}')[1],
        line
    )
    line = re.sub(
        r'"command"\s*,\s*""\s*(?=[,)])',
        lambda m: (counts.__setitem__('command', counts.get('command', 0) + 1), f'{Q}Command{{}}')[1],
        line
    )
    # info: "info", ""
    line = re.sub(
        r'"info"\s*,\s*""\s*,\s*',
        lambda m: (counts.__setitem__('info', counts.get('info', 0) + 1), f'{Q}Info{{}}, ')[1],
        line
    )
    # string: "string", "N"
    def _string(m):
        counts['string'] = counts.get('string', 0) + 1
        n = m.group(1).strip()
        return f'{Q}Text{{.maxLength={n}}}, '
    line = re.sub(
        r'"string"\s*,\s*"(\d+)"\s*,\s*',
        _string,
        line
    )
    # menu: "menu", "a,b,c"  — accept arbitrary comma-separated choices
    def _menu(m):
        counts['menu'] = counts.get('menu', 0) + 1
        choices = m.group(1)
        parts = re.split(r'(?<!\\),', choices)
        cleaned = [p.strip() for p in parts if p.strip()]
        quoted = ', '.join('"' + p.replace('"', '\\"') + '"' for p in cleaned)
        return f'{Q}Menu{{{quoted}}}, '
    line = re.sub(
        r'"menu"\s*,\s*"([^"]*)"\s*,\s*',
        _menu,
        line
    )
    # menu with named constant or computed choice list:
    #   "menu", IDENTIFIER_or_expr, val
    # Route through menuFromCsv at runtime.  The expression can be any
    # sequence of tokens not containing an opening-comma at depth 0;
    # approximated here as a call/name/member-access without commas.
    # Conservative: match only simple `IDENT` or `IDENT(...)` / `IDENT.method()`
    # so we don't accidentally break something.
    def _menu_ident(m):
        counts['menu-ident'] = counts.get('menu-ident', 0) + 1
        return f'{Q}menuFromCsv({m.group(1)}), '
    line = re.sub(
        r'"menu"\s*,\s*([A-Za-z_]\w*(?:\s*->\s*\w+\(\s*\))?(?:\s*\.\s*\w+\(\s*\))?)\s*,\s*',
        _menu_ident,
        line
    )
    # range / range:slider with step: "range[:slider]", "[A,B]:S"
    # Separator tolerant: accept either comma or colon between min/max.
    # (Some legacy sites used ":" — likely copy-paste from the step suffix
    # grammar `[A,B]:S` — even though "," is the intended separator.)
    def _range_step_slider(m):
        counts['range:slider+step'] = counts.get('range:slider+step', 0) + 1
        mn, mx, st = normalize_range(m.group(1), m.group(2), m.group(3))
        return f'{Q}Range{{.min={mn}, .max={mx}, .step={st}}}, '
    line = re.sub(
        r'"range(?::slider)?"\s*,\s*"\[\s*([^,:\]]+?)\s*[,:]\s*([^,:\]]+?)\s*\]\s*:\s*([^"\]]+?)\s*"\s*,\s*',
        _range_step_slider,
        line
    )
    # range / range:slider without step: "[A,B]"
    def _range_slider(m):
        counts['range:slider'] = counts.get('range:slider', 0) + 1
        mn, mx, _ = normalize_range(m.group(1), m.group(2), None)
        return f'{Q}Range{{.min={mn}, .max={mx}}}, '
    line = re.sub(
        r'"range(?::slider)?"\s*,\s*"\[\s*([^,:\]]+?)\s*[,:]\s*([^,:\]]+?)\s*\]"\s*,\s*',
        _range_slider,
        line
    )
    # range:spinbox with step
    def _range_spin_step(m):
        counts['range:spinbox+step'] = counts.get('range:spinbox+step', 0) + 1
        mn, mx, st = normalize_range(m.group(1), m.group(2), m.group(3))
        return f'{Q}Range{{.min={mn}, .max={mx}, .step={st}, .ui={Q}UI::Spinbox}}, '
    line = re.sub(
        r'"range:spinbox"\s*,\s*"\[\s*([^,\]]+?)\s*,\s*([^,\]]+?)\s*\]\s*:\s*([^"\]]+?)\s*"\s*,\s*',
        _range_spin_step,
        line
    )
    # range:spinbox without step
    def _range_spin(m):
        counts['range:spinbox'] = counts.get('range:spinbox', 0) + 1
        mn, mx, _ = normalize_range(m.group(1), m.group(2), None)
        return f'{Q}Range{{.min={mn}, .max={mx}, .ui={Q}UI::Spinbox}}, '
    line = re.sub(
        r'"range:spinbox"\s*,\s*"\[\s*([^,\]]+?)\s*,\s*([^,\]]+?)\s*\]"\s*,\s*',
        _range_spin,
        line
    )
    # float: same shape as range, no step
    def _float(m):
        counts['float'] = counts.get('float', 0) + 1
        mn = num_literal(m.group(1), True)
        mx = num_literal(m.group(2), True)
        return f'{Q}Range{{.min={mn}, .max={mx}}}, '
    line = re.sub(
        r'"float"\s*,\s*"\[\s*([^,\]]+?)\s*,\s*([^,\]]+?)\s*\]"\s*,\s*',
        _float,
        line
    )
    # int with step: "int", "[A,B]:S"
    def _int_step(m):
        counts['int+step'] = counts.get('int+step', 0) + 1
        mn = m.group(1).strip()
        mx = m.group(2).strip()
        st = m.group(3).strip()
        return f'{Q}Range{{.min={mn}, .max={mx}, .step={st}, .ui={Q}UI::Spinbox}}, '
    line = re.sub(
        r'"int"\s*,\s*"\[\s*([^,:\]]+?)\s*,\s*([^,:\]]+?)\s*\]\s*:\s*([^"\]]+?)\s*"\s*,\s*',
        _int_step,
        line
    )
    # int: same shape as range:spinbox, no step
    def _int(m):
        counts['int'] = counts.get('int', 0) + 1
        mn = m.group(1).strip()
        mx = m.group(2).strip()
        return f'{Q}Range{{.min={mn}, .max={mx}, .ui={Q}UI::Spinbox}}, '
    line = re.sub(
        r'"int"\s*,\s*"\[\s*([^,\]]+?)\s*,\s*([^,\]]+?)\s*\]"\s*,\s*',
        _int,
        line
    )

    return line

def unquote_numeric_values(text: str) -> tuple[str, int]:
    """Unquote the 3rd-arg value when it's a numeric string literal.

    After the rewrite, many Range-typed calls still have the value passed
    as a quoted string literal (legacy: the old API took AutoParse<string>).
    If it's plainly numeric, strip the quotes so it implicitly converts
    to Range<T>::value_type.

    Only rewrites when the 2nd arg is a utils::prop::Range{...} (numeric
    ranges); other constraint kinds keep string values intact.
    """
    count = 0
    def repl(m):
        nonlocal count
        count += 1
        return f'{m.group(1)}{m.group(2)}'
    # utils::prop::Range{...}, "NUMERIC"   → ... , NUMERIC
    # Numeric regex: optional sign, digits, optional decimal, optional exponent, optional f
    new = re.sub(
        r'(utils::prop::Range\{[^}]*\}\s*,\s*)"(-?\d+(?:\.\d*)?(?:[eE][+-]?\d+)?f?)"',
        repl,
        text,
        flags=re.DOTALL,
    )
    return new, count

def strip_str_wrap(text: str) -> tuple[str, int]:
    """Strip `str(X)` wrappers around values of typed addProperty calls.

    After the type-and-info → constraint rewrite, the value arg is still
    wrapped in `str()` in many legacy sites (the old API needed a
    std::string, the new one wants the raw typed value).  We match the
    pattern `utils::prop::*{...}, str(` and drop `str(` + its matching
    `)`, preserving nested parens inside.

    Works across line breaks; the match anchors on the closing `}` of
    the constraint brace-init and permits whitespace (including \\n)
    before `str(`.
    """
    out = []
    i = 0
    count = 0
    # Anchor only on Range constraints (numeric value_type).  Stripping
    # str() around Info/Text/Menu values would be wrong — those have
    # value_type = std::string, and str(...) is doing a real conversion
    # (e.g. size_t → string).
    pat = re.compile(r'utils::prop::Range\{[^}]*\}\s*,\s*str\(', re.DOTALL)
    while i < len(text):
        m = pat.search(text, i)
        if not m:
            out.append(text[i:])
            break
        # Copy up to the `str(` itself
        str_open_end = m.end()           # position just past 'str('
        str_open_start = str_open_end - 4  # position of 's' of 'str('
        out.append(text[i:str_open_start])
        # Find matching `)` — counting nested parens inside str(...)
        depth = 1
        j = str_open_end
        while j < len(text) and depth > 0:
            c = text[j]
            if c == '(':
                depth += 1
            elif c == ')':
                depth -= 1
            j += 1
        if depth != 0:
            # Unbalanced — give up on this match; copy `str(` verbatim
            # and continue past it.
            out.append(text[str_open_start:str_open_end])
            i = str_open_end
            continue
        # j is now just past the matching `)`.
        inner = text[str_open_end:j-1]
        out.append(inner)
        count += 1
        i = j
    return ''.join(out), count

def ensure_include(text: str) -> str:
    if INCLUDE_LINE in text:
        return text
    lines = text.split('\n')
    # Insert after the FIRST unconditional #include, not the last —
    # later #includes may be inside #ifdef blocks, where placing our
    # unconditionally-needed include would make it conditional.
    for i, line in enumerate(lines):
        if re.match(r'\s*#\s*include\b', line):
            lines.insert(i + 1, INCLUDE_LINE)
            return '\n'.join(lines)
    return INCLUDE_LINE + '\n' + text

def needs_using(text: str) -> bool:
    """Detect if this rewrite introduced any utils::prop:: references."""
    return 'utils::prop::' in text

def process_file(path: Path):
    orig = path.read_text()
    counts: dict = {}
    new = orig
    # Split into lines so each match is line-local (most addProperty calls are
    # one-liners; multi-line calls won't match and will be flagged by the
    # compiler after the rewrite).
    new = '\n'.join(rewrite_line(line, counts) for line in new.split('\n'))
    new, stripped = strip_str_wrap(new)
    if stripped:
        counts['str()-stripped'] = stripped
    new, unquoted = unquote_numeric_values(new)
    if unquoted:
        counts['numeric-unquoted'] = unquoted
    if new != orig and needs_using(new):
        new = ensure_include(new)
    if new != orig:
        path.write_text(new)
    return counts

def main():
    if len(sys.argv) < 2:
        print(__doc__, file=sys.stderr)
        sys.exit(1)
    totals: dict = {}
    for arg in sys.argv[1:]:
        p = Path(arg)
        if not p.is_file():
            print(f'skip: {p} (not a file)', file=sys.stderr)
            continue
        counts = process_file(p)
        if counts:
            print(f'{p}: {dict(sorted(counts.items()))}')
            for k, v in counts.items():
                totals[k] = totals.get(k, 0) + v
    if totals:
        print(f'\nTotal: {dict(sorted(totals.items()))}')

if __name__ == '__main__':
    main()
