// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/qt/ContainerGUIComponents.h>
#include <icl/qt/GUI.h>
#include <icl/qt/GUIComponent.h>
#include <icl/qt/GUIComponents.h>

#include <concepts>
#include <string>
#include <utility>

/// The public, positional + designated-init GUI component syntax.
///
/// Usage:
/// \code
///   gui << Slider(0, 255, 42, {.vertical=true, .step=2,
///                              .handle="gain", .label="Gain"});
/// \endcode
///
/// Shape: primary "obvious" data args (min/max/val for a slider, text
/// for a button, ...) are positional — required, conventionally ordered.
/// The trailing `Opts{}` pack holds everything else: component-specific
/// tuning (`vertical`, `step`, ...) plus shared UI metadata (`handle`,
/// `label`, `tooltip`, `size/minSize/maxSize`, `hide`).  One flat Opts
/// per component means call sites never need nested-designator syntax.
///
/// Each component's `toComponent()` builds a `GUIComponent` through the
/// legacy `detail::Xxx` factory (which only encodes the wire type + params);
/// `GUI` then builds the widget straight from that GUIComponent without a
/// string round-trip (ui-plan Phase 7A).  These were once the `qt::ui::`
/// structs — Phase 7B promoted them into `icl::qt` and retired the qualifier.
namespace icl::qt {

  /// Apply the shared metadata fields of an Opts-like struct to a
  /// legacy GUIComponent.
  /** Uses `if constexpr(requires{...})` so per-component Opts can
      omit fields that don't apply (e.g. containers have no `tooltip`).
      GUIComponent's chained setters are const-qualified and mutate
      through `mutable Options`, so the calls below discard their
      return values intentionally — the side effect is what matters. */
  template<class T>
  GUIComponent applyCommon(GUIComponent c, const T &o){
    if constexpr(requires{ o.handle; })  if(!o.handle.empty())  c.handle(o.handle);
    if constexpr(requires{ o.label; })   if(!o.label.empty())   c.label(o.label);
    if constexpr(requires{ o.tooltip; }) if(!o.tooltip.empty()) c.tooltip(o.tooltip);
    if constexpr(requires{ o.size; })    if(o.size    != utils::Size::null) c.size(o.size);
    if constexpr(requires{ o.minSize; }) if(o.minSize != utils::Size::null) c.minSize(o.minSize);
    if constexpr(requires{ o.maxSize; }) if(o.maxSize != utils::Size::null) c.maxSize(o.maxSize);
    if constexpr(requires{ o.hide; })    if(o.hide)             c.hideIf(true);
    return c;
  }

  /// Concept: types exposing `toComponent()` so the generic stream
  /// operators below can route them into a GUI.
  template<class T>
  concept Component = requires(const T &t){
    { t.toComponent() } -> std::convertible_to<GUIComponent>;
  };

  /// Options pack for Slider.
  /** Hoisted out of `Slider` because C++ forbids using a nested type's
      default member initializers as part of the enclosing class's own
      default argument (the enclosing class isn't complete yet at that
      point).  Named `SliderOpts` rather than `Slider::Opts` so the
      default argument `Slider::Slider(..., SliderOpts={})` is
      well-formed. */
  struct SliderOpts {
    // Slider-specific tuning.
    bool vertical = false;
    int  step     = 1;
    // Shared metadata (applyCommon picks these up via `if constexpr(requires)`).
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Integer slider.
  /**
      \code
      gui << Slider(0, 255, 42, {.handle="gain"});
      gui << Slider(0, 100, 50, {.vertical=true, .step=2, .label="Coarse"});
      \endcode
  */
  struct Slider : public GUIComponentT<Slider> {
    int        min;
    int        max;
    int        val;
    SliderOpts opts;

    Slider(int min, int max, int val, SliderOpts o = {})
      : GUIComponentT("slider"), min(min), max(max), val(val), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  // --- Phase 2 components -------------------------------------------------
  //
  // Numeric + text inputs + buttons.  Each component follows the spike
  // pattern: `XxxOpts` at namespace scope with component-specific tuning
  // up top + the shared 7-field metadata block; the primary struct has
  // positional data args + a trailing `XxxOpts opts = {}`.
  //
  // The 7-line shared-metadata block is duplicated verbatim across every
  // Opts (decision (a) from ui-plan.md Phase 1).  `applyCommon` picks
  // the fields up via `if constexpr(requires{...})`, so Opts that add
  // component-specific fields above the block work with no extra wiring.

