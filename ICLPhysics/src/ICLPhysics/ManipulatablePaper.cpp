#include <ICLPhysics/ManipulatablePaper.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/GeometricTools.h>
#include <ICLPhysics/RigidObject.h>
#include <ICLGeom/ViewRay.h>

#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ICLMath/StraightLine2D.h>
#include <ICLUtils/ConfigFile.h>

#include <QtWidgets/QMenu>

#include <set>

namespace icl{

  using namespace utils;
  using namespace core;
  using namespace qt;
  using namespace geom;
namespace physics{
  
  struct ManipulatablePaperMouseHandler : public MouseHandler{
    MouseHandler *h;
    SceneObject *obj;
    geom::ViewRay viewRay;
    Point nodeCoords;
    PlaneEquation plane;
    Mutex mutex;
    Scene *scene;
    PhysicsWorld *world;
    PhysicsPaper *paper;   // used for interaction
    ManipulatablePaper *parent;  // used as reference
    Time lastTime;
    int cameraIndex;
    SmartPtr<QMenu> menu;
    
    ManipulatablePaperMouseHandler(MouseHandler *h, PhysicsWorld *world, Scene *scene, ManipulatablePaper *parent, int cameraIndex):
      h(h), obj(0), scene(scene), world(world), paper(0), parent(parent), 
      lastTime(0), cameraIndex(cameraIndex)
    {}

    
    void applyImpulse(const MouseEvent &e){
      //TODO IMPLEMENT LOCKER PhysicsWorld::Locker lock(*world);
      geom::ViewRay v = scene->getCamera(0).getViewRay(e.getPos32f());
      Hit h = scene->findObject(v);
      obj = h.obj;
      Vec pos = h.pos;
      if(!obj) return;
      RigidObject *robj = dynamic_cast<RigidObject*>(obj->getParent());
      if(!robj) return;
      btRigidBody *rigid = robj->getRigidBody();
      if(!rigid) return;
      
      rigid->activate(true);
      
      btVector3 impulse = btVector3(v.direction[0],v.direction[1],v.direction[2]) * 1000;
      btVector3 relObjPos = btVector3(icl2bullet(pos[0]),icl2bullet(pos[1]),icl2bullet(pos[2])) - rigid->getCenterOfMassPosition();
      rigid->applyImpulse(impulse,relObjPos);
    }
    
    void showContexMenu(const MouseEvent &e){
      Hit h = scene->findObject(0,e.getX(),e.getY());
      SceneObject *o = h.obj;
      Vec pos = h.pos;
      if(o && o->getParent() == parent && !dynamic_cast<ManipulatablePaper::DraggedPositionIndicator*>(o)){
        o->setColor(Primitive::quad,GeomColor(255,150,0,255));
        if(!menu){
          menu = new QMenu;
          menu->addAction("fix here");
          menu->addAction("release");
          menu->addAction("release all");
          menu->addAction("add oscillator");
          menu->addAction("soften row");
          menu->addAction("harden row");
          menu->addAction("soften col");
          menu->addAction("harden col");
          menu->addAction("memorize all");
          menu->addAction("change show constraints");

        }
          
        QAction *action = menu->exec(QCursor::pos()+QPoint(-2,-2));
        if(!action) return;
        std::string selectedText = action->text().toLatin1().data();
        Point idx = parent->getNodeIndex(pos);
        if(selectedText == "fix here"){
          parent->addAttractor(idx,false);
        }else if(selectedText == "release"){
          parent->removeAttractor(idx);
        }else if (selectedText == "release all"){
          parent->removeAllAttractors();
        }else if (selectedText == "add oscillator"){
          parent->addAttractor(idx,true);
        }else if (selectedText == "soften col"){
          parent->adaptColStiffness(0.001,idx.x);
        }else if (selectedText == "soften row"){
          parent->adaptRowStiffness(0.001,idx.y);
        }else if (selectedText == "harden col"){
          parent->adaptColStiffness(0.9,idx.x);
        }else if (selectedText == "harden row"){
          parent->adaptRowStiffness(0.9,idx.y);
        }else if (selectedText == "memorize all"){
          parent->memorizeDeformation();
        }else if(selectedText == "change show constraints"){
          parent->setShowAllConstraints(!parent->getShowAllConstraints());
        }
      }
    }
    
