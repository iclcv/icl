/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/CornerDetectorCSS.h                    **
** Module : ICLCV                                                  **
** Authors: Erik Weitnauer                                         **
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
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Configurable.h>
#include <ICLCore/Types.h>
#include <vector>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  namespace cv{

    /// Curvature Corner Detector
    /** Implementation of the Curvature Scale Space corner detection algorithm
        described in the paper "Corner detector based on global and local curvature properties",
        by Chen He, Xiao and Yung, Nelson H.C. in Optical Engineering 2008.
        There also is a Matlab implementation of the algorithm by Chen He, Xiao available
        at http://www.mathworks.com/matlabcentral/fileexchange/7652

        The algorithm takes an array of contour points as input, that are then smoothed
        by a gaussian filter. Afterwards the curvature function is calculated and its
        maxima positions are taken as corner candidates.
        Afterwards all candidates which belong to round or "false" (noise induced, etc.) corners
        are removed and finally a list of corners is given back.

        Parameters for the algorithm are:
      - <b>rc_coeff</b>
        denotes the minimum ratio of major axis to minor axis of an ellipse,
        whose vertex could be detected as a corner by proposed detector
        The default value is 1.5.
      - <b>angle_thresh</b>
        denotes the maximum obtuse angle that a corner can have when
        it is detected as a true corner, default value is 162.
      - <b>sigma</b>
        denotes the standard deviation of the Gaussian filter when
        computeing curvature. The default sig is 3.
      - <b>straight_line_thresh</b>
        to estimate the angle of a corner, either a circle or a
        straight line approximation of the left and right surrounding is used.
        The straight line approximation is used, if the angle between the
        left neigbour, corner candidate and  and the point on the contour half
        way between them is smaller than straight_line_thresh.
        0 leads to circle approximation only, 180 to straight line approximation only.

        Diffent to the reference implementation of Xiao He, the second half of the contour
        is copied before the start of the contour and the first half of the contour is
        copied behind the end of the contour. That way, the (artificially) first and
        last corners in the closed contour are calculated more acurately, since there are
        no discontinuities.

        usage example:
        \code
        const std::vector<icl::Region> &rs = d.detect(&image);
        const std::vector<Point32f> &boundary = getThinnedBoundary(rs[0].getBoundary());
        CornerDetectorCSS css;
        const std::vector<Point32f> &corners = css.detectCorners(boundary);
        \endcode
    **/
    class ICLCV_API CornerDetectorCSS : public utils::Configurable, public utils::Uncopyable{
      public:
      /// 1 dim gaussian kernel
      struct ICLCV_API GaussianKernel {
        std::vector<icl32f> gau;
        float sigma;
        float cutoff;
        int width; // only one side of the gaussian, gau has width*2+1 elements!
        GaussianKernel(): sigma(0), cutoff(0), width(0) {}
      };

      /// Default constructor with given arguments
      CornerDetectorCSS(float angle_thresh=162.,
                        float rc_coeff=1.5,
                        float sigma=3.,
                        float curvature_cutoff=100.,
                        float straight_line_thresh=0.1,
                        bool accurate = false);

      /// Destructor
      ~CornerDetectorCSS();

      /// sets value of a property (always call call_callbacks(propertyName) or Configurable::setPropertyValue)
      virtual void setPropertyValue(const std::string &propertyName,
                                    const utils::Any &value) throw (utils::ICLException);

      /// returns Configurable property list
      virtual std::vector<std::string> getPropertyList() const;

      /// returns type of given property
      virtual std::string getPropertyType(const std::string &propertyName) const;

      /// returns info for given property
      virtual std::string getPropertyInfo(const std::string &propertyName) const;

      /// returns value for given property
      virtual utils::Any getPropertyValue(const std::string &propertyName) const;

      /// returns volatileness for given property
      virtual inline int getPropertyVolatileness(const std::string &propertyName) const { return 0; }

      /// returns property descriptions
      virtual std::string getPropertyToolTip(const std::string &propertyName) const;

      /// calculates a normalized 1d gaussian std::vector
      /**
          * @param gauss reference to GaussianKernel struct
          * @param sigma sigma^2 is the variance of the gaussian, default is 1
          * @param cutoff if value of gaussian drops below this value, it is set to zero, default is 0.001
          * @return width of the gaussian tails, so the size of the returned std::vector is 2*width+1
          */
      static int gaussian(GaussianKernel &gauss, float sigma, float cutoff);

      /// detects the corners in the passed contour
      /**
          * Use this function to detect the corners in an image.\n
          * A reference to the contour in which the corners should be detected must be passed. The contour
          * should be "thinned", meaning that it should look e.g. like
          * <pre>
          ooox                ooxx
          ooxo  and not like  oxxo
          oxoo                xxoo
          xooo                xooo
          </pre>
          */
      template<class T> ICLCV_API
      const std::vector<std::vector<utils::Point32f> > &detectCorners(const std::vector<std::vector<T> > &boundaries, const std::vector<icl32f> &sigmas);
      template<class T> ICLCV_API
      const std::vector<utils::Point32f> &detectCorners(const std::vector<T> &boundary);

      /// returns the result of last detectCorners call
      /** This function can be used as optimization e.g. whithin ICLCV::Region implementation */
      inline const std::vector<utils::Point32f> &getLastCorners() const {
        return corners;
      }

      inline void setAngleThreshold(float value){ angle_thresh = value; }
      inline void setRCCoeff(float value){ rc_coeff = value; }
      inline void setSigma(float value){ sigma = value; }
      inline void setCurvatureCutoff(float value){ curvature_cutoff = value; }
      inline void setStraightLineThreshold(float value) { straight_line_thresh = value; }
      inline void setAccurate(bool value) { accurate = value; }

      inline float getAngleThreshold() const { return angle_thresh;}
      inline float getRCCoeff() const { return rc_coeff; }
      inline float getSigma() const { return sigma; }
      inline float getCurvatureCutoff() const { return curvature_cutoff; }
      inline float getStraightLineThreshold() const { return straight_line_thresh;}
      inline bool getAccurate() const { return accurate; }

      /// Small wrapper for ippsConv_32f
      /** @param vec
    @param dim
    @param kernel
    @param kernelDim
    @param dst destination data pointer of size dim+kernelDim -1

          Performs the following operation


          \f[  \mbox{dst}[n] = \sum\limits_{k=0}^n \mbox{src}[k] \cdot \mbox{kernel}[n-k] \f]

          for all \f$ n \in [0,\mbox{dim}+\mbox{kernelDim}-1]\f$

          with \f$ \mbox{src}[x] = 0 \f$ for \f$ x \not\in [0,\mbox{dim}-1] \f$
          and \f$ \mbox{kernel}[x] = 0\f$ for \f$ x \not\in [0,\mbox{kernelDim}-1] \f$
          */
      static void convolute_1D(float *vec, int dim, float *kernel, int kernelDim, float *dst);

      private:
      /// parameters
      float angle_thresh, rc_coeff, sigma, curvature_cutoff, straight_line_thresh;
      /// use acurate corner detection
      bool accurate;

      int gauss_radius(float sigma, float cutoff);
      void fill_gauss(float *mask, float sigma, int width);
      void convolute(const float *data, int data_length, const float *mask , int mask_length, float *convoluted);
      void calculate_curvatures(const float *smoothed_x, const float *smoothed_y, int length, float curvature_cutoff, float *curvatures);
      void calculate_curvatures_bulk(int array_length, int num_boundaries, const int *lengths,
                                     const int *indices, const int *indices_padded, const float *smoothed_x, const float *smoothed_y, float curvature_cutoff, float *curvature);
      int findExtrema(int *extrema, int *num_extrema_out, float* k, int length);
      void removeRoundCorners(float rc_coeff, int maxima_offset, float* k, int length, int *extrema, int num_extrema, int *new_extrema, int *num_new_extrema_out);
      void removeRoundCornersAccurate(float rc_coeff, int maxima_offset, float* k, int length, int *extrema, int num_extrema, int *extrema_out, int *num_extrema_out);
      float cornerAngle(float *x, float *y, int prev, int current, int next, int length, float straight_line_thresh);
      float cornerAngleAccurate(float *x, float *y, int prev, int current, int next, int array_length, float straight_line_thresh);
      void removeFalseCorners(float angle_thresh, float* x, float* y, float* k, int length, int *maxima, int num_maxima, int *maxima_out, int *num_maxima_out);

      // result lists
      std::vector<utils::Point32f> corners;
      std::vector<std::vector<utils::Point32f> > corners_list;

      struct CLCurvature;
      CLCurvature *clcurvature;
      bool useOpenCL; // in case of no support, this is always false
    };
  } // namespace core
}

