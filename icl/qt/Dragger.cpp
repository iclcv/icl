// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Dragger.h>
#include <icl/qt/DrawWidget.h>

namespace icl::qt {
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
  } // namespace icl::qt