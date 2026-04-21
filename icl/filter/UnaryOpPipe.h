// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <icl/utils/CompatMacros.h>
#include <icl/filter/UnaryOp.h>
#include <vector>

namespace icl{
  /** \cond */
  namespace core{  class ImgBase; }
  /** \endcond */

  namespace filter{


    /// Utility class that helps applying UnaryOps one after another \ingroup UNARY
    /** Consider a default computer vision system, which has some
        preprocessing steps in the following operation order:
        -# resize the image to a given size
        -# calculate a feature map (e.g. a weighted sum of the color channels)
        -# binarize the feature map
        -# apply some morphological operation on this feature map

        To facilitate this, you can easily use a UnaryOpPipe, which
        internally creates a queue of UnaryOp instances and their
        individual result images (as ImgBase*). Once you have created
        this Pipe under a certain name (e.g. "MyProprocessor") you
        can easily add or remove specific preprocessing steps, without
        having to consider fall-outs elsewhere in your code.

        Take a look on the following example:
        \code
        #include <icl/qt/Quick.h>
        #include <icl/filter/UnaryOpPipe.h>
        #include <icl/filter/ScaleOp.h>
        #include <icl/filter/WeightedSumOp.h>
        #include <icl/filter/UnaryCompareOp.h>
        #include <icl/filter/MorphologicalOp.h>
#include <icl/core/Image.h>

        int main(){
           // create an input image (the nice parrot here!)
           ImgQ inputImage = create("parrot");

           // create a weight vector (used later on by an instance of the WeightedSumOp class)
           vector<icl64f> weights(3); weights[0] = 0.2; weights[1] = 0.5; weights[0] = 0.3;

           // create the empty pipe
           UnaryOpPipe pipe;

           // add the UnaryOp's in the correct order
           pipe << new ScaleOp(0.25,0.25)
                << new WeightedSumOp(weights)
                << new UnaryCompareOp(UnaryCompareOp::gt,110)
                << new MorphologicalOp(MorphologicalOp::erode3x3);

           // apply this pipe on the source image (use the last image as destination)
           const ImgBase *res = pipe.apply(&inputImage);

           // show the result using ICLQuick
           show(cvt(res));
        }
        \endcode
    **/
    class ICLFilter_API UnaryOpPipe : public UnaryOp{
      public:
      /// create an empty pipe
      UnaryOpPipe();

      /// Destructor
      ~UnaryOpPipe();

      /// appends a new op on the end of this pipe (ownership of op and im is passed to the pipe)
      void add(UnaryOp *op, core::ImgBase*im=0);

      /// stream based wrapper for the add function (calls add(op,0))
      /** ownership of op is passed to the pipe*/
      UnaryOpPipe &operator<<(UnaryOp *op){
        add(op); return *this;
      }
      /// applies all ops sequentially
      void apply(const core::Image &src, core::Image &dst) override;
      using UnaryOp::apply;

      /// returns the number of contained ops
      int getLength() const;

      /// returns the op at given index
      UnaryOp *&getOp(int i);

      /// returns the result buffer image at given index
      core::ImgBase *&getImage(int i);

      /// returns the last image (which is not used by default)
      /** This image is only used, if it is given a 2nd parameter to
          the apply function
          \code
          MyPipe.apply(mySourceImagePtr,&(MyPipe.getLastDisplay());
          \endcode
      **/
      core::ImgBase *&getLastDisplay();

      void applyImgBase(const core::ImgBase *, core::ImgBase **);
      private:
      /// Internal buffer of ops
      std::vector<UnaryOp*> ops;

      /// Internal buffer of result images
      std::vector<core::ImgBase*> ims;
    };
  } // namespace filter
}
