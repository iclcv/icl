/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DefineRectanglesMouseHandler.cpp       **
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

#include <ICLQt/DefineRectanglesMouseHandler.h>
#include <ICLQt/DrawWidget.h>
#include <ICLUtils/CompatMacros.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace qt{
    
    DefineRectanglesMouseHandler::Options::Options(const Color4D &edgeColor,
                                                   const Color4D &fillColor,
                                                   const Color4D &centerColor,
                                                   const Color4D &metaColor,
                                                   int handleWidth, 
                                                   bool visualizeCenter,
                                                   bool visualizeHovering,
                                                   bool showOffsetText,
                                                   bool showSizeText, 
                                                   bool showCenterText,
                                                   bool showMetaData,
                                                   int lineWidth,
                                                   float textSize):
    edgeColor(edgeColor),fillColor(fillColor),
      centerColor(centerColor),metaColor(metaColor),
      handleWidth(handleWidth),visualizeCenter(visualizeCenter),visualizeHovering(visualizeHovering),
      showOffsetText(showOffsetText),showSizeText(showSizeText),
      showCenterText(showCenterText),showMetaData(showMetaData),
      lineWidth(lineWidth),textSize(textSize),xStepping(1),yStepping(1),canDeleteRects(true){
    }
  
    DefineRectanglesMouseHandler::DefinedRect::DefinedRect(const Rect &r, 
                                                           DefineRectanglesMouseHandler::Options *options):
      Rect(r),options(options){
      std::fill(states,states+4,nothing);
    }
        
    inline Rect DefineRectanglesMouseHandler::DefinedRect::edge(DefineRectanglesMouseHandler::DefinedRect::Edge e) const{
      const int &D = options->handleWidth;
      const int D2 = 2*D;
      switch(e){
        case T: return Rect(x-D,y-D,width+D2,D2);
        case R: return Rect(right()-D, y-D, D2, height+D2);
        case B: return Rect(x-D,bottom()-D,width+D2,D2);
        case L: return Rect(x-D,y-D, D2, height+D2);
        default: return Rect::null;
      }
    }
    inline Rect DefineRectanglesMouseHandler::DefinedRect::edgei(int i) const{ return edge((Edge)i); }
    
    inline Rect DefineRectanglesMouseHandler::DefinedRect::inner() const { return enlarged(-options->handleWidth); }
    inline Rect DefineRectanglesMouseHandler::DefinedRect::outer() const { return enlarged(options->handleWidth); }
    inline bool DefineRectanglesMouseHandler::DefinedRect::allHovered() const { 
      return states[0] == hovered && states[1] == hovered &&
      states[2] == hovered && states[3] == hovered ;
    }
    inline bool DefineRectanglesMouseHandler::DefinedRect::allDragged() const { 
      return states[0] == dragged && states[1] == dragged &&
      states[2] == dragged && states[3] == dragged ;
    }
    inline bool DefineRectanglesMouseHandler::DefinedRect::anyDragged() const { 
      return states[0] == dragged || states[1] == dragged ||
      states[2] == dragged || states[3] == dragged ;
    }
        
    DefineRectanglesMouseHandler::DefinedRect::State DefineRectanglesMouseHandler::DefinedRect::event(const MouseEvent &e){
      int x = e.getX(), y=e.getY();
      x = (x/options->xStepping)*options->xStepping;
      y = (y/options->yStepping)*options->yStepping;
      if(!anyDragged() && !outer().contains(x,y)){
        for(int i=0;i<4;++i){
          states[i] = nothing;
        }
        return nothing;
      }
      
      if(e.isReleaseEvent() || e.isMoveEvent()){
        if(inner().contains(x,y)){
          std::fill(states,states+4,hovered);
        }else{
          for(int i=0;i<4;++i){
            states[i] = edgei(i).contains(x,y) ? hovered : nothing;
          }
        }
      }else if(e.isPressEvent()){
        if(allHovered()){
          std::fill(states,states+4,dragged);
          allDragOffs.x = x-this->x;
          allDragOffs.y = y-this->y;
          return dragged;
        }else{
          for(int i=0;i<4;++i){
            if(states[i] == hovered) states[i] = dragged;
          }
        }
      }else if(e.isDragEvent()){
        if(allDragged()){
          this->x = x - allDragOffs.x;
          this->y = y - allDragOffs.y;
        }else{
          if(states[L] == dragged) {
            this->width -= x-this->x;
            this->x = x;
          }
          if(states[T] == dragged){
            this->height -= y-this->y;
            this->y = y;
          }
          if(states[R] == dragged) this->width = x - this->x;
          if(states[B] == dragged) this->height = y - this->y;
          // move all dragged instances
        }
        (Rect&)(*this) = normalized();
        return dragged;
      }
      
      return hovered;
    }
    
    void DefineRectanglesMouseHandler::DefinedRect::visualize(ICLDrawWidget &w){
      w.linewidth(options->lineWidth);
      w.color(options->edgeColor);
      w.fill(options->fillColor);
      w.rect((Rect&)*this);
      w.color(0,0,0,0);
      
      if(options->visualizeHovering){
        for(int i=0;i<4;++i){
          switch(states[i]){
            case hovered:{
              Color4D c = options->edgeColor; c[3] = 40;
              w.fill(c);
              w.rect(edgei(i));
              w.rect(edgei(i).enlarged(-1));
              w.rect(edgei(i).enlarged(-2));
              break;
            }
            case dragged:{
              Color4D c = options->edgeColor; c[3] = 80;
              w.fill(c);
              w.rect(edgei(i));
              w.rect(edgei(i).enlarged(-1));
              w.rect(edgei(i).enlarged(-2));
              break;
            }
            default:
              break;
          }
        }
      }
      if(options->visualizeCenter){
        w.color(options->centerColor);
        w.line(ul(),lr());
        w.line(ur(),ll());
      }
      if(options->showOffsetText || options->showSizeText || options->showCenterText){
        w.color(options->edgeColor);
        int y = this->y + 10;
        if(options->showOffsetText){
          w.text("offs: " + str(ul()),x+1,y,options->textSize);
          y += options->textSize+2;
        }
        if(options->showSizeText){
          Size s = getSize();
          //w.text("size: " + str(getSize()),x+1,y,options->textSize);
          w.text(str(s.width), x+s.width*0.3, this->y + s.height - options->textSize-2);
          w.textangle(-90);
          w.text(str(s.height), this->x - this->options->textSize-2, this->y + s.height *0.3);
          w.textangle(0);
        }
        if(options->showCenterText){
          w.text("center: " + str(center()),x+1,y,options->textSize);
        }
      }
      if(options->showMetaData){
        w.color(options->metaColor);
        w.text(meta,center().x,center().y,options->textSize);
      }
    }
  
  
    DefineRectanglesMouseHandler::DefineRectanglesMouseHandler(int maxRects, int minDim):
      Lockable(true),maxRects(maxRects),minDim(minDim),draggedRect(0){
    }
  
    DefineRectanglesMouseHandler::Options &DefineRectanglesMouseHandler::getOptions(){
      return options;
    }
    const DefineRectanglesMouseHandler::Options &DefineRectanglesMouseHandler::getOptions() const{
      return options;
    }
  
  
    void DefineRectanglesMouseHandler::process(const MouseEvent &e){
      Mutex::Locker l(this);

      struct CallCallbacksAtEnd{
        Function<void> f;
        CallCallbacksAtEnd(Function<void> f):f(f){}
        ~CallCallbacksAtEnd(){ f(); }
      } call_callbacks_at_end(function(this, &DefineRectanglesMouseHandler::callCallbacks));
 
      if(draggedRect){
        draggedRect->event(e);
        if(e.isReleaseEvent()){
          if(draggedRect->getDim() < minDim){
            for(unsigned int i=0;i<rects.size();++i){
              if(&rects[i] == draggedRect){
                rects.erase(rects.begin()+i);
                break;
              }
            }
          }
          draggedRect = 0;
        }
        return;
      }

      int x = (e.getX()/options.xStepping)*options.xStepping;
      int y = (e.getY()/options.yStepping)*options.yStepping;
      Point pos(x,y);
      
      if(currBegin == Point::null && currCurr == Point::null){
        for(unsigned int i=0;i<rects.size();++i){
          if(e.isPressEvent() && rects[i].contains(x,y)){
            if(e.isRight() && options.canDeleteRects){
              rects.erase(rects.begin()+i);
              return;
            }else if(e.isMiddle()){
              if(i != rects.size()-1){
                std::swap(rects[i],rects.back());
              }
              return;
            }
          }
  
          DefinedRect::State state = rects[i].event(e);
          if(state == DefinedRect::nothing) {
            continue;
          }else if(state == DefinedRect::hovered){
            return;
          }else{
            draggedRect = &rects[i];
            return;
          }
        }
      }
      
      if(!e.isLeft()) return;
  
      if(e.isPressEvent()){
        currBegin  = pos;
        currCurr   = pos;
      }else if(e.isDragEvent()){
        currCurr   = pos;
      }else if(e.isReleaseEvent()){
        draggedRect = 0;
        Rect r(currBegin, Size(currCurr.x-currBegin.x,currCurr.y-currBegin.y ));
        if(r.getDim() >= minDim && (int)rects.size() < maxRects) {
          rects.push_back(DefinedRect(r.normalized(),&options));
        }
        currCurr = currBegin = Point::null;
      }
    }
    
    void DefineRectanglesMouseHandler::visualize(ICLDrawWidget &w){
      Mutex::Locker l(this);
      for(unsigned int i=0;i<rects.size();++i){
        rects[i].visualize(w);
      }
      
      if(currBegin != Point::null || currCurr != Point::null){
        w.color(options.edgeColor);
        w.fill(options.fillColor);
        Rect r(currBegin, Size(currCurr.x-currBegin.x,currCurr.y-currBegin.y ));
        w.rect(r.normalized());
        if(options.visualizeCenter){
          w.color(options.centerColor);
          w.line(r.ul(),r.lr());
          w.line(r.ll(),r.ur());
        }
      }
    }
  
    void DefineRectanglesMouseHandler::clearAllRects(){
      Mutex::Locker l(this);
      if(rects.size()){
        rects.clear();
      }
    }
    
    void DefineRectanglesMouseHandler::clearRectAt(int x, int y, bool all){
      Mutex::Locker l(this);
      
      bool anyChanged = false;
      for(unsigned int i=0;i<rects.size();++i){
        if(rects[i].contains(x,y)){
          rects.erase(rects.begin()+i--);
          anyChanged = true;
          if(!all) break;
        }
      }
      if(anyChanged){
      }
    }
    
    void DefineRectanglesMouseHandler::addRect(const Rect &rect){
      Mutex::Locker l(this);
      if((int)rects.size() < maxRects && rect.getDim() >= minDim){ 
        rects.push_back(DefinedRect(rect,&options));
      }
    }
    
    void DefineRectanglesMouseHandler::setMaxRects(int maxRects){
      Mutex::Locker l(this);
      this->maxRects = maxRects;
      if((int)rects.size() > maxRects){
        rects.resize(maxRects);
      }
    }


    void DefineRectanglesMouseHandler::callCallbacks(){
      std::vector<Rect> rects = getRects();
      for(std::map<std::string,Callback>::iterator it = callbacks.begin();
          it != callbacks.end();++it){
        it->second(rects);
      }
    }
    
    void DefineRectanglesMouseHandler::registerCallback(const std::string &id, Callback cb){
      Mutex::Locker l(this);
      callbacks[id] = cb;
    }
    
    void DefineRectanglesMouseHandler::unregisterCallback(const std::string &id){
      Mutex::Locker l(this);
      std::map<std::string,Callback>::iterator it = callbacks.find(id);
      if(it != callbacks.end()){
        callbacks.erase(it);
      }else{
        ERROR_LOG("could not unregister callback " << id << " (no callback registered under this ID");
      }
    }

    
    void DefineRectanglesMouseHandler::setMinDim(int minDim){
      Mutex::Locker l(this);
      this->minDim = minDim;
      for(unsigned int i=0;i<rects.size();++i){
        if(rects[i].getDim() < minDim){
          rects.erase(rects.begin()+i--);
        }
      }
    }
    
    
    std::vector<Rect> DefineRectanglesMouseHandler::getRects() const{
      Mutex::Locker l(this);
      return std::vector<Rect>(rects.begin(),rects.end());
    }
    
    Rect DefineRectanglesMouseHandler::getRectAt(int x, int y) const{
      Mutex::Locker l(this);
      for(unsigned int i=0;i<rects.size();++i){
        if(rects[i].contains(x,y)){
          return rects[i];
        }
      }
      return Rect::null;
    }
    
    std::vector<Rect> DefineRectanglesMouseHandler::getAllRectsAt(int x, int y) const{
      Mutex::Locker l(this);
      std::vector<Rect> rs;
      for(unsigned int i=0;i<rects.size();++i){
        if(rects[i].contains(x,y)){
          rs.push_back(rects[i]);
        }
      }
      return rs;
    }
    
    int DefineRectanglesMouseHandler::getMinDim() const{
      return minDim;
    }
    
    int DefineRectanglesMouseHandler::getMaxRects() const{
      return maxRects;
    }
  
    int  DefineRectanglesMouseHandler::getNumRects() const{
      Mutex::Locker l(this);
      return (int)rects.size();
    }
    
    Rect  DefineRectanglesMouseHandler::getRectAtIndex(int index) const{
      Mutex::Locker l(this);
      if(index < 0 || index >= (int)rects.size()) return Rect::null;
      return rects[index];
    }
      
    const Any &DefineRectanglesMouseHandler::getMetaData(int index) const{
      Mutex::Locker l(this);
      static Any null;
      if(index < 0 || index >= (int)rects.size()) return null;
      return rects[index].meta;
    }
    
    void DefineRectanglesMouseHandler::setMetaData(int index, const Any &data){
      Mutex::Locker l(this);
      ICLASSERT_RETURN(index >= 0 && index < (int)rects.size());
      rects[index].meta = data;
    }
  
    const Any &DefineRectanglesMouseHandler::getMetaDataAt(int x, int y) const{
      Mutex::Locker l(this);
      for(unsigned int i=0;i<rects.size();++i){
        if(rects[i].contains(x,y)){
          return rects[i].meta;
        }
      }
      static Any null;
      return null;
    }
  
    void DefineRectanglesMouseHandler::setMetaDataAt(int x, int y, const Any &meta){
      Mutex::Locker l(this);
      for(unsigned int i=0;i<rects.size();++i){
        if(rects[i].contains(x,y)){
          rects[i].meta = meta;
          return;
        }
      }
    }
  
    void DefineRectanglesMouseHandler::bringToFront(int idx){
      Mutex::Locker l(this);
      ICLASSERT_RETURN(idx >= 0 && idx < (int)rects.size());
      DefinedRect r = rects[idx];
      rects.erase(rects.begin()+idx);
      rects.insert(rects.begin(),1,r);
    }
      
    void DefineRectanglesMouseHandler::bringToBack(int idx){
      Mutex::Locker l(this);
      ICLASSERT_RETURN(idx >= 0 && idx < (int)rects.size());
      DefinedRect r = rects[idx];
      rects.erase(rects.begin()+idx);
      rects.push_back(r);
    }
    
  
  
  } // namespace qt
}
