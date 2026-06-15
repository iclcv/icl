#!/usr/bin/env python3
# SPDX-License-Identifier: LGPL-3.0-or-later
# ICL - Image Component Library (https://github.com/iclcv/icl)
# Copyright (C) 2006-2026 Christof Elbrechter
#
# Stage-1 converter: legacy qt:: fluent GUI builder  ->  qt::ui:: designated-init.
#
#   Component(posargs).setterA(x).setterB(y)   ->   ui::Component(posargs, {.A=x,.B=y})
#
# Conservative by design: only rewrites chains it fully understands (balanced
# parens / strings, known component, known setters, expected positional arity).
# Everything else is left untouched and printed to a skip-report so it can be
# hand-migrated.  Run with --apply to write files; default is a dry run.
#
# Usage:
#   scripts/ui-migrate.py [--apply] [--quiet] FILE [FILE ...]

import sys, re, os

# --------------------------------------------------------------------------
# Component specs.
#
#   pos        : number of leading ctor args kept POSITIONAL.  An int means
#                "exactly N"; (a, b) means "between a and b" (optional tail
#                positionals, e.g. Label text / Canvas viewport / Fps window).
#   trailing   : ordered Opts field names for ctor args BEYOND `pos` (in the
#                order they appear in the legacy ctor).
#   opt_order  : component-specific Opts fields in struct-declaration order
#                (drives designator emission order to avoid -Wreorder-init-list).
#   min_args   : minimum ctor-arg count to accept; fewer  -> skip (catches the
#                Range32f / default-arg overloads we don't convert).
# --------------------------------------------------------------------------
COMMON_ORDER = ["handle", "label", "tooltip", "size", "minSize", "maxSize", "hide"]
BOX_ORDER    = ["margin", "spacing"] + COMMON_ORDER

LEAF = {
    "Slider":      dict(pos=3, trailing=["vertical", "step"], opt_order=["vertical", "step"], min_args=3),
    "FSlider":     dict(pos=3, trailing=["vertical"],         opt_order=["vertical"],         min_args=3),
    "Int":         dict(pos=3, trailing=[],                   opt_order=[],                   min_args=3),
    "Float":       dict(pos=3, trailing=[],                   opt_order=[],                   min_args=3),
    "Spinner":     dict(pos=3, trailing=[],                   opt_order=[],                   min_args=3),
    "String":      dict(pos=1, trailing=["maxLen"],           opt_order=["maxLen"],           min_args=1),
    "Label":       dict(pos=(0, 1), trailing=[],              opt_order=[],                   min_args=0),
    "Combo":       dict(pos=1, trailing=["initialIndex"],     opt_order=["initialIndex"],     min_args=1),
    "CheckBox":    dict(pos=1, trailing=["checked"],          opt_order=["checked"],          min_args=1),
    "ColorSelect": dict(pos=3, trailing=["alpha"],            opt_order=["alpha"],            min_args=3),
    "ButtonGroup": dict(pos=1, trailing=[],                   opt_order=[],                   min_args=1),
    "Disp":        dict(pos=2, trailing=[],                   opt_order=[],                   min_args=2),
    "Display":     dict(pos=0, trailing=[],                   opt_order=[],                   min_args=0),
    "Canvas":      dict(pos=(0, 1), trailing=[],              opt_order=[],                   min_args=0),
    "Canvas3D":    dict(pos=(0, 1), trailing=[],              opt_order=[],                   min_args=0),
    "Fps":         dict(pos=(0, 1), trailing=[],              opt_order=[],                   min_args=0),
    "Ps":          dict(pos=(0, 1), trailing=[],              opt_order=[],                   min_args=0),
    "State":       dict(pos=(0, 1), trailing=[],              opt_order=[],                   min_args=0),
    "Prop":        dict(pos=1, trailing=[],                   opt_order=[],                   min_args=1),
    "Dummy":       dict(pos=0, trailing=[],                   opt_order=[],                   min_args=0),
    "Show":        dict(pos=0, trailing=[],                   opt_order=[],                   min_args=0),
    "Create":      dict(pos=0, trailing=[],                   opt_order=[],                   min_args=0),
    # CamCfg + Button are special-cased below.
    "CamCfg":      dict(pos=0, trailing=["deviceType", "deviceID"], opt_order=["deviceType", "deviceID"], min_args=0),
}

