/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/LUT2D.h                        **
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

namespace icl{
  namespace filter{

    /// Simple 2D indexed LUT Implementation
    /** The LUT2D template class provides functionalities for
        aranging 2D index data in a linear array for best
        and constant access performance, whereby x and y indices
        are allowed to be in an abitrary integer range [minVal,maxVal].

        Internally an array of size RANGE² named "lut" is created.
        Date element Lookup can be calculated using the following
        formula (\f$range := maxVal - minVal\f$):
        \f[
        LUTIDX_{naiv}(x,y) = x-minVal + range \cdot (y-minVal)
        \f]
        To enhance access performace, the lut-pointer can be adapted
        as follows:
        \f[
        lut -= (range+1)*minVal;
        \f]
        Now, data element lookup can be calculated much easier:
        \f[
        LUTIDX_{optimized}(x,y) = x + range\cdot y
        \f]


        A simple ()-operator can be used to access lut elements.

        */
    template<class RESULT_T=float,class IDX_T=int>
    class LUT2D{
      //    RESULT_T *lutOrig;
      RESULT_T *lut;
      int minVal;
      int range;

      public:
      /// creating a new LUT2D object with given element creation function
      LUT2D(RESULT_T (*generator_func)(IDX_T v1, IDX_T v2),IDX_T minVal, IDX_T maxVal):
      minVal(minVal),range(maxVal-minVal){

        lut = (new RESULT_T[range*range])-(range+1)*minVal;

        for(int i=minVal;i<maxVal;i++){
          for(int j=minVal;j<maxVal;j++){
            lut[i+range*j] = generator_func(i,j);
          }
        }
      }
      /// destructor
      ~LUT2D(){
        delete [] (lut + (range+1)*minVal);
      }
      /// inline access in constant time of data element at (v1,v2)
      inline RESULT_T operator()(IDX_T v1, IDX_T v2) const{
        return lut[v1+range*v2];
      }

    };
  } // namespace filter
}

