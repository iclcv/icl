// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/StateHandle.h>
#include <ICLQt/ThreadedUpdatableTextView.h>

namespace icl::qt {
  void StateHandle::append(const std::string &text){
    this->text()->appendTextFromOtherThread(text);
  }

  void StateHandle::clear(){
    text()->clearTextFromOtherThread();
  }

  void StateHandle::setMaxLen(int maxLen){
    this->maxLen = maxLen;
    removeOldLines();
  }

  int StateHandle::getMaxLen() const{
    return maxLen;
  }

  void StateHandle::removeOldLines(){
    // todo!
  }

  } // namespace icl::qt