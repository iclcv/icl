#ifndef ICLMEDIAN_H
#define ICLMEDIAN_H

#include <ICLBase.h>
#include <ICLFilter.h>

namespace icl{

  /// ICLFilter class that provides median filter abilities
  /** The median class provides the ability for arbitrary
      mask sizes. Although the functionality of the IPP-optimized
      implementation and the fall back C++-implementation is
      identical, the performances are of different orders of 
      magnitude.
      The fallback implementation uses the naive algorithm
      of sorting all N pixelvalues inside the median mask,
      and setting the destination pixelvalue to the N/2-the
      element of the sorted pixel list.
      This algorithm runs in O(w*h*N*log(N)) where (w,h) is the
      size of source images ROI, and N=n*n is the mask size used.
      The following code extract explains the operation of the
      fallback algorithm in ICL-style notation for a single
      channel image, and ICL8u type:
      <pre>

      void channel_median_8u(ICL8u &src, ICL8u &dst, int w, int h, int c)
      {
          std::vector<iclbyte> list;
          
          for(ICL8u::iterator s=src.begin(c), d=dst.begin(c); s.inRegion() ; s++, d++ )
          {
              for(ICL8u::iterator sR(s,w,h); sR.inRegion(); sR++)
              {
                 list.push_back(*sR);
              }
              std::sort(list.begin(),list.end());                                                                          \
              *d = list[w*h/2];
              list.clear();
          }
      }                                                                                                                 \
      </pre>

      The IPP implementation uses a fast median algorithm that
      estimates the median not by sorting a list, but by working
      on a histogramm. Look at the IPPI manual for more detail.
   

      <h2>No IPP for floats</h2>
      This time the IPP supports no median filtering for
      Ipp32f type, so the C++-fallback is used then.


      <h2>Mask-Sizes</h2>
      Although the fallback C++ implementation can work with
      arbitrary mask sizes, the ICLMedian will internally use
      odd mask dimension like 3x3 or 5x7. If an even width or
      height parameter is given to the ICLMedian constructor,
      the next higher odd value is used.

      <h2>Benchmarks</h2>
      <h3>Test A 1000x1000 iclbyte-image with IPP</h3>
      - mask size 3x3 <b>~5ms</b> (highly optimized)
      - mask size 5x5 <b>~32ms</b>
      - mask size 51x51 <b>~434ms</b> (still usable!)
      - mask size 51x3 <b>~38ms</b>
      - mask size 3x51 <b>~430ms</b> (mask height specifies time usage)

      <h3>Test B 1000x1000 iclbyte-image no IPP</h3>
      - mask size 3x3 <b>~146ms</b> (30 times slower)
      - mask size 5x5 <b>~334ms</b>
      - mask size 51x51 <b>~84000ms</b> (unusable!)
      - mask size 51x3 <b>~3500ms</b>
      - mask size 3x51 <b>~3500ms</b> 

      <h3>Test C 1000x1000 iclfloat-image (no IPP for floats)</h3>
      - mask size 3x3 <b>~181ms</b> (no special optimization)
      - mask size 5x5 <b>~464ms</b> 
      - mask size 51x51 <b>~11500ms</b> (unusable!)
      - mask size 51x3 <b>~4600ms</b>
      - mask size 3x51 <b>~4700ms</b> 
      
 

  */
  
  class ICLMedian : public ICLFilter{
    public:
    ICLMedian(int iWidth=3, int iHeight=3);
    ~ICLMedian();
    virtual void apply(ICLBase *poSrc, ICLBase *poDst);
    private:
    int m_iWidth,m_iHeight;
  };
}

#endif
