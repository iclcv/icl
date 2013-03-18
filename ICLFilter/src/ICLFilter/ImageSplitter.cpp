/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ImageSplitter.cpp              **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLFilter/ImageSplitter.h>
#include <math.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace filter{
    
    //      10------------------------50
    //n=4   10     20     30    40    50
    //
    
    void ImageSplitter::splitImage(ImgBase *src, std::vector<ImgBase*> &parts){
      Rect r = src->getROI();
      int n = (int)parts.size();
      ICLASSERT_RETURN(n);
      ICLASSERT_RETURN(r.getDim());
      ICLASSERT_RETURN(r.height >= n);
      
      float dh = float(r.height)/n;
      for(int i=0;i<n;++i){
        int yStart = r.y+(int)round(dh*i);
        int yEnd = r.y+(int)round(dh*(i+1));
        int dy = yEnd - yStart;
        parts[i] = src->shallowCopy(Rect(r.x,yStart,r.width,dy));
      }      
    }
    
    
    
    std::vector<ImgBase*> ImageSplitter::split(ImgBase *src, int nParts){
      std::vector<ImgBase*> v(nParts,(ImgBase*)0);
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
      
  } // namespace filter
}

