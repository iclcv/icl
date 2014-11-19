#include <ICLPhysics/PhysicsPaper3MouseHandler.h>
#include <ICLPhysics/PhysicsPaper3.h>
//#include <ICLPhysics/PhysicsPaper.h>
#include <ICLUtils/Lockable.h>
#include <ICLPhysics/PhysicsPaper3ContextMenu.h>

namespace icl{
  using namespace utils;
  using namespace geom;
  using namespace qt;

  namespace physics{
  
    struct PhysicsPaper3MouseHandler::Data : public Lockable{
      utils::Point32f start,curr;

      PhysicsPaper3 *model;
      Scene *scene;
      int cameraIndex;
      MouseHandler *alternativeHandler;
    
      Vec currDragWorld;
      Point32f currDragPaper;
      PlaneEquation dragPlane;
      VisualizationDescription linkHighlight;
      PhysicsPaper3::LinkCoords menuCoords;
      std::vector<PhysicsPaper3::NodeMovement> nodeMovements;
      float lastStreangth, lastRadius;
      PhysicsPaper3ContextMenu men;
      bool addLinksTwice;
    
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
        SceneObject *gaussian;
      
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
       
          gaussian = new SceneObject;
          gaussian->setLineWidth(2);
          gaussian->setVisible(false); // why?
    
          addChild(mousePos);
          addChild(paperDragPos);
          addChild(line);
          addChild(gaussian);
       
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

        
          gaussian->getVertices().clear();
          gaussian->getPrimitives().clear();
          gaussian->getVertexColors().clear();
        
          for(size_t i=0;i<parent->nodeMovements.size();++i){
            const PhysicsPaper3::NodeMovement &m = parent->nodeMovements[i];
            gaussian->addVertex(m.curr);
            Vec s = ((m.target - m.curr) * 0.2) + m.curr;
            s[3] = 1;
            gaussian->addVertex(s);
            gaussian->addLine(2*i, 2*i+1, GeomColor(100,0,200,255));
          }
        
          /* gaussian->removeAllChildren();
              gaussian->addChild(new GaussianHullObject(currMousePos, currPaperDragPos, parent->lastStreangth,
              parent->lastRadius, parent->nodeMovements, parent->model));
              */
        
          unlock();
        }
      };
    
      DragIndicator *indicator;
    };

    PhysicsPaper3MouseHandler::PhysicsPaper3MouseHandler(PhysicsPaper3 *model, Scene *scene, int camIndex):m_data(new Data){
      m_data->model = model;
      m_data->scene = scene;
      m_data->cameraIndex = camIndex;
      m_data->alternativeHandler = scene->getMouseHandler(camIndex);
      m_data->curr = m_data->start = Point32f(-1,-1);
      m_data->lastStreangth = 10;
      m_data->lastRadius = 0.25;

      m_data->indicator = new Data::DragIndicator(m_data);
      m_data->scene->addObject(m_data->indicator);
    
      m_data->men.addEntries("remove,streangthen,weaken,memorize weak,memorize strong");
      m_data->men.setCallback(function(this,&PhysicsPaper3MouseHandler::menuCallback));

      m_data->addLinksTwice = false;
    }
  
    void PhysicsPaper3MouseHandler::setAddLinksTwice(bool enabled){
      m_data->addLinksTwice = enabled;
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
      m_data->lastStreangth = streangth;
      m_data->lastRadius = radius;
      if(m_data->indicator->isVisible()){
        m_data->model->movePosition(m_data->currDragPaper,m_data->currDragWorld,
                                    streangth,radius,&m_data->nodeMovements);
      }
    }
  
    void PhysicsPaper3MouseHandler::process(const qt::MouseEvent &e){
      const Camera &cam = m_data->scene->getCamera(m_data->cameraIndex);
      try{
        if(e.isModifierActive(ControlModifier) || m_data->curr != Point32f(-1,-1)){
          if(e.isPressEvent()){
            m_data->start = m_data->curr = e.getPos();
          }else if(e.isDragEvent()){
            m_data->curr = e.getPos();
          }else if(e.isReleaseEvent()) {
            const Point32f &a = m_data->start, &b = m_data->curr;
            Point32f dir = b - a;
            float l = dir.norm();
            if(l){
              if(m_data->addLinksTwice){
                Point32f n = dir * (1./l);
                Point32f perp(n.y, -n.x);
                Point32f offs = perp*2; // distance is 4 pix??
                m_data->model->splitAlongLine(a + offs, b + offs, cam);
                m_data->model->splitAlongLine(a - offs, b - offs, cam);
              }else{
                m_data->model->splitAlongLine(m_data->start, m_data->curr, cam);
              }
            }
            m_data->start = m_data->curr = Point32f(-1,-1);
          }
          m_data->setLinkHighlight(VisualizationDescription());
        }else if(e.isModifierActive(ShiftModifier) || m_data->indicator->isVisible()){
          ViewRay vr = cam.getViewRay(e.getPos());
          if(e.isPressEvent()){
            Point32f p = m_data->model->hit(vr);
            if(p != Point32f(-1,-1)){
              m_data->currDragPaper = p;
              m_data->currDragWorld = m_data->model->interpolatePosition(p);
              m_data->indicator->update(m_data->currDragWorld, m_data->currDragWorld);
              m_data->dragPlane = PlaneEquation(m_data->currDragWorld,cam.getNorm());
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
          SmartPtr<PhysicsPaper3::LinkCoords> coords = m_data->model->getLinkCoords(e.getPos32f(), cam);
          VisualizationDescription d;
          if(coords){
            d = m_data->model->getFoldLineHighlight(*coords, cam);
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

}