    virtual void process(const MouseEvent &e){
      if(e.isModifierActive(ControlModifier)){
        if(e.isPressEvent()){
          applyImpulse(e);
        }else if(e.isReleaseEvent()){
          if(obj){
            obj = 0;
          }
        }
      }else if(e.isModifierActive(ShiftModifier)){
        if(e.isPressEvent()){
          if(e.isLeft()){
            Mutex::Locker lock2(mutex);
            //TODO Implement locker PhysicsWorld::Locker lock(*world);
#ifdef USE_OLD_INTERACTION_STYLE
            Hit h = scene->findObject(0,e.getX(),e.getY());
            paper = dynamic_cast<PhysicsPaper*>(h.obj);
            if(!paper) return;
            nodeCoords = paper->getNodeIndex(h.pos);
            paper->setDraggedNode(nodeCoords);
            viewRay = scene->getCamera(0).getViewRay(e.getPos());
            plane = PlaneEquation(h.pos,viewRay.direction);
#else
            Point32f p = parent->getPaperCoordinates(e.getPos());
            parent->setDraggedPosition(p); // also hides the indicator if p.x<0
            if(p.x < 0){
              paper = 0;
              return;
            }
            paper=parent;
            viewRay = scene->getCamera(0).getViewRay(e.getPos());
            plane = PlaneEquation(parent->getInterpolatedPosition(p),viewRay.direction);
#endif



          }else{
            showContexMenu(e);
          }
        }else if(paper && e.isDragEvent()){
          viewRay = scene->getCamera(cameraIndex).getViewRay(e.getPos());
        }else if(paper && e.isReleaseEvent()){
          Mutex::Locker lock(mutex);
#ifdef USE_OLD_INTERACTION_STYLE
          paper->setDraggedNode(Point(-1,-1));
#else
          parent->setDraggedPosition(Point32f(-1,-1));
#endif
          paper = 0;
        }
      }else{
        Mutex::Locker lock(mutex);
        if(paper && e.isReleaseEvent()){
          paper->setDraggedNode(Point(-1,-1));
          paper = 0;
        }
        h->process(e);        
      }
    }
    
    void applyForce(float factor){
      if(lastTime == Time(0)) lastTime = Time::now();
      Time now = Time::now();
      double dt = (now-lastTime).toSecondsDouble();
      if(dt < 0.0001) return;

      lastTime = now;

      Mutex::Locker lock(mutex);
      if(paper){
        paper->lock();
#ifdef USE_OLD_INTERACTION_STYLE
        paper->moveVertex(nodeCoords,viewRay.getIntersection(plane),factor/dt);
#else
        paper->movePosition(((ManipulatablePaper*)paper)->draggedPositionIndicator->p,viewRay.getIntersection(plane),factor/dt);
#endif
        paper->unlock();
      }
    }
  };



  ManipulatablePaper::ManipulatablePaper(PhysicsWorld *world, Scene *scene, 
                                         int W, int H, const Vec *init,
                                         bool initByCorners,
                                         const Img8u *texture,
                                         const Img8u *backTexture):
    PhysicsPaper(*world,W,H,init,initByCorners,texture,backTexture),world(world),scene(scene),showAllConstraints(false){
    const Size dim = getDimensions();
    
    for(int y=0;y<dim.height;++y){
      for(int x=0;x<dim.width;++x){
        SceneObject *cube = addCube(0,0,0,2);
        nodes.push_back(cube);
        cube->setColor(Primitive::quad,GeomColor(255,0,0,255));
      }
    }
    setVisible(Primitive::vertex,false);
    setVisible(Primitive::line,false);
    
    world->addObject(this);
    scene->addObject(this);
    setConfigurableID("paper");

    draggedPositionIndicator = new DraggedPositionIndicator(this);
    addChild(draggedPositionIndicator);
    draggedPositionIndicator->setVisible(false);

    setLineSmoothingEnabled(false);

  }
  
  Vec ManipulatablePaper::getPos(const Point &idx){
    lock();
    Size dim = getDimensions();
    Mat m = nodes[idx.x+dim.width*idx.y]->getTransformation();
    return Vec(m(3,0),m(3,1),m(3,2),1);
    unlock();
  }
  
  void ManipulatablePaper::prepareForRendering(){
    PhysicsPaper::prepareForRendering();
    Size dim = getDimensions();
    lock();
    for(int y=0;y<dim.height;++y){
      for(int x=0;x<dim.width;++x){
        Vec p = getNodePosition(x,y);
        nodes[x+dim.width*y]->removeTransformation();
        nodes[x+dim.width*y]->translate(p[0],p[1],p[2]);
      }
    }
    unlock();
  }

