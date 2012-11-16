/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PlotWidget3D.cpp                           **
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

#include <ICLGeom/PlotWidget3D.h>
#include <ICLGeom/PlotHandle3D.h>
#include <ICLQt/GUIDefinition.h>

namespace icl{

  using namespace utils;
  using namespace math;
  using namespace core;
  using namespace qt;

  namespace geom{

    static Range32f round_range(Range32f r){
      if(r.minVal > r.maxVal) std::swap(r.minVal,r.maxVal);
      const int signA = r.minVal<0?-1:1;
      const int signB = r.maxVal<0?-1:1;
      float a(0),b(0),f=1;
      if(fabs(r.minVal) > fabs(r.maxVal)){
        a = fabs(r.minVal);
        b = fabs(r.maxVal);
      }else{
        a = fabs(r.maxVal);
        b = fabs(r.minVal);
      }
      if(a > 1){
        while(a > 100){
          a *= 0.1; b *= 0.1; f *= 10;
        }
        if(a > 90) a = 100;
        a = ceil(a) * f;
        b = signB > 0 ? (ceil(b) * f) : (floor(b) * f);
        
      }else{
        while(a < 10){
          a *= 10;  b *= 10; f *= 0.1;
        }
        if ( a < 11) a = 10;
        a = floor(a) * f;
        b = signB > 0 ? (floor(b) * f) : (ceil(b) * f);
      }
      if(b < a){  
        return Range32f(signA * a, signB * b);
      }else{
        return Range32f(signB * b, signA * a);
      }
    }

    struct CoordinateFrameObject3D : public SceneObject{
      PlotWidget3D *parent;
      SceneObject *tics;
      std::vector<Primitive*> xlabels,ylabels,zlabels;
      
      CoordinateFrameObject3D(PlotWidget3D *parent):parent(parent),tics(0){
        addVertex(Vec(1,-1,1,1));
        addVertex(Vec(1,1,1,1));
        addVertex(Vec(-1,1,1,1));
        addVertex(Vec(-1,-1,1,1));

        addVertex(Vec(1,-1,-1,1));
        addVertex(Vec(1,1,-1,1));
        addVertex(Vec(-1,1,-1,1));
        addVertex(Vec(-1,-1,-1,1));

        for(int i=0;i<4;++i){
          addLine(i,(i+1)%4);
          addLine(4+i,4+(i+1)%4);
          addLine(i,i+4);
        }
        
        updateTics();
      }
      
      std::string create_label(float r){
        return str(r); // todo rounding and stuff
      }
      void updateTics(){
        // propable better: crate axes seperately to used dynamic number of tics
        // dependent on the axis ratio
        
        if(tics) removeChild(tics);
        tics = new SceneObject;
        const int N = 5;
        const float lenBase = 0.1;
        int l = 0;
        for(int i=-N/2; i<= N/2;++i, ++l){
          float r = float(i)/(N/2);
          float len = i ? lenBase : 2*lenBase;
          tics->addVertex(Vec(-1,-r,-1,1));
          tics->addVertex(Vec(len-1,-r,-1,1));
          tics->addVertex(Vec(-1,-r,len-1,1));
          tics->addLine(9*l,9*l+1);
          tics->addLine(9*l,9*l+2);

          tics->addVertex(Vec(-r,-1,-1,1));
          tics->addVertex(Vec(-r,len-1,-1,1));
          tics->addVertex(Vec(-r,-1,len-1,1));
          tics->addLine(9*l+3,9*l+4);
          tics->addLine(9*l+3,9*l+5);

          tics->addVertex(Vec(-1,-1,-r,1));
          tics->addVertex(Vec(len-1,-1,-r,1));
          tics->addVertex(Vec(-1,len-1,-r,1));
          tics->addLine(9*l+6,9*l+7);
          tics->addLine(9*l+6,9*l+8);
        }
        
        const float d = 0.1; // distance of lables to the tics
        l = (int)tics->getVertices().size();
        for(int i=-N/2; i<= N/2;++i){
          float r = float(i)/(N/2);

          {
            tics->addVertex(Vec(-r,-1-d,-1,1));
            Primitive *t = new TextPrimitive(l++,0,0,0,create_label(r),20,GeomColor(255,255,255,255),-1,-1,-1,-1,.1);
            tics->addCustomPrimitive(t);
            xlabels.push_back(t);
          }
          
          {
            tics->addVertex(Vec(-1-d,-r,-1,1));
            Primitive *t = new TextPrimitive(l++,0,0,0,create_label(r),20,GeomColor(255,255,255,255),-1,-1,-1,-1,.1);
            tics->addCustomPrimitive(t);
            ylabels.push_back(t);
          }
          
          {
            tics->addVertex(Vec(-1,-1-d,-r,1));
            Primitive *t = new TextPrimitive(l++,0,0,0,create_label(r),20,GeomColor(255,255,255,255),-1,-1,-1,-1,.1);
            tics->addCustomPrimitive(t);
            zlabels.push_back(t);
          }

        }
        
        addChild(tics);
      }


