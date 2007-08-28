#include <iclChromaGUI.h>
#include <QFileDialog>
#include <vector>

#include "iclParable.h"
#include "iclDragger.h"
#include <iclDrawWidget.h>
#include <iclMouseInteractionReceiver.h>
#include <iclImg.h>
#include <iclImgChannel.h>
#include <QFile>
#include <QTextStream>


using namespace std;
namespace icl{

  struct ChromaWidget : public ICLDrawWidget, public MouseInteractionReceiver{

    ChromaWidget(QWidget *parent = 0):ICLDrawWidget(parent){
      // {{{ open

      image = Img8u(Size(256,256),formatRGB);
      
      add(this);
      
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
    virtual void processMouseInteraction(MouseInteractionInfo *info){
      // {{{ open

      Point32f p = Point32f(info->relImageX,info->relImageY);
      
      bool left = *(info->downmask);
      
      switch(info->type){
        case MouseInteractionInfo::moveEvent:
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
        case MouseInteractionInfo::dragEvent:
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
        case MouseInteractionInfo::pressEvent:
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
        case MouseInteractionInfo::releaseEvent:
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
    const ParableSet getParables() const{
      // {{{ open

      return ParableSet((Parable*)P);
    }

    // }}}
    void save(const std::string &filename,const std::vector<int>&other) const{
      // {{{ open

      ICLASSERT_RETURN(other.size() == 7);
      printf("saving current state! \n");
      QFile file(filename.c_str());
      if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        ERROR_LOG("can't open file!");
        return;
      }
      QTextStream out(&file);
      
      const char *n = "\n";
      const char *c = ",";
      out << "ICL ChromaWidget Configuration File" << n;
      out << "Parable 0:" << n;
      out << P[0].a << c << P[0].b << c  << P[0].c << n;
      out << "Parable 1:" << n;
      out << P[1].a << c << P[1].b << c  << P[1].c << n;
      for(int i=0;i<6;i++){
        out << "Dragger "<< i <<":" << n;
        out << D[i].pos().x << c << D[i].pos().y << c << D[i].dim() << c << D[i].col().r << c <<  D[i].col().g << c <<D[i].col().b << n;
      }
      
      out << "Reference Colors and Thresholds:" << n;
      out << other[0] << c << other[1] << c << other[2] << c << other[3] << c << other[4] << c << other[5] << c << other[6] << n;  
      
      file.flush();
      file.close();
    }

    // }}}
    void load(const std::string &filename,std::vector<int> &other){
      // {{{ open

      QFile file(filename.c_str());
      if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        ERROR_LOG("can't open file!");
        return;
      }
      QTextStream in(&file);
      
      QString line = in.readLine(); // ICL Croma...
      if(line != "ICL ChromaWidget Configuration File"){
        ERROR_LOG("invalide file format!");
        return;
      }
      line = in.readLine(); // Parable 0:
      line = in.readLine(); // a,b,c
      P[0].a = line.section(',',0,0).toFloat();
      P[0].b = line.section(',',1,1).toFloat();
      P[0].c = line.section(',',2,2).toFloat();
      line = in.readLine(); // Parable 1:
      line = in.readLine(); // a,b,c
      P[1].a = line.section(',',0,0).toFloat();
      P[1].b = line.section(',',1,1).toFloat();
      P[1].c = line.section(',',2,2).toFloat();
      
      for(int i=0;i<6;i++){
        line = in.readLine(); // Draggable ...
        line = in.readLine(); // x,y,d,r,g,b
        D[i].setPos(Point32f(line.section(',',0,0).toFloat(),line.section(',',1,1).toFloat()));
        D[i].setDim(line.section(',',2,2).toFloat());
        D[i].setColor(line.section(',',3,3).toFloat(),line.section(',',4,4).toFloat(),line.section(',',5,5).toFloat());
      }
      
      other.clear();
      line = in.readLine(); // Reference Colors...
      line = in.readLine();
      for(int i=0;i<7;i++) other.push_back(line.section(',',i,i).toInt());
      
      file.close();
      
      updateDrawings();
    }

    // }}}
    void updateDrawings(){
      // {{{ open

      P[0] = Parable(D[0].pos()*255,D[1].pos()*255,D[2].pos()*255);
      P[1] = Parable(D[3].pos()*255,D[4].pos()*255,D[5].pos()*255);
      setImage(&image);
      lock();
      reset();
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
      unlock();
      update();
    }

    // }}}
    void updateImage(icl8u blue){
      // {{{ open

      ImgChannel8u cr = pickChannel(&image,0);
      ImgChannel8u cg = pickChannel(&image,1);    
      ImgChannel8u cb = pickChannel(&image,2);
      
      for(int x=0;x<256;x++){
        for(int y=0;y<255-x;y++){
          Dragger::Color::xyb_to_rg(cr(x,y),cg(x,y),blue,float(x)/255,float(y)/255);
          cb(x,y) = blue; 
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
            o[x+2][y+2] = Point(x,y);
          }
        }
      }
      for(int i=n;i<256-n;i++){
        int s[2] = {0,0};
        for(int p=0;p<nn*nn;p++){
          s[0] += cr(i+po[p].x,255-i+po[p].y); 
          s[1] += cg(i+po[p].x,255-i+po[p].y); 
        }
        cr(i,255-i) = float(s[0])/(nn*nn);
        cg(i,255-i) = float(s[1])/(nn*nn);
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


  ChromaGUI::ChromaGUI(QWidget *parent):GUI("vbox[@handle=parent]",parent){
    // {{{ open

    (*this) << "vbox[@handle=image@label=Chromaticity Space@minsize=18x16]";
    
    (*this) << ( GUI("hbox") 
             << "slider(0,255,128)[@handle=bluedisphandle@label=Disp. Blue@out=bluedisp]" 
             << "button(load)[@handle=load]" << "button(save)[@handle=save]" );
    (*this) << ( GUI("hbox") 
             << "slider(0,255,128)[@handle=red@label=Red Color@out=redval]" 
             << "slider(0,255,128)[@handle=red-thresh@label=Red Threshold@out=redtval]" );
    (*this) << ( GUI("hbox") 
             << "slider(0,255,128)[@handle=green@label=Green Color@out=greenval]" 
             << "slider(0,255,128)[@handle=green-thresh@label=Green Threshold@out=greentval]" );
    (*this) << ( GUI("hbox") 
             << "slider(0,255,128)[@handle=blue@label=Blue Color@out=blueval]" 
             << "slider(0,255,128)[@handle=blue-thresh@label=Blue Threshold@out=bluetval]" );
    
    show();
    
    BoxHandle &h = getValue<BoxHandle>("image");
    m_poChromaWidget = new ChromaWidget(*h);
    h.add(m_poChromaWidget);

    m_aoSliderHandles[0][0] = getValue<SliderHandle>("red");
    m_aoSliderHandles[0][1] = getValue<SliderHandle>("green");
    m_aoSliderHandles[0][2] = getValue<SliderHandle>("blue");

    m_aoSliderHandles[1][0] = getValue<SliderHandle>("red-thresh");
    m_aoSliderHandles[1][1] = getValue<SliderHandle>("green-thresh");  
    m_aoSliderHandles[1][2] = getValue<SliderHandle>("blue-thresh");

    QObject::connect((QObject*)*(getValue<SliderHandle>("bluedisphandle")),SIGNAL(valueChanged(int)),(QObject*)this,SLOT(blueSliderChanged(int)));

    QObject::connect((QObject*)*(getValue<ButtonHandle>("load")),SIGNAL(clicked(bool)),(QObject*)this,SLOT(load()));
    QObject::connect((QObject*)*(getValue<ButtonHandle>("save")),SIGNAL(clicked(bool)),(QObject*)this,SLOT(save()));
  }

  // }}}
  ChromaGUI::ChromaClassifier ChromaGUI::getChromaClassifier(){
    // {{{ open

    ChromaClassifier c = { m_poChromaWidget->getParables() };
    return c;
  }

  // }}}
  ChromaGUI::CombiClassifier ChromaGUI::getCombiClassifier(){
    // {{{ open

    CombiClassifier c;
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
  void ChromaGUI::load(){
    // {{{ open

    QString filename = QFileDialog::getOpenFileName( 0, "Select Filename...", "./");
    if(filename.isNull() || filename == ""){
      return;
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
    
    getValue<SliderHandle>("bluedisphandle") = data[6];
  }

  // }}}
  void ChromaGUI::save(){
    // {{{ open

    QString filename = QFileDialog::getSaveFileName( 0, "Select Filename...", "./");
    if(filename.isNull() || filename == ""){
      return;
    }

    vector<int> data;
    data.push_back(m_aoSliderHandles[0][0].getValue());
    data.push_back(m_aoSliderHandles[0][1].getValue());
    data.push_back(m_aoSliderHandles[0][2].getValue());
    data.push_back(m_aoSliderHandles[1][0].getValue());
    data.push_back(m_aoSliderHandles[1][1].getValue());
    data.push_back(m_aoSliderHandles[1][2].getValue());
    
    data.push_back(getValue<SliderHandle>("bluedisphandle").getValue());
    
    m_poChromaWidget->save(filename.toLatin1().data(),data);

  }

  // }}}

}
