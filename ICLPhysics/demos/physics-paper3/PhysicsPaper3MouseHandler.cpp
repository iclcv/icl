#include <PhysicsPaper3MouseHandler.h>
#include <ICLUtils/Lockable.h>
#include <Menu.h>

namespace icl{
  
  using namespace utils;
  using namespace geom;
  using namespace qt;

  
  struct PhysicsPaper3MouseHandler::Data : public Lockable{
    utils::Point32f start,curr;

    PhysicsPaper3 *model;
    Scene *scene;
    const Camera *camera;
    MouseHandler *alternativeHandler;
    
    Vec currDragWorld;
    Point32f currDragPaper;
    PlaneEquation dragPlane;
    VisualizationDescription linkHighlight;
    PhysicsPaper3::LinkCoords menuCoords;
    
    Menu men;
    
    void setLinkHighlight(const VisualizationDescription &d){
      Mutex::Locker lock(this);
      linkHighlight = d;
    }

    VisualizationDescription getLinkHighlight() const {
      Mutex::Locker lock(this);
      return linkHighlight;
    }

    
    struct DragIndicator : public SceneObject{
      SceneObject *mousePos;
      SceneObject *paperDragPos;
      SceneObject *line;
      
      Data *parent;

      DragIndicator(Data *parent):parent(parent){
       mousePos = SceneObject::sphere(0,0,0,3,10,10);
       mousePos->setColor(Primitive::quad,GeomColor(255,0,0,255));
       mousePos->setVisible(false);
       mousePos->setVisible(Primitive::line,false);
       mousePos->setVisible(Primitive::vertex,false);
       
       paperDragPos = SceneObject::sphere(0,0,0,3,10,10);
       paperDragPos->setColor(Primitive::quad,GeomColor(0,100,255,255));
       paperDragPos->setVisible(false);
       paperDragPos->setVisible(Primitive::line,false);
       paperDragPos->setVisible(Primitive::vertex,false);
       paperDragPos->setVisible(false);    

       line = new SceneObject;
       line->addVertex(Vec(0,0,0,1), GeomColor(255,0,0,255));
       line->addVertex(Vec(0,0,0,1), GeomColor(0,100,255,255));
       line->addLine(0,1);
       line->setColorsFromVertices(Primitive::line,true);
       line->setVisible(false);
       line->setLineWidth(3);
    
       addChild(mousePos);
       addChild(paperDragPos);
       addChild(line);
       
       setLockingEnabled(true);
       setVisible(false);
      }

      virtual void prepareForRendering(){
        if(isVisible()){
          paperDragPos->removeTransformation();
          Vec drag = parent->model->interpolatePosition(parent->currDragPaper);
          paperDragPos->translate(drag);
          line->getVertices()[1] = ensure_hom(drag);
        }
      }
      
      static inline Vec ensure_hom(Vec v){
        v[3] = 1; return v;
      }
      
      void update(const Vec &currMousePos, const Vec &currPaperDragPos){
        lock();
        
        setVisible(true);
        
        mousePos->removeTransformation();
        mousePos->translate(currMousePos);
        
        paperDragPos->removeTransformation();
        paperDragPos->translate(currPaperDragPos);
        
        line->getVertices()[0] = ensure_hom(currMousePos);
        line->getVertices()[1] = ensure_hom(currPaperDragPos);
        unlock();
      }
    };
    
    DragIndicator *indicator;
  };

  PhysicsPaper3MouseHandler::PhysicsPaper3MouseHandler(PhysicsPaper3 *model, Scene *scene, int camIndex):m_data(new Data){
    m_data->model = model;
    m_data->scene = scene;
    m_data->camera = &scene->getCamera(camIndex);
    m_data->alternativeHandler = scene->getMouseHandler(camIndex);
    m_data->curr = m_data->start = Point32f(-1,-1);

    m_data->indicator = new Data::DragIndicator(m_data);
    m_data->scene->addObject(m_data->indicator);
    
    m_data->men.addEntries("remove,streangthen,weaken,memorize weak,memorize strong");
    m_data->men.setCallback(function(this,&PhysicsPaper3MouseHandler::menuCallback));
  }
  
