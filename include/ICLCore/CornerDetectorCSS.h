/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCore module of ICL                         **
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

#ifndef ICL_CORNER_DETECTOR_CSS_H
#define ICL_CORNER_DETECTOR_CSS_H

#include <ICLUtils/Point32f.h>
#include <ICLCore/Types.h>
#include <vector>

namespace icl{

	/// Curvature Corner Detector
  /** 
   * Implementation of the Curvature Scale Space corner detection algorithm
   * described in the paper "Corner detector based on global and local curvature properties",
   * by Chen He, Xiao and Yung, Nelson H.C. in Optical Engineering 2008.
   * There also is a Matlab implementation of the algorithm by Chen He, Xiao available
   * at http://www.mathworks.com/matlabcentral/fileexchange/7652
   *
   * The algorithm takes an array of contour points as input, that are then smoothed
   * by a gaussian filter. Afterwards the curvature function is calculated and its
   * maxima positions are taken as corner candidates.
   * Afterwards all candidates which belong to round or "false" (noise induced, etc.) corners
   * are removed and finally a list of corners is given back.
	 * 
   * Parameters for the algorithm are:
   * rc_coeff - denotes the minimum ratio of major axis to minor axis of an ellipse, 
   *            whose vertex could be detected as a corner by proposed detector
   *            The default value is 1.5.
   * angle_thresh - denotes the maximum obtuse angle that a corner can have when 
   *            it is detected as a true corner, default value is 162.
   * sigma -    denotes the standard deviation of the Gaussian filter when
   *            computeing curvature. The default sig is 3.
   * straight_line_thresh - to estimate the angle of a corner, either a circle or a
   *            straight line approximation of the left and right surrounding is used.
   *            The straight line approximation is used, if the angle between the
   *            left neigbour, corner candidate and  and the point on the contour half
   *            way between them is smaller than straight_line_thresh.
   *            0 leads to circle approximation only, 180 to straight line approximation only.
   *
   * Diffent to the reference implementation of Xiao He, the second half of the contour
   * is copied before the start of the contour and the first half of the contour is
   * copied behind the end of the contour. That way, the (artificially) first and
   * last corners in the closed contour are calculated more acurately, since there are
   * no discontinuities.
   *
   * For visualizing the algorithm, you can call setDebugMode(true) after construction
   * of the CornerDetectionCSS object and retrieve detailed information of the detection
   * process afterwards with the getDebugInformation() method.
   * See the icl-corner-detection-css-demo as an example.
   *
   * usage example:
   *   \code 
   *   const std::vector<icl::Region> &rs = d.detect(&image);
   *   const std::vector<Point32f> &boundary = getThinnedBoundary(rs[0].getBoundary());
   *   CornerDetectorCSS css;
   *   const std::vector<Point32f> &corners = css.detectCorners(boundary);
   *   \endcode
   **/
  class CornerDetectorCSS {
    public:
    	/// 1 dim gaussian kernel
			struct GaussianKernel {
				std::vector<icl32f> gau;
				float sigma;
				float cutoff;
				int width; // only one side of the gaussian, gau has width*2+1 elements!
				GaussianKernel(): sigma(0), cutoff(0), width(0) {}
			};
			
    	/// Detailed information about one corner detection run.
    	struct DebugInformation {
				GaussianKernel gk;
				std::vector<Point32f> boundary;
				std::vector<Point32f> smoothed_boundary;
				std::vector<float> kurvature;
				std::vector<int> extrema;
				std::vector<int> maxima;
				std::vector<int> maxima_without_round_corners;
				std::vector<int> maxima_without_false_corners;
				std::vector<Point32f> corners;
				std::vector<float> angles;
				int offset; // the number of additional points at begin and end of
										// smoothed_boundary and kurvature
				float angle_thresh;
				float rc_coeff;
				float straight_line_thresh;
			};
    
