#ifndef WEIGHTEDSUM_H
#define WEIGHTEDSUM_H

#include "Filter.h"
#include "Img.h"
namespace icl{
    /// Class for weighted sum
    /** Summarize pixel values of all channels of one image
       for every channel, its pixel values are multiplied with an corresponding weight 
       The Result is written into an 1-Channel Img32f 
    */

  class WeightedSum : public Filter{
  public:
/// compute the weighted sum
    void ws (ImgI *poSrc, ImgI **ppoDst, std::vector<float> weights);
/// compute the weighted sum (Img32f version)
    void ws (Img32f *src, Img32f *dst,  const std::vector<float>& weights);
/// compute the weighted sum, the Img8u is internaly converted to an Img32f  (Img8u version)
    void ws (Img8u *src, Img32f *dst,  const std::vector<float>& weights);

  private:
  Img32f m_oDepthBuf;
  Img32f m_oAccuBuf;
  };
} // namespace icl

#endif
