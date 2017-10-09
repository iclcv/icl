/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/SteppingRange.h                  **
** Module : ICLUtils                                               **
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
#include <ICLUtils/Range.h>

namespace icl{
  namespace utils{
    /// class representing a range with defined stepping \ingroup TYPES
    /** A Stepping range is a well defined interval
        I = [minVal,maxVal] with an also well defined
        stepping e.g. S. the SteppingRange contains all
        values {minVal, minVal+S, minVal+2*S,...} that
        are less then or equal to the maxVal
    **/
    template<class Type>
    struct SteppingRange : public Range<Type>{
      /// create an empty range (min = max = 0)
      SteppingRange():Range<Type>(),stepping(0){}

      /// create a special Rage
      SteppingRange(Type minVal, Type maxVal, Type stepping):
        Range<Type>(minVal,maxVal), stepping(stepping){}

      /// internal stepping parameter
      Type stepping;

      /// casts this range into another depth
      template<class dstType>
      const SteppingRange<dstType> castTo() const{
        return SteppingRange<dstType>(clipped_cast<Type,dstType>(Range<Type>::minVal),
                                      clipped_cast<Type,dstType>(Range<Type>::maxVal),
                                      clipped_cast<Type,dstType>(stepping));
      }

      /// tests whether a given value is inside of this range
      virtual bool contains(Type value) const {
        if(stepping == 0) return Range<Type>(Range<Type>::minVal,Range<Type>::maxVal).contains(value);
        Type offs = value - Range<Type>::minVal;
        int n = (int)(offs/stepping);
        double n2 = (double)offs/stepping;
        return Range<Type>::contains(value) && double(n) == n2;
      }

      /// returns the nearest value to the given one
      /** -1st: value is clipped to the underlying range
          -2nd: if value is inside the range, the nearest
                implicit step is returned
      **/
      Type nearest(Type value){
        if(value < Range<Type>::minVal) return Range<Type>::minVal;
        if(value > Range<Type>::maxVal) return Range<Type>::maxVal;
        if(stepping == 0) return value;
        Type offs = value - Range<Type>::minVal;
        int n = (int)(offs/stepping);
        Type k = n*stepping;
        if(value-k >= (double)(stepping)/2){
          return k+stepping;
        }else{
          return k;
        }
      }
    };

    /// puts a string representation [min,max]:step of given range into the given stream
    /** Available for all icl-Types (icl8u,icl16s, icl32s, icl32f and icl64f and
        for unsigned int */
    template<class T> ICLUtils_API
    std::ostream &operator<<(std::ostream &s, const SteppingRange <T> &range);

    /// parses a range argument into a std::string
    template<class T> ICLUtils_API
    std::istream &operator>>(std::istream &s, SteppingRange <T> &range);


  #define ICL_INSTANTIATE_DEPTH(D) typedef SteppingRange<icl##D> SteppingRange##D;
    ICL_INSTANTIATE_ALL_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

  } // namespace utils
}