  /// Options for FSlider.
  struct FSliderOpts {
    bool vertical = false;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Float-valued slider.
  struct FSlider : public GUIComponentT<FSlider> {
    float       min;
    float       max;
    float       val;
    FSliderOpts opts;

    FSlider(float min, float max, float val, FSliderOpts o = {})
      : GUIComponentT("fslider"), min(min), max(max), val(val), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for Int.
  struct IntOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Integer text input (spinbox-like).
  struct Int : public GUIComponentT<Int> {
    int     min;
    int     max;
    int     val;
    IntOpts opts;

    Int(int min, int max, int val, IntOpts o = {})
      : GUIComponentT("int"), min(min), max(max), val(val), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for Float.
  struct FloatOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Float text input.
  struct Float : public GUIComponentT<Float> {
    float     min;
    float     max;
    float     val;
    FloatOpts opts;

    Float(float min, float max, float val, FloatOpts o = {})
      : GUIComponentT("float"), min(min), max(max), val(val), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for Spinner.
  struct SpinnerOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Integer spinbox.
  struct Spinner : public GUIComponentT<Spinner> {
    int         min;
    int         max;
    int         val;
    SpinnerOpts opts;

    Spinner(int min, int max, int val, SpinnerOpts o = {})
      : GUIComponentT("spinner"), min(min), max(max), val(val), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for String.
  struct StringOpts {
    int         maxLen = 100;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Single-line text input.
  struct String : public GUIComponentT<String> {
    std::string text;
    StringOpts  opts;

    String(std::string text, StringOpts o = {})
      : GUIComponentT("string"), text(std::move(text)), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for Label.
  struct LabelOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Read-only text label (for displaying status, values, etc.).
  /** The positional `text` is the label's displayed content.  The
      `.label` field inside LabelOpts is the separate border label —
      the two are deliberately distinct despite the name clash.  This
      is why the mixed syntax works better than a fully-aggregate form
      would have (where `Label{.label="x"}` would be ambiguous). */
  struct Label : public GUIComponentT<Label> {
    std::string text;
    LabelOpts   opts;

