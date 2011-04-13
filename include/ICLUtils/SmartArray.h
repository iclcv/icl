/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/SmartArray.h                          **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_SMART_ARRAY_H
#define ICL_SMART_ARRAY_H

#include <ICLUtils/SmartPtrBase.h>

namespace icl{
  
  /// Specialization of the SmartPtrBase class for Arrays
  /** If the internal reference counter becomes 0, the contained
      data pointer is release using <tt>delete []</tt>*/
  template<class T>
  struct SmartArray : public SmartPtrBase<T, ArrayDelOp>{
    // type definition for the parent class
    typedef SmartPtrBase<T,ArrayDelOp> super;
    /// creates a null pointer
    SmartArray():super(){}

    /// gets pointer, ownership is passed optionally
    template<class DerivedT>
    SmartArray(DerivedT *ptData, bool bOwn=true):super(ptData,bOwn){}

    /// gets pointer, ownership is passed optionally
    SmartArray(T *ptData, bool bOwn=true):super(ptData,bOwn){}

    /// reference counting copy constructor
    template<class DerivedT>
    SmartArray(const SmartPtrBase<DerivedT,ArrayDelOp>& r):super(r){}

    /// reference counting copy constructor
    SmartArray(const SmartPtrBase<T,ArrayDelOp>& r):super(r){}
    
    /// index access operator (no index checks)
    T &operator[](int idx){ ICLASSERT(super::e); return super::e[idx]; }
    
    /// index access operator (const, no index checks)
    const T&operator[](int idx) const{ ICLASSERT(super::e); return super::e[idx]; }
  };


}

#endif
