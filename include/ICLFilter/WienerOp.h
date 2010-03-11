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

#ifndef WIENER_OP_H
#define WIENER_OP_H

#include <ICLFilter/NeighborhoodOp.h>
#include <vector>

namespace icl {
  /// Class for Wiener Filter \ingroup UNARY \ingroup NBH
   /** Wiener filters are commonly used in image processing applications to
       remove additive noise from degraded images, to restore a blurred image.
       
       The following operation is performed on each pixel:
       \f[
       R(x,y,c) = \mu_m(x,y,c) + \frac{\sigma_m^2(x,y,c)-\nu^2}{\sigma^2} * (S(x,y,c) - \mu_m(x,y,c))
       \f]
       
       where: 
       - \f$R(x,y,c)\f$ is the result image at position (x,y) and channel c
       - \f$\mu_m(x,y,c)\f$ is the mean of the image in region m (mask) centered at (x,y), channel c
       - \f$\sigma^2_m(x,y,c)\f$ is the variance of the image in region m (mask) centered at (x,y), channel c
       - \f$\sigma^2 \f$ is the image variance
       - \f$S(x,y,c)\f$ is the source image at position (x,y) and channel c
   */
  class WienerOp : public NeighborhoodOp {
   public:

    /// Constructor that creates a wiener filter object, with specified mask size
    /** @param maskSize of odd width and height
        Even width or height is increased to next higher odd value.
        @param noise nois factor
    **/
    WienerOp (const Size &maskSize, icl32f noise=0): NeighborhoodOp(maskSize),m_fNoise(noise){}

    /// Filters an image using the Wiener algorithm.
    /** @param poSrc Source image
        @param ppoDst Destination image
    **/
    void apply (const ImgBase *poSrc, ImgBase **ppoDst);

    /// Import unaryOps apply function without destination image
    NeighborhoodOp::apply;

    /// returns the current noise factor
    /** @return current noise factor **/
    icl32f getNoise() const { return m_fNoise; }
    
    /// sets up a new noise factor
    /** @ param noise new noise factor **/
    void setNoise(icl32f noise) { m_fNoise = noise; }
    
    private:
    /// internal buffer for applying the wiener operation
    std::vector<icl8u> m_vecBuffer;
    
    /// internal storage for the current noise factor
    icl32f m_fNoise;
  };
} // namespace icl

#endif // WIENER_H
