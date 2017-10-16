/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ChromaGUI.cpp                          **
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


#include <ICLQt/SliderHandle.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QFileDialog>
#include <QPushButton>

#include <ICLQt/ChromaGUI.h>

#include <ICLCore/Parable.h>
#include <ICLCore/Img.h>

#include <ICLQt/ContainerGUIComponents.h>
#include <ICLQt/Dragger.h>
#include <ICLQt/DrawWidget.h>
#include <ICLQt/MouseHandler.h>

#include <ICLQt/BoxHandle.h>
#include <ICLQt/ButtonHandle.h>
#include <ICLQt/ChromaClassifierIO.h>

using namespace icl::utils;
using namespace icl::core;

using namespace std;
namespace icl{
  namespace qt{


    class ChromaWidget : public ICLDrawWidget, public MouseHandler{
      public:
      ChromaWidget(QWidget *parent = 0):ICLDrawWidget(parent){
        // {{{ open

        image = Img8u(Size(256,256),formatRGB);

        install(this);

        static const float d = 0.015;
        D[0] = Dragger(Point32f(0.1,0.1),d, Dragger::Color(255,0,0));
        D[1] = Dragger(Point32f(0.2,0.1),d, Dragger::Color(255,0,0));
        D[2] = Dragger(Point32f(0.3,0.1),d, Dragger::Color(255,0,0));

        D[3] = Dragger(Point32f(0.1,0.4),d, Dragger::Color(0,100,255));
        D[4] = Dragger(Point32f(0.2,0.4),d, Dragger::Color(0,100,255));
        D[5] = Dragger(Point32f(0.3,0.4),d, Dragger::Color(0,100,255));

        updateImage(128);
        updateDrawings();
      }

      // }}}
      virtual void process(const MouseEvent &event){
        // {{{ open

        Point32f p = event.getRelPos(); //Point32f(info->relImageX,info->relImageY);

        bool left = event.isLeft();//*(info->downmask);

        switch(event.getType()){
          case MouseMoveEvent:
            if(left){
              for(int i=0;i<6;i++){
                D[i].setOver(D[i].hit(p));
              }
            }else{
              if(D[0].hit(p) || D[1].hit(p) || D[2].hit(p)){
                D[0].setOver(true);D[1].setOver(true);D[2].setOver(true);
                D[3].setOver(false);D[4].setOver(false);D[5].setOver(false);
              }else if(D[3].hit(p) || D[4].hit(p) || D[5].hit(p)){
                D[0].setOver(false);D[1].setOver(false);D[2].setOver(false);
                D[3].setOver(true);D[4].setOver(true);D[5].setOver(true);
              }else{
                D[0].setOver(false);D[1].setOver(false);D[2].setOver(false);
                D[3].setOver(false);D[4].setOver(false);D[5].setOver(false);
              }
            }
            break;
          case MouseDragEvent:
            if(left){
              for(int i=0;i<6;i++){
                if(D[i].dragged()){
                  D[i].dragTo(p);
                  break;
                }
              }
            }else{
              if(D[0].dragged() || D[1].dragged() || D[2].dragged()){
                D[0].dragTo(p);D[1].dragTo(p);D[2].dragTo(p);
              }else if(D[3].dragged() || D[4].dragged() || D[5].dragged()){
                D[3].dragTo(p);D[4].dragTo(p);D[5].dragTo(p);
              }
            }
            break;
          case MousePressEvent:
            if(left){
              for(int i=0;i<6;i++){
                if(D[i].hit(p)){
                  D[i].drag(p);
                  break;
                }
              }
            }else{
              if(D[0].hit(p) || D[1].hit(p) || D[2].hit(p)){
                D[0].drag(p); D[1].drag(p); D[2].drag(p);
              }else if(D[3].hit(p) || D[4].hit(p) || D[5].hit(p)){
                D[3].drag(p); D[4].drag(p); D[5].drag(p);
              }
            }
            break;
          case MouseReleaseEvent:
            for(int i=0;i<6;i++){
              D[i].drop();
            }
            break;
          default: // do nothing
            break;
        }
        updateDrawings();
      }

      // }}}
      void setBlue(icl8u blue){
        // {{{ open

        this->blue = blue;
        updateImage(blue);
      }

      // }}}
      const core::Parable* getParables() const{
        // {{{ open

        return P;
      }

