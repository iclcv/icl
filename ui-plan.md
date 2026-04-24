# qt::ui:: — Designated-Init GUI Component Syntax

Bolt on a modern, designated-init friendly shape for the GUI builder
while keeping the legacy stream-insertion builder unchanged.  Call
sites opt in when they want; no flag days.

## Target shape

```cpp
gui << ui::Slider(0, 255, 42, {.vertical=true, .step=2,
                               .handle="gain", .label="Gain"})
    << ui::Button("OK", {.handle="ok"})
    << ui::Label("Status", {.label="Info"})
    << Show();
```

Primary "obvious" data args are **positional** (required, conventionally
ordered — `min/max/val` for a slider, text for a button, ...).  The
trailing `XxxOpts{}` pack carries **everything else** via C++20
designated init: component-specific tuning (`vertical`, `step`,
`toggledText`, ...) plus the shared UI metadata (`handle`, `label`,
`tooltip`, `size`, `minSize`, `maxSize`, `hide`).

One flat `Opts` per component means call sites never need
nested-designator syntax.  Fields can be omitted entirely when they
don't apply (e.g. containers have no `tooltip`).

## Design decisions (locked in by the Slider spike)

1. **Per-component Opts**, hoisted to namespace scope (`ui::SliderOpts`,
   not `ui::Slider::Opts`).  C++ forbids a nested type's default member
   initializers being used in the enclosing class's own default
   argument — `Slider(..., SliderOpts={})` only works with Opts at
   namespace scope.

2. **Shared-metadata block duplicated** across every `XxxOpts`.  The
   block is 8 lines:
   ```cpp
   std::string handle;
   std::string label;
   std::string tooltip;
   utils::Size size{};
   utils::Size minSize{};
   utils::Size maxSize{};
   bool        hide = false;
   ```
   Rejected alternatives:
   - Inheritance (`XxxOpts : CommonOpts`) — wrecks flat designated-init
     (caller would have to spell `{.CommonOpts={.handle=...}, .step=2}`).
   - Macro for the 8-line block — works, mild preference to avoid.  Can
     re-introduce later if we find the repetition actually harmful.

3. **`applyCommon` uses `if constexpr(requires{o.field;})`** so each
   Opts struct can include or omit shared-metadata fields independently
   of which other fields it has.  Containers can drop `tooltip`,
   scrollable containers can add `margin`/`spacing`, etc.

4. **Legacy `qt::Slider(...)` / `qt::Button(...)` / ... unchanged.**
   `ui::Xxx` builds on top of `qt::Xxx` via `toComponent()` — no
   duplication of wire-format knowledge.  Both shapes route through
   the same `XxxGUIWidget` factory.

5. **Free `operator<<` in `icl::qt`** guarded on a `ui::Component`
   concept (requires `toComponent()`).  lvalue + rvalue overloads,
   rvalue mirrors `ContainerGUIComponent::operator<<(const
   GUIComponent&) const` so `HBox() << ui::Slider(...)` chains
   starting from a temporary.

## Component inventory (33 total — 32 registered widgets + Dummy)

Grouped by shape of the positional args, since that drives the ctor
signature.  "Primary args" names match the legacy constructor's
parameters.

### Numeric inputs — (min, max, val, Opts)

| Component | Positional     | Opts fields                                | Legacy widget     |
|-----------|----------------|--------------------------------------------|-------------------|
| Slider    | int min,max,val | `.vertical`, `.step` + common               | SliderGUIWidget   |
| FSlider   | float min,max,val | `.vertical` + common                      | FloatSliderGUIWidget |
| Int       | int min,max,val | common                                     | IntGUIWidget      |
| Float     | float min,max,val | common                                    | FloatGUIWidget    |
| Spinner   | int min,max,val | common                                     | SpinnerGUIWidget  |

### Text inputs — (text, Opts)

| Component | Positional     | Opts fields                   | Legacy widget     |
|-----------|----------------|-------------------------------|-------------------|
| String    | string text    | `.maxLen` + common            | StringGUIWidget   |
| Label     | string text=""| common                        | LabelGUIWidget    |
| State     | —              | `.maxLines` + common          | StateGUIWidget    |

### Buttons & toggles — (text, Opts)

| Component   | Positional   | Opts fields                                         | Legacy widget            |
|-------------|--------------|-----------------------------------------------------|--------------------------|
| Button      | string text  | `.toggledText`, `.initiallyToggled` + common        | ButtonGUIWidget / Toggle |
| CheckBox    | string label | `.checked` + common                                 | CheckBoxGUIWidget        |
| ButtonGroup | string csv   | common                                              | ButtonGroupGUIWidget     |
| Combo       | string csv   | `.initialIndex` + common                            | ComboGUIWidget           |

