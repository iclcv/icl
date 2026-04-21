// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/utils/Random.h>
#include <icl/math/QuadTree.h>

GUI gui;

void init(){
  gui << Plot().handle("plot").minSize(64*0.7,48*0.7) << Show();

  PlotHandle plot = gui["plot"];

  URand rx(0,639), ry(0,479);

  using QT = QuadTree<icl32s,32,1,1024>;
  using Pt = QT::Pt;
  QT tree(Size::VGA);

  // create and insert 100k random points
  std::vector<Pt> ps(100000);
  for(auto &p : ps) p = Pt(rx, ry);

  Time t = Time::now();
  for(auto &p : ps) tree.insert(p);
  t.showAge("insertion of 100k points");

  // visualize all points
  plot->sym('x');
  plot->scatter(ps.data(), ps.size());

  // query rectangular region (57% coverage)
  Rect r(100, 100, 500, 350);
  t = Time::now();
  std::vector<Pt> qresult;
  for(int i = 0; i < 100; ++i) qresult = tree.query(r);
  t.showAge("100x query (57% coverage)");

  plot->color(0, 255, 0);
  plot->rect(r);
  plot->scatter(qresult.data(), qresult.size());
  plot->setDataViewPort(Rect32f(0, 0, 640, 480));
  plot->setPropertyValue("tics.x-distance", 50);
  plot->setPropertyValue("tics.y-distance", 50);

  // visualize quad-tree structure
  plot->nofill();
  plot->color(0, 100, 255);
  plot->draw(tree.vis());

  // nearest neighbor search
  ps.resize(1000);
  for(auto &p : ps) p = Pt(rx, ry);
  std::vector<Pt> nn(ps.size());

  t = Time::now();
  for(size_t i = 0; i < ps.size(); ++i) nn[i] = tree.nn_approx(ps[i]);
  t.showAge("1000x approx nn search");

  t = Time::now();
  for(size_t i = 0; i < ps.size(); ++i) nn[i] = tree.nn(ps[i]);
  t.showAge("1000x exact nn search");

  // visualize nn-search results
  plot->color(255, 100, 0);
  plot->sym('o');
  plot->linewidth(2);
  plot->scatter(ps.data(), ps.size());
  for(size_t i = 0; i < ps.size(); ++i){
    plot->line(ps[i][0], ps[i][1], nn[i][0], nn[i][1]);
  }
}

int main(int n, char **ppc){
  return ICLApp(n, ppc, "", init).exec();
}
