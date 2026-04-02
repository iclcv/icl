// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/DrawHandle3D.h>
#include <ICLQt/DrawWidget3D.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::qt {
    void DrawHandle3D::setImage(const ImgBase *image){
      (**this)->setImage(image);
    }
    void DrawHandle3D::render(){
      (**this)->render();
    }
    void DrawHandle3D::registerCallback(const GUI::Callback &cb, const std::string &events){
      (**this)->registerCallback(cb,events);
    }

    void DrawHandle3D::removeCallbacks(){
      (**this)->removeCallbacks();
    }

  } // namespace icl::qt