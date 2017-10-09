/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ChamferOp.h                    **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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
#include <ICLUtils/Point.h>
#include <ICLCore/Img.h>
#include <ICLFilter/UnaryOp.h>
#include <vector>

namespace icl{
  namespace filter{
    /// Chamfering Unit \ingroup UNARY
    /** Chamfering is a procedure called Euclidean-Distance-Transformation (EDT).
        Input of the Chamfering operation is a binary image. The Chamfering operation
        creates a map (also called the <em>voronoi surface</em>) which's
        value at location (x,y) is the distance to the nearest white pixel
        to the pixel at (x,y) in the image.\n
        Because the calculation of the <em>real</em> euclidean distance to the next white pixel
        is very expensive \f$ O(number\,of\,white\,pixels^2)\f$, an approximation of the euclidean
        distance is calculated instead.\n
        A good approximation can be obtained by moving a small (3x2)-mask successively
        over the image in two cycles, where each pixel is assigned to the minimum of all
        pixels values in the 3x2-neighborhood plus a distance value which approximates the
        distance to a neighborhood pixel. The following pseudo-code illustrates the
        chamfering algorithm:
        \code

        INPUT  := I   // image
        OUTPUT := C   // chamfer-image

        // step 1 prepare chamfer image
        D1 := "distance between horizontally or vertically adjacent pixels" (e.g. 3)
        D2 := "distance between diagonally adjacent pixels" ( e.g. 4)
        MAX_DISTANCE_VALUE := (I.width+I.height)*max(D1,D2)

        for all pixel (x,y) of I do
           if I(x,y) == 255 then
              C(x,y) = 0   // distance is null there as it is white
           else
              C(x,y) = MAX_DISTANCE_VALUE
           endif
        endfor

        // step 2 forward loop
        w := C.width
        h := C.height
        for x = 1 to w-2 do
            for y = 1 to h-1 do
                 C(x,y) = min( C(x,y), C(x-1,y)+d1 , C(x-1,y-1)+d2 , C(x,y-1)+d1 , C(x+1,y-1)+d2 )
            endfor
        endfor


        // step 3 backward loop
        for x = w-2 to 1 do
            for y = h-2 to 0 do
                 C(x,y) = min( C(x,y) , C(x+1,y)+d1 , C(x+1,y+1)+d2 , C(x,y+1)+d1 , C(x-1,y+1)+d2 )
            endfor
        endfor


        // step 4 finalize borders ( this is a performance approximation here !)
        // performs a "replicate-border" call to C with a ROI which is one pixel
        // smaller then the whole image to each direction
        h1 := h-1;
        h2 := h1-1;
        w1 := w-1;
        w2 := w1-1;

        for y = 0 to h-1 do
            C(0,y) = C(1,y);
            C(w1,y) = C(w2,y) ;
        endfor
        for x = 0 to w-1 do
            C(x,0) = C(x,1);
            C(x,h1) = C(x,h2) ;
        endfor

        // done: C contains an approximation of the voronoi surface of the
        // input image I
        return C
        \endcode
        \section BENCH Benchmarks

        The chamfering operation complexity is linear to pixel count of an
        image and it does not change with different input image types:

        Image-size 640x480 single channel (Pentium M 1.6GHz)
        - scaleFactor 1:   approx. 20ms
        - scaleFactor 2:   approx. 5.5ms (with up-scaling 14ms)
        - scaleFactor 4:   approx. 2ms (with up-scaling 11ms)
        - scaleFactor 8:   approx. 1ms (with up-scaling 10ms)

        \section RS ROI support
        Currently image ROIs are supported, but the chamfering operation will
        perform step 4 of the above presented algorithm with the image ROI Rect
        if there is a ROI defined on I.

        \section SCALE Explicit scaling
        A new feature of the ChamferOp class can be activated using the
        "scaleFactor" argument of the class constructor. If the given scale-factor is
        greater then 1, the chamfering operation is applied only on an image,
        that is scaled down by that factor. The last constructor argument
        (scaleUpResult) can be used to define whether to scale up the resulting
        chamfer Image (using NN-Interpolation this time), or to let the result image
        become smaller (by the scale-factor) then the input image.\n
        Internally the down-scaling is not actually performed, but it is emulated
        by iterating creating a downscaled chamfer image (Step 1 in the algorithm above).
        By this means, a scaleFactor of 2 (in x and y direction) provides nearly
        400% of the computation speed compared to scale factor 1.\n
        The "scaleUpResult" Feature, however slows down the performace again, as the
        image scaling operation is more expensive too. (@see BENCH)

        \section Hausdorff-Distance
        The Hausdorff-Distance is a metric to measure the similarity of two point
        sets \f$A=\{a_1,...,a_n\}\f$ and \f$B=\{b_1,...,b_m\}\f$. It is defined
        by:
        \f[
        H(A,B) := max ( h(A,B), b(B,A) )
        \f]
        where
        \f[
        h(A,B) := \max\limits_{a_i \in A} \min\limits_{b_j \in B} || a - b ||
        \f]
        and \f$ || . || \f$ is some underlying norm of the points of A and B (e.g. the Euclidean norm)
        <b>\f$H(A,B)\f$ is often called the symmetric Hausdorff-Distance and
        \f$h(a,b)\f$ is often called the directed Hausdorff-Distance</b>.\n
        The implementation of the Hausdorff-Distance measurement refers to
        the chamfering algorithm to calculate point the "min" distances in
        constant time.

        \section FU Hausdorff-Distance measurement functions
        In addition to the Chamfering-operation provided by the ChamferOp class
        as an implementation of the UnaryOp interface, the class provides some
        static functions to calculate the Hausdorff-Distance. Point-sets can here
        be defined as a std::vector<Point> or directly as a <em>chamfer'd</em> image,
        which can increase calculation performance when comparing one <em>Base</em>-
        image (chamfered once) with many models.

        \section HP Hausdorff-Distance ROI penalties
        When comparing images using the Hausdorff-Distance, sometimes a model, represented by
        a point set \f$A=\{a_1,...,a_n\}\f$ must be compared with an image that is represented
        by a chamfering image \f$I(x,y)\f$. If the image has no full ROI, it is not clear
        what to do with model points \f$a_i\f$ that are inside of the image rect but outside the
        images ROI. To increase performance, the image is just chamfered inside of its ROI, so
        no <em>correct</em> chamfering information (nearest white pixel distances) are available
        outside of the images ROI. The ChamferOp class provides 3 heuristics to tackle outer-ROI
        outliers of the model which are defined by a so called "outerROIPenaltyMode" (implemented
        as enum).
        -# noPenalty outliers are skipped
        -# constPenalty outliers are punished with a constant value, that can be set manually
        -# distancePenalty outliers are punished proportionally to the distance to the images ROI
           (the distance value can be weighted linearly by a manually given factor)

        \section COPY_CO Copying
        Please note, that ChampferOp instances are copied shallowly. By this means, you can cheaply
        pass ChampferOp instances to function calls, but this ops share their internal image buffer,
        so in some cases inpredictable behaviour can occur.
    */
    class ICLFilter_API ChamferOp : public UnaryOp{
      public:
      /// decides which metric is used to calculate the Hausdorff distance
      enum hausdorffMetric{
        hausdorff_max, /**< implements  \f$h(A,B) := \max\limits_{a_i \in A} \min\limits_{b_j \in B} || a - b || \f$*/
        hausdorff_mean /**< implements  \f$h(A,B) := < \min\limits_{b_j \in B} || a - b || >_{a_i \in A} \f$*/
      };
      /// decides how to punish model point, that are outside the images ROI, but inside of the image Rect
      enum outerROIPenaltyMode{
        noPenalty,      /**< outer ROI model pixels are not regarded */
        constPenalty,   /**< outer ROI model pixels are punished with a constant penalty value */
        distancePenalty /**< outer ROI model pixels are punished proportionally to the distance to the ROI */
      };

