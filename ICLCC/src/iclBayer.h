#ifndef iclBayer_H
#define iclBayer_H

#include <iclCore.h>

namespace icl {

class BayerConverter
{
 public:

  enum bayerConverterMethod {
    nearestNeighbor = 0,
    simple,
    bilinear,
    hqLinear,
    edgeSense,
    vng
  };
  
  enum bayerPattern {
    bayerPattern_RGGB = 512,
    bayerPattern_GBRG,
    bayerPattern_GRBG,
    bayerPattern_BGGR
  };
  
  BayerConverter(bayerConverterMethod eConvMethod, 
                 bayerPattern eBayerPattern, 
                 Size s);
  ~BayerConverter();
  
  void apply(const Img8u *src, ImgBase *dst);
  
  void setBayerPattern(bayerPattern eBayerPattern) { 
    m_eBayerPattern = eBayerPattern; }
  void setConverterMethod(bayerConverterMethod eConvMethod) { 
    m_eConvMethod = eConvMethod; }
  void setBayerImgSize(Size s) {
    m_pucRGBInterImg = new unsigned char[s.width * s.height * 3]; }
  
 private:
  icl8u *m_pucRGBInterImg; // RGB image in interleaved data format
  bayerConverterMethod m_eConvMethod;
  bayerPattern m_eBayerPattern;
  
  // Interpolation methods
  void nnInterpolation(const Img8u *poBayerImg, ImgBase *dst);
  void bilinearInterpolation(const Img<icl8u> *poBayerImg, ImgBase *dst);
  void hqLinearInterpolation(const Img<icl8u> *poBayerImg, ImgBase *dst);
  void edgeSenseInterpolation(const Img<icl8u> *poBayerImg, ImgBase *dst);
  void simpleInterpolation(const Img<icl8u> *poBayerImg, ImgBase *dst);
  
  // Helper functions
  void clearBorders(icl8u *rgb, int sx, int sy, int w);
  void clip(int *iIn, icl8u *iOut) {
    *iIn = *iIn < 0 ? 0 : *iIn; 
    *iIn = *iIn > 255 ? 255 : *iIn; 
    *iOut = *iIn;
  }
};
 
} // namespace icl

#endif
