/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/HoughLineDetector.h                    **
** Module : ICLCV                                                  **
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


#include <cmath>
#include <vector>
#include <ICLMath/FixedVector.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>
#include <ICLMath/DynMatrix.h>
#include <ICLMath/StraightLine2D.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Configurable.h>

namespace icl{
  namespace cv{
    
    /** LineDetection class using hough-space line detection
        
        @section ALG Algorithm
        Image lines (of infinite length) can be described by a tuple (rho,r) and the line
        equation \f$r = x\cdot cos(rho) + y\cdot sin(rho)\f$. Each pixel (x,y) belongs potentially
        to a all lines (rho,r) where this equation is true. 
        Internally, the space of possible lines (sometimes referred a the hough-space) is sampled 
        (internally we use an instance of core::Img32s).
        Rho is mapped to the x-axis (x=0 -> rho=0, x=width-1 -> rho = 2*M_PI) and r is mapped to
        the y-axis (y=0 -> r=minR, y=height-1 -> r=maxR).
        So the size of the internal hough-space becomes 2*M_PI/dRho x (rRange.max-rRange.min)/dR.
        
        When a new border pixel (x,y) is added, the probability of each potential line that intersects
        (x,y) is increased. Technically this is performed by computing r(rho) for all possible 
        values rho (all samples) with the formula for r above which leads to values (rho,r) which's
        hough-space entries are increased by one. 
        The best line matches can be computed from the maxima in the resulting hough space.
        
        @section OPT Optimizations
        To enhance line detection performance, the HoughLineDetector provides a set of optimization
        approaches:
  
        @subsection S1 Local inhibition around detected lines
        As a single image line, might result in 2 or even more lines in the hough-space (because of
        spacial noise in the pixels detection procedure), the hough-space is locally inhibited around
        already detected lines. The amount of this inhibition procedure can be set by the constructor
        parameters (rInhibitionRange and rhoInhibitionRange).
        
        @subsection S2 Gaussian inhibition
        Sometimes hard borders of the inhibition sub-window lead to random 2nd and 3rd detected line
        values. To stabilize this, inhibition can be set up to be 'gaussian-blurred', which means, that
        hough-table entries that are very close to already detected lines are nearly removed completely
        and those that are also in the inhibition sub-rect but some pixels away from the detected line
        center are inhibited less.
  
        @subsection S2 Stabilization
        
        Resulting from the sampling procedure of both, that actual image space and the hough-space,
        entries in the hough-space might have hard borders or even gaps. To avoid this, three different
        optimization techniques are provided, which can all be combined.
        
        @subsubsection S3 Blurring the hough space
        This means, that the hough-space is simply blurred using a 3x3 linear gaussian filter. This
        optimization provides much more stable results, but it also entails a higher computational 
        expense.
  
        @subsubsection S4 Dilation of pixel entries
        If this optimization is activated, no only the given pixel (x,y) is added internally, but
        also it's 4 direct neighbour pixels. This also leads to a kind of blurred hough table
  
        @subsubsection S5 Blurred sampling of lines
        This optimization changes the line-sampling procedure. When a line is sampled for all rho-values
        using r = x cos(rho) + y sin(rho), not only the resulting pixel p in the hough table is increased,
        but also the pixel above, and the pixel below (actually, the exactly computed pixel p is increased
        by 2 and it's upper and lower neighbour's are increased by one). This again leads to a blurring effect.
        however, the other two optimzations provides better results.
        It's worth mention, that this optimization's additional computational expense is low in comparison
        to the other two optizations.
        
    */
    class ICLCV_API HoughLineDetector : public utils::Configurable{
      struct Data;
      Data *m_data;
      
      public:
  
      /// Create a new HoughLineDetectorInstance
      /** @param dRho line angle sampling distance 
          @param dR line distance to 0 sampling factor 
          @param rRange range of line distances 
          @param rInhibitionRange vertical radius for local inhibition 
          @param rhoInhibitionRange horizontal radius for local inhibition
          @param gaussianInhibition enables gaussian inhibition
          @param blurHoughSpace enables hough space blurring
          @param dilateEntries enables entry dilatation
          @param blurredSampling enables blurred sampling */
      HoughLineDetector(float dRho=0.1, float dR=10, 
                        const utils::Range32f &rRange=utils::Range32f(0,::sqrt(640*640+480*480)), 
                        float rInhibitionRange=10, 
                        float rhoInhibitionRange=0.3,
                        bool gaussianInhibition=true,
                        bool blurHoughSpace=true,
                        bool dilateEntries=true,
                        bool blurredSampling=false);

      ~HoughLineDetector();
  
      
      /// adds a new point
      void add(const utils::Point &p);
  
      /// adds new points
      void add(const std::vector<utils::Point> &ps);
      
      /// adds a new point
      void add(const utils::Point32f &p);
  
      /// adds new points
      void add(const std::vector<utils::Point32f> &ps);
  
      /// adds a new point
      template<class T>
      void add(const math::FixedColVector<T,2> &p){ 
        add_intern(p[0],p[1]); 
      }    
      
      /// adds new points
      template<class T>
      void add(const std::vector<const math::FixedColVector<T,2> > &ps){
        for(unsigned int i=0;i<ps.size();++i) add(ps[i]);
      }
  
      /// adds all non zero pixels of the given binary image
      void add(const core::Img8u &binaryImage);
  
      /// returns current hough-table image
      const core::Img32s &getImage() const;
  
      /// returns current gaussian inhibition map
      const core::Img32f &getInhibitionMap() const;
  
      /// sets all hough table entries to 0
      void reset();
      
      /// detects up to max lines
      std::vector<math::StraightLine2D> getLines(int max, bool resetAfterwards=true);
  
      /// detects up to max lines and pushes the line significances into given destination vector
      /** significances are relative to the first line match, which's significance is always 1.0*/
      std::vector<math::StraightLine2D> getLines(int max, std::vector<float> &significances, bool resetAfterwards=true);
      
      private:
      void prepare_all();
      
      /// initializes the HoughLineDetector
      void prepare(float dRho, float dR, const utils::Range32f &rRange, 
                   float rInhibitionRange, float rhoInhibitionRange,
                   bool gaussianInhibition=true,
                   bool blurHoughSpace=true,
                   bool dilateEntries=true,
                   bool blurredSampling=false);
      
      /// internal utility function
      float r(float rho, float x, float y) const;

      /// internal utility function
      int getX(float rho) const;

      /// internal utility function
      int getY(float r) const;

      /// internal utility function
      float getRho(int x) const;

      /// internal utility function
      float getR(int y) const;

      /// internal utility function
      void incLut(float rho, float r);

      /// internal utility function
      void incLutBlurred(float rho, float r);

      /// internal utility function
      int &cyclicLUT(int x, int y);
    
      /// internal utility function
      void apply_inhibition(const utils::Point &p);
  
      /// internal utility function
      void add_intern(float x, float y);
  
      /// internal utility function
      void add_intern2(float x, float y);
  
      /// internal utility function
      void blur_hough_space_if_necessary();
      
    };
    
  } // namespace cv
}

