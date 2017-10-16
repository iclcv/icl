/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/Range.h                          **
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
#include <ICLUtils/BasicTypes.h>
#include <ICLUtils/ClippedCast.h>
#include <ICLUtils/Macros.h>

#include <algorithm>
#include <limits>
#include <iostream>
#include <string>

namespace icl{
  namespace utils{


    /// class representing a range defined by min and max value \ingroup TYPES
    template<class Type>
    struct Range{

      /** \cond */

      template<class T> struct MinMaxFunctor{
      MinMaxFunctor(Range<T> *r):r(r){}
        Range<T> *r;
        inline void operator()(const T &x){
          r->minVal = std::min(r->minVal,x);
          r->maxVal = std::max(r->maxVal,x);
        }
      };
      /** \endcond */

      virtual ~Range(){}
      /// create an empty range (min = max = 0)
      Range():minVal(Type(0)),maxVal(Type(0)){}

      /// create a special Range
      Range(Type minVal, Type maxVal): minVal(minVal), maxVal(maxVal){}

      /// returns type range (using std::numeric_limits<Type>)
      static inline Range<Type> limits(){
        return Range<Type>((std::numeric_limits<Type>::min)(),(std::numeric_limits<Type>::max)());
      }

      /// returns inverted limits range [numeric-limits.max,numeric-limits.min]
	  static inline Range<Type> inv_limits(){
        return Range<Type>((std::numeric_limits<Type>::max)(),(std::numeric_limits<Type>::min)());
      }

      /// estimate range of given iterator range (using std::for_each)
      static inline Range<Type> from(const Type *begin,const Type *end){
        Range<Type> r = Range<Type>::limits();
        std::swap(r.minVal,r.maxVal);
        std::for_each(begin,end,MinMaxFunctor<Type>(&r));
        return r;
      }

      /// minimum value of this range
      Type minVal;

      /// maximum value of this range
      Type maxVal;

      /// return max-min
      Type getLength() const { return maxVal - minVal; }

      /// casts this range into another depth
      template<class dstType>
      const Range<dstType> castTo() const{
        return Range<dstType>(clipped_cast<Type,dstType>(minVal),clipped_cast<Type,dstType>(maxVal));
      }

      /// tests whether a given value is inside of this range
      virtual bool contains(Type value) const { return value >= minVal && value <= maxVal; }

      // camparison operator
      bool operator==(const Range<Type> &other) const{
        return minVal==other.minVal && maxVal == other.maxVal;
      }

      // camparison operator
      bool operator!=(const Range<Type> &other) const{
        return minVal!=other.minVal || maxVal != other.maxVal;
      }

      /// extends the range so that the given value is within this range
      void extend(const Type &t){
        if(t < minVal) minVal = t;
        if(t > maxVal) maxVal = t;
      }

      /// extends the range so that the given range is within this range
      void extend(const Range &r){
        if(r.minVal < minVal) minVal = r.minVal;
        if(r.maxVal > maxVal) maxVal = r.maxVal;
      }
    };

  #define ICL_INSTANTIATE_DEPTH(D) typedef Range<icl##D> Range##D;
    ICL_INSTANTIATE_ALL_DEPTHS
  #undef ICL_INSTANTIATE_DEPTH

    /// puts a string representation [min,max] of given range into the given stream
    /** Available for all icl-Types (icl8u,icl16s, icl32s, icl32f and icl64f and
        for unsigned int */
    template<class T> ICLUtils_API
    std::ostream &operator<<(std::ostream &s, const Range <T> &range);

    /// parses a range argument into a std::string
    template<class T> ICLUtils_API
    std::istream &operator>>(std::istream &s, Range <T> &range);

  } // namespace utils
}
