// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/DrawHandle3D.h>
#include <icl/qt/DrawWidget3D.h>

#include <icl/utils/dispatch/AssignRegistry.h>
#include <icl/core/Img.h>

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

  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::DrawHandle3D;
  namespace core = icl::core;
  __attribute__((constructor))
  static void icl_register_draw_handle3d_assignments() {
    AssignRegistry::enroll_receiver<DrawHandle3D,
        core::Img8u,  core::Img16s,  core::Img32s,  core::Img32f,  core::Img64f,
        core::Img8u*, core::Img16s*, core::Img32s*, core::Img32f*, core::Img64f*, core::ImgBase*,
        const core::Img8u*, const core::Img16s*, const core::Img32s*,
        const core::Img32f*, const core::Img64f*, const core::ImgBase*,
        core::Image>();
    AssignRegistry::enroll_identity<DrawHandle3D>();
  }
}