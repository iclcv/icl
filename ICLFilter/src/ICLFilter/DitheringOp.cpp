/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/DitheringOp.cpp                **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
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

#include <ICLFilter/DitheringOp.h>
#include <ICLCore/Img.h>

namespace icl{
  using namespace utils;
  using namespace core;
  
  namespace filter{
    DitheringOp::DitheringOp (Algorithm a, int l){
      setAlgorithm(a);
      setLevels(l);
    }
    
    void DitheringOp::apply (const core::ImgBase *poSrc, core::ImgBase **ppoDst){
      if(!prepare(ppoDst, depth8u, poSrc->getSize(), poSrc->getFormat(), poSrc->getChannels(),
                  poSrc->getROI(), poSrc->getTime())){
        throw ICLException("DitheringOp::apply: prepare failed");
      }
      Rect roi;
      if(getClipToROI()){
        poSrc->convertROI(*ppoDst);
        roi = (*ppoDst)->getImageRect();
      }else{
        poSrc->convert(*ppoDst);
        roi = (*ppoDst)->getROI();
      }

      icl8u lut[256] = {0};
      int dl = 256/m_levels, dval=255/(m_levels-1);
      
      for(int i=0;i<m_levels;++i){
        std::fill(lut+i*dl, lut+(i+1)*dl, i*dval);
      }

      const int maxx = roi.x + roi.width;
      const int maxy = roi.y + roi.height;
      for(int c=0;c<poSrc->getChannels();++c){
        Channel8u img = (*(*ppoDst)->as8u())[c];
        
        for (int y=roi.y; y<maxy; y++) {
          for (int x=roi.x; x<maxx; x++) {
            icl8u o = img(x,y);
            icl8u n = 0;
            if(o <= 0) n = 0;
            else if(o >= 255) n = 255;
            else n = lut[o];
            img(x,y) = n;
            int e = o - n;
            // img(x+1,y) += e * 7./16.;
            // img(x-1,y+1) +=  e * 3./16.;
            // img(x,y+1) += e * 3./16.;
            // img(x+1,y+1) += e * 1./16.;
            bool xin = x+1<maxx, yin = y+1<maxy;
            if(xin){
              img(x+1,y) += (e*7)/16;
            }
            if(yin){
              img(x-1,y+1) +=  (e*3)/16;
              img(x,y+1) += (e*3)/16;
              if(xin){
                img(x+1,y+1) += e/16;
              }
            }            
          }
        }
      }
    }
  } // namespace filter
}


