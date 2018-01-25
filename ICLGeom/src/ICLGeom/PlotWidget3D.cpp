/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PlotWidget3D.cpp                   **
** Module : ICLGeom                                                **
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

#include <ICLGeom/PlotWidget3D.h>
#include <ICLGeom/PlotHandle3D.h>
#include <ICLGeom/GridSceneObject.h>
#include <ICLQt/GUIDefinition.h>
#include <ICLMath/LinearTransform1D.h>

namespace icl{

  using namespace utils;
  using namespace math;
  using namespace core;
  using namespace qt;

  namespace geom{

    static Range32f round_range(Range32f r){
      if(r.minVal > r.maxVal){
        std::swap(r.minVal,r.maxVal);
      }
      float m = fabs(r.maxVal - r.minVal);
      float f = 1;
      if( m > 1 ){
        while( m/f > 100){
          f *= 10;
        }
      }else{
        while( m/f < 10){
          f *= 0.1;
        }
      }
      r.minVal = floor(r.minVal/f) * f;
      r.maxVal = ceil(r.maxVal/f) * f;
      return r;
    }

    std::string create_label(float r){
      return str(fabs(r) < 0.0000001 ? 0 : r); // todo rounding and stuff
    }

    struct Axis : public SceneObject{
      PlotWidget3D *parent;
      Range32f roundedRange;
      std::string label;
      std::vector<TextPrimitive*> labels;
      Axis(PlotWidget3D *parent,const Range32f &range, bool invertLabels, const std::string &label):
        parent(parent),label(label){
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

          addVertex(Vec(invertLabels ? -r : r,-d,0,1));

          addLine(4*l,4*l+1);
          addLine(4*l,4*l+2);

          labels.push_back(new TextPrimitive(4*l+3,0,0,0,create_label(min + l*step),
                                             20,GeomColor(255,255,255,255),
                                             -1,-1,-1,-1,.1));
          addCustomPrimitive(labels.back());

        }
        addVertex(Vec((invertLabels ? -1 : 1) * (1+2*d),0,0,1));
        labels.push_back(new TextPrimitive(m_vertices.size()-1,0,0,0,label,
                                           25,GeomColor(255,255,255,255),
                                           -1,-1,-1,-1,.13));

        addCustomPrimitive(labels.back());

      }
    };

    struct CoordinateFrameObject3D : public SceneObject{
      PlotWidget3D *parent;
      SceneObject *rootObject;
      Range32f ranges[3];
      Axis *axes[3];

