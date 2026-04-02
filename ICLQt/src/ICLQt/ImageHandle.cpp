// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/ImageHandle.h>
#include <ICLQt/Widget.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::qt {
  void ImageHandle::setImage(const ImgBase *image){
    (**this)->setImage(image);
  }
  void ImageHandle::render(){
    (**this)->render();
  }

  void ImageHandle::registerCallback(const GUI::Callback &cb, const std::string &events){
    (**this)->registerCallback(cb,events);
  }

  void ImageHandle::removeCallbacks(){
    (**this)->removeCallbacks();
  }


  } // namespace icl::qt