/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/Extrapolator.cpp                       **
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

#include <ICLCV/Extrapolator.h>
#include <stdio.h>

namespace icl{
  namespace cv{

    //s(t+1) = s(t) + dt v(t) + (dt)²/2 a(t)
    //v(t+1) = v(t) + dt a(t)  empirisch!
    //a(t+1) = a(t)            empirisch!

    template<class valueType,class timeType>
    valueType Extrapolator<valueType,timeType>::predict(valueType x2, valueType x1){
      // {{{ open

      return valueType(x1 + x1 - x2);
    }

    // }}}

    template<class valueType, class timeType>
    valueType Extrapolator<valueType,timeType>::predict(valueType x3, valueType x2, valueType x1){
      // {{{ open
      valueType v1 = x1-x2;
      valueType a = v1-( x2-x3 );//v1-v2
      //    printf("prediciton for %d %d %d --> %d \n",(int)x3,(int)x2,(int)x1,(int)(x1 + v1 +  a/2.0));
      return valueType(x1 + v1 +  a/2.0);
    }

    // }}}

    template<class valueType, class timeType>
    valueType Extrapolator<valueType,timeType>::predict(valueType x2, timeType t2, valueType x1, timeType t1, timeType t){
      // {{{ open

      timeType dt1 = t1 - t2;
      timeType dt0 = t - t1;
      valueType v1 = (x1-x2)/dt1;
      return valueType(x1 + dt0*v1);
    }

    // }}}

    template<class valueType, class timeType>
    valueType Extrapolator<valueType,timeType>::predict(valueType x3, timeType t3, valueType x2, timeType t2, valueType x1, timeType t1, timeType t){
      // {{{ open

      timeType dt2 = t2 - t3;
      timeType dt1 = t1 - t2;
      timeType dt0 = t - t1;
      valueType v2 = (x2-x3)/dt2;
      valueType v1 = (x1-x2)/dt1;
      valueType a = (v1-v2)/((dt1+dt2)/2);
      return valueType(x1 + dt0*v1 + (dt0*dt0)/2.0 * a);

    }

    // }}}

    template<class valueType, class timeType>
    valueType Extrapolator<valueType,timeType>::predict(int n, valueType *xs, timeType *ts, timeType t){
      // {{{ opem
      if(n == 0) return 0;
      if(n == 1) return *xs;
      if(ts){
        if(n == 2){
          return predict(xs[0],ts[0],xs[1],ts[1],t);
        }else{  // 3
          return predict(xs[0],ts[0],xs[1],ts[1],xs[2],ts[2],t);
        }
      }else{
        if(n == 2){
          return predict(xs[0],xs[1]);
        }else{
          return predict(xs[0],xs[1],xs[2]);
        }
      }
    }

    // }}}

    template ICLCV_API class Extrapolator<icl32s, int>;
    template ICLCV_API class Extrapolator<icl32f, long int>;
    template ICLCV_API class Extrapolator<icl32f, int>;
    template ICLCV_API class Extrapolator<icl32f, icl32f>;
    template ICLCV_API class Extrapolator<icl64f, long int>;
    template ICLCV_API class Extrapolator<icl64f, int>;
    template ICLCV_API class Extrapolator<icl64f, icl32f>;
  } // namespace cv
}