### Display / canvases — (Opts or one positional)

| Component  | Positional          | Opts fields                                        | Legacy widget         |
|------------|---------------------|----------------------------------------------------|------------------------|
| Display    | —                   | common                                             | ImageGUIWidget         |
| Canvas     | —                   | `.viewport={800,600}` + common                     | DrawGUIWidget          |
| Canvas3D   | —                   | `.viewport={800,600}` + common                     | DrawGUIWidget3D        |
| Disp       | int nx, ny          | common                                             | DispGUIWidget          |
| Plot       | Range xr, yr        | `.openGL`, `.xLabel`, `.yLabel` + common           | PlotGUIWidget          |
| Fps        | —                   | `.timeWindow=10` + common                          | FPSGUIWidget           |
| ColorSelect| int r, g, b         | `.alpha` + common                                  | ColorGUIWidget         |

### Introspection — (target, Opts)

| Component | Positional                | Opts fields        | Legacy widget            |
|-----------|---------------------------|--------------------|--------------------------|
| Prop      | Configurable* OR string id | common             | ConfigurableGUIWidget    |
| CamCfg    | —                         | `.deviceType`, `.deviceID` + common | CamCfgGUIWidget |
| Ps        | —                         | `.updateFPS=10` + common            | ProcessMonitorGUIWidget |

### Containers — (Opts with layout tuning)

| Component | Positional | Opts fields                            | Legacy widget           |
|-----------|------------|----------------------------------------|-------------------------|
| VBox      | —          | `.margin`, `.spacing` + common         | VBoxGUIWidget           |
| HBox      | —          | `.margin`, `.spacing` + common         | HBoxGUIWidget           |
| VScroll   | —          | `.margin`, `.spacing` + common         | VScrollGUIWidget        |
| HScroll   | —          | `.margin`, `.spacing` + common         | HScrollGUIWidget        |
| VSplit    | —          | `.margin`, `.spacing` + common         | VSplitterGUIWidget      |
| HSplit    | —          | `.margin`, `.spacing` + common         | HSplitterGUIWidget      |
| Tab       | string csv | `.margin`, `.spacing` + common         | TabGUIWidget            |
| Border    | string label | `.margin`, `.spacing` + common       | BorderGUIWidget         |

Containers need a child-streaming API (the `<< ui::XXX` chain inside).
Two approaches:
- (i) make `ui::HBox` itself a `GUI`-derived container so existing
      `operator<<` chains work naturally.
- (ii) model it as a `ui::Xxx` value that produces a `ContainerGUIComponent`
      on `toComponent()`, and provide an overload for
      `operator<<(ContainerGUIComponent &&, const ui::Component &)`.

Lean toward (ii) — keeps `ui::` values plain structs.  Decide in Phase 2.

### Finalizers / special — (no args, often)

| Component | Opts | Notes                                           |
|-----------|------|-------------------------------------------------|
| Show      | —    | finalize + display                              |
| Create    | —    | finalize + stay hidden                          |
| Dummy     | —    | no-op placeholder, respects `.hide`             |

Trivial wrappers.  Could also be free functions returning a marker
struct; handy symmetry to keep them in `ui::`.

## Phases

### Phase 1 — Spike — ✅ LANDED (commit `103e9316f`, session 59)

- `ui.h` header + `applyCommon` + `Component` concept + lvalue/rvalue
  `operator<<`.
- First worked component: `ui::Slider` + `ui::SliderOpts`.
- `ui-syntax-demo` showing legacy + ui:: side-by-side in an HBox.
- meson wiring.

### Phase 2 — Core numeric + text inputs + buttons — ✅ LANDED (session 59)

12 components landed in one commit (Slider was the spike):

- **Numeric:** `FSlider`, `Int`, `Float`, `Spinner`
- **Text:** `String`, `Label`, `State`
- **Buttons / selections:** `Button`, `CheckBox`, `ButtonGroup`, `Combo`

Resolved open question: `ui::Button` toggle semantics — empty
`.toggledText` → push button; non-empty → toggle.  Matches legacy.

`ui::Label` case that motivated the mixed-syntax decision: positional
`text` is the displayed content, `.label` field inside LabelOpts is
the separate border label.  Disambiguation works cleanly.

