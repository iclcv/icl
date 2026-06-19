// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Range.h>
#include <icl/utils/Size.h>
#include <memory>

/** \cond */
class QLayout;
class QWidget;
/** \endcond */

namespace icl::qt {

  /** \cond */
  class GUI;
  class GUIWidget;
  class ProxyLayout;
  /** \endcond */

  /// Transient context handed to GUIComponent::createWidget at create() time.
  /** Carries the Qt parent/layout plumbing the widget needs but the component
      spec itself does not (it is filled in only while GUI::create() walks the
      tree). Replaces the non-parsed half of the former GUIDefinition. */
  struct CreateContext {
    GUI         *gui          = nullptr;  //!< owning GUI (datastore + tree)
    QLayout     *parentLayout = nullptr;  //!< parent widget's layout (or null)
    ProxyLayout *parentProxy  = nullptr;  //!< parent widget's proxy layout
    QWidget     *parentWidget = nullptr;  //!< parent widget (avoids reparenting)
  };

  /// Polymorphic base of every GUI component (Slider, Button, HBox, ...).
  /** <b>Please refer to the ICL-manual for an introduction to the GUI toolkit</b>\n
      A component owns its typed parameters + shared layout Options and knows how
      to build its own Qt widget via the virtual createWidget(). The owning GUI
      tree holds components as `shared_ptr<GUIComponent>`, so polymorphism is
      preserved end-to-end. */
  class GUIComponent{

    /// friend container class
    friend struct ContainerGUIComponent;

    /// friend GUI class
    friend class GUI;

    public:
    /// virtual: components are handed around polymorphically via shared_ptr
    virtual ~GUIComponent() = default;

    /// builds this component's Qt widget
    /** Overridden by each concrete component; the override is DECLARED in the
        public header but DEFINED in GUI.cpp, next to the (private) *GUIWidget
        classes, so Qt widget internals never leak into installed headers. The
        base implementation delegates to the legacy string-tag registry — the
        transitional bridge while components are migrated one by one. */
    virtual GUIWidget *createWidget(const CreateContext &ctx) const;

    /// polymorphic copy (the GUI tree stores cloned shared_ptr<GUIComponent>s)
    virtual std::shared_ptr<GUIComponent> clone() const {
      return std::shared_ptr<GUIComponent>(new GUIComponent(*this));
    }

    /// Actual options (set using the .xxx methods)
    struct Options {
    Options():margin(-1),spacing(-1), hide(false){}
      std::string handle;  //!< the component handle
      std::string in;      //!< not used!
      std::string label;   //!< label (results in a titeld border
      std::string tooltip; //!< component tooltip (not for containers)
      int margin;          //!< layout margin (only for containers)
      int spacing;         //!< layout spacing (onyl for containers)
      utils::Size minSize;        //!< minimum size constraint of the component (in units of 20px)
      utils::Size maxSize;        //!< maximum size constraint of the component (in units of 20px)
      utils::Size size;           //!< intial size of the component (in units of 20px)
      bool hide;           //!< if true, the component is not created at all
    };

    /// the shared layout/handle options (read by GUIWidget at build time)
    const Options &options() const { return m_options; }

    /// the component's type tag (used for debug / XML only)
    const std::string &type() const { return m_type; }

    protected:

    /// copy the shared-metadata fields of an Opts-like struct into m_options.
    /** Used by migrated components' ctors. `if constexpr(requires{...})` so each
        Opts may include or omit any field (containers add margin/spacing). */
    template<class O>
    void setOptions(const O &o){
      if constexpr(requires{ o.handle; })  if(!o.handle.empty())  m_options.handle = o.handle;
      if constexpr(requires{ o.label; })   if(!o.label.empty())   m_options.label = o.label;
      if constexpr(requires{ o.tooltip; }) if(!o.tooltip.empty()) m_options.tooltip = o.tooltip;
      if constexpr(requires{ o.size; })    if(o.size    != utils::Size::null) m_options.size = o.size;
      if constexpr(requires{ o.minSize; }) if(o.minSize != utils::Size::null) m_options.minSize = o.minSize;
      if constexpr(requires{ o.maxSize; }) if(o.maxSize != utils::Size::null) m_options.maxSize = o.maxSize;
      if constexpr(requires{ o.hide; })    if(o.hide)             m_options.hide = true;
      if constexpr(requires{ o.margin; })  if(o.margin  >= 0)     m_options.margin = o.margin;
      if constexpr(requires{ o.spacing; }) if(o.spacing >= 0)     m_options.spacing = o.spacing;
    }