      // }}}
      void save(const std::string &filename,const std::vector<int>&other) const{
        // {{{ open

        ICLASSERT_RETURN(other.size() == 7);
        ChromaAndRGBClassifier carc;
        carc.c.parables[0] = P[0];
        carc.c.parables[1] = P[1];
        std::copy(other.begin(),other.begin()+3,carc.ref);
        std::copy(other.begin()+3,other.begin()+6,carc.thresh);

        ChromaClassifierIO::save(carc,filename);

        // now reopen that file and add gui-informamtion
        ConfigFile f(filename);

        static std::string x[6] = {"xpos","ypos","dim","red","green","blue"};
        for(int i=0;i<6;i++){
          const Dragger &d = D[i];
          float fs[6] = { (float)d.pos().x, (float)d.pos().y,(float)d.dim(),
                          (float)d.col().r, (float)d.col().g, (float)d.col().b };
          for(int j=0;j<6;++j){
            f.set(std::string("config.gui-info.dragger-")+str(i)+"."+x[j],fs[j]);
          }
        }
        f.set("config.gui-info.blue-slider-value",other[6]);
        f.save(filename);


      }

      // }}}
      void load(const std::string &filename,std::vector<int> &other){
        // {{{ open
        if(other.size() != 7){
          other.resize(7);
        }
        ChromaAndRGBClassifier carc = ChromaClassifierIO::loadRGB(filename);
        P[0] = carc.c.parables[0];
        P[1] = carc.c.parables[1];
        std::copy(carc.ref,carc.ref+3,other.begin());
        std::copy(carc.thresh,carc.thresh+3,other.begin()+3);

        // now reopen that file and add gui-informamtion
        ConfigFile f(filename);
        //static std::string x[6] = {"xpos","ypos","dim","red","green","blue"};

        for(int i=0;i<6;i++){
          std::string pfx = std::string("config.gui-info.dragger-")+str(i)+".";
          D[i].setColor(f.get<float>(pfx+"red"),f.get<float>(pfx+"green"),f.get<float>(pfx+"blue"));
          D[i].setDim(f.get<float>(pfx+"dim"));
          D[i].setPos(Point32f(f.get<float>(pfx+"xpos"),f.get<float>(pfx+"ypos")));

        }
        other[6] = f.get<int>("config.gui-info.blue-slider-value");

        updateDrawings();
      }

      // }}}
      void updateDrawings(){
        // {{{ open
        P[0] = Parable(D[0].pos()*255,D[1].pos()*255,D[2].pos()*255);
        P[1] = Parable(D[3].pos()*255,D[4].pos()*255,D[5].pos()*255);
        setImage(&image);
        rel();
        color(255,0,0,255);
        for(float x=0;x<255;x+=1.0){
          line(x/255,P[0](x)/255,(x+1)/255, P[0](x+1)/255);
        }
        color(0,100,255,255);
        for(float x=0;x<255;x+=1.0){
          line(x/255,P[1](x)/255,(x+1)/255, P[1](x+1)/255);
        }
        for(int i=0;i<6;i++){
          D[i].draw(this);
        }
        color(255,255,255,255);
        render();
      }

      // }}}
      void updateImage(icl8u blue){
        // {{{ open


        Channel8u rgb[3];
        image.extractChannels(rgb);

        for(int x=0;x<256;x++){
          for(int y=0;y<255-x;y++){
            Dragger::Color::xyb_to_rg(rgb[0](x,y),rgb[1](x,y),blue,float(x)/255,float(y)/255);
            rgb[2](x,y) = blue;
          }
        }



        // some misc
        static const int n = 1;
        static const int nn = 2*n+1;
        static Point o[nn][nn];

        static Point *po = &(o[0][0]);
        static bool first = true;
        if(first){
          first = false;
          for(int x=-n;x<=n;x++){
            for(int y=-n;y<=n;y++){
              o[x+1][y+1] = Point(x,y);
            }
          }
        }
        for(int i=n;i<256-n;i++){
          int s[2] = {0,0};
          for(int p=0;p<nn*nn;p++){
            s[0] += rgb[0](i+po[p].x,255-i+po[p].y);
            s[1] += rgb[1](i+po[p].x,255-i+po[p].y);
          }
          rgb[0](i,255-i) = clipped_cast<icl32f,icl8u>(float(s[0])/(nn*nn));
          rgb[1](i,255-i) = clipped_cast<icl32f,icl8u>(float(s[1])/(nn*nn));
        }
        setImage(&image);
        update();
      }

      // }}}

      Dragger D[6];
      Parable P[2];
      Img8u image;
      icl8u blue;
    };


