/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ProximityOp.h                  **
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

#include <ICLFilter/BinaryOp.h>
#include <ICLFilter/NeighborhoodOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Configurable.h>

namespace icl {
  namespace filter{

    /// Class for computing proximity measures  \ingroup BINARY
    /** (Only available for Img8u and Img32f, IPP only!)
        \section OV Overview (taken from the IPPI-Manual)

        "The functions described in this section compute the proximity (similarity) measure between an
        image and a template (another image). These functions may be used as feature detection functions,
        as well as the components of more sophisticated techniques.
        There are several ways to compute the measure of similarity between two images. One way is to
        compute the Euclidean distance, or sum of the squared distances (SSD), of an image and a
        template. The smaller is the value of SSD at a particular pixel, the more similarity exists between
        the template and the image in the neighborhood of that pixel."

        The ProximityOp class summarizes these image similarity measurement techniques
        and provides their functionality by implementing the ICLFilter packages BinaryOp
        interface.\n
        There are two different variables, that influencing the internal functionality of
        the ProximityOps apply function.

        \section AM ApplyMode
        The first variable - the so called "applymode" - determines in which region of the
        source image a specific proximity measure is applied. The following ASCII image
        describes the differences between the values "full", "valid" and "same"

        <pre>
        Image: iiiiiiiiiiiiii   Mask: mmmmm       resulting images:
               iiiiiiiiiiiiii         mmxmm
               iiiiiiiiiiiiii         mmmmm       +---+
               iiiiiiiiiiiiii        (5x3)        |   | := original image area
               iiiiiiiiiiiiii                     +---+
                (14 x 5)

        full: mmmmm                               full result:
              mmx<--- fist pos.                      rrrrrrrrrrrrrrrr
              mmmmmiiiiiiiiiiiii                     r+------------+r
                  iiiiiiiiiiiiii                     r|rrrrrrrrrrrr|r
                  iiiiiiiiiiiiii                     r|rrrrrrrrrrrr|r
                  iiiiiiiiiiiiii                     r|rrrrrrrrrrrr|r
                  iiiiiiiiiiiiimmmmm                 r+------------+r
                               mmx<-- last pos.      rrrrrrrrrrrrrrrr
                               mmmmm
        same:                                      same result:
                mmmmm
         first: mmxmmiiiiiiiiiii                     +------------+
                mmmmmiiiiiiiiiii                     |rrrrrrrrrrrr|
                  iiiiiiiiiiiiii                     |rrrrrrrrrrrr|
                  iiiiiiiiiiimmmmm                   |rrrrrrrrrrrr|
                  iiiiiiiiiiimmxmm <-- last          +------------+
                             mmmmm

        valid:                                     valid result:
                  mmmmmiiiiiiiii                     +------------+
           first: mmxmmiiiiiiiii                     | rrrrrrrrrr |
                  mmmmmiiiimmmmm                     | rrrrrrrrrr |
                  iiiiiiiiimmxmm  <-- last           | rrrrrrrrrr |
                  iiiiiiiiimmmmm                     +------------+


        </pre>

        \section OP Operation Type
        This time three different metrics for the similarity measurements
        are implemented (IPP Only)

        The formulas can be found in the ippi-manual!

        optypes:
        - sqrDistance
        - crossCorr
        - crossCorrCoeff


    */
    class ProximityOp : public BinaryOp, public utils::Uncopyable, public utils::Configurable{
      public:

      /// enum to specify the current apply mode of a ProximityOp
      /** @see ProximityOp */
      enum applymode{
        full, /**< destination image has size (w1+w2-1)x(h1+h2-1) */
        same, /**< destination image has size (w1)x(h1)           */
        valid /**< destination image has size (w1-w2+1)x(h1-h2+1) */
      };

      /// enum to specify the current operation type of a ProximityOp
      /** @see ProximityOp */
      enum optype{
        sqrDistance,   /**< square distance metric               */
        crossCorr,     /**< cross correlation metric             */
        crossCorrCoeff /**< cross correlation coefficient metric */
      };

      /// Creates a new ProximityOp object with given apply mode and optype
      /** @param ot optype for the ProximityOp
          @param am apply mode for the ProximityOp (default = "valid")
      **/
      ICLFilter_API ProximityOp(optype ot, applymode am=valid);

      /// Destructor
      virtual ~ProximityOp(){
        if(m_poImageBuffer) delete m_poImageBuffer;
        if(m_poTemplateBuffer) delete m_poTemplateBuffer;
      }

      /// applies the current op given source image, template and destination image
      /** allowed input image types are icl8u and icl32f other types are converted internally
          to float images. The destination image is adapted automatically; it depth becomes
          depth32f.
          @param poSrc1 source image
          @param poSrc2 template
          @param ppoDst destination image (apated automatically)
      **/
      ICLFilter_API virtual void apply(const core::ImgBase *poSrc1, const core::ImgBase *poSrc2, core::ImgBase **ppoDst);

      /// import apply symbol from parent class
      using BinaryOp::apply;

      /// sets the current optype
      /** @param ot new optype **/
      ICLFilter_API void setOpType(optype ot);

      /// sets the current applymode
      /** @param am new applymode value **/
      ICLFilter_API void setApplyMode(applymode am);

      /// returns the current optype
      /** @return current optype **/
      ICLFilter_API optype getOpType() const;

      /// returns the current applymode
      /** @return current applymode **/
      ICLFilter_API applymode getApplyMode() const;

      private:

      /// internal used buffer for handling unsupported formats
      core::Img32f *m_poImageBuffer;

      /// internal used buffer for handling unsupported formats
      core::Img32f *m_poTemplateBuffer;
    };
  } // namespace filter
} // namespace icl
