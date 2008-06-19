#include <iclBorderedObject.h>
#include <stack>
#include <algorithm>
#include <iclImg.h>
#include <iclImgChannel.h>
#include <iclDrawWidget.h>
#include <vector>
#include <iclQuick.h>

#include "iclConvexHullMonotoneChain.h"

using std::vector;

namespace icl{
  bool BorderedObject::VISUALIZE_BORDERS = true;

  BorderedObject::BorderedObject(){
    // {{{ open

    m_vecBorders.reserve(MAX_BORDERS);
    m_poConvexHullLineStrip = addNewBorder();
  }

  // }}}
  
  BorderedObject::~BorderedObject(){
    /// Borders are objects and might not be deleted!
  }
  
  BorderedObject::LineStrip *BorderedObject::addNewBorder(){
    // {{{ open

    m_vecBorders.push_back(LineStrip());
    return &m_vecBorders[m_vecBorders.size()-1];
  }

  // }}}
  
  void BorderedObject::project(const Mat &m){
    // {{{ open

    Object::project(m);
    updateBorders();
  }

  // }}}
  
  void BorderedObject::updateBorders(){
    // {{{ open
    calculateConvexHull(*m_poConvexHullLineStrip);
  }

  // }}}
  
  void BorderedObject::render(ICLDrawWidget *widget) const{
    // {{{ open

    Object::render(widget);
    
    if (VISUALIZE_BORDERS){
      widget->color(0,255,0,255);
      widget->fill(0,0,0,0);
      const LineStripArray &borders = getBorders();
      for(unsigned int i=0;i<borders.size();++i){
        const LineStrip &b = borders[i];
        for(unsigned int j=0;j<b.size();j++){
          const Vec *v = b[j];
          widget->ellipse(v->x()-1,v->y()-1,1,1);
        }
      }
    }

  }

  // }}}

  void BorderedObject::render(Img32f *image) const{
    // {{{ open

    Object::render(image);
    if (VISUALIZE_BORDERS){
      /// Vis into the image
    }
  }

  // }}}
  
  bool BorderedObject::check() const{
    // {{{ open

    bool checkParent = Object::check();
    
    // Check the lineStrip of the convex border here!
    return checkParent;
  }

  // }}}

  
  void BorderedObject::calculateConvexHull(BorderedObject::LineStrip &dst){
    // {{{ open
    /* new:::
    dst.clear();
    VecArray &p = Object::getPointsProj();
    vector<Point> input(p.size());
    for(unsigned int i=0;i<p.size();++i){
      input[i] = Point(p[i].x(),p[i].y(),&p[i]);
    } 
    vector<Point> output == convexHull(input);

    for(int i=0;i<nHullPts;i++){
      dst.push_back(Vec(output[i].x,output[i].y));
    }
    */
    VecArray &p = Object::getPointsProj();
    vector<CHPoint> input(p.size());
    for(unsigned int i=0;i<p.size();++i){
      input[i] = CHPoint(p[i].x(),p[i].y(),&p[i]);
    } 
    std::sort(input.begin(),input.end());
    vector<CHPoint> output(p.size());
    int nHullPts = chainHull_2D(&input[0], (int)input.size(), &output[0]);
    for(int i=0;i<nHullPts;i++){
      dst.push_back(output[i].v);
    }
    
  }

  // }}}

} 
 