    /// all component options (mutable for C++-reasons)
    mutable Options m_options;

    /// component type tag (debug / XML / finalizer dispatch only)
    std::string m_type;

    /// creates a component with the given type tag
    explicit GUIComponent(std::string type): m_type(std::move(type)){}
    public:

    /// sets the component handle
    const GUIComponent &handle(const std::string &handle) const{
      m_options.handle = handle;
      return *this;
    }

    /// sets the component label
    const GUIComponent &label(const std::string &label) const{
      m_options.label = label;
      return *this;
    }

    /// sets the component tooltip
    const GUIComponent &tooltip(const std::string &tooltip) const{
      m_options.tooltip = tooltip;
      return *this;
    }

    /// sets the component initial size
    const GUIComponent &size(const utils::Size &size) const {
      m_options.size = size;
      return *this;
    }

    /// sets the component initial size
    const GUIComponent &size(int w, int h) const {
      return size(utils::Size(w,h));
    }

    /// sets the component minimum size constraint
    const GUIComponent &minSize(const utils::Size &minSize) const {
      m_options.minSize = minSize;
      return *this;
    }

    /// sets the component minimum size constraint
    const GUIComponent &minSize(int w, int h) const {
      return minSize(utils::Size(w,h));
    }

    /// sets the component maximum size constraint
    const GUIComponent &maxSize(const utils::Size &maxSize) const {
      m_options.maxSize = maxSize;
      return *this;
    }

    /// sets the component maximum size constraint
    const GUIComponent &maxSize(int w, int h) const {
      return maxSize(utils::Size(w,h));
    }

    /// hides the component if the given flag is true
    /** this can be used to circumvent C++-language issues when creating
        GUI components optionally, e.g.
        \code
        bool flag = ....;
        GUI gui;
        gui << (flag ? Display() : Dummy()).handle("image"); // does not work

        gui << Display().hideIf(!flag).handle("image");    // works
        \endcode

    */
    const GUIComponent &hideIf(bool flag) const{
      if(flag) m_options.hide = true;
      return *this;
    }

    /// sets the component handle
    GUIComponent &handle(std::string &handle) {
      m_options.handle = handle;
      return *this;
    }

    /// sets the component label
    GUIComponent &label(std::string &label) {
      m_options.label = label;
      return *this;
    }

    /// sets the component tooltip
    GUIComponent &tooltip(std::string &tooltip) {
      m_options.tooltip = tooltip;
      return *this;
    }

    /// sets the component initial size
    GUIComponent &size(utils::Size &size)  {
      m_options.size = size;
      return *this;
    }

    /// sets the component initial size
    GUIComponent &size(int w, int h)  {
      m_options.size = utils::Size(w,h);
      return *this;
    }

    /// sets the component minimum size constraint
    GUIComponent &minSize(utils::Size &minSize)  {
      m_options.minSize = minSize;
      return *this;
    }

    /// sets the component minimum size constraint
    GUIComponent &minSize(int w, int h)  {
      m_options.minSize = utils::Size(w,h);
      return *this;
    }

    /// sets the component maximum size constraint
    GUIComponent &maxSize(utils::Size &maxSize)  {
      m_options.maxSize = maxSize;
      return *this;
    }

    /// sets the component maximum size constraint
    GUIComponent &maxSize(int w, int h)  {
      m_options.maxSize = utils::Size(w,h);
      return *this;
    }

    /// hides the component if the given flag is true
    /** \copydoc icl::qt::GUIComponent::hideIf(bool)const */
    GUIComponent &hideIf(bool flag)  {
      if(flag) m_options.hide = true;
      return *this;
    }
  };

  /// CRTP convenience base supplying the polymorphic clone().
  /** Concrete components derive from `GUIComponentT<Self>` and implement
      createWidget(); clone() is provided once here so each component need not
      repeat the `make_shared<Self>(*this)` boilerplate. The base GUIComponent
      (type, params) ctor is inherited. */
  template<class Derived>
  class GUIComponentT : public GUIComponent {
    protected:
    using GUIComponent::GUIComponent;
    public:
    std::shared_ptr<GUIComponent> clone() const override {
      return std::make_shared<Derived>(static_cast<const Derived&>(*this));
    }
  };

  } // namespace icl::qt