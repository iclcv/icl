/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQuick/QuickRegions.h                        **
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

#ifndef ICLQUICK_REGIONS_H
#define ICLQUICK_REGIONS_H

#include <ICLQuick/Quick.h>
#include <ICLBlob/Region.h>

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
  vector<RegionPCAInfo> pca(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<float> formfactors(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  
  vector<vector<Point> > pixels(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);
  vector<vector<ScanLine> > scanlines(const ImgQ &image, int minSize=0, int maxSize=1000000000, int minVal=0, int maxVal=255);


  void draw(ImgQ &image,const RegionPCAInfo &pcainfo);
  void draw(ImgQ &image,const vector<RegionPCAInfo> &pcainfos);
  void draw(ImgQ &image,const ScanLine &scanline);
  void draw(ImgQ &image,const vector<ScanLine> &scanlines);
  void draw(ImgQ &image,const vector< vector<ScanLine> > &scanlines);
}

#endif