BOX_CONTAINERS = {"HBox", "VBox", "HScroll", "VScroll", "HSplit", "VSplit"}
# Tab(csv) keeps one positional; the rest share BoxOpts.
CONTAINERS = BOX_CONTAINERS | {"Tab"}

# Components we deliberately do NOT convert (left for manual follow-up).
SKIP_COMPONENTS = {"Plot", "Border"}

ALL_NAMES = set(LEAF) | CONTAINERS | {"Button"} | SKIP_COMPONENTS

# Setter -> (opts_field, kind).  kind drives value formatting.
SETTERS = {
    "handle":  ("handle",  "scalar"),
    "label":   ("label",   "scalar"),
    "tooltip": ("tooltip", "scalar"),
    "size":    ("size",    "size"),
    "minSize": ("minSize", "size"),
    "maxSize": ("maxSize", "size"),
    "hideIf":  ("hide",    "scalar"),
    "margin":  ("margin",  "scalar"),
    "spacing": ("spacing", "scalar"),
}


class ParseError(Exception):
    pass


def split_args(s):
    """Split a ctor/setter arg string on TOP-LEVEL commas, honouring
    (), [], {} nesting and string / char literals.  Returns the list of
    trimmed arg strings ([] for empty)."""
    args, buf, depth = [], [], 0
    i, n = 0, len(s)
    while i < n:
        c = s[i]
        if c in '"\'':
            buf.append(c)
            i += 1
            while i < n:
                buf.append(s[i])
                if s[i] == '\\':
                    if i + 1 < n:
                        buf.append(s[i + 1]); i += 2; continue
                elif s[i] == c:
                    i += 1; break
                i += 1
            continue
        if c in "([{":
            depth += 1
        elif c in ")]}":
            depth -= 1
        if c == ',' and depth == 0:
            args.append(''.join(buf).strip()); buf = []
        else:
            buf.append(c)
        i += 1
    tail = ''.join(buf).strip()
    if tail or args:
        args.append(tail)
    return [a for a in args] if (args and not (len(args) == 1 and args[0] == '')) else []


def scan_balanced(text, open_idx):
    """text[open_idx] is '('.  Return (inner, end_idx) where inner is the
    content between the matched parens and end_idx is the index just past
    the closing ')'.  Honours strings/char literals."""
    assert text[open_idx] == '('
    depth, i, n = 0, open_idx, len(text)
    while i < n:
        c = text[i]
        if c in '"\'':
            i += 1
            while i < n:
                if text[i] == '\\':
                    i += 2; continue
                if text[i] == c:
                    break
                i += 1
            i += 1
            continue
        if c == '(':
            depth += 1
        elif c == ')':
            depth -= 1
            if depth == 0:
                return text[open_idx + 1:i], i + 1
        i += 1
    raise ParseError("unbalanced parens")


def is_ident_char(c):
    return c.isalnum() or c == '_'


def fmt_size(args):
    if len(args) == 2:
        return "{%s, %s}" % (args[0], args[1])
    if len(args) == 1:
        return args[0]
    raise ParseError("size setter arity")


def build_call(name, pos_args, fields):
    """Render a leaf/Button ui:: call.  Containers go through
    build_call_container (BoxOpts order)."""
    order = LEAF.get(name, {}).get("opt_order", []) + COMMON_ORDER
    desigs = []
    for f in order:
        if f in fields:
            desigs.append(".%s=%s" % (f, fields[f]))
    for f in fields:                       # safety net
        if f not in order:
            desigs.append(".%s=%s" % (f, fields[f]))
    pos = ", ".join(pos_args)
    if desigs:
        inner = "{%s}" % ", ".join(desigs)
        return "ui::%s(%s)" % (name, ", ".join([p for p in [pos] if p] + [inner]))
    return "ui::%s(%s)" % (name, pos)


