/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_DISP_HANDLE_H
#define ICL_DISP_HANDLE_H

#include <ICLUtils/SimpleMatrix.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/GUIHandle.h>

namespace icl{
  
  /** \cond */
  struct LabelHandleAlloc{
    static LabelHandle create() { return LabelHandle(); }
  };
  /** \endcond */
  
  /// Type definition for handling GUI-"disp" components \ingroup HANDLES
  /** @see GUI, SimpleMatrix template class of the ICLUtils package */
  typedef SimpleMatrix<LabelHandle,LabelHandleAlloc>  LabelMatrix;

  /// Handle class for disp components \ingroup HANDLE
  class DispHandle : public GUIHandle<LabelMatrix>{
    public:
    /// create an empty handle
    DispHandle(){}

    /// Create a new DispHandle with given 
    DispHandle(LabelMatrix *lm, GUIWidget *w) : GUIHandle<LabelMatrix>(lm,w){}
    
    /// column access operator (use h[x][y] to access elements)
    inline LabelHandle *operator[](int x){ return (***this)[x]; }

    /// column access operator (use h[x][y] to access elements)
    inline const LabelHandle *operator[](int x) const{ return (***this)[x]; }

    /// width of the matrix (max x index is w()-1
    inline int w() const { return (***this).w(); }

    /// height of the matrix (max y index is h()-1
    inline int h() const { return (***this).h(); }

    /// returns the number of matrix elements w*h
    inline int dim() const { return (***this).dim(); }
  
  };

}

#endif
