/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLUtils/DynVector.h                           **
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

#ifndef ICL_DYN_VECTOR_H
#define ICL_DYN_VECTOR_H

#include <valarray>

namespace icl{
  template<class T>
  struct DynVector : public std::valarray<T>{
    DynVector():std::valarray<T>(){}
    explicit DynVector(size_t dim):std::valarray<T>(dim){};
    DynVector(const T &val, int dim):std::valarray<T>(val,dim){}
    DynVector(const T *data, int dim):std::valarray<T>(data,dim){}
    DynVector(const DynVector &v):std::valarray<T>(v){}
    
    
    DynVector(const std::slice_array< T> &a):std::valarray<T>(a){}
    DynVector(const std::gslice_array< T> &a):std::valarray<T>(a){}
    DynVector(const std::mask_array< T> &a):std::valarray<T>(a){}
    DynVector(const std::indirect_array< T> &a):std::valarray<T>(a){}
    
    void ensureSize(size_t dim){
      if(std::valarray<T>::size() != dim) std::valarray<T>::resize(dim);    
    }
    
    DynVector<T> &operator=(const DynVector<T> &v){
      ensureSize(v.size());
      return std::valarray<T>::operator=(v);
    }
    DynVector<T> &operator=(const T &t){
      return std::valarray<T>::operator=(t); 
    }
    DynVector<T> &operator=(const std::slice_array<T> &a){
      ensureSize(a.size());
      return std::valarray<T>::operator=(a);
    }
    DynVector<T> &operator=(const std::gslice_array<T> &a){ 
      ensureSize(a.size());
      return std::valarray<T>::operator=(a); 
    }
    DynVector<T> &operator=(const std::mask_array<T> &a){
      ensureSize(a.size());
      return std::valarray<T>::operator=(a); 
    }
    DynVector<T> &operator=(const std::indirect_array<T> &a){
      ensureSize(a.size());
      return std::valarray<T>::operator=(a); 
    }
  };
  
}

#endif
