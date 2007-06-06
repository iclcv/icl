#include "iclQuickRegions.h"
#include <iclImgChannel.h>
#include <iclImgRegionDetector.h>
namespace icl{

  namespace {
    ImgRegionDetector s_oRD;
  }
   
  ImgQ colormap(const ImgQ &image, float r, float g, float b){ 
  // {{{ open
    ICLASSERT_RETURN_VAL(image.getChannels()==3,ImgQ());
    ImgQ result = zeros(image.getWidth(),image.getHeight());
    
    ImgChannel32f rgb[3] = {pickChannel(&image,0),pickChannel(&image,1),pickChannel(&image,2)};
    ImgChannel32f m = pickChannel(&result,0);

    for(int x=0;x<image.getWidth();x++){
      for(int y=0;y<image.getHeight();y++){
        float dr = pow((rgb[0])(x,y)-r,2);
        float dg = pow((rgb[1])(x,y)-g,2);
        float db = pow((rgb[2])(x,y)-b,2);
        float val = ::sqrt(  dr + dg + db );
        m(x,y) = 255- (val / ::sqrt(3));
      }
    }
    
    return result;
    //    return norm(result);
    
  }

  // }}}

  vector<Point> centers(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<BlobData> & bd = s_oRD.detect(&image);
    vector<Point> result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getCOG());
    }
    return result;
  }

  // }}}
  vector<Rect> boundingboxes(const ImgQ &image,int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<BlobData> & bd = s_oRD.detect(&image);
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
    const vector<BlobData> & bd = s_oRD.detect(&image);
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
    const vector<BlobData> & bd = s_oRD.detect(&image);
    vector<int> result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getBoundaryLength());
    }
    return result;
  }

  // }}}
  vector<PCAInfo> pca(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<BlobData> & bd = s_oRD.detect(&image);
    vector<PCAInfo> result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getPCAInfo());
    }
    return result;
        
  }

  // }}}
  vector<float> formfactors(const ImgQ &image, int minSize, int maxSize, int minVal, int maxVal){
    // {{{ open

    s_oRD.setRestrictions(minSize,maxSize,minVal,maxVal);
    const vector<BlobData> & bd = s_oRD.detect(&image);
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
    const vector<BlobData> & bd = s_oRD.detect(&image);
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
    const vector<BlobData> & bd = s_oRD.detect(&image);
    vector<vector<ScanLine> > result;
    for(unsigned int i=0;i<bd.size();++i){
      result.push_back(bd[i].getScanLines());
    }
    return result;
    
  }

  // }}}


  void draw(ImgQ &image, const PCAInfo &pcainfo){
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
  void draw(ImgQ &image, const vector<PCAInfo> &pcainfos){
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
