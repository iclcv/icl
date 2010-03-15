/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/ClippedCast.h                         **
** Module : ICLUtils                                               **
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
*********************************************************************/

#ifndef ICL_CLIPPED_CAST_H
#define ICL_CLIPPED_CAST_H

#include <limits>

namespace icl{
  /// clips a value into the range [tMin,tMax] \ingroup GENERAL
  template <class T>
  inline T clip(T tX, T tMin, T tMax){ 
    return tX < tMin ? tMin : tX > tMax ? tMax : tX; 
  }
  
  
  template<class T>
  inline bool is_float_type(){
    return false;
  }
  
  /** \cond */
  template<> inline bool is_float_type<float>() { return true; }  
  template<> inline bool is_float_type<double>() { return true; }  
  /** \endcond */
  
  /// utility cast function wrapping the standard lib's numerical_limits template
  template<class S, class D> 
  inline D clipped_cast(S src){
    if(is_float_type<D>()){ //hopefully this is const enough for optimize this expresseion out
      return src < -std::numeric_limits<D>::max() ? -std::numeric_limits<D>::max() : 
      src > std::numeric_limits<D>::max() ? std::numeric_limits<D>::max() : 
      static_cast<D>(src);
    }else{
      return src < std::numeric_limits<D>::min() ? std::numeric_limits<D>::min() : 
      src > std::numeric_limits<D>::max() ? std::numeric_limits<D>::max() : 
      static_cast<D>(src);
    }
  }
  
  /** \cond */
  /// specializations for all buildin data types
#define SPECIALISE_CLIPPED_CAST(T) template<> inline T clipped_cast<T,T>(T t) { return t; }
  SPECIALISE_CLIPPED_CAST(int)
  SPECIALISE_CLIPPED_CAST(unsigned int)
  SPECIALISE_CLIPPED_CAST(char)
  SPECIALISE_CLIPPED_CAST(unsigned char)
  SPECIALISE_CLIPPED_CAST(short)
  SPECIALISE_CLIPPED_CAST(unsigned short)
  SPECIALISE_CLIPPED_CAST(long)
  SPECIALISE_CLIPPED_CAST(unsigned long)
  SPECIALISE_CLIPPED_CAST(bool)
  SPECIALISE_CLIPPED_CAST(float)
  SPECIALISE_CLIPPED_CAST(double)
#undef SPECIALISE_CLIPPED_CAST
  /** \endcond */

}


#endif
