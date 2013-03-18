/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ColorHandle.h                          **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

#include <ICLQt/GUIHandle.h>
#include <ICLQt/ColorLabel.h>

namespace icl{
  namespace qt{
  
    /// Class wrapping ColorLabel GUI compoenent handling \ingroup HANDLES
    class ColorHandle : public GUIHandle<ColorLabel>{
      public:
      
      /// Create an empty handle
      ColorHandle(){}
      
      /// Create a new LabelHandle
      ColorHandle(ColorLabel *l, GUIWidget *w):GUIHandle<ColorLabel>(l,w){}
      
      /// sets new rgb color
      inline void operator=(const core::Color &rgb){
        lab()->setColor(rgb);
      }
  
      /// sets new rgba color (alpha is only used if alpha is enabled for the gui component)
      void operator=(const core::Color4D &rgba){
        lab()->setColor(rgba);
      }
      
      /// returns current rgb color
      inline core::Color getRGB() const { return lab()->getRGB(); }
  
      /// convenienc function that is the same as getRGBA()
      inline core::Color4D getColor() const { return lab()->getRGBA(); }
      
      /// returns current rgba color
      inline core::Color4D getRGBA() const { return lab()->getRGBA(); }
      
      /// return whether wrapped ColorLabel supports alpha
      inline bool hasAlpha() const { return lab()->hasAlpha(); }
      
      private:
      /// utitlity function
      ColorLabel *lab() { return **this; }
  
      /// utitlity function
      const ColorLabel *lab() const { return **this; }
    };
  
   
    
   
  } // namespace qt
}                               