  void ManipulatablePaper::addAttractor(Point coords, bool oscillating){
    Mutex::Locker lock(attractorMutex);
    std::string hash = str(coords);
    AttractorMap::iterator it = attractors.find(hash);
    if(it != attractors.end()){
      delete it->second;
      it->second = new VertexAttractor(scene, coords, oscillating, this);
    }else{
      attractors[hash] = new VertexAttractor(scene, coords, oscillating, this);
      GeomColor color = oscillating ? GeomColor(255,255,0,255) : GeomColor(0,255,0,255);
      nodes[coords.x + getDimensions().width * coords.y]->setColor(Primitive::quad,color);
    }
  }
    
  void ManipulatablePaper::removeAttractor(Point coords){
    Mutex::Locker lock(attractorMutex);
    std::string hash = str(coords);
    AttractorMap::iterator it = attractors.find(hash);
    if(it != attractors.end()){
      delete it->second;
      attractors.erase(it);
      nodes[coords.x + getDimensions().width * coords.y]->setColor(Primitive::quad,geom_red(255));
    }
  }
  
  void ManipulatablePaper::removeAllAttractors(){
    Mutex::Locker lock(attractorMutex);
    for(AttractorMap::iterator it = attractors.begin(); it != attractors.end(); ++it){
      delete it->second;
      attractors.erase(it);
    }
    for(size_t i=0;i<nodes.size();++i){
      nodes[i]->setColor(Primitive::quad,geom_red(255));
    }
  }

  void ManipulatablePaper::applyAllForces(float attractorForce, float mouseForce){
    Mutex::Locker lock(attractorMutex);
    for(AttractorMap::iterator it = attractors.begin(); it != attractors.end(); ++it){
      it->second->apply(attractorForce);
    }
    if(mouse) ((ManipulatablePaperMouseHandler*)mouse.get())->applyForce(mouseForce);
  }

  MouseHandler *ManipulatablePaper::createMouseHandler(int cameraIndex){
    if(!mouse) mouse = new ManipulatablePaperMouseHandler(scene->getMouseHandler(cameraIndex),
                                                          world, scene, this, cameraIndex);
    return mouse.get();
  }

  void ManipulatablePaper::setCubesVisible(bool on){
    for(size_t i=0;i<nodes.size();++i){
      nodes[i]->setVisible(on);
    }
  }

  /// sets texture or shaded mode
  void ManipulatablePaper::setTextureVisible(bool on){
    setVisible(Primitive::quad,!on,false);
    setVisible(Primitive::texture,on);
  }


  // Vertex Attractor ---------------------------------------------------




  ManipulatablePaper::VertexAttractor::VertexAttractor(Scene *scene, Point idx, bool oscillating, ManipulatablePaper *parent):
    idx(idx),oscillating(oscillating), scene(scene), parent(parent){
    if(oscillating){
      time = Time::now();
    };
    startPos = parent->getNodePosition(idx);
    
    addVertex(startPos,geom_red(255));
    addVertex(startPos,geom_blue(255));
    addLine(0,1,geom_red(255));
    setColorsFromVertices(Primitive::line,true);
    setLineWidth(3);
    scene->lock();
    scene->addObject(this);
    scene->unlock();
  }
  
  ManipulatablePaper::VertexAttractor::~VertexAttractor(){
    scene->lock();
    scene->removeObject(this);
    scene->unlock();
  }
  
  void ManipulatablePaper::VertexAttractor::apply(float streangth){
    m_vertices[0] = parent->getNodePosition(idx);
    if(oscillating){
      Vec dst = startPos+Vec(0,0,sin(0.99*(time-Time::now()).toSecondsDouble())*200,0);
      m_vertices[1] = dst;
      parent->moveVertex(idx,dst,streangth);
    }else{
      parent->moveVertex(idx,startPos,streangth);
    }
  }


  void ManipulatablePaper::adaptRowStiffness(float val, int row){
    PhysicsPaper::adaptRowStiffness(val,row);
    Size s = getDimensions();
    for(int x=0;x<s.width;++x){
      //      nodes[x + s.width*row]->setColor(Primitive::quad,GeomColor(255,255-255*val,0,255));
      if(x) addLineAnnotation(Point32f(x-1,row), Point32f(x,row));
    }
  }
    
