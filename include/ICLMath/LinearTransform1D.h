/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMath/LinearTransform1D.h                    **
** Module : ICLGeom                                                **
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

#pragma once

#include <ICLUtils/Range.h>

namespace icl{
  namespace math{
  
    /// A standard linear mapping class for the 1D case f(x) = m * x + b
    struct LinearTransform1D{
      /// slope
      float m;
      
      /// offset
      float b;
      
      /// base constructor with given parameters m and b
      inline LinearTransform1D(float m=0, float b=0):m(m),b(b){}
      
      /// spedical constructor template with given source and destination range
      template<class T>
      inline LinearTransform1D(const utils::Range<T> &s, const utils::Range<T> &d):
        m(d.getLength()/s.getLength()), b(-m*s.minVal + d.minVal){
      }
      
      /// applies the mapping to a given x -> f(x) = m * x + b
      float operator()(float x) const { return m*x+b; }
    };
  
  } // namespace math
}

