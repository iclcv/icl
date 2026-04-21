// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/ImageHandle.h>
#include <icl/qt/Widget.h>

#include <icl/utils/AssignRegistry.h>
#include <icl/core/Img.h>

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


  }  // namespace icl::qt

namespace {
  using icl::utils::AssignRegistry;
  using icl::qt::ImageHandle;
  namespace core = icl::core;
  __attribute__((constructor))
  static void icl_register_image_handle_assignments() {
    // ImageHandle accepts an image (by value, by ref/pointer, by Image).
    // All Img<T> paths fall through to operator=(const ImgBase&) or
    // operator=(const ImgBase*) via derived-to-base conversion; the
    // trait's DirectlyAssignable picks them up.
    AssignRegistry::enroll_receiver<ImageHandle,
        core::Img8u,  core::Img16s,  core::Img32s,  core::Img32f,  core::Img64f,
        core::Img8u*, core::Img16s*, core::Img32s*, core::Img32f*, core::Img64f*, core::ImgBase*,
        const core::Img8u*, const core::Img16s*, const core::Img32s*,
        const core::Img32f*, const core::Img64f*, const core::ImgBase*,
        core::Image>();
    AssignRegistry::enroll_identity<ImageHandle>();
  }
}