    ChromaGUI::ChromaGUI(QWidget *parent):QObject(parent),GUI("vsplit[@handle=parent]",parent){
      // {{{ open
      (*this) << VBox().handle("image").label("Chromaticity Space").minSize(18,16)
              << ( HBox()
                   << Slider(0,255,128).handle("bluedisphandle").label("Disp. Blue").out("bluedisp")
                   << Button("load").handle("load")
                   << Button("save").handle("save")
                  )
              << ( HBox()
                   << Slider(0,255,128).handle("red").label("Red Color").out("redval")
                   << Slider(0,255,128).handle("red-thresh").label("Red Threshold").out("redtval")
                  )
              << ( HBox()
                   << Slider(0,255,128).handle("green").label("Green Color").out("greenval")
                   << Slider(0,255,128).handle("green-thresh").label("Green Threshold").out("greentval")
                  )
              << ( HBox()
                   << Slider(0,255,128).handle("blue").label("Blue Color").out("blueval")
                   << Slider(0,255,128).handle("blue-thresh").label("Blue Threshold").out("bluetval")
                  )
              << Show();

      BoxHandle &h = get<BoxHandle>("image");
      m_poChromaWidget = new ChromaWidget(*h);
      h.add(m_poChromaWidget);


      m_aoSliderHandles[0][0] = get<SliderHandle>("red");
      m_aoSliderHandles[0][1] = get<SliderHandle>("green");
      m_aoSliderHandles[0][2] = get<SliderHandle>("blue");

      m_aoSliderHandles[1][0] = get<SliderHandle>("red-thresh");
      m_aoSliderHandles[1][1] = get<SliderHandle>("green-thresh");
      m_aoSliderHandles[1][2] = get<SliderHandle>("blue-thresh");


      QObject::connect((QObject*)*(get<SliderHandle>("bluedisphandle")),SIGNAL(valueChanged(int)),
                       (QObject*)this,SLOT(blueSliderChanged(int)));

      QObject::connect((QObject*)*(get<SliderHandle>("bluedisphandle")),SIGNAL(valueChanged(int)),
                       (QObject*)this,SLOT(blueSliderChanged(int)));

      QObject::connect((QObject*)*get<ButtonHandle>("load"),SIGNAL(clicked(bool)),
                       (QObject*)this,SLOT(load()));


      QObject::connect((QObject*)*(get<ButtonHandle>("save")),SIGNAL(clicked(bool)),
                       (QObject*)this,SLOT(save()));
    }

    // }}}
    ChromaClassifier ChromaGUI::getChromaClassifier(){
      // {{{ open

      ChromaClassifier c;
      c.parables[0] = m_poChromaWidget->getParables()[0];
      c.parables[1] = m_poChromaWidget->getParables()[1];
      return c;
    }

    // }}}
    ChromaAndRGBClassifier ChromaGUI::getChromaAndRGBClassifier(){
      // {{{ open

      ChromaAndRGBClassifier c;
      c.c = getChromaClassifier();
      c.ref[0] = m_aoSliderHandles[0][0].getValue();
      c.ref[1] = m_aoSliderHandles[0][1].getValue();
      c.ref[2] = m_aoSliderHandles[0][2].getValue();

      c.thresh[0] = m_aoSliderHandles[1][0].getValue();
      c.thresh[1] = m_aoSliderHandles[1][1].getValue();
      c.thresh[2] = m_aoSliderHandles[1][2].getValue();

      return c;
    }

    // }}}
    void ChromaGUI::blueSliderChanged(int val){
      // {{{ open

      m_poChromaWidget->setBlue(val);
      m_poChromaWidget->update();
    }

    // }}}
    void ChromaGUI::load(const std::string &filenameIn){
      // {{{ open

      QString filename;
      if(filenameIn == ""){
        filename = QFileDialog::getOpenFileName( 0, "Select Filename...", "./");
        if(filename.isNull() || filename == ""){
          return;
        }
      }else{
        filename = filenameIn.c_str();
      }

      vector<int> data;
      m_poChromaWidget->load(filename.toLatin1().data(),data);
      ICLASSERT_RETURN(data.size() == 7);

      m_aoSliderHandles[0][0] = data[0];
      m_aoSliderHandles[0][1] = data[1];
      m_aoSliderHandles[0][2] = data[2];

      m_aoSliderHandles[1][0] = data[3];
      m_aoSliderHandles[1][1] = data[4];
      m_aoSliderHandles[1][2] = data[5];

      get<SliderHandle>("bluedisphandle") = data[6];
    }

    // }}}
    void ChromaGUI::save(const std::string &filenameIn){
      // {{{ open

      QString filename;
      if(filenameIn == ""){
        filename = QFileDialog::getSaveFileName( 0, "Select Filename...", "./");
        if(filename.isNull() || filename == ""){
          return;
        }
      }else{
        filename = filenameIn.c_str();
      }

      vector<int> data;
      data.push_back(m_aoSliderHandles[0][0].getValue());
      data.push_back(m_aoSliderHandles[0][1].getValue());
      data.push_back(m_aoSliderHandles[0][2].getValue());
      data.push_back(m_aoSliderHandles[1][0].getValue());
      data.push_back(m_aoSliderHandles[1][1].getValue());
      data.push_back(m_aoSliderHandles[1][2].getValue());

      data.push_back(get<SliderHandle>("bluedisphandle").getValue());

      m_poChromaWidget->save(filename.toLatin1().data(),data);

    }

    // }}}

  } // namespace qt
}
