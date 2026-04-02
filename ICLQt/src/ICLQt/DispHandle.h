// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Array2D.h>
#include <ICLQt/LabelHandle.h>
#include <ICLQt/GUIHandle.h>

namespace icl::qt {
  /// Type definition for handling GUI-"disp" components \ingroup HANDLES
  /** @see GUI, Array2D template class of the ICLUtils package */
  using LabelMatrix = utils::Array2D<LabelHandle>;

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

  } // namespace icl::qt