    	/// Default constructor with given arguments
      CornerDetectorCSS(float angle_thresh=162.,
                        float rc_coeff=1.5, 
                        float sigma=3., 
                        float curvature_cutoff=100., 
                        float straight_line_thresh=0.1):
    	angle_thresh(angle_thresh), rc_coeff(rc_coeff), sigma(sigma), 
    	curvature_cutoff(curvature_cutoff), straight_line_thresh(straight_line_thresh),
    	debug_mode(false) {};
    
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
      const std::vector<Point32f> &detectCorners(const std::vector<Point32f> &boundary);

      /// detects corners on int-points
      const std::vector<Point32f> &detectCorners(const std::vector<Point> &boundary);

      /// returns the result of last detectCorners call
      /** This function can be used as optimization e.g. whithin ICLBlob::Region implementation */
      inline const std::vector<Point32f> &getLastCorners() const {
        return corners;
      }
      
      /// Returns approximated angles of corners in deg. Call detectCorners method first.
      const std::vector<float> &getCornerAngles() const { return corner_angles; }

      inline void setAngleThreshold(float value){ angle_thresh = value; }
      inline void setRCCoeff(float value){ rc_coeff = value; }
      inline void setSigma(float value){ sigma = value; }
      inline void setCurvatureCutoff(float value){ curvature_cutoff = value; }
      inline void setStraightLineThreshold(float value) { straight_line_thresh = value; }

      inline float getAngleThreshold() const { return angle_thresh;}
      inline float getRCCoeff() const { return rc_coeff; }
      inline float getSigma() const { return sigma; }
      inline float getCurvatureCutoff() const { return curvature_cutoff; }
      inline float getStraightLineThreshold() const { return straight_line_thresh;}

			inline void setDebugMode(bool value) { debug_mode = value; }
			const DebugInformation &getDebugInformation() const { return debug_inf; } 

      /// Small wrapper for ippsConv_32f
      /** @param dst destination data pointer of size dim+kernelDim -1

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
      /// debug mode flag
      bool debug_mode;

			/// gausian kernel
			GaussianKernel m_gauss;
			
      /// finds the indicies of extrema
      /**
       * @param extrema reference to a std::vector in which the extrema are stored
       * @param x function values
       * @param length number of points in array x
       */
      static void findExtrema(std::vector<int> &extrema, icl32f* x, int length);

      /// removes round corners by comparing all corner candidates with adaptive local threshold
      /**
       * The round corner coefficient denotes the minimum ratio of major axis to minor
       * axis of an ellipse, whose vertex could be detected as a corner by proposed detector.
       * @param rc_coeff round corner coefficient
       * @param k curvature function
       * @param extrema indicies of extrem points in curvature function k
       */
      static void removeRoundCorners( float rc_coeff, icl32f* k, std::vector<int> &extrema);

      /// remove false corners by checking the corner angle
      /**
       * The angle threshold denotes the maximum obtuse angle that a corner can have when
       * it is detected as a true corner.
       * @param angle_thresh angle threshold
       * @param xx,yy x,y dimension of smoothed contour
       * @param k curvature function
       * @param length number of points in smoothed contour (and curvature function)
       * @param maxima indicies of maximum points in curvature function k
       */
      static void removeFalseCorners(float angle_thresh, icl32f* xx, icl32f* yy, icl32f* k, 
                                     int length, std::vector<int> &maxima, std::vector<float> &corner_angles,
                                     float straight_line_thresh);

      /// estimates the angle of a corner
       /**
        * Eestimates the angle of a corner in a part of the curve by fitting a circle
        * on each side of the corner and calculating the angle between the two tangents.
        * @param x,y x,y dimension of the contour segment around the corner
        * @param length number of points in contour segment
        * @param center position of corner in contour segment
        */
      static float tangentAngle(icl32f* x, icl32f* y, int length, int center, float straight_line_thresh);

      // result lists
      std::vector<Point32f> corners;
      std::vector<float> corner_angles;
      std::vector<int> extrema;
      std::vector<Point32f> inputBuffer;
			DebugInformation debug_inf;
  };
}

#endif
