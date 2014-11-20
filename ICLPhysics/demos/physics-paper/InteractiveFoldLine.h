/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/demos/physics-paper/InteractiveFoldLine.h   **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
#ifndef ICL_INTERACTIVE_FOLD_LINE_H
#define ICL_INTERACTIVE_FOLD_LINE_H

#include <ICLQt/MouseHandler.h>
#include <ICLQt/MouseEvent.h>
#include <ICLUtils/Function.h>

namespace icl{
  
  struct InteractiveFoldLine : public MouseHandler{
    Point32f a,b;
    bool done;
    typedef Function<void,const Point32f&,const Point32f&> FinishCB;
    FinishCB cb;
    
    InteractiveFoldLine():a(Point32f::null),b(Point32f::null),done(false){}
    
    void process(const MouseEvent &e){
      if(!e.isModifierActive(ControlModifier) || e.isRight()){
        a = b = Point32f::null;
        done = false;
      }else{
      if(e.isPressEvent()){
        done = false;
        a = b = e.getPos();
      }else if(e.isReleaseEvent()){
        done = true;
        if(cb) cb(a,b);
      }else if(!done){
        b = e.getPos();
      }
      }
    }
    void visualize(ICLDrawWidget &w){
      if(a != Point32f::null){
      if(done){
        w.color(255,0,0);
      }else{
        w.color(0,255,0);
      }
      w.linewidth(3);
      w.line(a,b);
      }    
    }
  };

}

#endif
  
