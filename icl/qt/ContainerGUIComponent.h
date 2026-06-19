// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUI.h>

namespace icl::qt {
  /// Special GUI extension, that mimics the GUIComponent interface
  /** The Container GUIComponent mimics the GUIComponent interface
      in order to provide a unified look and feel within a hierarchical
      GUI definition section. Internally a mutable GUIComponent is used
      for parameter accumulation
  */
  struct ContainerGUIComponent : public GUI{
    protected:
    /// protected constructor
    /** The base GUI holds the single structured GUIComponent (a container node);
        the chained setters below accumulate options straight into it via
        GUI::mutableComponent() — there is no separate component copy. */
    ContainerGUIComponent(const std::string &type, const std::string &params, QWidget *parent):
    GUI(GUIComponent(type, params), parent){}

    public:

    /// hierarchical stream operator to create complex GUIs
    GUI &operator<<(const GUIComponent &component) const{
      return const_cast<GUI*>(static_cast<const GUI*>(this))->operator<<(component);
    }

    /// hierarchical stream operator to create complex GUIs
    GUI &operator<<(const GUI &g) const{
      return const_cast<GUI*>(static_cast<const GUI*>(this))->operator<<(g);
    }

    /// sets the component's handle
    const ContainerGUIComponent &handle(const std::string &handle) const{
      mutableComponent()->handle(handle); return *this;
    }

    /// sets the component's label
    const ContainerGUIComponent &label(const std::string &label) const{
      mutableComponent()->label(label); return *this;
    }

    /// sets the component's initial size
    const ContainerGUIComponent &size(const utils::Size &size) const {
      mutableComponent()->size(size); return *this;
    }

    /// sets the component's initial size
    const ContainerGUIComponent &size(int w, int h) const {
      return size(utils::Size(w,h));
    }

    /// sets the component's minimum size constraint
    const ContainerGUIComponent &minSize(const utils::Size &minSize) const {
      mutableComponent()->minSize(minSize); return *this;
    }

    /// sets the component's minimum size constraint
    const ContainerGUIComponent &minSize(int w, int h) const {
      return minSize(utils::Size(w,h));
    }

    /// sets the component's maximum size constraint
    const ContainerGUIComponent &maxSize(const utils::Size &maxSize) const {
      mutableComponent()->maxSize(maxSize); return *this;
    }

    /// sets the component's maximum size constraint
    const ContainerGUIComponent &maxSize(int w, int h) const {
      return maxSize(utils::Size(w,h));
    }

    /// sets the component's layout margin
    const ContainerGUIComponent &margin(int margin) const{
      mutableComponent()->m_options.margin = margin; return *this;
    }

    /// sets the component's layout spacing
    const ContainerGUIComponent &spacing(int spacing) const{
      mutableComponent()->m_options.spacing = spacing; return *this;
    }

    /// sets the component's handle
    ContainerGUIComponent &handle(const std::string &handle){
      mutableComponent()->handle(handle); return *this;
    }

    /// sets the component's label
    ContainerGUIComponent &label(const std::string &label){
      mutableComponent()->label(label); return *this;
    }

    /// sets the component's initial size
    ContainerGUIComponent &size(const utils::Size &size){
      mutableComponent()->size(size); return *this;
    }

    /// sets the component's initial size
    ContainerGUIComponent &size(int w, int h){
      return size(utils::Size(w,h));
    }

    /// sets the component's minimum size constraint
    ContainerGUIComponent &minSize(const utils::Size &minSize){
      mutableComponent()->minSize(minSize); return *this;
    }

    /// sets the component's minimum size constraint
    ContainerGUIComponent &minSize(int w, int h){
      return minSize(utils::Size(w,h));
    }

    /// sets the component's maximum size constraint
    ContainerGUIComponent &maxSize(const utils::Size &maxSize){
      mutableComponent()->maxSize(maxSize); return *this;
    }

    /// sets the component's maximum size constraint
    ContainerGUIComponent &maxSize(int w, int h){
      return maxSize(utils::Size(w,h));
    }

    /// sets the component's layout margin
    ContainerGUIComponent &margin(int margin){
      mutableComponent()->m_options.margin = margin; return *this;
    }

    /// sets the component's layout spacing
    ContainerGUIComponent &spacing(int spacing){
      mutableComponent()->m_options.spacing = spacing; return *this;
    }
  };



  } // namespace icl::qt