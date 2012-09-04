/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLCV/FloodFiller.h                            **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_FLOOD_FILLER_H
#define ICL_FLOOD_FILLER_H

#include <ICLCore/Img.h>

namespace icl{
  namespace cv{
    
    /// Utility class for image flood filling
    /** The flood filler implements a generic flood filling algorithm.
        Starting with a seed region that contains just a single point,
        the flood filler will collect all adjacent pixels of this region
        recursively that meet a certain criterion. The criterion is 
        the template parameter of the FloodFiller's 
        icl::FloodFiller::applyGeneric template method. For convenience,
        the most common criterions are pre-coded and directly usable
        by calling the non-template methods icl::FloodFiller::apply
        and icl::FloodFiller::applyColor.
        
        \section _COLOR_ Color and GrayScale images
        The flood filler provides separate implementations for 1- and 3-channel images.
        The 3-channel image methods have the *<em>Color</em>-ending: 
        icl::FloodFiller::applyColor and icl::FloodFiller::applyColorGeneric
        
        \section _NBH_ Neighbourhood
        The floodfilling algorithm uses the 8-Pixel neighbourhood of each pixel.
        
        \section _ALGO_ Algorithm
        The floodfilling algorithm is implemented in an iterative manner. 
        It uses a mask, which is initialized with zero and set to 1 for already
        processed pixels. In addition, a to-be-processed-pixels queue is used. 
        In order to speed up the queue implementation, a static and <em>large enough</em>
        array two pointers n (next to-be-processed point) and e (end of to-be-processed-points)
        are used. At start, only the seed point is put into the queue. Now
        while there are pixels in the queue, each of the 8 neighbours, that have not
        been processed earlier and that match the filling criterion are filled
        and put into the queue.
        
    */
    class FloodFiller{
      /// internal list of to-be-processed points
      std::vector<utils::Point> futurePoints;
      
      /// internal utility method
      utils::Rect prepare(const utils::Size &imageSize, const utils::Point &seed);
  
      public:
  
      /// result structure, returned by the 'apply' methods
      struct Result{
        std::vector<utils::Point> pixels; /// list of filled pixels
        core::Img8u ffLUT;               /// binary mask with filled pixels
      } result;
      
      /// flood-fills the given grayscale image starting from given seed point.
      /** Image pixels are filled if their pixel value difference to the given
          referenceValue is less then the given threshold */
      const Result &apply(const core::ImgBase *image, const utils::Point &seed, double referenceValue, double threshold);
  
      
      /// flood-fills the given 3-channel image starting from the given seed point
      /** Here, the fill-criterion is the euclidian distance to the given (refR,refG,regB)
          reference color, which must be less than the given threshold */
      const Result &applyColor(const core::ImgBase *image, const utils::Point &seed, 
                          double refR, double refG, double refB, double threshold);
      
      /// generic grayscale image floodfilling using an arbitrary filling criterion
      /** The criterion-function or functor is evaluated by giving it pixel values
          Take a look at the icl::FloodFiller::DefaultCriterion class for more details. */
      template<class T, class Criterion>
      inline const Result &applyGeneric(const core::Img<T> &image, const utils::Point &seed, Criterion crit){
        utils::Rect r = prepare(image.getSize(),seed);
        
        utils::Point *n = futurePoints.data(); // next point to seed from
        utils::Point *e = n;                   // end of stored seed points
        
        *e++ = seed;
        
        const core::Channel<T> im = image[0];
        core::Channel8u ff = result.ffLUT[0];
        
        ff(seed.x,seed.y) = 255;
  
        while(n != e){
          const int x = n->x;
          const int y = n->y;
          ++n;
          if(crit(im(x,y))){
            result.pixels.push_back(utils::Point(x,y));
  #define ICL_DYNAMIC_FLOOD_FILLER_P(x,y)                                 \
            if(r.contains(x,y) && !ff(x,y)){                              \
              *e++ = utils::Point(x,y);                                          \
              ff(x,y) = 255;                                              \
            }
            
            ICL_DYNAMIC_FLOOD_FILLER_P(x-1,y-1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x,y-1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x+1,y-1);
            
            ICL_DYNAMIC_FLOOD_FILLER_P(x-1,y);
            ICL_DYNAMIC_FLOOD_FILLER_P(x+1,y);
            
            ICL_DYNAMIC_FLOOD_FILLER_P(x-1,y+1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x,y+1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x+1,y+1);
  #undef ICL_DYNAMIC_FLOOD_FILLER_P
          }
        }
        return result;
      }
  