      CoordinateFrameObject3D(PlotWidget3D *parent, SceneObject *rootObject):
        parent(parent),rootObject(rootObject){
        axes[0] = axes[1] = axes[2] = 0;
        setLockingEnabled(true);

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
        static const std::string labels[3] = { "X", "Y", "Z" };
        for(int i=0;i<3;++i){
          if(axes[i]) removeChild(axes[i]);
          axes[i] = new Axis(parent,ranges[i],i==1,labels[i]);
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


        //use ranges


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
      Range32f givenViewport[3];
      Range32f computedViewport[3];

      // TODO: all visible stuff is always added to the root object
      // the root object is used to set the scale-part of it's transformation
      // so that all scene objects (or more precisely the plot3D's 3D-viewport)
      // is automatically fit into the [-1,1]^3 cube, which is the 3D coordinate frame

      SceneObject *rootObject;
      CoordinateFrameObject3D *coordinateFrame;

      float pointsize;
      float linewidth;
      bool smoothfill;
      GeomColor color,fill;

      void add(SceneObject *obj, bool passOwnership=false, bool addOnly=false){
        if(!addOnly){
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
        }
        rootObject->lock();
        rootObject->addChild(obj,passOwnership);

        updateBounds(); // todo: only extend bounds by newly added object!!

        LinearTransform1D tx(computedViewport[0],Range32f(-1,1));
        LinearTransform1D ty(computedViewport[1],Range32f(-1,1));
        LinearTransform1D tz(computedViewport[2],Range32f(-1,1));

        rootObject->setTransformation(Mat(tx.m,0,0,tx.b,
                                          0,ty.m,0,ty.b,
                                          0,0,tz.m,tz.b,
                                          0,0,0,1));
        rootObject->unlock();

        coordinateFrame->lock();
        coordinateFrame->updateTics();
        coordinateFrame->unlock();
      }

      Data(){
        pointsize = 1;
        linewidth = 1;
        smoothfill = true;
        color = geom_red(255);
        fill = geom_blue(255);
      }

      template<bool X, bool Y, bool Z>
      void update_bounds(Range32f computedViewport[3], SceneObject *o){
        //        DEBUG_LOG("updating bounds for object " << (void*)o << (rootObject ? "[root]" : "[other]"));
        if(o != rootObject){
          const std::vector<Vec> &vs = o->getVertices();
          //DEBUG_LOG("checking " << vs.size() << " nodes! [" << (X?"X":"") << (Y?"Y":"") << (Z?"Z":"") << "]" );
          for(size_t i=0;i<vs.size();++i){
            const Vec &v = vs[i];
            if(X){
              if(v[0] < computedViewport[0].minVal) computedViewport[0].minVal = v[0];
              if(v[0] > computedViewport[0].maxVal) computedViewport[0].maxVal = v[0];
            }
            if(Y){
              if(v[1] < computedViewport[1].minVal) computedViewport[1].minVal = v[1];
              if(v[1] > computedViewport[1].maxVal) computedViewport[1].maxVal = v[1];
            }
            if(Z){
              if(v[2] < computedViewport[2].minVal) computedViewport[2].minVal = v[2];
              if(v[2] > computedViewport[2].maxVal) computedViewport[2].maxVal = v[2];
            }
          }
        }
        for(int i=0;i<o->getChildCount();++i){
          update_bounds<X,Y,Z>(computedViewport,o->getChild(i));
        }
      }

      void updateBounds(){
        const bool dyn[3] = {
          !givenViewport[0].getLength(),
          !givenViewport[1].getLength(),
          !givenViewport[2].getLength()
        };
        for(int i=0;i<3;++i){
          computedViewport[i] = dyn[i] ? Range32f::inv_limits() : givenViewport[i];
        }

        switch( dyn[0] + dyn[1]*2 + dyn[2]*4 ){
          case 0: break; // nothing to do here!
          case 1: update_bounds<true,false,false>(computedViewport,rootObject); break;
          case 2: update_bounds<false,true,false>(computedViewport,rootObject); break;
          case 3: update_bounds<true,true,false>(computedViewport,rootObject); break;
          case 4: update_bounds<false,false,true>(computedViewport,rootObject); break;
          case 5: update_bounds<true,false,true>(computedViewport,rootObject); break;
          case 6: update_bounds<false,true,true>(computedViewport,rootObject); break;
          case 7: update_bounds<true,true,true>(computedViewport,rootObject); break;
          default: break;
        }
      }
    };

    const Range32f *PlotWidget3D::getViewPort() const{
      return m_data->computedViewport;
    }



    PlotWidget3D::PlotWidget3D(QWidget *parent):ICLDrawWidget3D(parent),m_data(new Data){
      std::fill(m_data->givenViewport,m_data->givenViewport+3,Range32f(0,0));
      m_data->scene.setBounds(5);

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
      m_data->rootObject->setLockingEnabled(true);
      m_data->coordinateFrame = new CoordinateFrameObject3D(this,m_data->rootObject);

      m_data->scene.addObject(m_data->coordinateFrame); // must be rendered first (adapts rootObj-transform)
      m_data->scene.addObject(m_data->rootObject);

      SceneLight &l = m_data->scene.getLight(1);
      l.setSpecularEnabled(true);
      l.setOn();
      l.setPosition(Vec(0,0,10,1));
      l.setAnchorToWorld();
      l.setSpecular(GeomColor(255,255,255,255));
    }


    PlotWidget3D::~PlotWidget3D(){
      delete m_data;
    }

    const Camera &PlotWidget3D::getCamera() const{
      return m_data->scene.getCamera(0);
    }

    void PlotWidget3D::setCamera(const Camera &cam){
      m_data->scene.getCamera(0) = cam;
    }

    void PlotWidget3D::setViewPort(const Range32f &xrange,
                                   const Range32f &yrange,
                                   const Range32f &zrange){
      m_data->givenViewport[0] = xrange;
      m_data->givenViewport[1] = yrange;
      m_data->givenViewport[2] = zrange;

      m_data->updateBounds();

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
    void PlotWidget3D::clear(){
      m_data->rootObject->removeAllChildren();
    }

    void PlotWidget3D::nocolor(){
      m_data->color[3] = 0;
    }

    void PlotWidget3D::nofill(){
      m_data->fill[3] = 0;
    }

    void PlotWidget3D::smoothfill(bool on){
      m_data->smoothfill = on;
    }

    void PlotWidget3D::lock(){
      m_data->scene.lock();
    }
    void PlotWidget3D::unlock(){
      m_data->scene.unlock();
    }


    PlotWidget3D::Handle PlotWidget3D::scatter(const std::vector<Vec> &points){
      SceneObject *obj = new SceneObject;
      obj->getVertices() = points;
      obj->getVertexColors().resize(points.size());
      m_data->add(obj);
      return obj;
    }

    namespace{
      struct scale_color_range{
        LinearTransform1D t;
        scale_color_range(const Range32f &r) : t(r,Range32f(0,1)){}
        GeomColor operator()(const GeomColor &c) const{
          return GeomColor(t(c[0]),t(c[1]),t(c[2]),t(c[3]));
        }
      };
    }

    PlotWidget3D::Handle PlotWidget3D::scatter(const std::vector<Vec> &points,
                                               const std::vector<GeomColor> &colors,
                                               const Range32f &colorRange){
      ICLASSERT_THROW(points.size() == colors.size(), ICLException("PlotWidget3D::scatter(points,colors):"
                                                                   " points.size() must be equal to colors.size()!"));
      SceneObject *obj = new SceneObject;
      obj->getVertices() = points;
      obj->getVertexColors().resize(points.size());
      if(colorRange == Range32f(0,1)){
        std::copy(colors.begin(),colors.end(), obj->getVertexColors().begin());
      }else{
        std::transform(colors.begin(),colors.end(), obj->getVertexColors().begin(), scale_color_range(colorRange));
      }
      obj->setPointSize(m_data->pointsize);
      m_data->add(obj,true,true);
      return obj;

    }


    PlotWidget3D::Handle PlotWidget3D::linestrip(const std::vector<Vec> &points){
      SceneObject *obj = new SceneObject;
      obj->getVertices() = points;
      obj->getVertexColors().resize(points.size());
      for(size_t i=1;i<points.size();++i){
        obj->addLine(i-1,i);
      }
      m_data->add(obj);
      return obj;
    }



    PlotWidget3D::Handle PlotWidget3D::surf(const std::vector<Vec> &points, int nx, int ny){
      SceneObject *grid = new GridSceneObject(nx,ny,points,m_data->color[3],m_data->fill[3]);
      grid->setLockingEnabled(true);
      grid->createAutoNormals(m_data->smoothfill);
      m_data->add(grid);
      return grid;
    }

    PlotWidget3D::Handle PlotWidget3D::surf(Function<float,float,float> fxy,
                                            const Range32f &rx, const Range32f &ry,
                                            int nx, int ny,
                                            PlotWidget3D::Handle reuseObject){
      std::vector<Vec> pointsVec;
      std::vector<Vec> &points = reuseObject ? reuseObject->getVertices() : pointsVec;
      points.resize(nx*ny);
      if(reuseObject) reuseObject->lock();
      if(reuseObject) reuseObject->getVertexColors().resize(nx*ny);
      float dx = rx.getLength()/ (nx-1);
      float dy = ry.getLength()/ (ny-1);
      for(int y=0;y<ny;++y){
        float yv = ry.minVal + y * dy;
        for(int x=0;x<nx;++x){
          float xv = rx.minVal + x * dx;
          points[x + nx*y] = Vec(xv,yv,fxy(xv,yv),1);
        }
      }

      if(reuseObject){
        if(m_data->rootObject->hasChild(reuseObject)){
          reuseObject->createAutoNormals(m_data->smoothfill);
        }else{
          m_data->add(reuseObject);
        }
        reuseObject->unlock();
        return reuseObject;
      }else{
        return surf(points,nx,ny);
      }
    }


    PlotWidget3D::Handle PlotWidget3D::label(const Vec &p, const std::string &text){
      SceneObject *obj = new SceneObject;

      if(p[3]) obj->addVertex(p);
      else obj->addVertex(Vec(p[0],p[1],p[2],1));
      obj->addText(0,text,8,m_data->color,20);
      obj->setVisible(Primitive::vertex,false);
      m_data->add(obj,true,true);
      return obj;
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

      struct Plot3DGUIWidgetRegisterer{
        Plot3DGUIWidgetRegisterer(){
          qt::GUI::register_widget_type("plot3D",create_plot_3D_widget_instance);

          qt::DataStore::register_trivial_assignment_rule<PlotHandle3D,PlotHandle3D>("PlotHandle3D","PlotHandle3D");
        }
      } plot3DWidgetRegisterer;
    }
  }
}
