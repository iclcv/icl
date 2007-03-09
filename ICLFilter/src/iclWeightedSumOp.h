#include <iclImg.h>
#include <iclUnaryOp.h>
#ifndef WEIGHTEDSUMOP_H
#define WEIGHTEDSUMOP_H


namespace icl {

   /// Accumulate weighted pixel values of all image channels
   /** Pixels of all channels in source image are weighted 
       by a channel-wise weight and accumulated to the destination 
       image:
       \f[
       D(x,y,0) = \sum\limits_{c=0}^C S(x,y,c)*w(c) 
       \f]
       where \f$D\f$ is the destination image, \f$S\f$ is the source
       image, \f$w\f$ is the weights vector and \f$C\f$ is the source 
       images channel count.
       The result image is created with depth32f by default. Only if
       the source image has a 64Bit floating point depth (depth64f),
       the destination image is adapted to depth64f to avoid loss of
       precession.
       
       
       Performance: 1000x1000 image with 10 channels (1400Mhz Centrino)
       - icl8u: 82ms
       - icl32f: 96ms
       - icl64f: 170ms
       
       Performance: 1000x1000 image with 3 channels (1400Mhz Centrino)
       - icl8u: 21ms
       - icl32f: 25ms
       - icl64f: 47ms
       
  **/
  class WeightedSumOp : public UnaryOp {
    public:
    /// creates a new WeightedSumOp object
    WeightedSumOp(){}
    
    /// creates an new WeightedSumOp object  with a given weights vector
    /** @param weights channel weights vector 
    **/
    WeightedSumOp(const std::vector<icl64f> &weights):
     m_vecWeights(weights){}
     
    /// applies this operation on the source image
    /** @param poSrc source image
        @param ppoDst destination image (adapted to icl32f by default,
                      if the source image has depth64f, ppoDst is adapted to
                      icl64f too.

    **/
    void apply (const ImgBase *poSrc, ImgBase **ppoDst);
     
    /// returns the current weight vector
    /** @return reference to the current weight vector **/
    const std::vector<icl64f> &getWeights() const { return m_vecWeights; }

    /// sets up the current weights vector
    /** @param weights new weight vector **/
    void setWeights(const std::vector<icl64f> &weights){ m_vecWeights = weights; }

    private:
    /// internal storage for the channel weights
    std::vector<icl64f> m_vecWeights;
  };
  
} // namespace icl

#endif
