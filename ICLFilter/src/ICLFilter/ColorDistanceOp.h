/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ColorDistanceOp.h              **
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
#include <ICLFilter/UnaryOp.h>

namespace icl {
  namespace filter{
    
    /// Class for creating a color distance map
    class ICLFilter_API ColorDistanceOp : public UnaryOp{
      public:
      
      /// Dummy constructor (creates an uninitialized instance)
      ColorDistanceOp(){}
      
      /// create a new instance with given reference color and optional threshold
      /** If the given threshold is not -1, the resulting distance values are 
          thresholded (so that pixels, that are closer than threshold to the
          referenceColor are set to 255. The reference color needs at least 3 entries.*/
      template<class Container>
      ColorDistanceOp(const Container &vec, double threshold=-1){
        init(vec,threshold);
      }
      
      /// create with given iterator range for the reference color initialization
      template<class ForewardIterator>
      ColorDistanceOp(ForewardIterator begin, ForewardIterator end, double threshold=-1){
        init(std::vector<double>(begin,end), threshold);
      }
  
      /// creates a color distance map
      /** If the current threshold is not -1, then the result image becomes
          a binary 8-image. Otherwise, the resulting image is adapted to an Img32f.
          Only if the source depth is depth64f, and Img64f is also used for the result. 
  
          The source image is assumed to have 3 channels
      */
      void apply(const core::ImgBase *src, core::ImgBase **dst);
      
      /// Import unaryOps apply function without destination image
      using UnaryOp::apply;
      
      /// sets the threshold value
      inline void setThreshold(double threshold){
        m_threshold = threshold;
      }
  
      /// sets the current reference color
      inline void setReferenceColor(const std::vector<double> &refColor){
        init(refColor,m_threshold);
      }
  
      /// sets both reference color and threshold at once
      template<class Container>
      inline void init(const Container &refColor, double threshold=-1){
        m_refColor.assign(refColor.begin(),refColor.end());
        m_threshold = threshold;
        ICLASSERT_THROW(m_refColor.size() >= 3, 
                        utils::ICLException("ColorDistanceOp::setReferenceColor: ref color needs at least 3 entries"));
      }
      
      private:
      /// internal reference color
      std::vector<double> m_refColor;
      
      /// internal threshold
      double m_threshold;
    };
   
  } // namespace filter
}