      /// Creates a new ChampferOp object with given distances for adjacent image pixels
      /** @param horizontalAndVerticalNeighbourDistance distance between horizontal adjacent pixels
          @param diagonalNeighborDistance distance between diagonal adjacent pixels
          useful parameter combinations are e.g. :
          - 1,1 ( equal distance )
          - 1,2 ( equal to the city block distance )
          - 2,3 ( weak approximation of the euclidean distance 1:sqrt(2) )
          - 3,4 ( better approximation of the euclidean distance 1:sqrt(2) )
          - 7,10 ( good approximation of the euclidean distance 1:sqrt(2) )
          - 7071,10000 ( nearly perfect approximation of the euclidean distance 1:sqrt(2) )
          <b>Note:</b> As it is more common, this constructor automatically sets the
          clipToROI property of the parent UnaryOp class to false
          @param scaleFactor TODO
          @param scaleUpResult TODO
      */
      ChamferOp( icl32s horizontalAndVerticalNeighbourDistance=3, icl32s diagonalNeighborDistance=4, int scaleFactor=1, bool scaleUpResult=true);

      /// destructor
      virtual ~ChamferOp(){}

      /// apply function
      /** @param poSrc source; image arbitrary parameters; ROI is regarded. If the image has more than one channels,
                       the operation is performed on each channel separately.
          @param ppoDst destination image, adapted to the source images ROI (dependent on the clipToROI settings @see UnaryOp)
                       and set up to depth32s. <b>Note:</b> to perform an in-place chamfering operation, use
                       \code
                       ImgBase *image = new Img32s(...);
                       ...
                       apply(image,&image)
                       \endcode
      */
      virtual void apply(const core::ImgBase *poSrc, core::ImgBase **ppoDst);

      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;

