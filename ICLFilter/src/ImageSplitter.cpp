/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLFilter/ImageSplitter.h>
#include <math.h>

using std::vector;

namespace icl{
  
  //      10------------------------50
  //n=4   10     20     30    40    50
  //
  
  void ImageSplitter::splitImage(ImgBase *src, vector<ImgBase*> &parts){
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
    
}

