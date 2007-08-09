#ifndef ICLQUICK_REGIONS_H
#define ICLQUICK_REGIONS_H

#include "iclQuick.h"
#include <iclBlobData.h>

namespace icl{
  /// creates a color map of a 3-channel color image
  /** This function uses a the following algorithm to calculate the
      color map \f$M\f$ of an 3-channel image \f$I\f$:
      \f[
      M(x,y) = 255 - \frac{\sqrt{(I(x,y,0)-r)^2 + (I(x,y,1)-g)^2 + (I(x,y,2)-b)^2}}{\sqrt{3}}
      \f]
      @param image input image
      @param r red referene color value
      @param g green referecne color value
      @param b blue color value
  */
  ImgQ colormap(const ImgQ &image, float r, float g, float b);

  /// detects all region centers withing the given image
  /** a*/
  vector<Point> centers(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<Rect> boundingboxes(const ImgQ &image,int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<vector<Point> > boundaries(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<int> boundarielengths(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<PCAInfo> pca(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<float> formfactors(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  
  vector<vector<Point> > pixels(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<vector<ScanLine> > scanlines(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);


  void draw(ImgQ &image,const PCAInfo &pcainfo);
  void draw(ImgQ &image,const vector<PCAInfo> &pcainfos);
  void draw(ImgQ &image,const ScanLine &scanline);
  void draw(ImgQ &image,const vector<ScanLine> &scanlines);
  void draw(ImgQ &image,const vector< vector<ScanLine> > &scanlines);
}

#endif
