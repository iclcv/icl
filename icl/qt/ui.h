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