      /// static utility function to convert a model represented by a point set into a binary image
      /** @param model model to convert into the binary image representation
          @param image destination image (adapted to depth 32s, so it can be chamfered in-place)
          @param size destination image size
          @param bg image background color value for the destination image
          @param fg image foreground (all pixels that are defined by model are set to this value)
          @param roi optionally given destination image ROI ( if null, the whole image rect is used, otherwise, only
                     model-points, which are inside the images ROI are rendered
      */
      static void renderModel(const std::vector<utils::Point> &model, core::ImgBase **image, const utils::Size &size, icl32s bg=0, icl32s fg=255, utils::Rect roi=utils::Rect::null);


      /// utility function to calculate the directed Hausdorff distance between an image and a model
      /** For each model point the nearest image pixel distance (expressed by the entries of the given chamferImage
          is calculated.
          @param chamferImage base image
          @param model model to compare the image with
          @param m hausforffMetric to use
          @param pm penaltyMode to use
          @param penaltyValue penalty value to use (if pm is not noPenalty)
      */
      static double computeDirectedHausdorffDistance(const core::Img32s *chamferImage,
                                                     const std::vector<utils::Point> &model,
                                                     hausdorffMetric m=hausdorff_mean,
                                                     outerROIPenaltyMode pm=noPenalty,
                                                     icl32s penaltyValue=0);
      /// utility function to calculate the directed Hausdorff distance between an image and a model-image
      /** For each model point - each point inside the model images ROI, that is 0 (zero value in a chamfer image complies a point there) - the
          nearest image pixel distance (expressed by the entries of the given chamferImage
          is calculated.
          @param chamferImage base image
          @param modelChamferImage model to compare the image with
          @param m hausforffMetric to use
          @param pm penaltyMode to use
          @param penaltyValue penalty value to use (if pm is not noPenalty)
      */
      static double computeDirectedHausdorffDistance(const core::Img32s *chamferImage,
                                                     const core::Img32s *modelChamferImage,
                                                     ChamferOp::hausdorffMetric m,
                                                     ChamferOp::outerROIPenaltyMode pm=noPenalty,
                                                     icl32s penaltyValue=0);


