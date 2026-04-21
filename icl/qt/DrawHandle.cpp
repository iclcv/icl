// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DrawHandle.h>
#include <icl/qt/DrawWidget.h>

using namespace icl::core;

namespace icl::qt {
  void DrawHandle::setImage(const ImgBase *image){
    (**this)->setImage(image);
  }
  void DrawHandle::render(){
    (**this)->render();
  }
  void DrawHandle::registerCallback(const GUI::Callback &cb, const std::string &events){
    (**this)->registerCallback(cb,events);
  }

  void DrawHandle::removeCallbacks(){
    (**this)->removeCallbacks();
  }

  } // namespace icl::qt