/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQuick/src/QuickRegions.cpp                          **
** Module : ICLQuick                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLQuick/QuickRegions.h>
#include <ICLBlob/RegionDetector.h>
namespace icl{

  namespace {
    RegionDetector s_oRD;
  }
   
  ImgQ colormap(const ImgQ &image, float r, float g, float b){ 
  // {{{ open
    ICLASSERT_RETURN_VAL(image.getChannels()==3,ImgQ());
    ImgQ result = zeros(image.getWidth(),image.getHeight());
    
    Channel32f rgb[]={image[0],image[2],image[2]};
    Channel32f m = result[0];

    for(int x=0;x<image.getWidth();++x){
      for(int y=0;y<image.getHeight();++y){
        float dr = pow((rgb[0])(x,y)-r,2);
        float dg = pow((rgb[1])(x,y)-g,2);
        float db = pow((rgb[2])(x,y)-b,2);
        float val = ::sqrt(  dr + dg + db );
        m(x,y) = 255- (val / ::sqrt(3.0));
      }
    }
    
    return result;
    //    return norm(result);
    
  }

  // }}}

  vector<Point> centers(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<Point> result;
    for(unsigned int i=0;i<bd.size();++i){
      Point32f cog = bd[i].getCOG();
      result.push_back(Point(cog.x,cog.y));
    }
    return result;
  }

  // }}}
  vector<Rect> boundingboxes(const ImgQ &image,int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<Rect> result;
    for(unsigned int i=0;i<bd.size();++i){
       result.push_back(bd[i].getBoundingBox());
    }
    return result;
  }

  // }}}
  vector<vector<Point> > boundaries(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<vector<Point> > result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getBoundary());
    }
    return result;
  }

  // }}}
  vector<int> boundarielengths(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<int> result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getBoundaryLength());
    }
    return result;
  }

  // }}}
  vector<RegionPCAInfo> pca(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<RegionPCAInfo> result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getPCAInfo());
    }
    return result;
        
  }

  // }}}
  vector<float> formfactors(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<float> result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getFormFactor());
    }
    return result;
  }

  // }}}
  vector<vector<Point> > pixels(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<vector<Point> >result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getPixels());
    }
    return result;
    
  }

  // }}}
  vector<vector<ScanLine> > scanlines(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<Region> & bd = s_oRD.detect(&image);
    vector<vector<ScanLine> > result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getScanLines());
    }
    return result;
    
  }

  // }}}


  void draw(ImgQ &image, const RegionPCAInfo &pcainfo){
    // {{{ open

    float x1 = float(pcainfo.cx)-cos(pcainfo.arc1)*pcainfo.len1;
    float y1 = float(pcainfo.cy)-sin(pcainfo.arc1)*pcainfo.len1;
    
    float x2 = float(pcainfo.cx)+cos(pcainfo.arc1)*pcainfo.len1;
    float y2 = float(pcainfo.cy)+sin(pcainfo.arc1)*pcainfo.len1;
    
    line(image,(int)x1,(int)y1,(int)x2,(int)y2);

    x1 = float(pcainfo.cx)-cos(pcainfo.arc2)*pcainfo.len2;
    y1 = float(pcainfo.cy)-sin(pcainfo.arc2)*pcainfo.len2;
    
    x2 = float(pcainfo.cx)+cos(pcainfo.arc2)*pcainfo.len2;
    y2 = float(pcainfo.cy)+sin(pcainfo.arc2)*pcainfo.len2;
    
    line(image,(int)x1,(int)y1,(int)x2,(int)y2);
    
  }

  // }}}
  void draw(ImgQ &image, const vector<RegionPCAInfo> &pcainfos){
    // {{{ open
    for(unsigned int i=0;i<pcainfos.size();++i){
      draw(image,pcainfos[i]);
    }
  }

  // }}}
  void draw(ImgQ &image, const ScanLine &scanline){
    // {{{ open

    line(image,scanline.x,scanline.y,scanline.x+scanline.len,scanline.y);
  }

  // }}}

  void draw(ImgQ &image, const vector<ScanLine> &scanlines){
    // {{{ open

    for(unsigned int i=0;i<scanlines.size();++i){
      draw(image,scanlines[i]);
    }
  }

  // }}}

  void draw(ImgQ &image,const vector< vector<ScanLine> > &scanlines){
    // {{{ open

    for(unsigned int i=0;i<scanlines.size();++i){
      draw(image,scanlines[i]);
    }
  }

  // }}}

}
