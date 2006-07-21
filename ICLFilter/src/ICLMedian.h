#ifndef ICLMEDIAN_H
#define ICLMEDIAN_H

#include <ICLBase.h>
#include <ICLFilter.h>

namespace icl{

  /// Class that provides median filter abilities
  /** The median class provides the ability for arbitrary
      mask sizes. Although the functionality of the IPP-optimized
      implementation and the fall back C++-implementation is
      identical, the performances are of different orders of 
      magnitude.
      The fallback implementation uses the naive algorithm
      of sorting all N pixel values inside the median mask,
      and setting the destination pixel value to the N/2-the
      element of the sorted pixel list.
      This algorithm runs in O(w*h*N*log(N)) where (w,h) is the
      size of source images ROI, and N=n*n is the mask size used.
      The following code extract explains the operation of the
      fallback algorithm in ICL-style notation for a single
      channel image, and ICL8u type (the real implementation
      uses some special optimizations, that are not mentioned
      further):
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
              std::sort(list.begin(),list.end());
              *d = list[w*h/2];
              list.clear();
          }
      }
      </pre>

      The IPP implementation uses a fast median algorithm that
      estimates the median not by sorting a list, but by working
      on a histogram. Look at the IPPI manual for more detail.
   

      <h2>No IPP for floats</h2>
      This time the IPP supports no 2D-median filtering for
      Ipp32f type, so the C++-fallback is used then.


      <h2>Mask-Sizes</h2>
      Although the fallback C++ implementation can work with
      arbitrary mask sizes, the ICLMedian will internally use
      odd mask dimension like 3x3 or 5x7. If an even width or
      height parameter is given to the ICLMedian constructor,
      the next higher odd value is used.

      <h2>Benchmarks</h2>
      <h3>table</h3>
      <table>
         <tr>  
            <td></td>
            <td><b>3x3</b></td>
            <td><b>5x5</b></td>
            <td><b>51x51</b></td> 
            <td><b>51x3</b></td>
            <td><b>3x51</b></td> 
         </tr><tr>  
            <td><b>iclbyte, ipp</b></td>    
            <td>~5ms</td>
            <td>~32ms</td>
            <td>~434ms</td>
            <td>~38ms</td>
            <td>~430ms</td>
         </tr><tr> 
            <td><b>iclbyte, c++</b></td>  
            <td>~146ms</td> 
            <td>~334ms</td>
            <td>~84000ms</td>
            <td>~3400ms</td>
            <td>~3500ms</td> 
         </tr><tr> 
            <td><b>float, c++</b></td>  
            <td>~181ms</td> 
            <td>~464ms</td>  
            <td>~115000ms</td>
            <td>~4600ms</td>
            <td>~4700ms</td> 
         </tr>
      </table>

      <h3>Details</h3>
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
      - mask size 51x51 <b>~115000ms</b> (unusable!)
      - mask size 51x3 <b>~4600ms</b>
      - mask size 3x51 <b>~4700ms</b> 

    <h2>Example</h2>
    Here is an examle, how to use the ICLMedian object.

    <pre>
    // create source and destination image
    ICL8u src(640,480,3), dst;
  
    // acquire some image data
    ...
  
    // create the median object
    ICLMedian m(5,5);

    // apply the median on the images - first call (slow)
    // source image is renewed to 640x480x3 (memory allocation)
    m.apply(&src,&dst);

    // enter iteration loop 
    while(1)
    {
        // aquire some new image data
        ...       

        // apply the median filter (now fast, as no memory
        // allocation must be performed)
        m.apply(&src,&dst);

        // further processing steps
        ...
    }
    </pre>
  */
  class ICLMedian : public ICLFilter{
    public:

    /// Constructor that creates a median filter object, with specified mask size
    /** @param iWidth width of mask to use (if even, the iWidth+1 is used)
        @param iHeight height of mask to use (if even, the iHeight+1 is used)
    */
    ICLMedian(int iWidth=3, int iHeight=3);

    /// Destructor
    virtual ~ICLMedian();
    
    /// applies the median operation on poSrc and stores the result in poDst
    /** poDst and poSrc must be of the same depth - the programm will exit with
        -1 if not. The size and channel count of poDst is adapted to 
        work correctly together with the kernel size, and the source images 
        ROI:
        - channel count of poDst is set to the channel count of poSrc
        - width of poDst is set to W-MW/2 where W is the ROI width of poSrc
          and MW is the used mask width.
        - height of poDst is set to H-MH/2 where H is he ROI height of poSrc
          and MH is the used mask height.
        
        To avoid problems with the execution of the median operations on the
        src image, the source images ROI is <i>eroded</i> by MW/2 pixel on
        both sides and MH/2 pixels on top and bottom, before the function call
        and <i>dilated</i> afterwards.

        @param poSrc source image
        @param poDSt destination image
    */
    virtual void apply(ICLBase *poSrc, ICLBase *poDst);
    private:

    /// internal storage for the mask size
    int m_iWidth,m_iHeight;
  };
}

#endif