def convert_chain(text, name_start, name, skips, lineno):
    """Try to convert one chain beginning at name_start (index of the
    component name).  Returns (replacement, end_idx) or None (skip)."""
    paren = name_start + len(name)
    while paren < len(text) and text[paren] in ' \t\n':
        paren += 1
    if paren >= len(text) or text[paren] != '(':
        return None
    try:
        ctor_inner, idx = scan_balanced(text, paren)
    except ParseError as e:
        skips.append((lineno, name, str(e)))
        return None
    ctor_args = split_args(ctor_inner)

    # collect trailing .setter(...) calls
    setters = []
    j = idx
    while True:
        k = j
        while k < len(text) and text[k] in ' \t\n':
            k += 1
        if k >= len(text) or text[k] != '.':
            break
        # allow the access dot to trail the previous line: `foo.\n  bar(`
        m = re.match(r'\.\s*([A-Za-z_]\w*)\s*\(', text[k:])
        if not m:
            break
        sname = m.group(1)
        sopen = k + m.end() - 1
        try:
            sinner, send = scan_balanced(text, sopen)
        except ParseError as e:
            skips.append((lineno, name, "setter parse: " + str(e)))
            return None
        setters.append((sname, split_args(sinner)))
        j = send

    end_idx = j
    if name in SKIP_COMPONENTS:
        skips.append((lineno, name, "component not supported in ui:: (manual)"))
        return None

    # validate + map setters to common/box fields
    fields = {}
    is_container = name in CONTAINERS
    for sname, sargs in setters:
        if sname not in SETTERS:
            skips.append((lineno, name, "unknown setter .%s()" % sname))
            return None
        field, kind = SETTERS[sname]
        if field in ("margin", "spacing") and not is_container:
            skips.append((lineno, name, ".%s() on non-container" % sname))
            return None
        if kind == "size":
            try:
                fields[field] = fmt_size(sargs)
            except ParseError:
                skips.append((lineno, name, "%s() arity" % sname))
                return None
        else:
            if len(sargs) != 1:
                skips.append((lineno, name, "%s() arity" % sname))
                return None
            fields[field] = sargs[0]

    # ---- component-specific positional / trailing handling ----
    if name == "Button":
        repl = handle_button(ctor_args, fields, skips, lineno)
        if repl is None:
            return None
        return repl, end_idx

    if name == "CamCfg":
        # all ctor args are device hints; drop empty-string literals
        for field, val in zip(["deviceType", "deviceID"], ctor_args):
            if val.strip() not in ('""', "''", ""):
                fields[field] = val
        return build_call("CamCfg", [], fields), end_idx

    if is_container:
        if name == "Tab":
            if len(ctor_args) != 1:
                skips.append((lineno, name, "Tab expects 1 positional"))
                return None
            return build_call_container("Tab", [ctor_args[0]], fields), end_idx
        if ctor_args:
            skips.append((lineno, name, "container with unexpected ctor args"))
            return None
        return build_call_container(name, [], fields), end_idx

    spec = LEAF[name]
    if len(ctor_args) < spec["min_args"]:
        skips.append((lineno, name, "only %d ctor args (< %d; likely Range/default overload)"
                      % (len(ctor_args), spec["min_args"])))
        return None

    pos = spec["pos"]
    pos_max = pos[1] if isinstance(pos, tuple) else pos
    pos_min = pos[0] if isinstance(pos, tuple) else pos
    n_pos = min(len(ctor_args), pos_max)
    positional = ctor_args[:n_pos]
    trailing = ctor_args[n_pos:]
    if len(trailing) > len(spec["trailing"]):
        skips.append((lineno, name, "more ctor args than known trailing opts"))
        return None
    for field, val in zip(spec["trailing"], trailing):
        fields[field] = val
    return build_call(name, positional, fields), end_idx


def build_call_container(name, pos_args, fields):
    order = BOX_ORDER
    desigs = [".%s=%s" % (f, fields[f]) for f in order if f in fields]
    pos = ", ".join(pos_args)
    if desigs:
        inner = "{%s}" % ", ".join(desigs)
        return "ui::%s(%s)" % (name, ", ".join([p for p in [pos] if p] + [inner]))
    return "ui::%s(%s)" % (name, pos)


def handle_button(ctor_args, fields, skips, lineno):
    if len(ctor_args) == 0:
        skips.append((lineno, "Button", "no text arg"))
        return None
    text = ctor_args[0]
    toggled = ctor_args[1] if len(ctor_args) >= 2 else None
    init = ctor_args[2] if len(ctor_args) >= 3 else None

    # push button: no toggledText, or explicitly empty
    if toggled is None or toggled.strip() in ('""', "''"):
        return build_call("Button", [text], fields)

    # toggle -> ui::ToggleButton(text, toggled, init, {opts})
    initiallyToggled = "false"
    if init is not None:
        initiallyToggled = init.strip()
    # legacy "!"-prefix on a string-literal toggledText means initially toggled
    tg = toggled.strip()
    if len(tg) >= 2 and tg[0] in '"' and tg[1] == '!':
        tg = tg[0] + tg[2:]          # strip the '!'
        initiallyToggled = "true"
    pos = [text, tg, initiallyToggled]
    desigs = [".%s=%s" % (f, fields[f]) for f in COMMON_ORDER if f in fields]
    if desigs:
        return "ui::ToggleButton(%s, {%s})" % (", ".join(pos), ", ".join(desigs))
    return "ui::ToggleButton(%s)" % ", ".join(pos)


