// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>
#include <vector>
#include <string>

/** \cond */
class QRadioButton;
/** \endcond */

namespace icl::qt {
  /// type definition for the ButtonGroup handle \ingroup HANDLES
  using RadioButtonVec = std::vector<QRadioButton*>;

  /// Class for handling button goups \ingroup HANDLE
  class ButtonGroupHandle : public GUIHandle<RadioButtonVec> {
    public:
    /// Create an empty handle
    ButtonGroupHandle(){}

    /// Craete a valid handle
    ButtonGroupHandle(RadioButtonVec *buttons, GUIWidget *w) : GUIHandle<RadioButtonVec>(buttons, w){ }

    /// select a button with given index
    ICLQt_API void select(int id);

    /// get the selected index
    ICLQt_API int getSelected() const;

    /// get the text of the currently selected button
    ICLQt_API std::string getSelectedText() const;

    /// returns the text of a button with given index
    ICLQt_API std::string getText(int id) const;

    /// sets the text of a button with index ot a given text
    ICLQt_API void setText(int id, const std::string &text);

    /// disables all radio buttons
    ICLQt_API void disable();

    /// enables all radio buttons
    ICLQt_API void enable();

    /// disables button at index
    ICLQt_API void disable(int index);

    /// enables button at index
    ICLQt_API void enable(int index);

    private:
    /// utility function (number of elements)
    int n() const{ return static_cast<int>(vec().size()); }

    /// utility function (check indices for being valid)
    bool valid(int id) const{ return id >= 0 && id < static_cast<int>(vec().size()); }

    /// utitliy function returns the underlying vector
    RadioButtonVec &vec() { return ***this; }

    /// utitliy function returns the underlying vector (const)
    const RadioButtonVec &vec() const { return const_cast<ButtonGroupHandle*>(this)->vec(); }
  };

  } // namespace icl::qt