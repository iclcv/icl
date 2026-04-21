// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/ImageSplitter.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  //      10------------------------50
  //n=4   10     20     30    40    50
  //

  void ImageSplitter::splitImage(ImgBase *src, std::vector<ImgBase*> &parts){
    Rect r = src->getROI();
    int n = static_cast<int>(parts.size());
    ICLASSERT_RETURN(n);
    ICLASSERT_RETURN(r.getDim());
    ICLASSERT_RETURN(r.height >= n);

    float dh = float(r.height)/n;
    for(int i=0;i<n;++i){
      int yStart = r.y+static_cast<int>(round(dh*i));
      int yEnd = r.y+static_cast<int>(round(dh*(i+1)));
      int dy = yEnd - yStart;
      parts[i] = src->shallowCopy(Rect(r.x,yStart,r.width,dy));
    }
  }



  std::vector<ImgBase*> ImageSplitter::split(ImgBase *src, int nParts){
    std::vector<ImgBase*> v(nParts, nullptr);
    splitImage(src,v);
    return v;
  }

  const std::vector<ImgBase*> ImageSplitter::split(const ImgBase *src, int nParts){
    ImgBase *srcX = const_cast<ImgBase*>(src);
    std::vector<ImgBase*> v= split(srcX,nParts);
    return v;
  }

  void ImageSplitter::release(const std::vector<ImgBase*> &v){
    for(unsigned int i=0;i<v.size();i++){
      delete v[i];
    }
  }

  } // namespace icl::filter