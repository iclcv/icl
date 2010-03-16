/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/IntegralImgOp.h                      **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#ifndef INTEGRALIMG_OP_H
#define INTEGRALIMG_OP_H

#include <ICLCore/Img.h>
#include <vector>
#include <ICLFilter/UnaryOp.h>

namespace icl{
  /// class for creating integral images \ingroup UNARY
  /** 
  The integral image A of an image a (introduced by Viola & Jones (2001) in the
  paper : "Rapid Object Detection using a Boosted Cascade of Simple Features" )
  defined as follows:
  \f[
  A(i,j) = \sum\limits_{x=0}^i \sum\limits_{y=0}^j  a(i,j)
  \f]
  It can be calculated incrementally using the following equation:
  \f[
  A(i,j) = a(i,j)+A(i-1,j)+A(i,j-1)-A(i-1,j-1)
  \f]
  <h3>illustration:</h3>
  <pre>
    +++++..
    +++CA..
    +++BX..
    .......
    X = src(x) + A + B - C
  </pre>

  <h1>Implementation</h1>
  <h3>Other definitions</h3>
  There are alternative definitions (like the ipp): 
   \f[
  A(i,j) = \sum\limits_{x=0}^{i-1} \sum\limits_{y=0}^{j-1}  a(i,j)
  \f]
  From this it follows, that the first integral image row and colum is filled with
  zeros and the internal image grows by one pixel in each dimension, with the benefit
  of some better usability of the result image. We use the above definition, which
  is a bit more convenient for the most common applications in our sight.
  
  <h3>functions and borders</h3>
  The provided functions offer the ability for the definition of an optional 
  <em>borderSize</em> parameter, which is 0 by default. If it is set, The result image
  is enfolded with a border. The border pixels are filled with values as if the result
  image would be 0 there. More precisely, upper and left border pixels are set to zero
  and lower and right border pixels are a copy of the nearest <em>real</em> pixel
  like a <em>replicated border</em>.
  An application of the border-mechanism is the local threshold algorithm.

  <h1>Supported Type-Combinations</h1>
  The templates are instantiated for:
  - Img8u -> int (IPP-OPTIMIZED)
  - Img8u -> icl32f (IPP-OPTIMIZED)
  - Img32f -> icl32f
  
  <h1>Performance</h1>
  The calculation is very fast! The IPP is about twice as fast as the C++ code.
  As Example: Img8u 640x480, 1-channel, borderSize = 10, 1.4MHz Centrino: 
  - no ipp: approx. 4.4ms (inclusive memory allocation!!)
  - with ipp: approx. 2.8ms (inclusive memory allocation!!)

  The fallback implementations performance could be optimized by using the algorithm
  proposed by Viola & Jones:
  first step: percalculate a row-sum image  
  \f[
  s(x,y) = s(x-1,y) + a(x,y) \;\;with\; s(-1,y)=0
  \f]
  second step: calculate the integral image
  \f[
  A(x,y) = s(x-1,y) + A(x,y-1) \;\;with\; A(x,-1)=0
  \f]
  But since it is a fallback implementation, this optimization is not yet implemented 
  
  */
  class IntegralImgOp : public UnaryOp{
    public:
    /// Constructor
    /** @param borderSize the border size
        @param integralImageDepth the depth of the integralImage (depth8u etc)
    */
    IntegralImgOp(depth integralImageDepth=depth32s);

    /// Destructor
    ~IntegralImgOp();
    
    /// sets the depth of the integralImage (depth8u etc)
    /** @param integralImageDepth the depth of the integralImage (depth8u etc)
    */
    void setIntegralImageDepth(depth integralImageDepth);
    
    /// returns the depth of the integralImage
    /** @return the depth of the integralImage
    */
    depth getIntegralImageDepth() const;
    
    /// applies the integralimage Operaor
    /** @param posrc The source image
      @param ppoDst Pointer to the destination image
    */
    void apply(const ImgBase *src, ImgBase **dst); 

    /// Import unaryOps apply function without destination image
    UnaryOp::apply;
    
    private:
    depth m_integralImageDepth; //!< destination depth
    ImgBase *m_buf; //!< used only if IPP is available
  };
}

#endif
