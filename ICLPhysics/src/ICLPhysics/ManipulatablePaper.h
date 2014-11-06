#pragma once
#include <ICLGeom/Scene.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <ICLPhysics/PhysicsPaper.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Mutex.h>
#include <ICLQt/MouseHandler.h>

#include <map>

namespace icl{
namespace physics{
  
  /// extension of the standard physical paper class
  class ICLPhysics_API ManipulatablePaper : public PhysicsPaper{
    struct VertexAttractor : public SceneObject{
      utils::Point idx;
      utils::Time time;
      geom::Vec startPos;
      bool oscillating;
      geom::Scene *scene;
      ManipulatablePaper *parent;

      VertexAttractor(geom::Scene *scene, utils::Point coords, 
                      bool oscillating, ManipulatablePaper *parent);
      ~VertexAttractor();
      void apply(float streangth);
    };
    
    
    /// Utility class for extra annotations, rendered in paper coordinates
    /** hack: if a and b are identical, the annotation defines a point annotation */
    struct LineAnnotation : public SceneObject{
      utils::Point32f a;
      utils::Point32f b;
      PhysicsPaper *parent;

      LineAnnotation(PhysicsPaper *paper, const utils::Point32f &a, 
                     const utils::Point32f &b, const geom::GeomColor &color);
      virtual void prepareForRendering();
    };
  

    struct DraggedPositionIndicator : public SceneObject{
      DraggedPositionIndicator(ManipulatablePaper *parent);
      utils::Point32f p;
      ManipulatablePaper *parent;
      virtual void prepareForRendering();
    };
      
    
    std::vector<SceneObject *> nodes;
    typedef std::map<std::string,VertexAttractor*> AttractorMap;
    AttractorMap attractors;
    PhysicsWorld *world;
    geom::Scene *scene;
    utils::Mutex attractorMutex;
    utils::SmartPtr<qt::MouseHandler> mouse;
    std::vector<LineAnnotation*> lines;
    DraggedPositionIndicator *draggedPositionIndicator;
    friend class ManipulatablePaperMouseHandler;
    bool showAllConstraints;

    public:
    /// create with given parameters
    ManipulatablePaper(PhysicsWorld *world, geom::Scene *scene, 
                       int W, int H, const geom::Vec *init,
                       bool initByCorners,
                       const core::Img8u *frontTexture=0,
                       const core::Img8u *backTexture=0 );
    
    /// get's current position of node at given coorndinats
    geom::Vec getPos(const utils::Point &idx);
    
    /// updates all cubes from the underlying paper
    void prepareForRendering();
    
    /// adds a new attractor at given coordinates
    void addAttractor(utils::Point coords, bool oscillating=false);
    
    /// removes an attractor at given coordinates
    void removeAttractor(utils::Point coords);
    
    /// removes all current attractors
    void removeAllAttractors();

    /// applies all attractors and optional mouse force
    void applyAllForces(float attractorForce, float mouseForce);
    
    /// creates a mouse handler using the internal scene
    qt::MouseHandler *createMouseHandler(int cameraIndex = 0);
    
    /// sets whether the handle cubes are visible
    void setCubesVisible(bool on);
    
    /// sets texture or shaded mode
    void setTextureVisible(bool on);

    /// adation to base class (colors the cubes)
    virtual void adaptRowStiffness(float val, int row);
    
    /// adation to base class (colors the cubes)
    virtual void adaptColStiffness(float val, int col);
    
    /// adation to base class (colors the cubes)
    virtual void adaptGlobalStiffness(float val);

    /// removes all line annotations
    void removeAllLineAnnoations();

    
    /// adapts all constraints, that intersect the given 2D line (using the current camera)
    /** Actually, this function will just use the current camera position to 
        create a PlaneEquation that is passed the parent 
        PhysicsPaper::adaptStiffnessAlongIntersection method */
    void adaptStiffnessAlongLine(const utils::Point32f &a, const utils::Point32f &b, float val);
    
    /// adds a line annotation (given in paper coordinats=;
    void addLineAnnotation(const utils::Point32f &a, const utils::Point32f &b, 
                           const geom::GeomColor &color=geom::GeomColor(255,100,0,255));

    void removeAllAnnoations();
    
    /// returns the paper position for a given 2D screen position or (-1,-1) if not found
    utils::Point32f getPaperCoordinates(const utils::Point32f &screenPosition2D);

    /// adds a highlight at given position
    void setDraggedPosition(const utils::Point32f &paperPos);
    
    /// import name for parent class
    using PhysicsPaper::getPaperCoordinates;

    /// sets whether contraints are visualized
    void setShowAllConstraints(bool show){
      showAllConstraints = show;
    }
    
    /// returns whether constraints are shown
    bool getShowAllConstraints() const{
      return showAllConstraints;
    }
    
    /// custom rendering for constraint visualization
    virtual void complexCustomRender(icl::geom::ShaderUtil *u);

    /// saves the current constraints
    virtual void saveCFG(const std::string &filename);
    
    /// loads current constraints 
    virtual void loadCFG(const std::string &filename);
    
    struct Shadow : public geom::SceneObject{
      Shadow(float zLevel, ManipulatablePaper *parent);
      virtual void customRender();
      float zLevel;
      ManipulatablePaper *parent;
    };
    
    utils::SmartPtr<Shadow> m_shadow;
    
    void addShadow(float zLevel){
      if(m_shadow){
        throw utils::ICLException("shadow cannot be added twice!");
      }
      m_shadow = new Shadow(zLevel,this);
      addChild(m_shadow.get());
    }
  };

}
}
