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
#include <ICLGeom/GridSceneObject.h>
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
    std::string create_label(float r){
      return str(r < 0.00001 ? 0 : r); // todo rounding and stuff
    }

    struct Axis : public SceneObject{
      PlotWidget3D *parent;
      Range32f roundedRange;
      std::vector<TextPrimitive*> labels;

      Axis(PlotWidget3D *parent,const Range32f &range, bool invertLabels):parent(parent){
        roundedRange = round_range(range);
        const float min = roundedRange.minVal;
        const float max = roundedRange.maxVal;
        const int N = 10;  // todo find something better
        const float step = (max - min)/N;
        
        const float lenBase = 0.1;
        const float d = 0.1; // distance of lables to the tics

        for(int i=-N/2,l=0; i<= N/2;++i, ++l){
          float r = float(i)/(N/2);
          float len = i ? lenBase : 2*lenBase;
          addVertex(Vec(r,0,0,1));
          addVertex(Vec(r,len,0,1));
          addVertex(Vec(r,0,len,1));
          
          addVertex(Vec(invertLabels ? r : -r,-d,0,1));
          
          addLine(4*l,4*l+1);
          addLine(4*l,4*l+2);

          labels.push_back(new TextPrimitive(4*l+3,0,0,0,create_label(min + l*step),
                                             20,GeomColor(255,255,255,255),
                                             -1,-1,-1,-1,.1));
          addCustomPrimitive(labels.back());
        }
      }
    };

    struct CoordinateFrameObject3D : public SceneObject{
      PlotWidget3D *parent;
      Range32f ranges[3];
      Axis *axes[3];
      
      CoordinateFrameObject3D(PlotWidget3D *parent):parent(parent){
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
      
      void updateTics(){
        const Range32f *pranges = parent->getViewPort();
        if(ranges[0] == pranges[0] &&
           ranges[1] == pranges[1] &&
           ranges[2] == pranges[2] ){
          return;
        }

        std::copy(pranges,pranges+3,ranges);
           
        for(int i=0;i<3;++i){
          if(axes[i]) removeChild(axes[i]);
          axes[i] = new Axis(parent,ranges[i],i==1);
          addChild(axes[i]);
        }
        
        
        axes[0]->translate(0,-1,-1);

        axes[1]->rotate(0,0,M_PI/2);
        axes[1]->translate(-1,0,-1);



        axes[2]->rotate(-M_PI/2,0,0);
        axes[2]->rotate(0,M_PI/2,0);
        axes[2]->translate(-1,-1,0);

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
      
      float pointsize;
      float linewidth;
      GeomColor color,fill;

      void add(SceneObject *obj, bool passOwnership=false){
        obj->setPointSize(pointsize);
        obj->setLineWidth(linewidth);
        if(!color[3]){
          obj->setVisible(Primitive::vertex,false);
          obj->setVisible(Primitive::line,false);
        }else{
          obj->setVisible(Primitive::vertex,true);
          obj->setVisible(Primitive::line,true);
          obj->setColor(Primitive::vertex,color);
          obj->setColor(Primitive::line,color);
        }

        if(!fill[3]){
          obj->setVisible(Primitive::triangle,false);
          obj->setVisible(Primitive::quad,false);
          obj->setVisible(Primitive::polygon,false);
        }else{
          obj->setVisible(Primitive::triangle,true);
          obj->setVisible(Primitive::quad,true);
          obj->setVisible(Primitive::polygon,true);

          obj->setColor(Primitive::triangle,fill);
          obj->setColor(Primitive::quad,fill);
          obj->setColor(Primitive::polygon,fill);
        }

        rootObject->addChild(obj,passOwnership);
        
        updateBounds();
      }
      
      Data(){
        pointsize = linewidth = 1;
        color = geom_red(255);
        fill = geom_blue(255);
      }
      
      void updateBounds(){
        TODO_LOG("estimate all [0,0] bounds from all contained object's getTransformedVertices() ...");
      }
    };

    const Range32f *PlotWidget3D::getViewPort() const{
      return m_data->viewport;
    }


    
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


    void PlotWidget3D::add(SceneObject *obj, bool passOwnerShip){
      m_data->add(obj,passOwnerShip);
    }
    
    void PlotWidget3D::remove(Handle h){
      m_data->rootObject->removeChild(h);
    }
    
    void PlotWidget3D::color(int r, int g, int b, int a){
      m_data->color = GeomColor(r,g,b,a);
    }
    void PlotWidget3D::fill(int r, int g, int b, int a){
      m_data->fill = GeomColor(r,g,b,a);
    }
    void PlotWidget3D::pointsize(float size){
      m_data->pointsize = size;
    }
    void PlotWidget3D::linewidth(float width){
      m_data->linewidth = width;
    }
    
    PlotWidget3D::Handle PlotWidget3D::scatter(const std::vector<Vec> &points, bool connect){
      SceneObject *obj = new SceneObject;
      obj->getVertices() = points;
      obj->getVertexColors().resize(points.size());
      if(connect){
        for(size_t i=1;i<points.size();++i){
          obj->addLine(i-1,i);
        }
      }
      m_data->add(obj);
      return obj;
    }
    
    PlotWidget3D::Handle PlotWidget3D::surf(const std::vector<Vec> &points, int nx, int ny, 
                                            bool lines, bool fill, bool smoothfill){
      SceneObject *grid = new GridSceneObject(nx,ny,points,lines, fill);
      grid->createAutoNormals(smoothfill);
      m_data->add(grid);
      return grid;
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
