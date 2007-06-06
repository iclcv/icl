#ifndef ICLQUICK_REGIONS_H
#define ICLQUICK_REGIONS_H

#include "iclQuick.h"
#include <iclBlobData.h>

namespace icl{
  ImgQ colormap(const ImgQ &image, float r, float g, float b);

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
