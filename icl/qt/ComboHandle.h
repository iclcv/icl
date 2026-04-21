// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/qt/GUIHandle.h>
#include <string>

/** \cond */
class QComboBox;
/** \endcond */

namespace icl::qt {
  /// Handle class for combo components \ingroup HANDLES
  class ComboHandle : public GUIHandle<QComboBox>{
    public:
    /// create an empty handle
    ComboHandle(){}

    /// create a new ComboHandle wrapping a given QComboBox
    ComboHandle(QComboBox *cb, GUIWidget *w):GUIHandle<QComboBox>(cb,w){}

    /// add an item
    ICLQt_API void add(const std::string &item);

    /// remove an item
    ICLQt_API void remove(const std::string &item);

    /// remove item at given index
    ICLQt_API void remove(int idx);

    /// void remove all items
    ICLQt_API void clear();

    /// returns the item at given index
    ICLQt_API std::string getItem(int idx) const;

    /// returns the index of a given item
    ICLQt_API int getIndex(const std::string &item) const;

    /// returns the currently selected index
    ICLQt_API int getSelectedIndex() const;

    /// returns the currently selected item
    ICLQt_API std::string getSelectedItem() const;

    /// returns the count of elements
    ICLQt_API int getItemCount() const;

    /// sets the current index
    ICLQt_API void setSelectedIndex(int idx);

    /// sets the current item
    ICLQt_API void setSelectedItem(const std::string &item);

    /// convenience operator wrapping getSelectedIndex
    inline operator int() const { return getSelectedIndex(); }

    /// convenience operator wrapping getSelectedItem
    inline operator std::string() const { return getSelectedItem(); }

    private:
    /// utility function (internally used)
    QComboBox *cbx() { return **this; }

    /// utility function (internally used)
    const QComboBox *cbx() const{ return **this; }
  };

  } // namespace icl::qt