  void PhysicsPaper3MouseHandler::menuCallback(const std::string &entry){
    if(entry == "remove"){
      m_data->model->adaptFoldStiffness(m_data->menuCoords, 1.0);
    }else if(entry == "weaken"){
      m_data->model->adaptFoldStiffness(m_data->menuCoords, 1.e-5);
    }else if(entry == "streanghten"){
      m_data->model->adaptFoldStiffness(m_data->menuCoords, 1.e-5);
    }else if(entry == "memorize weak"){
      m_data->model->adaptFoldStiffness(m_data->menuCoords, 0.01, true);
    }else if(entry == "memorize strong"){
      m_data->model->adaptFoldStiffness(m_data->menuCoords, 1, true);
    }
  }

  void PhysicsPaper3MouseHandler::applyForceToModel(float streangth, float radius){
    if(m_data->indicator->isVisible()){
      m_data->model->movePosition(m_data->currDragPaper,m_data->currDragWorld,streangth,radius);
    }
  }
  void PhysicsPaper3MouseHandler::process(const qt::MouseEvent &e){
    std::cout<<"event"<<std::endl;
    try{
      if(e.isModifierActive(ControlModifier) || m_data->curr != Point32f(-1,-1)){
        if(e.isPressEvent()){
          m_data->start = m_data->curr = e.getPos();
        }else if(e.isDragEvent()){
          m_data->curr = e.getPos();
        }else if(e.isReleaseEvent()) {
          m_data->model->splitAlongLine(m_data->start, m_data->curr, *m_data->camera);
          m_data->start = m_data->curr = Point32f(-1,-1);
        }
        m_data->setLinkHighlight(VisualizationDescription());
      }else if(e.isModifierActive(ShiftModifier) || m_data->indicator->isVisible()){
        ViewRay vr = m_data->camera->getViewRay(e.getPos());
        if(e.isPressEvent()){
          Point32f p = m_data->model->hit(vr);
          if(p != Point32f(-1,-1)){
            m_data->currDragPaper = p;
            m_data->currDragWorld = m_data->model->interpolatePosition(p);
            m_data->indicator->update(m_data->currDragWorld, m_data->currDragWorld);
            m_data->dragPlane = PlaneEquation(m_data->currDragWorld,m_data->camera->getNorm());
          }
        }else if(e.isDragEvent() && m_data->indicator->isVisible()){
          m_data->currDragWorld = vr.getIntersection(m_data->dragPlane);
          m_data->indicator->update(m_data->currDragWorld, m_data->model->interpolatePosition(m_data->currDragPaper));
        }else if(e.isReleaseEvent()){
          m_data->indicator->setVisible(false);
          m_data->indicator->removeTransformation();
        }      
        m_data->setLinkHighlight(VisualizationDescription());
      }else{
        SmartPtr<PhysicsPaper3::LinkCoords> coords = m_data->model->getLinkCoords(e.getPos32f(), *m_data->camera);
        VisualizationDescription d;
        if(coords){
          d = m_data->model->getFoldLineHighlight(*coords, *m_data->camera);
        }
        m_data->setLinkHighlight(d);
        if(coords && e.isPressEvent() && e.isRight()){
          m_data->menuCoords = *coords;
          QPoint p = e.getWidget()->mapToGlobal(QPoint(e.getPos().x, e.getPos().y));
          m_data->men.show(Point(p.x(), p.y()));
        }else{
        m_data->alternativeHandler->process(e);
        }
      }
    }catch(...){}
  }
  
  VisualizationDescription PhysicsPaper3MouseHandler::vis() const{
    VisualizationDescription d = m_data->getLinkHighlight();
    if(m_data->start != Point32f(-1,-1)){
      d.linewidth(2);
      d.color(0,100,255,255);
      d.line(m_data->start,m_data->curr);
    }
    return d;  
  }

}
