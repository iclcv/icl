/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/LUTOp.h                        **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter, Andre Justus                      **
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
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <vector>

namespace icl {
  namespace filter{

    /// class for applying table lookup transformation to Img8u images \ingroup UNARY
    /** \section SEC1 General Information
        The class knows two modes: first: given count of quantization levels,
        it is able to calculate an internal lut for applying a quantization
        on the given src image. The other mode requires a given LUT, that
        is used to apply a LUT-function on the source image.

        \section SEC2 Modes
        The mode, that is currently used depend on the specific constructor,
        that is used to create the LUT object, or on the last setter-function,
        that was called to the object. The current mode can be read out by
        calling the isLUTSet() or isLevelsSet() getter functions. These two
        functions return values exclude each others.

        \section SEC3 Datatypes
        The LUT function is only implemented for Img8u images with range [0,255].
        Other image depths are emulated by converting the given image into an
        internal Img8u buffer.

        \section SEC4 Static functions
        For a fast access two additional static functions are provided - one
        for each mode. This functions are only available for depth8u images.

        \section SEC5 IPP
        Yet, only the reduceBits function and therewith the according LUT-objects
        mode with given count of quantization levels is IPP optimized.
    */
    class ICLFilter_API LUTOp : public UnaryOp, public utils::Uncopyable{
     public:
       /// creates a LUT object with given lut (LUT-mode)
       /** @param lut LUT-vector to use */
       LUTOp(const std::vector<icl8u> &lut);

       /// creates a LUT object with given count of quatization levels (Levels-mode)
       /** @param quantizationLevels count of quanzation levels to use*/
       LUTOp(icl8u quantizationLevels=255);

       /// destructor
       virtual ~LUTOp(){}

       /// Common Filter apply function using current mode
       /** @param src source image
           @param dst destination image**
       */
       virtual void apply(const core::ImgBase *src, core::ImgBase **dst);

       /// Import unaryOps apply function without destination image
       using UnaryOp::apply;

       /// simple lut transformation dst(p) = lut(src(p))
       /** @param src source image
           @param dst destination image
           @param lut lut-vector to used
       */
       static void simple(const core::Img8u *src, core::Img8u *dst, const std::vector<icl8u>& lut);

       /// specialization of a lut transformation to reduce the number colors levels image a given image
       /** @param src source image
           @param dst destination image
           @param levels count of quantization levels to use
       */
       static void reduceBits(const core::Img8u *src, core::Img8u *dst, icl8u levels);

       /// sets the current lut and switches to the lut-mode
       /** @param lut new lut vector to use*/
       void setLUT(const std::vector<icl8u> &lut);

       /// sets current count of quatization levels and switches to the levels-mode
       /** @param levels new count of quantization levels*/
       void setQuantizationLevels(int levels);

       /// returns the current count of quatization levels or 0 if current mode is lut-mode
       icl8u getQuantizationLevels() const;

       /// return the current used lut, of a 0-sized vector if current mode is levels-mode
       const std::vector<icl8u> &getLUT() const;

       /// retruns whether current mode is lut-mode
       bool isLUTSet() const;

       /// retruns whether current mode is levels-mode
       bool isLevelsSet() const;

       private:
       bool m_bLevelsSet;
       bool m_bLutSet;
       std::vector<icl8u> m_vecLUT;
       icl8u m_ucQuantizationLevels;
       core::Img8u *m_poBuffer;

     };
  } // namespace filter
} // namespace icl