      virtual void prepareForRendering(){
        // todo obtain parent view-port (which could be dynamically computed)
        // adapt tics and tic-labels (if neccessary)
        
        // perhaps, the cs-object could have 2 modes (either showing all 6 sides)
        // or only the back-sides (which could be opaque or semitransparent then)
        
        // actually, what do we do with all the properties:
        // IDEA: the PlotWidget3D could be configurable and add a new
        // Special Button to itself (because it is actually also an ICLWidget)

        // perhaps: adapt the tics object (only if neccessary)
      }

      // we do this in OpenGL directly
      virtual void customRender(){
        /*
        float color[] = { 1,1,1,1 };
        glColor4fv(color);
        glBegin(GL_LINE_STRIP);
        glVertex3f(-1,-1,-1);
        glVertex3f(1,-1,-1);
        glVertex3f(1,1,-1);
        glVertex3f(-1,1,-1);
        glVertex3f(-1,-1,-1);
        glEnd();

        glBegin(GL_LINE_STRIP);
        glVertex3f(-1,-1,1);
        glVertex3f(1,-1,1);
        glVertex3f(1,1,1);
        glVertex3f(-1,1,1);
        glVertex3f(-1,-1,1);
        glEnd();
        
        glBegin(GL_LINES);
        glVertex3f(-1,-1,-1); glVertex3f(-1,-1,1);
        glVertex3f(1,-1,-1); glVertex3f(1,-1,1);
        glVertex3f(1,1,-1); glVertex3f(1,1,1);
        glVertex3f(-1,1,-1); glVertex3f(-1,1,1);
        glEnd();
        */
      }
    };

    struct PlotWidget3D::Data{
      Scene scene;
      Range32f viewport[3];
      
      // TODO: all visible stuff is always added to the root object
      // the root object is used to set the scale-part of it's transformation
      // so that all scene objects (or more precisely the plot3D's 3D-viewport)
      // is automatically fit into the [-1,1]^3 cube, which is the 3D coordinate frame
      
      SceneObject *rootObject;
      CoordinateFrameObject3D *coordinateFrame;
    };
    
    PlotWidget3D::PlotWidget3D(QWidget *parent):ICLDrawWidget3D(parent),m_data(new Data){
      std::fill(m_data->viewport,m_data->viewport+3,Range32f(0,1));
      m_data->scene.setBounds(1);


      Camera cam(Vec(8,1.5,1.5,1),
                 -Vec(8,1.5,1.5,1).normalized(),
                 Vec(0,0,-1,1),
                 7, Point32f(320,240),200,200,
                 0, Camera::RenderParams(Size(640,480),
                                         1,10000, 
                                         Rect(0,0,640,480),
                                         0,1));
      m_data->scene.addCamera(cam);
      
      install(m_data->scene.getMouseHandler(0));
      link(m_data->scene.getGLCallback(0));
      
      m_data->rootObject = new SceneObject;
      m_data->scene.addObject(m_data->rootObject);
      
      m_data->coordinateFrame = new CoordinateFrameObject3D(this);
      m_data->scene.addObject(m_data->coordinateFrame);
    }


