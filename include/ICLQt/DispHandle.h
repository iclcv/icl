/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/DispHandle.h                             **
** Module : ICLQt                                                  **
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

    /// enables all components
    inline void enable(){
      for(int i=0;i<w();++i){
        for(int j=0;j<h();++j){
          (*this)[i][j].enable();
        }
      }
    }
    
    /// disables all components
    inline void disable(){
      for(int i=0;i<w();++i){
        for(int j=0;j<h();++j){
          (*this)[i][j].disable();
        }
      }
    }


  };

}

#endif