      /// utility function to calculate the symmetric Hausdorff distance between an two model images
      /** The following code explains this function
          \code
          double ab = computeDirectedHausdorffDistance(chamferImageA,chamferImageB,m,pm,penaltyValue);
          double ba = computeDirectedHausdorffDistance(chamferImageB,chamferImageA,m,pm,penaltyValue);
          return m==hausdorff_mean ? (ab+ba)/2 : iclMax(ab,ba);
          \endcode
          @param chamferImageA first image
          @param chamferImageB first image
          @param m hausforffMetric to use
          @param pm penaltyMode to use
          @param penaltyValue penalty value to use (if pm is not noPenalty)
      */
      static double computeSymmetricHausdorffDistance(const core::Img32s *chamferImageA,
                                                      const core::Img32s *chamferImageB,
                                                      hausdorffMetric m=hausdorff_mean,
                                                      ChamferOp::outerROIPenaltyMode pm=noPenalty,
                                                      icl32s penaltyValue=0);
      /// utility function to calculate the symmetric Hausdorff distance between an two point sets
      /** The following code explains this function
          \code
          renderModel(setA,bufferA,sizeA,0,255,roiA);
          renderModel(setB,bufferB,sizeB,0,255,roiB);
          coA.apply(*bufferA,bufferA);
          coB.apply(*bufferB,bufferB);
          return computeSymmetricHausdorffDistance((*bufferA)->asImg<icl32s>(),(*bufferB)->asImg<icl32s>(),m,pm,penaltyValue);
          \endcode
          @param setA first point set
          @param sizeA the size of the chamfering image which is created to represent setA
          @param roiA the ROI of the chamfering image which is created to represent setA
          @param bufferA image buffer to exploit to chamfer setA
          @param setB second point set
          @param sizeB the size of the chamfering image which is created to represent setB
          @param roiB the ROI of the chamfering image which is created to represent setB
          @param bufferB image buffer to exploit to chamfer setB
          @param m hausforffMetric to use
          @param pm penaltyMode to use
          @param penaltyValue penalty value to use (if pm is not noPenalty)
          @param coA ChamferOp object to exploit for the creation of the chamfer-map for point setA
          @param coB ChamferOp object to exploit for the creation of the chamfer-map for point setB
      */
      static double computeSymmetricHausdorffDistance(const std::vector<utils::Point> setA, const utils::Size &sizeA, const utils::Rect &roiA, core::ImgBase **bufferA,
                                                      const std::vector<utils::Point> setB, const utils::Size &sizeB, const utils::Rect &roiB, core::ImgBase **bufferB,
                                                      hausdorffMetric m=hausdorff_mean,
                                                      ChamferOp::outerROIPenaltyMode pm=noPenalty,
                                                      icl32s penaltyValue=0,
                                                      ChamferOp coA=ChamferOp(),
                                                      ChamferOp coB=ChamferOp() );

      /// utility function to calculate the symmetric Hausdorff distance between an a point set and an already chamfered image
      /** In some cases, the user has a current observation (an image) and a model, which should be fitted into this image by
          variation of some intrinsic model parameters. In this case, it is much faster to chamfer the observation image once,
          before chamfering variated models into an image buffer, <b>so this is actually the most common function.</b>
          @param chamferImage observation image ( represents point set A by zero entries )
          @param model second point set setB
          @param modelImageSize the size of the chamfering image which is created to represent setB
          @param modelImageROI the ROI of the chamfering image which is created to represent setB
          @param bufferImageA, bufferImageB image buffer to exploit to chamfer setA and setB
          @param m hausforffMetric to use
          @param pm penaltyMode to use
          @param penaltyValue penalty value to use (if pm is not noPenalty)
          @param co ChamferOp object to exploit for the creation of the chamfer-map for point setB
      */
      static double computeSymmeticHausdorffDistance(const core::Img32s *chamferImage,
                                                     const std::vector<utils::Point> &model,
                                                     const utils::Size &modelImageSize,
                                                     const utils::Rect &modelImageROI,
                                                     core::ImgBase **bufferImageA,
                                                     core::ImgBase **bufferImageB,
                                                     hausdorffMetric m=hausdorff_mean,
                                                     ChamferOp::outerROIPenaltyMode pm=noPenalty,
                                                     icl32s penaltyValue=0,
                                                     ChamferOp co=ChamferOp());


      private:
      /// internally used variable for horizontally or vertically adjacent pixels
      icl32s m_iHorizontalAndVerticalNeighbourDistance;

      /// internally used variable for diagonal adjacent pixels
      icl32s m_iDiagonalNeighborDistance;

      /// internal scale factor
      int m_iScaleFactor;

      /// defines whether to scale up the result image if m_iScaleFactor is > 1
      bool m_bScaleUpResult;

      /// temporarily use buffer
      core::Img32s m_oBufferImage;
    };
  } // namespace filter
}
