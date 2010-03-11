/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
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

#ifndef ICL_CONVOLUTIONOP_H
#define ICL_CONVOLUTIONOP_H

#include <ICLFilter/NeighborhoodOp.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLFilter/ConvolutionKernel.h>

namespace icl {
  
  /// Class for Image convolutions 
  /** (Img8u, Img32f: IPP + Fallback, all other Types: Fallback only!) \ingroup UNARY \ingroup NBH
  The ConvolutionOp class provides functionality for any kind of convolution
  filters. As most other filter operations, it operates on the source images
  ROI only. Because the filter mask has to fit into the image at every point
  of the ROI, the ROI is eventually <em>shrinked</em> as described in base
  class Filter.

  <h2>Performance (IPP-Optimized)</h2>

  All possible filter operations can be divided in 4 cases, depending on the
  source and destination images depths and the depth of the used filter
  kernel.  While all image depths are supported, the only available kernel 
  depths are depth32f (floating point) and depth32s (32-bit signed integer)
  Note the differences of the following cases:
  
  <h3>case images: depth8u, depth16s, depth32s </h3>
  In this case, an integer kernel is preferred. That means, that an integer
  kernel will be used, if available. Using a float kernel causes a marginal
  decline in performance.

  <h3>case images: depth32f, depth64f </h3>
  In this case, a float kernel is preferred. If it is not available, the
  fallback integer-kernel must be used. As convolution operations of float
  images with integer kernels are not supported by the IPP, the kernel is
  converted internally into a float-kernel. 

  <h3>Benchmarks</h3>
  The IPP-optimized functions are <b>VERY</b> fast in comparison to the 
  fallback C++ implementations. The optimized 3x3 convolution functions
  provided by the IPP are more then 20 times faster. Here are some benchmarks:
  - arbitrary 3x3-convolution 1000x1000 single channel image (IPP-OPTIMIZED)
     - icl8u images & int kernel <b>~11.6ms</b>
     - icl32f images & int kernel <b>~11.1ms</b>
     - icl8u images & icl32f kernel <b>~13.5ms</b>
     - icl32f-image & icl32f kernel <b>~11.3ms</b>
  - fixed 3x3 convolution 1000x1000 single channel sobelx (IPP-OPTIMIZED)
     - icl8u images & int mask <b>~4ms (!!!)</b>
     - icl8u images & icl32f mask <b>~8ms (!!!)</b>
  - arbitrary 3x3-convolution 1000x1000 single channel image (C++-Fallback)
     - icl8u images & int kernel <b>~56ms</b> (further implem. ~81ms) ???
     - icl32f images & int kernel <b>~76ms</b> (further implem. ~370ms)
     - icl8u images & icl32f kernel <b>~135ms</b> (further implem. ~230ms)
     - icl32f-image & icl32f kernel <b>~60ms</b> (further implem. ~60ms)
  
  <h2>Buffering Kernels</h2>
  In some applications the ConvolutionOp object has to be created
  during runtime. If the filter-kernel is created elsewhere, and it
  is persistent over the <i>lifetime</i> of the ConvolutionOp object,
  it may not be necessary to deeply copy the kernel into an internal buffer
  of the ConvolutionOp object. To make the ConvolutionOp object just using a
  given kernel pointer, an additional flag <b>iBufferData</b> can be set
  in two Constructors.
  */

  class ConvolutionOp : public NeighborhoodOp, public Uncopyable{
    public:

    /// Default constructor (force unsigned is set to false)
    ConvolutionOp(const ConvolutionKernel &kernel=ConvolutionKernel());

    /// create with optional force unsigned output flag
    ConvolutionOp(const ConvolutionKernel &kernel, bool forceUnsignedOutput);

    /// performs the convolution operation on the image
    /** The destination image is automatically set up to correct size and its
        channel count is set to the source images channel count.  
        @param src  source image
        @param dst destination image
    */
    void apply(const ImgBase *src, ImgBase **dst);
    
    /// Import unaryOps apply function without destination image
    NeighborhoodOp::apply;
    
    /// change kernel
    void setKernel (const ConvolutionKernel &kernel){ m_kernel = kernel; }
    
    /// returns currently used kernel (const)
    const ConvolutionKernel &getKernel() const { return m_kernel; }
    
    ///  returns currently used kernel (const)
    ConvolutionKernel &getKernel() { return m_kernel; }

    private:
    ConvolutionKernel m_kernel;
    bool m_forceUnsignedOutput;
  };
 
}
#endif
