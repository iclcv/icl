// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Andre Justus

#pragma once

#include <ICLFilter/BinaryOp.h>
#include <ICLCore/Image.h>
#include <ICLCore/Img.h>
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
    class ProximityOp : public BinaryOp, public utils::Configurable{
      public:
      ProximityOp(const ProximityOp&) = delete;
      ProximityOp& operator=(const ProximityOp&) = delete;


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
      virtual ~ProximityOp();

      /// Applies the proximity operation (IPP only, 8u and 32f; other depths converted to 32f)
      ICLFilter_API void apply(const core::Image &src1, const core::Image &src2, core::Image &dst) override;

      /// import BinaryOp apply overloads
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

      /// internal buffer for converting unsupported source depths to 32f
      core::Img32f *m_poImageBuffer;

      /// internal buffer for converting unsupported template depths to 32f
      core::Img32f *m_poTemplateBuffer;
    };
  } // namespace filter
} // namespace icl