def find_chains(text):
    """Yield (start_index, name) for each component-name token that looks
    like the head of a builder chain (preceded by '<<' or '(' modulo
    whitespace, followed by '(' modulo whitespace, not already ui::/qt::
    qualified or a member access)."""
    for m in re.finditer(r'\b(' + '|'.join(sorted(ALL_NAMES, key=len, reverse=True)) + r')\b', text):
        name = m.group(1)
        s = m.start()
        # not qualified (ui::Name / qt::Name) or member (.Name) or part of ident
        p = s - 1
        while p >= 0 and text[p] in ' \t\n':
            p -= 1
        if p >= 0:
            if text[p] == ':' or text[p] == '.':
                continue
            if is_ident_char(text[p]):
                continue
            # require the chain to start a stream insertion or container nesting
            if text[p] not in '<(':
                continue
        # followed by '(' modulo whitespace
        e = m.end()
        while e < len(text) and text[e] in ' \t\n':
            e += 1
        if e >= len(text) or text[e] != '(':
            continue
        yield s, name


def lineno_at(text, idx):
    return text.count('\n', 0, idx) + 1


def ensure_include(text):
    if 'icl/qt/ui.h' in text:
        return text
    lines = text.split('\n')
    last_qt = None
    for i, ln in enumerate(lines):
        if re.match(r'\s*#\s*include\s*<icl/qt/.*>', ln):
            last_qt = i
    inc = '#include <icl/qt/ui.h>'
    if last_qt is not None:
        lines.insert(last_qt + 1, inc)
    else:
        # after the first #include, else at top
        for i, ln in enumerate(lines):
            if re.match(r'\s*#\s*include', ln):
                lines.insert(i + 1, inc)
                break
        else:
            lines.insert(0, inc)
    return '\n'.join(lines)


def convert_file(path):
    with open(path) as f:
        text = orig = f.read()

    repls = []            # (start, end, replacement)
    skips = []
    for s, name in find_chains(text):
        ln = lineno_at(text, s)
        res = convert_chain(text, s, name, skips, ln)
        if res is None:
            continue
        replacement, end = res
        repls.append((s, end, replacement))

    if not repls:
        return 0, skips, orig, orig

    # apply right-to-left so indices stay valid
    repls.sort(key=lambda r: r[0], reverse=True)
    out = text
    # guard against overlaps (nested matches): keep the outermost-first by
    # dropping any repl whose span intersects an already-applied one.
    applied_spans = []
    kept = []
    for s, e, r in sorted(repls, key=lambda r: (r[0], -r[1])):
        if any(not (e <= a or s >= b) for a, b in applied_spans):
            continue
        applied_spans.append((s, e))
        kept.append((s, e, r))
    for s, e, r in sorted(kept, key=lambda r: r[0], reverse=True):
        out = out[:s] + r + out[e:]

    out = ensure_include(out)
    return len(kept), skips, orig, out


def main(argv):
    apply = "--apply" in argv
    quiet = "--quiet" in argv
    files = [a for a in argv if not a.startswith("--")]
    if not files:
        print("usage: ui-migrate.py [--apply] [--quiet] FILE ...", file=sys.stderr)
        return 2

    total_conv, total_skip = 0, 0
    for path in files:
        n, skips, orig, out = convert_file(path)
        total_conv += n
        total_skip += len(skips)
        if n and apply and out != orig:
            with open(path, "w") as f:
                f.write(out)
        tag = "WROTE" if (apply and n) else "would convert"
        if n or skips:
            print("%-7s %2d chains  %2d skipped  %s" % (tag if n else "—", n, len(skips), path))
        if skips and not quiet:
            for ln, name, why in skips:
                print("    skip %s:%d  %s  (%s)" % (os.path.basename(path), ln, name, why))
    print("---\n%d chains converted, %d sites skipped, across %d file(s)"
          % (total_conv, total_skip, len(files)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
