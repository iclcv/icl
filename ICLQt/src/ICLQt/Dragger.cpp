/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/Dragger.cpp                            **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLQt/Dragger.h>
#include <ICLQt/DrawWidget.h>

namespace icl{
  namespace qt{
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
  } // namespace qt
}
