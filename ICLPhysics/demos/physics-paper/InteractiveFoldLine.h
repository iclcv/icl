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
  