  void ManipulatablePaper::adaptColStiffness(float val, int col){
    PhysicsPaper::adaptColStiffness(val,col);
    Size s = getDimensions();
    for(int y=0;y<s.height;++y){
      // nodes[col + s.width*y]->setColor(Primitive::quad,GeomColor(255,255-255*val,0,255));
      if(y) addLineAnnotation(Point32f(col,y-1), Point32f(col,y));
    }    
  }
    
  void ManipulatablePaper::adaptGlobalStiffness(float val){
    PhysicsPaper::adaptGlobalStiffness(val);
    for(size_t i=0;i<nodes.size();++i){
      nodes[i]->setColor(Primitive::quad,GeomColor(255,255-255*val,0,255));
    }  
    
    removeAllLineAnnoations();
  }
  
  namespace{
    struct LineIntersection{
      int idx;
      LineIntersection(int idx=0):idx(idx){}
      Point32f p;
      float r,s;
    };
  }
  
  /// gets the boundary as complete loop
  static std::vector<LineIntersection> find_quad_intersections(const Point32f quad[5], const Point32f &a, const Point32f &b){
    std::vector<LineIntersection> intersections;
    for(int i=0;i<4;++i){
      LineIntersection its(i);
      if(line_segment_intersect(quad[i],quad[i+1],a,b,&its.p,&its.r,&its.s)){
        intersections.push_back(its);
      }
    }
    return intersections;
  }

  void ManipulatablePaper::adaptStiffnessAlongLine(const Point32f &a, const Point32f &b, float val){
    const Camera &cam = scene->getCamera(0);
    Vec pa = cam.getViewRay(a).direction;
    Vec pb = cam.getViewRay(b).direction;
    
    PlaneEquation plane(cam.getPosition(),cross(pa,pb));

    adaptStiffnessAlongIntersection(plane,val);
        
    // --- create line-annotations -------------

    // 1st: get projected 2D grid positions
    int w = getDimensions().width, h = getDimensions().height;
    std::vector<Vec> nodeCenters(w*h);
    for(int x=0;x<w;++x){
      for(int y=0;y<h;++y){
        nodeCenters[x+w*y] = getNodePosition(x,y);
      }
    }
    std::vector<Point32f> ps1D = cam.project(nodeCenters);
    Array2D<Point32f> ps(w,h,ps1D.data(),false);
    
    // 2nd: check_intersection for every quad
    Array2D<Point32f> projections(w,h);
    for(int x=1;x<w;++x){
      for(int y=1;y<h;++y){
        const Point32f quad[] = {ps(x-1,y-1), ps(x,y-1), ps(x,y) , ps(x-1,y), ps(x-1,y-1)};
        std::vector<LineIntersection> its = find_quad_intersections(quad,a,b);

        if(its.size() < 2) continue;

        Point32f ps[2];
        for(int i=0;i<2;++i){
          switch(its[i].idx){
            case 0: ps[i] = Point32f(x-its[i].r,y-1); break;
            case 1: ps[i] = Point32f(x,y-its[i].r); break;
            case 2: ps[i] = Point32f(x-1+its[i].r,y); break;
            case 3: ps[i] = Point32f(x-1,y-1+its[i].r); break;
          }
        }
        addLineAnnotation(ps[0],ps[1],val > 0.5 ? geom_blue(255) : GeomColor(255,100,0,255));
      }
    }
    
  }


  void ManipulatablePaper::addLineAnnotation(const Point32f &a, const Point32f &b, const GeomColor &color){
    lines.push_back(new LineAnnotation(this,a,b,color));
    addChild(lines.back());
  }
  
  void  ManipulatablePaper::removeAllAnnoations(){
    lock();
    for(size_t i=0;i<lines.size();++i){
      removeChild(lines[i]);
      delete lines[i];
    }
    lines.clear();
    unlock();
  }

