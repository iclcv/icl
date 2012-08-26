/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/simplex-2D-demo.cpp                   **
** Module : ICLGeom                                                **
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

#include <ICLCV/Common.h>
#include <ICLMath/SimplexOptimizer.h>


GUI gui;

typedef FixedColVector<float,2> Pos;

float error_function(const Pos &p){
  return ::sqrt(sqr(p[0]-400) + sqr(p[1]-550));
}

void init(){
  gui << Draw().minSize(20,20).handle("draw") << Show();
  
  Img32f bg(Size(1000,1000),1);
  Channel32f bgc = bg[0];
  for(int y=0;y<bg.getHeight();++y){
    for(int x=0;x<bg.getWidth();++x){
      bgc(x,y) = error_function(Pos(x,y));
    }     
  }
  DrawHandle draw = gui["draw"];
  draw = norm(bg);
  draw->setAutoResetQueue(false);
  
}

void run(){
  static SimplexOptimizer<float,Pos> opt(error_function,2,1);
  static std::vector<Pos> curr = SimplexOptimizer<float,Pos>::createDefaultSimplex(Pos(950,840));
  static float err = 10000;
  
  DrawHandle draw = gui["draw"];
  
  std::vector<Point32f> ps(curr.size());
  for(unsigned int i=0;i<ps.size();++i) ps[i] = Point32f(curr[i][0],curr[i][1]);
  static int step = 0;
  ++step;
  draw->color((!(step%3))*255, (!((step+1)%3))*255, (!((step+2)%3))*255,255);
  draw->linestrip(ps);
  draw->fill(255,255,255,255);
  draw->color(0,0,0,0);
  draw->rect(0,25,400,50);

  draw->color(255,0,0,255);
  draw->text("error: " + str(err), 30,30,10);
  draw.render();

  SimplexOptimizer<float,Pos>::Result r = opt.optimize(curr);

  curr = r.vertices;
  err = r.fx;
  Thread::msleep(100);  
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init,run).exec();
};
