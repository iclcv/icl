// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/qt/GUI.h>
#include <icl/qt/GUIComponent.h>
#include <icl/qt/GUIComponents.h>

#include <concepts>
#include <string>
#include <utility>

/// Mixed positional + designated-init GUI component syntax.
///
/// Usage target:
/// \code
///   gui << ui::Slider(0, 255, 42, {.vertical=true, .step=2,
///                                  .handle="gain", .label="Gain"});
/// \endcode
///
/// Shape: primary "obvious" data args (min/max/val for a slider, text
/// for a button, ...) are positional — required, conventionally ordered.
/// The trailing `Opts{}` pack holds everything else: component-specific
/// tuning (`vertical`, `step`, ...) plus shared UI metadata (`handle`,
/// `label`, `tooltip`, `size/minSize/maxSize`, `hide`).  One flat Opts
/// per component means call sites never need nested-designator syntax.
///
/// This coexists with the legacy `qt::Slider(0,255,42).handle("gain")`
/// stream-insertion builder — both route through the same
/// SliderGUIWidget factory.  The full retirement of GUIComponent's
/// string round-trip is a separate, larger arc (see TODO.md "Rework
/// GUIComponent internal representation" + the TODO comment at
/// `GUI.cpp:2338`).  `ui::` sits on top of today's toString()/parse
/// pipeline; once that pipeline goes, `ui::` becomes the primary
/// storage type.
namespace icl::qt::ui {

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

  /// Options pack for ui::Slider.
  /** Hoisted out of `Slider` because C++ forbids using a nested type's
      default member initializers as part of the enclosing class's own
      default argument (the enclosing class isn't complete yet at that
      point).  Named `SliderOpts` rather than `Slider::Opts` so the
      default argument `ui::Slider::Slider(..., SliderOpts={})` is
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
      gui << ui::Slider(0, 255, 42, {.handle="gain"});
      gui << ui::Slider(0, 100, 50, {.vertical=true, .step=2, .label="Coarse"});
      \endcode
  */
  struct Slider {
    int        min;
    int        max;
    int        val;
    SliderOpts opts;

    Slider(int min, int max, int val, SliderOpts opts = {})
      : min(min), max(max), val(val), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::Slider(min, max, val, opts.vertical, opts.step), opts);
    }
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

  /// Options for ui::FSlider.
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
  struct FSlider {
    float       min;
    float       max;
    float       val;
    FSliderOpts opts;

    FSlider(float min, float max, float val, FSliderOpts opts = {})
      : min(min), max(max), val(val), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::FSlider(min, max, val, opts.vertical), opts);
    }
  };

  /// Options for ui::Int.
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
  struct Int {
    int     min;
    int     max;
    int     val;
    IntOpts opts;

    Int(int min, int max, int val, IntOpts opts = {})
      : min(min), max(max), val(val), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::Int(min, max, val), opts);
    }
  };

  /// Options for ui::Float.
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
  struct Float {
    float     min;
    float     max;
    float     val;
    FloatOpts opts;

    Float(float min, float max, float val, FloatOpts opts = {})
      : min(min), max(max), val(val), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::Float(min, max, val), opts);
    }
  };

  /// Options for ui::Spinner.
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
  struct Spinner {
    int         min;
    int         max;
    int         val;
    SpinnerOpts opts;

    Spinner(int min, int max, int val, SpinnerOpts opts = {})
      : min(min), max(max), val(val), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::Spinner(min, max, val), opts);
    }
  };

  /// Options for ui::String.
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
  struct String {
    std::string text;
    StringOpts  opts;

    String(std::string text, StringOpts opts = {})
      : text(std::move(text)), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::String(text, opts.maxLen), opts);
    }
  };

  /// Options for ui::Label.
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
      would have (where `ui::Label{.label="x"}` would be ambiguous). */
  struct Label {
    std::string text;
    LabelOpts   opts;

    Label(std::string text = "", LabelOpts opts = {})
      : text(std::move(text)), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::Label(text), opts);
    }
  };

  /// Options for ui::State.
  struct StateOpts {
    int         maxLines = 100;
    std::string handle;
    std::string label;
    std::string tooltip;
    utils::Size size{};
    utils::Size minSize{};
    utils::Size maxSize{};
    bool        hide = false;
  };

  /// Scrolling log-style state panel.
  struct State {
    StateOpts opts;

    State(StateOpts opts = {}) : opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::State(opts.maxLines), opts);
    }
  };

  /// Options for ui::Button.
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
      gui << ui::Button("Run", {.handle="go"});
      gui << ui::Button("Play", {.toggledText="Pause", .handle="pp"});
      \endcode
  */
  struct Button {
    std::string text;
    ButtonOpts  opts;

    Button(std::string text, ButtonOpts opts = {})
      : text(std::move(text)), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(
        qt::Button(text, opts.toggledText, opts.initiallyToggled), opts);
    }
  };

  /// Options for ui::CheckBox.
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
  struct CheckBox {
    std::string text;
    CheckBoxOpts opts;

    CheckBox(std::string text, CheckBoxOpts opts = {})
      : text(std::move(text)), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::CheckBox(text, opts.checked), opts);
    }
  };

  /// Options for ui::ButtonGroup.
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
  struct ButtonGroup {
    std::string entries;
    ButtonGroupOpts opts;

    ButtonGroup(std::string commaSepEntries, ButtonGroupOpts opts = {})
      : entries(std::move(commaSepEntries)), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::ButtonGroup(entries), opts);
    }
  };

  /// Options for ui::Combo.
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
  struct Combo {
    std::string entries;
    ComboOpts   opts;

    Combo(std::string commaSepEntries, ComboOpts opts = {})
      : entries(std::move(commaSepEntries)), opts(std::move(opts)) {}

    GUIComponent toComponent() const {
      return applyCommon(qt::Combo(entries, opts.initialIndex), opts);
    }
  };

} // namespace icl::qt::ui

namespace icl::qt {
  /// Stream a ui::Component into a GUI.
  /** Free-function overload in `icl::qt` so normal lookup finds it when
      the LHS is a GUI.  Delegates to the existing
      `GUI::operator<<(const GUIComponent&)` via the aggregate's
      `toComponent()`.  The rvalue overload mirrors the const-member
      pattern on `ContainerGUIComponent::operator<<` — it lets
      `HBox() << ui::Slider(...)` chain starting from a temporary
      container. */
  template<ui::Component T>
  GUI &operator<<(GUI &g, const T &t){
    return g << t.toComponent();
  }
  template<ui::Component T>
  GUI &operator<<(GUI &&g, const T &t){
    return g << t.toComponent();
  }
} // namespace icl::qt
