/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
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

#pragma once

#include <ICLUtils/Array2D.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/GUIHandle.h>

namespace icl{
  namespace qt{
    
    
    /// Type definition for handling GUI-"disp" components \ingroup HANDLES
    /** @see GUI, Array2D template class of the ICLUtils package */
    typedef Array2D<LabelHandle>  LabelMatrix;
  
    /// Handle class for disp components \ingroup HANDLE
    class DispHandle : public GUIHandle<LabelMatrix>{
      public:
      /// create an empty handle
      DispHandle(){}
  
      /// Create a new DispHandle with given 
      DispHandle(LabelMatrix *lm, GUIWidget *w) : GUIHandle<LabelMatrix>(lm,w){}
      
      /// index access operator
      inline LabelHandle &operator()(int x,int y){ return (***this)(x,y); }
  
      /// index access operator (const)
      inline const LabelHandle &operator()(int x,int y) const { return (***this)(x,y); }
  
  
      /// width of the matrix (max x index is w()-1
      inline int getWidth() const { return (***this).getWidth(); }
  
      /// height of the matrix (max y index is h()-1
      inline int getHeight() const { return (***this).getHeight(); }
  
      /// returns the number of matrix elements w*h
      inline int getDim() const { return (***this).getDim(); }
  
      /// enables all components
      inline void enable(){
        for(int i=0;i<getWidth();++i){
          for(int j=0;j<getHeight();++j){
            (*this)(i,j).enable();
          }
        }
      }
      
      /// disables all components
      inline void disable(){
        for(int i=0;i<getWidth();++i){
          for(int j=0;j<getHeight();++j){
            (*this)(i,j).disable();
          }
        }
      }
  
  
    };
  
  } // namespace qt
}