      /// generic floodfilling algorithm for 3-channel color images
      /** the criterion function or functor gets all 3-channel pixel values as arguments.
          Take a look at the icl::Floodfiller::ReferenceColorCriterion class template
          for more details. */
      template<class T, class Criterion3Channels>
      inline const Result &applyColorGeneric(const core::Img<T> &image, const utils::Point &seed, Criterion3Channels crit){
        utils::Rect r = prepare(image.getSize(),seed);
        
        utils::Point *n = futurePoints.data(); // next point to seed from
        utils::Point *e = n;                   // end of stored seed points
        
        *e++ = seed;
        
        const core::Channel<T> im0 = image[0];
        const core::Channel<T> im1 = image[1];
        const core::Channel<T> im2 = image[2];
  
        core::Channel8u ff = result.ffLUT[0];
        
        ff(seed.x,seed.y) = 255;
  
        while(n != e){
          const int x = n->x;
          const int y = n->y;
          ++n;
          if(crit(im0(x,y),im1(x,y),im2(x,y))){
            result.pixels.push_back(utils::Point(x,y));
  #define ICL_DYNAMIC_FLOOD_FILLER_P(x,y)                                 \
            if(r.contains(x,y) && !ff(x,y)){                              \
              *e++ = utils::Point(x,y);                                          \
              ff(x,y) = 255;                                              \
            }
            
            ICL_DYNAMIC_FLOOD_FILLER_P(x-1,y-1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x,y-1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x+1,y-1);
            
            ICL_DYNAMIC_FLOOD_FILLER_P(x-1,y);
            ICL_DYNAMIC_FLOOD_FILLER_P(x+1,y);
            
            ICL_DYNAMIC_FLOOD_FILLER_P(x-1,y+1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x,y+1); 
            ICL_DYNAMIC_FLOOD_FILLER_P(x+1,y+1);
  #undef ICL_DYNAMIC_FLOOD_FILLER_P
          }
        }
        return result;
      }
  
      /// predefined criterion for simple reference-value based filling for 1-channel images
      template<class T>
      struct DefaultCriterion{
        T val;
        T thresh;
        inline DefaultCriterion(T val, T thresh):val(val),thresh(thresh){}
        inline bool operator()(const T &pix) const{
          return ::abs(val-pix) < thresh;
        }
      };
  
      /// predefined criterion for simple reference-value based filling for 3-channel images
      template<class T>
      struct ReferenceColorCriterion{
        T refcol[3];
        T maxSquaredEuklDist;
        
        inline ReferenceColorCriterion(T refr, T refg, T refb, T maxEuclDist):
          maxSquaredEuklDist(maxEuclDist*maxEuclDist){
          refcol[0] = refr;
          refcol[1] = refg;
          refcol[2] = refb;
        }
        inline bool operator()( const T &r, const T &g, const T &b) const{
          return sqr(r-refcol[0]) + sqr(g-refcol[1]) + sqr(b-refcol[2])  < maxSquaredEuklDist;
        }
      };
    };
  
    /** \cond **/
    // specialized
    template<>
    struct FloodFiller::ReferenceColorCriterion<icl8u>{
      int refcol[3];
      int maxSquaredEuklDist;
      
      inline ReferenceColorCriterion(icl8u refr, icl8u refg, icl8u refb, int maxEuclDist):
      maxSquaredEuklDist(maxEuclDist*maxEuclDist){
        refcol[0] = refr;
        refcol[1] = refg;
        refcol[2] = refb;
      }
      inline bool operator()( const icl8u &r, const icl8u &g, const icl8u &b) const{
        return utils::sqr(int(r)-refcol[0]) + utils::sqr(int(g)-refcol[1]) + utils::sqr(int(b)-refcol[2]) < maxSquaredEuklDist;
      }
    };
    /** \endcond **/
  
        
  } // namespace cv
}


#endif
