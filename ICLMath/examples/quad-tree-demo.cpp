/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/examples/quad-tree-example.cpp                 **
** Module : ICLMath                                                **
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


#include <ICLQt/Common.h>
#include <ICLUtils/Random.h>
#include <ICLMath/QuadTree.h>

Time t;
std::string what;
inline void tic(const std::string &what){
  ::what = what;
  t = Time::now();
}

inline void toc(){
  std::cout << "Time for " << what << ": " << t.age().toMilliSecondsDouble() << "ms" << std::endl;
}

void init(){
  HBox gui;
  
  gui << Plot().handle("plot").minSize(64*0.7,48*0.7) << Show();
  
  PlotHandle plot = gui["plot"];
  
  //GRandClip rx(320,3*32, Range64f(0,640));
  //GRandClip ry(240,3*24, Range64f(0,480));
  URand rx(0,639), ry(0,479);
  
  typedef QuadTree<icl32s,32,1,1024> QT;
  typedef QT::Pt Pt;
  QT t(Size::VGA);

  
  // create data
  std::vector<Pt> ps(100*1000);
  for(size_t i=0;i<ps.size();++i){
    ps[i] = Pt(rx,ry);
  }
  
  // insert data into the QuadTree
  ::tic("insertion");
  for(size_t i=0;i<ps.size();++i){
    t.insert(ps[i]);
  }
  ::toc();

  //  t.printStructure();
  
  plot->sym('x');  
  plot->scatter(ps.data(),ps.size());
  
  
  /// Query a huge rectangular region with 57% coverage
  Rect r(100,100,500,350);
  ::tic("query");
  for(int i=0;i<100;++i){
    ps = t.query(r);
  }
  ::toc();


  // visualize
  plot->color(0,255,0);
  plot->rect(r);
  plot->scatter(ps.data(),ps.size());
  plot->setDataViewPort(Rect32f(0,0,640,480));
  plot->setPropertyValue("tics.x-distance",50);
  plot->setPropertyValue("tics.y-distance",50);

  plot->nofill();
  plot->color(0,100,255);
  VisualizationDescription d = t.vis();
  plot->draw(d);
  plot->color(255,100,0);


  // create seed points for nn-search
  ps.resize(1000);
  for(size_t i=0;i<ps.size();++i){
    ps[i] = Pt(rx,ry);
  }
  std::vector<Pt> nn(ps.size());

  // precaching ...
  for(size_t i=0;i<ps.size();++i){
    nn[i] = t.nn(ps[i]);
  }

 /// for each seed point: find nn
  ::tic("approx nearest neighbor search");
  for(size_t i=0;i<ps.size();++i){
    nn[i] = t.nn_approx(ps[i]);
  }
  ::toc();

#if 1
  /// for each seed point: find nn
  ::tic("nearest neighbor search");
  for(size_t i=0;i<ps.size();++i){
    nn[i] = t.nn(ps[i]);
  }
  ::toc();
#endif
 


  // visualize nn-search results
  plot->sym('o');
  plot->linewidth(2);
  plot->scatter(ps.data(),ps.size());
  for(size_t i=0;i<ps.size();++i){
    Pt p = ps[i], n = nn[i];
    plot->line(p[0],p[1],n[0],n[1]);
  }
}


int main(int n, char **ppc){
  return ICLApp(n,ppc,"",init).exec();
}