    PlotWidget3D::~PlotWidget3D(){
      delete m_data;
    }

    const Camera &PlotWidget3D::getCamera() const{
      return m_data->scene.getCamera(0);
    }
    
    void PlotWidget3D::setViewPort(const Range32f &xrange,
                                   const Range32f &yrange,
                                   const Range32f &zrange){
      m_data->viewport[0] = xrange;
      m_data->viewport[1] = yrange;
      m_data->viewport[2] = zrange;
      
      // compute an actual view-port used, that is slightly larger
      // or equal to the given viewport. 
      /** Strategy: use larger abs value of min,max A
          A = XXX * 10^k (e.g. A = 122*10^0 -> 122
                               A = 160*10^2 -> 16000
                               A = 881*10-3 -> 0.881
          
          then set viewport to next higher multiple of 10
          122 -> 130
          16000 -> 16000
          0.881 -> 0.89
          
          if this is only or or two away from the next multiple of 100 use this one
          122 -> 130
          16000 -> 16000
          0.881 -> 0.9

          use same strategy for minValue, but wrt. the quatisation of the max value
          
          max = 175*10^1 = 1750 -> use 180*10^1
          min = 884*10^-2 = 8.84
          
          min is transfered and rounded to upper exponent first
          min' = 0.884 * 10^1 
          
          which could be interpreted as 0 or 1, but the runding of max was done 
      */
    }
      
    Scene &PlotWidget3D::getScene(){
      return m_data->scene;
    }
    const Scene &PlotWidget3D::getScene() const{
      return m_data->scene;
    }
    
    SceneObject *PlotWidget3D::getRootObject(){
      return m_data->rootObject;
    }
    const SceneObject *PlotWidget3D::getRootObject() const{
      return m_data->rootObject;
    }

    

    
    namespace{ // only for registering this class as a GUI component!

      struct Plot3DGUIWidget : public GUIWidget{
        PlotWidget3D *draw;
        Plot3DGUIWidget(const GUIDefinition &def):GUIWidget(def,6,6,GUIWidget::gridLayout,Size(16,12)){
          draw = new PlotWidget3D(this);
          draw->setViewPort(Range32f(def.floatParam(0),def.floatParam(1)),
                            Range32f(def.floatParam(2),def.floatParam(3)),
                            Range32f(def.floatParam(4),def.floatParam(5)));
                            
          addToGrid(draw);
          
          if(def.handle() != ""){
            getGUI()->lockData();
            getGUI()->allocValue<PlotHandle3D>(def.handle(),PlotHandle3D(draw,this));
            getGUI()->unlockData();
          }
        }
      };

      qt::GUIWidget *create_plot_3D_widget_instance(const qt::GUIDefinition &def){
        Plot3DGUIWidget *w = 0;
        try{
          w = new  Plot3DGUIWidget(def);
        }catch(ICLException &ex){
          std::cout << ex.what() << std::endl
                    << "syntax is:" 
                    << "draw3D() (no parameters supported)" 
                    << std::endl;
          return 0;
        }
        return w;
      }
      
      void assign_plot_handle_3D(const PlotHandle3D &src, PlotHandle3D &dst){
        dst = src;
      }
      
      struct Plot3DGUIWidgetRegisterer{
        Plot3DGUIWidgetRegisterer(){ 
          DEBUG_LOG("registering type plot3D");
          qt::GUI::register_widget_type("plot3D",create_plot_3D_widget_instance);
          
          qt::DataStore::register_trivial_assignment_rule<PlotHandle3D,PlotHandle3D>("PlotHandle3D","PlotHandle3D");
        }
      } plot3DWidgetRegisterer; 
    }



  }
}