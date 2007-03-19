#ifndef ICLMEDIAN_H
#define ICLMEDIAN_H

#include <iclNeighborhoodOp.h>
#include <iclArray.h>

namespace icl {

  /// Class that provides median filter abilities
  /** The median class provides the ability for arbitrary
      mask sizes. Although the functionality of the IPP-optimized
      implementation and the fallback C++-implementation is
      identical, the performances are of different orders of 
      magnitude.
      The fallback implementation uses the naive algorithm
      of sorting all N pixel values inside the median mask,
      and setting the destination pixel value to the mid-element
      of the sorted pixel list.
      This algorithm runs in O(w*h*N*log(N)) where (w,h) is the
      size of source images ROI, and N=n*n is the mask size used.
      The following code extract explains the operation of the
      fallback algorithm in Img-style notation for a single
      channel image, and Img8u type (the real implementation
      uses some special optimizations, that are not mentioned
      further):

      <pre>
      void channel_median_8u(Img8u &src, Img8u &dst, int w, int h, int c)
      {
          std::vector<icl8u> list;
          
          for(Img8u::iterator s=src.begin(c), d=dst.begin(c); s.inRegion() ; s++, d++ )
          {
              for(Img8u::iterator sR(s,w,h); sR.inRegion(); sR++)
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
      Currently the IPP supports no 2D-median filtering for
      Ipp32f type, so the C++-fallback is used then.


      <h2>Mask-Sizes</h2>
      Although the fallback C++ implementation can work with
      arbitrary mask sizes, the Median will internally use
      odd mask dimension like 3x3 or 5x7. If an even width or
      height parameter is given to the Median constructor,
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
            <td><b>icl8u, ipp</b></td>    
            <td>~5ms</td>
            <td>~32ms</td>
            <td>~434ms</td>
            <td>~38ms</td>
            <td>~430ms</td>
         </tr><tr> 
            <td><b>icl8u, c++</b></td>  
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
      <h3>Test A 1000x1000 icl8u-image with IPP</h3>
      - mask size 3x3 <b>~5ms</b> (highly optimized)
      - mask size 5x5 <b>~32ms</b>
      - mask size 51x51 <b>~434ms</b> (still usable!)
      - mask size 51x3 <b>~38ms</b>
      - mask size 3x51 <b>~430ms</b> (mask height specifies time usage)

      <h3>Test B 1000x1000 icl8u-image no IPP</h3>
      - mask size 3x3 <b>~146ms</b> (30 times slower)
      - mask size 5x5 <b>~334ms</b>
      - mask size 51x51 <b>~84000ms</b> (unusable!)
      - mask size 51x3 <b>~3500ms</b>
      - mask size 3x51 <b>~3500ms</b> 

      <h3>Test C 1000x1000 icl32f-image (no IPP for floats)</h3>
      - mask size 3x3 <b>~181ms</b> (no special optimization)
      - mask size 5x5 <b>~464ms</b> 
      - mask size 51x51 <b>~115000ms</b> (unusable!)
      - mask size 51x3 <b>~4600ms</b>
      - mask size 3x51 <b>~4700ms</b> 

    <h2>Example</h2>
    Here is an examle, how to use the Median object.

    <pre>
    // create source and destination image
    Img8u src(640,480,3), *poDst=0;
  
    // acquire some image data
    ...
  
    // create the median object
    Median m(Size(5,5));

    // apply the median on the images - first call (slow)
    // destination image is renewed to 640x480x3 (memory allocation)
    m.apply(&src,&poDst);

    // enter iteration loop 
    while(1)
    {
        // aquire some new image data
        ...       

        // apply the median filter (now fast, as no memory
        // allocation must be performed)
        m.apply(&src,&poDst);

        // further processing steps
        ...
    }
    </pre>
  */
  class MedianOp : public NeighborhoodOp {
  public:

    /// Constructor that creates a median filter object, with specified mask size
    /** @param maskSize of odd width and height
        Even width or height is increased to next higher odd value.
    **/
    MedianOp (const Size &maskSize):NeighborhoodOp(adaptSize(maskSize)){}
    
    /// applies the median operation on poSrc and stores the result in poDst
    /** The depth, channel count and size of poDst is adapted to poSrc' ROI:
        @param poSrc  source image
        @param ppoDst pointer to destination image
    **/
    void apply(const ImgBase *poSrc, ImgBase **ppoDst);
    
    
    /// ensures that mask width and height are odd 
    /** This is a workaround, necessary because of an ipp Bug that allows no
        even mask sizes here! 
        @param size size to adjust
        @return adjusted size (width and height are rounded up to the next
                         higher odd value
    **/
    virtual Size adaptSize(const Size &size){
      return Size(1+ 2*(size.width/2),1+ 2*(size.height/2));
    }

    
  };

}
#endif
