/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLBlob module of ICL                         **
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

#include <ICLBlob/MeanShiftTracker.h>
#include <ICLCore/Channel.h>

namespace icl {

  Point32f MeanShiftTracker::applyMeanShiftStep(const Img32f &image, const Point32f &pos){

    Channel32f k = m_kernelImage[0];
    Channel32f w = image[0];

    double dx = 0;
    double dy = 0;
    double accu = 0;

    for (int x = -m_bandwidth; x<= m_bandwidth; ++x){
      int ix = x+ pos.x;
      if (ix <0) continue;
      if (ix >= image.getWidth()) break;
      
      for (int y = -m_bandwidth;y<=m_bandwidth; ++y){
        int iy = y+pos.y;
        if (iy < 0) continue;
        if (iy >= image.getHeight()) break;

        float sum = k(x+m_bandwidth, y+m_bandwidth) * w(ix,iy);

        dx += sum * ix;
        dy += sum * iy;
        accu += sum;
      }
    }

    //setting the new center; sum has to be != zero
    if (accu != 0) {
      return Point32f(dx,dy) *(1/accu);
    }else{
      return pos;
    }
  }

  Img32f MeanShiftTracker::generateEpanechnikov(int bandwidth) {
    Img32f k(Size(2*bandwidth+1,2*bandwidth+1),1);
    for (int i = -bandwidth; i < bandwidth; i++) {
      for (int j = -bandwidth; j < bandwidth; j++) {
        float value = ::sqrt ((i*i)+(j*j))/bandwidth;
        k(i+bandwidth,j+bandwidth, 0) = ( value > 1 ? 0 : (1.0-value*value) );
      }
    }
    return k;
  }
  
  Img32f MeanShiftTracker::generateGauss(int bandwidth, float stdDev) {
    Img32f k(Size(2*bandwidth+1,2*bandwidth+1),1);
    float var = stdDev*stdDev;
    for (int i = -bandwidth; i < bandwidth; i++) {
      for (int j = -bandwidth; j < bandwidth; j++) {
        k(i+bandwidth,j+bandwidth,0) = exp(-(i*i+j*j)/var);
      }
    }
    return k;
  }

  void MeanShiftTracker::setKernel(kernelType type, int bandwidth, float stdDev){
    m_bandwidth = bandwidth;
    m_kernelType = type;
    switch(type){
      case epanechnikov: m_kernelImage = generateEpanechnikov(bandwidth); break;
      case gauss: m_kernelImage = generateGauss(bandwidth,stdDev); break;
      default:
        ERROR_LOG("unsupported kernel type");
    }
  }


  MeanShiftTracker::MeanShiftTracker(kernelType type, int bandwidth, float stdDev){
    setKernel(type,bandwidth,stdDev);
  }


  const Point32f MeanShiftTracker::step(const Img32f &weigthImage, const Point32f &initialPoint,  
                                        int maxCycles, float convergenceCriterion, bool *converged){
    if (maxCycles < 0) {
      maxCycles = 10000;
    }
    if(converged) converged = false;
    Point32f lastPos = initialPoint;
    while(maxCycles--){
      Point32f newPos = applyMeanShiftStep(weigthImage,lastPos);
      if((lastPos-newPos).norm() <= convergenceCriterion){
        if(converged) *converged = true;
        break;
      }
      lastPos = newPos;
    }
    return lastPos;
  }

}
