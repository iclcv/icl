#ifndef LOCAL_THRESHOLD_OP_H
#define LOCAL_THRESHOLD_OP_H

#include "iclUnaryOp.h"
#include "iclSize.h"
#include "iclIntegralImgOp.h"
#include <vector>
#include <iclUncopyable.h>

namespace icl{
  
  /// LocalThreshold Filter class \ingroup UNARY
  /** C.E.: **TODO** document this
  
  */
  class LocalThresholdOp : public UnaryOp, public Uncopyable{
    public:
    
    /// create a new LocalThreshold object with given mask-size and global threshold
    /** @param maskSize size of the mask to use for calculations, the image width and
                        height must each be larger than 2*maskSize.
        @param globalThreshold additive Threshold to the calculated reagions mean
        @param gammaSlope gammaSlope (Default=0)
    */
    LocalThresholdOp(unsigned int maskSize=10, int globalThreshold=0, float gammaSlope=0);
    void apply(const ImgBase *src, ImgBase **dst);

    void setMaskSize(unsigned int maskSize);
    void setGlobalThreshold(int globalThreshold);
    void setGammaSlope(float gammaSlope);
    
    unsigned int getMaskSize() const;
    int getGlobalThreshold() const;
    float getGammaSlope() const; 

    private:
    unsigned int m_uiMaskSize;
    int m_iGlobalThreshold;
    float m_fGammaSlope;
    
    ImgBase *m_poROIImage;

    unsigned int m_uiROISizeImagesMaskSize;

    Img32s m_oROISizeImage;
    Img32s m_oIntegralImage;
    Img32f m_oIntegralImageF;
  };
  
}  
#endif
