// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLQt/Common.h>
#include <ICLMath/SimplexOptimizer.h>


GUI gui;

typedef FixedColVector<float,2> Pos;

float error_function(const Pos &p){
  return ::sqrt(sqr(p[0]-400) + sqr(p[1]-550));
}

void init(){
  gui << Canvas().minSize(20,20).handle("draw") << Show();

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