Demo `ui-syntax-demo` expanded to exercise all 12 components in a
VBox-of-HBoxes layout.  871/871 green.

### Phase 3 — Display / canvas / introspection — ✅ LANDED (session 59)

10 components landed (State was already covered in Phase 2):

- **Display / canvases:** `Display`, `Canvas`, `Canvas3D`, `Disp`, `Plot`
- **Monitors:** `Fps`, `Ps`
- **Introspection:** `ColorSelect`, `CamCfg`, `Prop`

`Prop` preserved the dual-ctor shape (live `Configurable*` / `&` vs
string ID).  The legacy pointer-encoding trick in GUIComponents.h:309
(`encode_pointer` → `"@pointer@:" + binary`) lives untouched inside
`qt::Prop`; `ui::Prop` just forwards.

`Plot` adopted the four-float (`.minX/.maxX/.minY/.maxY`) form rather
than `Range32f` wrappers — flat designated-init is more readable, and
the zero-default matches legacy's "derive range from data" behavior.

Demo picked up a fourth HBox exercising `Display`, `Disp`, `Fps`,
`ColorSelect`.  871/871 green.

### Phase 4 — Containers — ✅ LANDED (session 59)

7 containers landed (Border dropped — its qt:: ctor is friend-only,
and any container's `.label` opts field produces an equivalent titled
border):

- `HBox`, `VBox`, `HScroll`, `VScroll`, `HSplit`, `VSplit`, `Tab`.

Approach picked: **inherit from the legacy `qt::` container**, apply
a `BoxOpts{}` pack in the ctor body via the existing `.margin()` /
`.spacing()` / `.label()` / ... setters.  Rationale captured in-header:
containers are accumulators, not values; inheriting means
`<<`-chaining of children keeps working through
`ContainerGUIComponent::operator<<` for free, and top-level
`gui << ui::HBox({...})` routes through the existing
`GUI::operator<<(const GUI&)` overload — no new dispatch needed.
Leaf children inside a container are picked up by the free
`operator<<(GUI&&, ui::Component)` template established in Phase 1.

All 6 box-style containers share a single `BoxOpts` struct (margin,
spacing + common metadata) — unlike leaves, where per-component Opts
was needed to accommodate per-component tuning (`.vertical`, `.step`,
etc.).  Tab uses the same BoxOpts since its only tuning is the
positional CSV.

### Phase 5 — Finalizers — ✅ LANDED (session 59)

`ui::Show` / `ui::Create` / `ui::Dummy` — trivial wrappers around
`qt::Show()` / `qt::Create()` / `qt::Dummy()`.  Routed through the
existing `ui::Component` concept since they expose `toComponent()`.

### Phase 6 — Docs + migration exemplar

- Update the ICL manual's GUI chapter with the new syntax.
- Port *one* demo end-to-end from legacy to `ui::` as an exemplar.
- Leave the 155 existing call sites on legacy — migration is opt-in.

### Phase 7 — (optional, much later) String round-trip retirement

Not part of this plan.  Separate arc per TODO.md "Rework GUIComponent
internal representation".  Once that lands, `ui::Xxx` becomes the
storage type and `qt::Xxx` / `toString()` / `GUIDefinition` parsing
fall away.

## Open questions deferred

- **Range variants.**  Legacy `Slider(Range32f, ...)` takes a range
  object.  Do we add an `ui::Slider(utils::Range32f, int val, SliderOpts)`
  overload, or do users spell `.min=r.minVal, .max=r.maxVal`?  Vote:
  skip the overload — the primary form is clear enough, and adding
  overloads defeats the "one shape per component" simplicity.

- **Finalizers as values vs statements.**  `gui << ui::Show()` with an
  empty struct works but looks odd.  Could alternatively be
  `gui.show()` / `gui.create()` methods.  Punt to Phase 5.

- **Migration ergonomics for nested containers.**  Phase 4 issue.  The
  existing `gui << (VBox() << a << b)` pattern must keep working with
  all `ui::` children.  Overload plan TBD until we write it.

## Checkboxes

- [x] Phase 1 — Slider spike
- [x] Phase 2 — numeric + text + buttons (13 components)
- [x] Phase 3 — display + canvas + introspection (11 components)
- [x] Phase 4 — containers + child-streaming (7 containers; Border dropped)
- [x] Phase 5 — finalizers (3 components)
- [ ] Phase 6 — docs + exemplar demo migration
- [ ] Phase 7 — (separate arc) string round-trip retirement