    /// opts-only ctor (text defaults to empty) — needed so
    /// `Label({.handle="x"})` resolves; the positional ctor has no
    /// default on `text`, else the two would be ambiguous for Label().
    Label(LabelOpts o = {}) : GUIComponentT("label"), text(), opts(std::move(o)) { setOptions(opts); }
    Label(std::string text, LabelOpts o = {})
      : GUIComponentT("label"), text(std::move(text)), opts(std::move(o)) { setOptions(opts); }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for State (maxLines is a primary positional arg, not
  /// an Opts field).
  struct StateOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Scrolling log-style state panel.
  /** `maxLines` (the scrollback line count) is the conventional primary
      arg, so it stays positional: `State(50)` rather than
      `State({.maxLines=50})`.  The opts-only ctor keeps the
      default. */
  struct State : public GUIComponentT<State> {
    int       maxLines;
    StateOpts opts;

    State(StateOpts o = {}) : GUIComponentT("state"), maxLines(100), opts(std::move(o)) { setOptions(opts); }
    State(int maxLines, StateOpts o = {})
      : GUIComponentT("state"), maxLines(maxLines), opts(std::move(o)) { setOptions(opts); }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for Button.
  struct ButtonOpts {
    /// Non-empty → toggle button that alternates between `text` and
    /// this string.  Empty → plain push button.
    std::string toggledText;
    bool        initiallyToggled = false;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Push or toggle button.
  /**
      \code
      gui << Button("Run", {.handle="go"});
      gui << Button("Play", {.toggledText="Pause", .handle="pp"});
      \endcode
  */
  struct Button : public GUIComponentT<Button> {
    std::string text;
    ButtonOpts  opts;

    Button(std::string text, ButtonOpts o = {})
      : GUIComponentT(o.toggledText.empty() ? "button" : "togglebutton"),
        text(std::move(text)), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for ToggleButton (no toggle-specific fields — the
  /// toggle texts and initial state are mandatory positional args).
  struct ToggleButtonOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Two-state toggle button.
  /** Spells out the toggle texts and initial state as required
      positional args, so call sites read self-documentingly instead of
      the `Button("a", {.toggledText="b"})` form:
      \code
      gui << ToggleButton("play", "pause", false, {.handle="pp"});
      \endcode
      `untoggledText` is shown while the button is up, `toggledText`
      while it is down; `initiallyToggled` picks the starting state. */
  struct ToggleButton : public GUIComponentT<ToggleButton> {
    std::string      untoggledText;
    std::string      toggledText;
    bool             initiallyToggled;
    ToggleButtonOpts opts;

    ToggleButton(std::string untoggledText, std::string toggledText,
                 bool initiallyToggled, ToggleButtonOpts o = {})
      : GUIComponentT("togglebutton"),
        untoggledText(std::move(untoggledText)),
        toggledText(std::move(toggledText)),
        initiallyToggled(initiallyToggled), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for CheckBox.
  struct CheckBoxOpts {
    bool        checked = false;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Check box with a text label.
  struct CheckBox : public GUIComponentT<CheckBox> {
    std::string text;
    CheckBoxOpts opts;

    CheckBox(std::string text, CheckBoxOpts o = {})
      : GUIComponentT("checkbox"), text(std::move(text)), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for ButtonGroup.
  struct ButtonGroupOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Vertical radio-button group (comma-separated entries).
  struct ButtonGroup : public GUIComponentT<ButtonGroup> {
    std::string entries;
    ButtonGroupOpts opts;

    ButtonGroup(std::string commaSepEntries, ButtonGroupOpts o = {})
      : GUIComponentT("buttongroup"), entries(std::move(commaSepEntries)), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  /// Options for Combo.
  struct ComboOpts {
    /// Index into the CSV entries list that starts out selected.
    int         initialIndex = 0;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Drop-down combo box (comma-separated entries).
  struct Combo : public GUIComponentT<Combo> {
    std::string entries;
    ComboOpts   opts;

    Combo(std::string commaSepEntries, ComboOpts o = {})
      : GUIComponentT("combo"), entries(std::move(commaSepEntries)), opts(std::move(o)) {
      setOptions(opts);
    }

    GUIWidget *createWidget(const CreateContext &ctx) const override;
  };

  // --- Phase 3 components -------------------------------------------------
  //
  // Display / canvas / introspection.  Same pattern as Phase 2.  Notable:
  // `Prop` has two ctors to preserve the legacy Configurable* vs string-id
  // dispatch (the pointer-encoding trick at GUIComponents.h:309).

  /// Options for Display.
  struct DisplayOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Image visualization widget (ICLWidget).
  struct Display {
    DisplayOpts opts;
    Display(DisplayOpts opts = {}) : opts(std::move(opts)) {}
    GUIComponent toComponent() const {
      return applyCommon(detail::Display(), opts);
    }
  };

  /// Options for Canvas (viewport is a primary positional arg, not
  /// an Opts field).
  struct CanvasOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// 2D drawing canvas (ICLDrawWidget).
  /** `viewport` (the canvas's logical resolution) is the conventional
      primary arg, so it stays positional: `Canvas({640,480})`
      rather than `Canvas({.viewport={640,480}})`.  The opts-only
      ctor keeps the VGA default. */
  struct Canvas {
    utils::Size viewport;
    CanvasOpts  opts;
    Canvas(CanvasOpts opts = {})
      : viewport(utils::Size::VGA), opts(std::move(opts)) {}
    Canvas(utils::Size viewport, CanvasOpts opts = {})
      : viewport(viewport), opts(std::move(opts)) {}
    GUIComponent toComponent() const {
      return applyCommon(detail::Canvas(viewport), opts);
    }
  };

  /// Options for Canvas3D (viewport is a primary positional arg,
  /// not an Opts field).
  struct Canvas3DOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// 3D-capable drawing canvas (ICLDrawWidget3D).
  /** `viewport` is positional, mirroring Canvas: `Canvas3D({640,480})`.
      The opts-only ctor keeps the VGA default. */
  struct Canvas3D {
    utils::Size  viewport;
    Canvas3DOpts opts;
    Canvas3D(Canvas3DOpts opts = {})
      : viewport(utils::Size::VGA), opts(std::move(opts)) {}
    Canvas3D(utils::Size viewport, Canvas3DOpts opts = {})
      : viewport(viewport), opts(std::move(opts)) {}
    GUIComponent toComponent() const {
      return applyCommon(detail::Canvas3D(viewport), opts);
    }
  };

  /// Options for Disp.
  struct DispOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// 2D grid of labels (nx × ny cells).
  struct Disp {
    int      nx;
    int      ny;
    DispOpts opts;

    Disp(int nx, int ny, DispOpts opts = {})
      : nx(nx), ny(ny), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(detail::Disp(nx, ny), opts);
    }
  };

  /// Options for Plot.
  /** The four range fields default to 0 — matches legacy `Plot()`'s
      "derive range from data" behavior. */
  struct PlotOpts {
    float       minX = 0, maxX = 0;
    float       minY = 0, maxY = 0;
    bool        openGL = false;
    std::string xLabel;
    std::string yLabel;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// 2D function / data plotter.
  /**
      \code
      gui << Plot({.handle="p"});                       // auto-ranged
      gui << Plot({.minX=-3.14f, .maxX=3.14f,
                       .minY=-1.f,   .maxY=1.f,
                       .handle="p", .xLabel="rad"});
      \endcode
  */
  struct Plot {
    PlotOpts opts;
    Plot(PlotOpts opts = {}) : opts(std::move(opts)) {}
    GUIComponent toComponent() const {
      return applyCommon(
        detail::Plot(opts.minX, opts.maxX, opts.minY, opts.maxY,
                 opts.openGL, opts.xLabel, opts.yLabel), opts);
    }
  };

  /// Options for Fps (timeWindow is a primary positional arg, not
  /// an Opts field).
  struct FpsOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Running-average FPS monitor.
  /** `timeWindow` (the averaging window in frames) is the conventional
      primary arg, so it stays positional: `Fps(100)` rather than
      `Fps({.timeWindow=100})`.  The opts-only ctor keeps the
      default window. */
  struct Fps {
    int     timeWindow;
    FpsOpts opts;
    Fps(FpsOpts opts = {}) : timeWindow(10), opts(std::move(opts)) {}
    Fps(int timeWindow, FpsOpts opts = {})
      : timeWindow(timeWindow), opts(std::move(opts)) {}
    GUIComponent toComponent() const {
      return applyCommon(detail::Fps(timeWindow), opts);
    }
  };

  /// Options for ColorSelect.
  /** `alpha == -1` means "no alpha channel exposed" (matches legacy). */
  struct ColorSelectOpts {
    int         alpha = -1;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// RGB(+A) color picker.
  struct ColorSelect {
    int r, g, b;
    ColorSelectOpts opts;

    ColorSelect(int r, int g, int b, ColorSelectOpts opts = {})
      : r(r), g(g), b(b), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(detail::ColorSelect(r, g, b, opts.alpha), opts);
    }
  };

  /// Options for CamCfg.
  struct CamCfgOpts {
    std::string deviceType;
    std::string deviceID;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Camera-configuration dialog button.
  struct CamCfg {
    CamCfgOpts opts;
    CamCfg(CamCfgOpts opts = {}) : opts(std::move(opts)) {}
    GUIComponent toComponent() const {
      return applyCommon(detail::CamCfg(opts.deviceType, opts.deviceID), opts);
    }
  };

  /// Options for Ps (updateFPS is a primary positional arg, not an
  /// Opts field).
  struct PsOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Process-monitor component (CPU / memory / thread count).
  /** `updateFPS` (the refresh rate) is the conventional primary arg, so
      it stays positional: `Ps(10)` rather than
      `Ps({.updateFPS=10})`.  The opts-only ctor keeps the default. */
  struct Ps {
    int    updateFPS;
    PsOpts opts;
    Ps(PsOpts opts = {}) : updateFPS(10), opts(std::move(opts)) {}
    Ps(int updateFPS, PsOpts opts = {})
      : updateFPS(updateFPS), opts(std::move(opts)) {}
    GUIComponent toComponent() const {
      return applyCommon(detail::Ps(updateFPS), opts);
    }
  };

  /// Options for Prop.
  struct PropOpts {
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Configurable-property inspector.
  /**
      Two ctors mirror the legacy `Prop`: one takes a live
      `Configurable*` (pointer survives via GUIComponents.h's
      `encode_pointer` trick), the other takes a string ID for
      Configurables previously registered with `Configurable::register`.

      \code
      MyConf conf;
      gui << Prop(&conf, {.handle="p"});
      gui << Prop("registered_id", {.handle="p"});
      \endcode
  */
  struct Prop {
    const utils::Configurable *cfg = nullptr;  // non-null → pointer ctor
    std::string                cfgID;          // used when cfg == nullptr
    PropOpts                   opts;

    Prop(const utils::Configurable *cfg, PropOpts opts = {})
      : cfg(cfg), opts(std::move(opts)) {}

    Prop(const utils::Configurable &cfg, PropOpts opts = {})
      : cfg(&cfg), opts(std::move(opts)) {}

    Prop(std::string id, PropOpts opts = {})
      : cfgID(std::move(id)), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(cfg ? detail::Prop(cfg) : detail::Prop(cfgID), opts);
    }
  };

  // --- Phase 4 containers -------------------------------------------------
  //
  // Containers diverge from the leaf-component pattern: they inherit from
  // their legacy `detail::` counterparts (which are `ContainerGUIComponent ->
  // GUI`) rather than being plain structs with `toComponent()`.
  //
  // Why inheritance here: containers are accumulators, not values.
  // `HBox({...}) << Slider(...) << Button(...)` needs the
  // `<<` chain to push children into the container.  Legacy containers
  // already do this via `ContainerGUIComponent::operator<<(const
  // GUIComponent&) const`; inheriting means we keep that plumbing for
  // free and get the top-level `gui << HBox({...})` to route through
  // the existing `GUI::operator<<(const GUI&)` overload — no new
  // dispatch needed.  Leaf children going into a container are picked
  // up by the free `operator<<(GUI&&, Component)` template further
  // down.
  //
  // (Border is intentionally not provided — detail::Border's ctor is
  //  private friend-only.  Any container's `.label` opts field produces
  //  an equivalent titled border.)

  /// Options for HBox / VBox / HScroll / VScroll /
  /// HSplit / VSplit.  All layout containers share the same
  /// knobs (margin, spacing, plus the metadata block).
  struct BoxOpts {
    /// -1 → use qt default; 0+ → explicit pixel margin.
    int margin  = -1;
    /// -1 → use qt default; 0+ → explicit pixel spacing.
    int spacing = -1;
    std::string handle;
    std::string label;   //!< non-empty → titled border around the container
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Helper that applies a BoxOpts pack to a live ContainerGUIComponent.
  /** Called from every container ctor below.  Mutates through the
      legacy const-qualified setters (which use mutable internals). */
  inline void applyBoxOpts(ContainerGUIComponent &c, const BoxOpts &o){
    if(o.margin  >= 0) c.margin(o.margin);
    if(o.spacing >= 0) c.spacing(o.spacing);
    if(!o.handle.empty())  c.handle(o.handle);
    if(!o.label.empty())   c.label(o.label);
    if(o.size    != utils::Size::null) c.size(o.size);
    if(o.minSize != utils::Size::null) c.minSize(o.minSize);
    if(o.maxSize != utils::Size::null) c.maxSize(o.maxSize);
    // ContainerGUIComponent has no tooltip/hide — skip silently.
    (void)o.tooltip; (void)o.hide;
  }

  /// Horizontal layout container.
  struct HBox : public detail::HBox {
    HBox(BoxOpts opts = {}) : detail::HBox() { applyBoxOpts(*this, opts); }
    explicit HBox(QWidget *parent, BoxOpts opts = {}) : detail::HBox(parent) { applyBoxOpts(*this, opts); }
  };

  /// Vertical layout container.
  struct VBox : public detail::VBox {
    VBox(BoxOpts opts = {}) : detail::VBox() { applyBoxOpts(*this, opts); }
    explicit VBox(QWidget *parent, BoxOpts opts = {}) : detail::VBox(parent) { applyBoxOpts(*this, opts); }
  };

  /// Horizontal scroll area.
  struct HScroll : public detail::HScroll {
    HScroll(BoxOpts opts = {}) : detail::HScroll() { applyBoxOpts(*this, opts); }
    explicit HScroll(QWidget *parent, BoxOpts opts = {}) : detail::HScroll(parent) { applyBoxOpts(*this, opts); }
  };

  /// Vertical scroll area.
  struct VScroll : public detail::VScroll {
    VScroll(BoxOpts opts = {}) : detail::VScroll() { applyBoxOpts(*this, opts); }
    explicit VScroll(QWidget *parent, BoxOpts opts = {}) : detail::VScroll(parent) { applyBoxOpts(*this, opts); }
  };

  /// Horizontal splitter (draggable pane divider).
  struct HSplit : public detail::HSplit {
    HSplit(BoxOpts opts = {}) : detail::HSplit() { applyBoxOpts(*this, opts); }
    explicit HSplit(QWidget *parent, BoxOpts opts = {}) : detail::HSplit(parent) { applyBoxOpts(*this, opts); }
  };

  /// Vertical splitter.
  struct VSplit : public detail::VSplit {
    VSplit(BoxOpts opts = {}) : detail::VSplit() { applyBoxOpts(*this, opts); }
    explicit VSplit(QWidget *parent, BoxOpts opts = {}) : detail::VSplit(parent) { applyBoxOpts(*this, opts); }
  };

  /// Tab container — positional CSV of tab titles + BoxOpts.
  /**
      \code
      gui << ( Tab("Signal,Plot,Log", {.handle="tabs"})
               << Display({.handle="sig"})
               << Plot({.handle="plt"})
               << State({.handle="log"}) );
      \endcode
  */
  struct Tab : public detail::Tab {
    Tab(const std::string &commaSepTitles, BoxOpts opts = {})
      : detail::Tab(commaSepTitles) { applyBoxOpts(*this, opts); }
    Tab(const std::string &commaSepTitles, QWidget *parent, BoxOpts opts = {})
      : detail::Tab(commaSepTitles, parent) { applyBoxOpts(*this, opts); }
  };

  /// Status bar — a thin strip docked to the bottom of its container.
  /** Unlike the other  containers there is no legacy `detail::StatusBar`
      to inherit (the component is new), so this inherits
      `ContainerGUIComponent` directly and emits the `statusbar(...)`
      definition itself.  Regardless of the container's layout direction
      the bar pins to the bottom edge, is capped to ~20px tall, and always
      carries an initial left-aligned label reachable as `gui["status"]`.
      Components streamed in are packed to the right of that label.

      \code
      gui << ( VBox()
               << Display({.handle="img"})
               << StatusBar() );           // docked at the bottom
      // ... later, from any thread:
      gui["status"] = str("ready");
      \endcode

      \b Note: add the StatusBar as the last component of its container. */
  struct StatusBar : public ContainerGUIComponent {
    StatusBar(BoxOpts opts = {})
      : ContainerGUIComponent("statusbar","",nullptr) { applyBoxOpts(*this, opts); }
    explicit StatusBar(QWidget *parent, BoxOpts opts = {})
      : ContainerGUIComponent("statusbar","",parent) { applyBoxOpts(*this, opts); }
  };

  // --- Phase 5 finalizers -------------------------------------------------
  //
  // Trivial markers.  Legacy shapes emit the magic `"!show"` / `"!create"`
  // strings / empty component name (Dummy), which GUI::operator<<(string)
  // special-cases.   wrappers just forward.

  /// Finalize GUI creation and show the window.
  struct Show {
    GUIComponent toComponent() const { return detail::Show(); }
  };

  /// Finalize GUI creation but keep the window hidden.
  struct Create {
    GUIComponent toComponent() const { return detail::Create(); }
  };

  /// No-op placeholder (pairs with `.hide` on other components to make
  /// conditional layout readable).
  struct Dummy {
    GUIComponent toComponent() const { return detail::Dummy(); }
  };

  /// Stream a designated-init Component into a GUI.
  /** Free-function overload in `icl::qt` so normal lookup finds it when
      the LHS is a GUI.  Delegates to the existing
      `GUI::operator<<(const GUIComponent&)` via the aggregate's
      `toComponent()`.  The rvalue overload mirrors the const-member
      pattern on `ContainerGUIComponent::operator<<` — it lets
      `HBox() << Slider(...)` chain starting from a temporary
      container. */
  template<Component T>
  GUI &operator<<(GUI &g, const T &t){
    return g << t.toComponent();
  }
  template<Component T>
  GUI &operator<<(GUI &&g, const T &t){
    return g << t.toComponent();
  }

} // namespace icl::qt
