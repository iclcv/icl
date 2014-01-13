/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/MedianOp.h                     **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Robert Haschke, Andre Justus,     **
**          Sergius Gaulik                                         **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLFilter/NeighborhoodOp.h>

namespace icl {
  namespace filter{
  
    /// Class that provides median filter abilities \ingroup UNARY \ingroup NBH
    /** The median class provides the ability for arbitrary
        mask sizes. Although the functionality of the IPP-optimized
        implementation and the fallback C++-implementation is
        identical, the performances are of different orders of 
        magnitude.
        Fallback-functions for masks with dimensions 3x3 and 5x5
        are optimizied with SSE-instructions, but the computation time
        for images of types Img8u and Img16s is still greater than the one of
        IPP-implementations. However SSE-implementations show a better
        performance for the type icl32f.
        Here the algorithm just takes the median using min and max
        functions.
        Images of the type Img8u and Img16s with other mask sizes
        are processed by the algorithm, that was introduced by
        Thomas S. Huang. With the help of a histogram the algorithm
        runs in O(n).
        For all other types a trivial implementation was designed to
        calculate the median values. It uses the naive algorithm
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
     
  
        <h2>IPP for floats</h2>
        Currently the IPP implementation of 2D-median filtering for
        Ipp32f type is slower than the C++-implementation,
        so the C++-fallback is always used then.
  
  
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
              <td>~1.8ms</td>
              <td>~2.6ms</td>
              <td>~292ms</td>
              <td>~32.4ms</td>
              <td>~328ms</td>
           </tr><tr> 
              <td><b>icl8u, c++</b></td>  
              <td>~2.6ms</td> 
              <td>~11.7ms</td>
              <td>~323ms</td>
              <td>~350ms</td>
              <td>~47ms</td> 
           </tr><tr>  
              <td><b>icl16s, ipp</b></td>    
              <td>~2.7ms</td>
              <td>~4ms</td>
              <td>~21100ms</td>
              <td>~338ms</td>
              <td>~420ms</td>
           </tr><tr> 
              <td><b>icl16s, c++</b></td>  
              <td>~4.2ms</td> 
              <td>~16.2ms</td>
              <td>~370ms</td>
              <td>~384ms</td>
              <td>~90ms</td> 
           </tr><tr> 
              <td><b>float, ipp</b></td>  
              <td>~25ms</td> 
              <td>~257ms</td>  
              <td>~?ms</td>
              <td>~52000ms</td>
              <td>~49000ms</td> 
           </tr><tr> 
              <td><b>float, c++</b></td>  
              <td>~11.6ms</td> 
              <td>~39ms</td>  
              <td>~217000ms</td>
              <td>~8700ms</td>
              <td>~8700ms</td> 
           </tr>
        </table>
  
        <h3>Details</h3>
        <h3>Test A 1000x1000 icl8u-image with IPP</h3>
        - mask size 3x3 <b>~1.8ms</b> (highly optimized)
        - mask size 5x5 <b>~2.6ms</b>
        - mask size 51x51 <b>~292ms</b> (still usable!)
        - mask size 51x3 <b>~32.4ms</b>
        - mask size 3x51 <b>~328ms</b> (mask height specifies time usage)
  
        <h3>Test B 1000x1000 icl8u-image no IPP</h3>
        - mask size 3x3 <b>~2.6ms</b> (less than 1.5 times slower)
        - mask size 5x5 <b>~11.7ms</b> (about 4 times slower)
        - mask size 51x51 <b>~323ms</b> (IPP is not much faster)
        - mask size 51x3 <b>~350ms</b> (mask width specifies time usage)
        - mask size 3x51 <b>~47ms</b> 
  
        <h3>Test A 1000x1000 icl8u-image with IPP</h3>
        - mask size 3x3 <b>~2.7ms</b> (highly optimized)
        - mask size 5x5 <b>~4ms</b>
        - mask size 51x51 <b>~21100ms</b> (unusable!)
        - mask size 51x3 <b>~338ms</b>
        - mask size 3x51 <b>~420ms</b>
  
        <h3>Test B 1000x1000 icl16s-image no IPP</h3>
        - mask size 3x3 <b>~4.2ms</b> (1.5 times slower)
        - mask size 5x5 <b>~16.2ms</b> ( 4 times slower)
        - mask size 51x51 <b>~370ms</b> (IPP is not much faster)
        - mask size 51x3 <b>~384ms</b> (mask width specifies time usage)
        - mask size 3x51 <b>~90ms</b> 
  
        <h3>Test C 1000x1000 icl32f-image with IPP</h3>
        - mask size 3x3 <b>~25ms</b> 
        - mask size 5x5 <b>~257ms</b> 
        - mask size 51x51 <b>~?ms</b> (calculation time is to great)
        - mask size 51x3 <b>~52000ms</b> (unusable!)
        - mask size 3x51 <b>~49000ms</b> (unusable!)

        <h3>Test C 1000x1000 icl32f-image no IPP</h3>
        - mask size 3x3 <b>~11.6ms</b> (sse optimization)
        - mask size 5x5 <b>~39ms</b> (sse optimization)
        - mask size 51x51 <b>~217000ms</b> (unusable!)
        - mask size 51x3 <b>~8700ms</b>
        - mask size 3x51 <b>~8700ms</b> 
  
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
    class ICLFilter_API MedianOp : public NeighborhoodOp {
    public:
  
      /// Constructor that creates a median filter object, with specified mask size
      /** @param maskSize of odd width and height
          Even width or height is increased to next higher odd value.
      **/
      MedianOp (const utils::Size &maskSize):NeighborhoodOp(adaptSize(maskSize)){}
      
      /// applies the median operation on poSrc and stores the result in poDst
      /** The depth, channel count and size of poDst is adapted to poSrc' ROI:
          @param poSrc  source image
          @param ppoDst pointer to destination image
      **/
      void apply(const core::ImgBase *poSrc, core::ImgBase **ppoDst);
      
      /// Import unaryOps apply function without destination image
      using NeighborhoodOp::apply;
  
      /// ensures that mask width and height are odd 
      /** This is a workaround, necessary because of an ipp Bug that allows no
          even mask sizes here! 
          @param size size to adjust
          @return adjusted size (width and height are rounded up to the next
                           higher odd value
      **/
      virtual utils::Size adaptSize(const utils::Size &size){
        return utils::Size(1+ 2*(size.width/2),1+ 2*(size.height/2));
      }
  
      
    };
  
  } // namespace filter
}