  ManipulatablePaper::DraggedPositionIndicator::DraggedPositionIndicator(ManipulatablePaper *parent):
    SceneObject("sphere",Mat(0,0,0,4,15,15).data()),parent(parent){
    setColor(Primitive::quad, GeomColor(255,0,0,255));
    setVisible(false);
    setVisible(Primitive::line,false);
    setVisible(Primitive::vertex,false);
                 
  }
  void ManipulatablePaper::DraggedPositionIndicator::prepareForRendering(){
    if(p.x < 0){
      setVisible(false);
    }else{
      setVisible(true);
      removeTransformation();
      translate(parent->getInterpolatedPosition(p));
    }
  }

  
  ManipulatablePaper::LineAnnotation::LineAnnotation(PhysicsPaper *parent,
                                                     const Point32f &a, 
                                                     const Point32f &b,
                                                     const GeomColor &color):a(a),b(b),parent(parent){
    setDepthTestEnabled(false);
    addVertex(Vec(0,0,0,1), color);
    addVertex(Vec(0,0,0,1), color);
    addLine(0,1,color);
    setVisible(Primitive::line,true);
    setVisible(Primitive::vertex,false);
    setPointSize(10);
    setLineWidth(3);
  }
  

  void ManipulatablePaper::LineAnnotation::prepareForRendering(){
    if(a == b){
      m_vertices[0] = m_vertices[1] = parent->getInterpolatedPosition(a);
      setVisible(Primitive::line,false);
      setVisible(Primitive::vertex,true);
    }else{
      m_vertices[0] = parent->getInterpolatedPosition(a);
      m_vertices[1] = parent->getInterpolatedPosition(b);
      setVisible(Primitive::line,true);
      setVisible(Primitive::vertex,false);
    }
  }

  /// removes all line annotations
  void ManipulatablePaper::removeAllLineAnnoations(){
    for(size_t i=0;i<lines.size();++i){
      removeChild(lines[i]);
    }
    lines.clear();
  }

  Point32f ManipulatablePaper::getPaperCoordinates(const Point32f &screenPosition2D){
    return getPaperCoordinates(scene->getCamera(0).getViewRay(screenPosition2D));
  }

 
 
  void ManipulatablePaper::setDraggedPosition(const Point32f &paperPos){
    if(paperPos.x < 0){
      draggedPositionIndicator->setVisible(false);
    }else{
      std::cout << "current paper position is " << paperPos << std::endl;
      draggedPositionIndicator->setVisible(true);
      draggedPositionIndicator->p = paperPos;
    }
  }


  void ManipulatablePaper::complexCustomRender(icl::geom::ShaderUtil *u){
    u->deactivateShaders();
    glDisable(GL_LIGHTING);
    //glDisable(GL_DEPTH_TEST);
      
    if(showAllConstraints){
      glBegin(GL_LINES);
    
      glLineWidth(2);
      for(size_t i=0;i<constraints.size();++i){
        const BendingConstraint &c = constraints[i];

        if(c.getStiffness() < 0.1) continue;
        glColor4f(0,1,0,c.getStiffness());

        std::pair<Vec,Vec> l = c.getLine();
        glVertex3fv(&l.first[0]);
        glVertex3fv(&l.second[0]);
      }
      glEnd();
    }
    glEnable(GL_LIGHTING);
    //glEnable(GL_DEPTH_TEST);
          
  }
                          
  ManipulatablePaper::Shadow::Shadow(float zLevel, ManipulatablePaper *parent):
    zLevel(zLevel),parent(parent){
  }
  
  void ManipulatablePaper::Shadow::customRender(){
    lock();
    Size s =  parent->getDimensions();
    const std::vector<Vec> &vs = parent->getVertices();

    glDisable(GL_LIGHTING);
    glColor3f(.2,.2,.2);
    
    for(int y=1;y<s.height;++y){
      glBegin(GL_QUAD_STRIP);
      for(int x=0;x<s.width;++x){
        const float *a = vs[x+y*s.width].data();
        const float *b = vs[x+(y-1)*s.width].data();
        glVertex3f(a[0],a[1],zLevel);
        glVertex3f(b[0],b[1],zLevel);
      }
      glEnd();
    }
    glEnable(GL_LIGHTING);
    
    unlock();
  }


  /// saves the current constraints
  void ManipulatablePaper::saveCFG(const std::string &filename){
    (void)filename;
#if 0
    ConfigFile cfg;
    cfg.setPrefix("config.");
    
    cfg["orig-rest-length"] = cat(originalRestLengths);
    cfg["num-constraints"] = (int)constaints.size();

    for(size_t i=0;i<constaints.size();++i){
      BendingConstaint &c = constaints[i];
      cfg["constraints."+str(i)+".stiffness"] = c.getStiffness();
      
    }
    
    removeAllLineAnnoations();
#endif
  }
  
  /// loads current constraints 
  void ManipulatablePaper::loadCFG(const std::string &filename){
    (void)filename;
  }
  

}
}
