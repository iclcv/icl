#include <ICLQt/Dragger.h>
#include <ICLQt/DrawWidget.h>

namespace icl{
  void Dragger::draw( ICLDrawWidget *w ) const{
    w->color(c.r,c.g,c.b,c.a);
    if(dragged()){
      w->fill(iclMin(c.r+30,255),iclMin(c.g+30,255), iclMin(c.b+30,255),c.a/2);
    }else if(over()){
      w->fill(c.r,c.g,c.b,c.a/2);
    }else{
      w->nofill();
    }
    w->rect(r.x,r.y,r.width,r.height);
    w->line(p.x-d/2,p.y-d/2, p.x+d/2,p.y+d/2);
    w->line(p.x+d/2,p.y-d/2, p.x-d/2,p.y+d/2);
  }  
}
