// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Robert Haschke

// C++ backend for the ImgOps `replicateBorder` dispatch entry.
// Img<T>::fillBorder() (extrude variant) routes through this dispatcher
// so the IPP-optimized variant in Img_Ipp.cpp can replace it when IPP
// is available.

#include <icl/core/Img.h>
#include <icl/core/dispatch/ImgOps.h>

using namespace icl::utils;

namespace icl::core {
  namespace {
    template<class T>
    inline void copy_border(Img<T> *poImage){
      Rect im = Rect(Point::null, poImage->getSize());
      Rect roi = poImage->getROI();

      Rect aR[4] = {
        Rect(0,             0,              roi.x+1,                roi.y+1),                  // upper left
        Rect(roi.right()-1, 0,              im.width-roi.right()+1, roi.y+1),                  // upper right
        Rect(roi.right()-1, roi.bottom()-1, im.width-roi.right()+1, im.height-roi.bottom()+1), // lower right
        Rect(0,             roi.bottom()-1, roi.x+1,                im.height-roi.bottom()+1)  // lower left
      };

      T aPix[4];
      for (int c = 0; c < poImage->getChannels(); c++) {
        aPix[0] = (*poImage)(roi.x,         roi.y,          c);  // upper left
        aPix[1] = (*poImage)(roi.right()-1, roi.y,          c);  // upper right
        aPix[2] = (*poImage)(roi.right()-1, roi.bottom()-1, c);  // lower right
        aPix[3] = (*poImage)(roi.x,         roi.bottom()-1, c);  // lower left

        for (int i = 0; i < 4; i++) {
          clearChannelROI<T>(poImage, c, aPix[i], aR[i].ul(), aR[i].getSize());
        }

        Point srcOffs;
        Size srcDstSize;

        // left
        srcOffs = Point(roi.x, roi.y+1);
        srcDstSize = Size(1, roi.height-2);
        if (roi.x > 0) {
          for (Point p(0, roi.y+1); p.x != roi.x; p.x++) {
            deepCopyChannelROI(poImage, c, srcOffs, srcDstSize, poImage, c, p, srcDstSize);
          }
        }

        // right
        srcOffs.x = roi.right()-1;
        if (roi.right() < im.right()) {
          for (Point p(srcOffs.x+1, srcOffs.y); p.x < im.width; p.x++) {
            deepCopyChannelROI(poImage, c, srcOffs, srcDstSize, poImage, c, p, srcDstSize);
          }
        }

        // top
        srcOffs = Point(roi.x+1, roi.y);
        srcDstSize = Size(roi.width-2, 1);
        if (roi.y > 0) {
          for (Point p(srcOffs.x, roi.y+1); p.y >= 0; p.y--) {
            deepCopyChannelROI(poImage, c, srcOffs, srcDstSize, poImage, c, p, srcDstSize);
          }
        }

        // bottom
        if (roi.bottom() < im.bottom()) {
          srcOffs.y = roi.bottom()-1;
          for (Point p(srcOffs.x, roi.bottom()); p.y < im.height; p.y++) {
            deepCopyChannelROI(poImage, c, srcOffs, srcDstSize, poImage, c, p, srcDstSize);
          }
        }
      }
    }

    void cpp_replicateBorder(ImgBase &img) {
      switch (img.getDepth()) {
#define ICL_INSTANTIATE_DEPTH(D) \
        case depth##D: copy_border(img.asImg<icl##D>()); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
        default: break;
      }
    }

    static int _rrb = [] {
      ImgOps::instance().backends(Backend::Cpp)
        .add<ImgOps::ReplicateBorderSig>(ImgOps::Op::replicateBorder, cpp_replicateBorder, "C++ border replication");
      return 0;
    }();
  }
} // namespace icl::core
