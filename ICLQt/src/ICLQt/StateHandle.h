// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/StringUtils.h>
#include <ICLQt/ThreadedUpdatableTextView.h>
#include <ICLQt/GUIHandle.h>
#include <string>

namespace icl::qt {
  /// Class for GUI-Label handling a so called state component\ingroup HANDLES
  /** You can add a state component as an alternative std::cout for debugging
      or system state messages
      @see GUI */
  class ICLQt_API StateHandle : public GUIHandle<ThreadedUpdatableTextView>{
    int maxLen;
    public:

    /// Create an empty handle
    StateHandle(){}

    /// Create a new LabelHandle
    StateHandle(ThreadedUpdatableTextView *l, GUIWidget *w, int maxLen):
      GUIHandle<ThreadedUpdatableTextView>(l,w),maxLen(maxLen){}

    /// appends a string
    void append(const std::string &text);

    /// appends anything in std::ostream manner
    template<class T>
    StateHandle &operator<<(const T &t){
      append(utils::str(t));
      return *this;
    }

    /// erases all lines
    void clear();

    /// sets max line length (odd lines are removed)
    void setMaxLen(int maxLen);

    /// returns curren max line length
    int getMaxLen() const;

    private:
    /// utitlity function
    ThreadedUpdatableTextView *text() { return **this; }

    /// utitlity function
    const ThreadedUpdatableTextView *text() const{ return **this; }

    /// utility function
    void removeOldLines();
  };




  } // namespace icl::qt