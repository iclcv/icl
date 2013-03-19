/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/FixedVector.h                      **
** Module : ICLMath                                                **
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

#include <ICLMath/FixedMatrix.h>

namespace icl{
  namespace math{
    
    template<class T, int DIM>
    struct FixedColVector : public FixedMatrix<T,1,DIM>{
      typedef FixedMatrix<T,1,DIM> super;
      FixedColVector(){}
      FixedColVector(const T &init):super(init){}
      FixedColVector(const T *srcData):super(srcData){}
      FixedColVector(const FixedMatrix<T,1,DIM> &other):super(other){}
      FixedColVector(const T&v0,const T&v1, const T&v2=0, const T&v3=0, const T&v4=0, const T&v5=0, 
                     const T&v6=0,const T&v7=0, const T&v8=0, const T&v9=0, const T&v10=0, const T&v11=0):
      super(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11){}
  
      template<class Iterator>
      FixedColVector (const FixedMatrixPart<T,DIM,Iterator> &r):super(r){}
  
      template<class otherT, class Iterator>
      FixedColVector(const FixedMatrixPart<otherT,DIM,Iterator> &r):super(r){}
  
    
    };
  
    
    template<class T, int DIM>
    struct FixedRowVector : public FixedMatrix<T,DIM,1>{
      typedef FixedMatrix<T,DIM,1> super;
      FixedRowVector(){}
      FixedRowVector(const T &init):super(init){}
      FixedRowVector(const T *srcData):super(srcData){}
      FixedRowVector(const FixedMatrix<T,DIM,1> &other):super(other){}
      FixedRowVector(const T&v0,const T&v1, const T&v2=0, const T&v3=0, const T&v4=0, const T&v5=0, 
                     const T&v6=0,const T&v7=0, const T&v8=0, const T&v9=0, const T&v10=0, const T&v11=0):
      super(v0,v1,v2,v3,v4,v5,v6,v7,v8,v9,v10,v11){}
  
      template<class Iterator>
      FixedRowVector (const FixedMatrixPart<T,DIM,Iterator> &r):super(r){}
  
      template<class otherT, class Iterator>
      FixedRowVector(const FixedMatrixPart<otherT,DIM,Iterator> &r):super(r){}
  
    };
    
    
    
  } // namespace math
}

