#pragma once

#include <ICLPhysics/SoftObject.h>
#include <ICLCore/Img.h>
#include <ICLGeom/Camera.h>
#include <ICLUtils/Array2D.h>
#include <ICLPhysics/BendingConstraint.h>
#include <ICLGeom/PlaneEquation.h>
#include <ICLGeom/ViewRay.h>
#include <ICLUtils/Function.h>
#include <ICLUtils/VisualizationDescription.h>


#include <map>
#include <set>

/** \cond */
class btSoftBody;
/** \endcond */

namespace icl{

namespace physics{

  /** \cond */
  class PhysicsWorld;
  /** \endcond */

  class PhysicsPaper3 : public SoftObject{
    protected:
    struct Data;
    Data *m_data;

    struct LinkState{
      LinkState(bool isFirstOrder=true, bool isFold=false,
                bool hasMemorizedRestDist=false):
        isFirstOrder(isFirstOrder),isFold(isFold),
        hasMemorizedRestDist(hasMemorizedRestDist){}
      bool isFirstOrder;
      bool isFold;
      bool hasMemorizedRestDist;
      LinkState *p() const { return new LinkState(*this); }
      static bool is_first_order(void *tag){
        return ((LinkState*)tag)->isFirstOrder;
      }
      static bool is_fold(void *tag){
        return ((LinkState*)tag)->isFold;
      }
      static bool has_memorized_rest_dist(void *tag){
        return ((LinkState*)tag)->hasMemorizedRestDist;
      }
    };

    public:

    typedef std::pair<utils::Point32f,utils::Point32f> LinkCoords;

    void updateSceneObject(btSoftBody *soft);

    PhysicsPaper3(PhysicsWorld *world,
                  bool enableSelfCollision,
                  const utils::Size &cellsInit,
                  const geom::Vec corners[4]=0,
                  const core::Img8u *front_texture=0,
                  const core::Img8u *back_texture=0,
                  float initialStiffness=-1,
                  float initialMaxLinkDistnace=0.5);

    virtual ~PhysicsPaper3();

    void splitAlongLine(const utils::Point32f &a, const utils::Point32f &b, const geom::Camera &currCam);

    geom::Vec interpolatePosition(const utils::Point32f &p) const;

    /// returns paper coordinates of given (or Point(-1,-1) in case of no hit)
    utils::Point32f hit(const geom::ViewRay &ray) const;

    void movePosition(const utils::Point32f &coords, const geom::Vec &target, float streangth=1, float radius=0.1);

    virtual void customRender();

    SceneObject *approximateSurface(int nx=100, int ny=150) const;

    core::Img32f paperCoordinateTest(const geom::Camera &cam) const;

    /// if fixedStiffness is in ]0,1], it is use for all links
    void createBendingConstraints(float maxDistance, float fixedStiffness=-1);

    void setLinksVisible(bool visible);

    void simulateSelfCollision();

    static inline void free_link_state(void *p) { delete (LinkState*)p; }

    const core::Img32f &getFoldMap() const;

    void setFoldMapChangedCallback(utils::Function<void,const core::Img32f &> cb);

    void setFacesVisible(bool visible);

    virtual void lock();

    virtual void unlock();

    const std::vector<utils::Point32f> getTexCoords() const;

    /// returns the possible paper coords of link at given camera pixel
    utils::SmartPtr<LinkCoords> getLinkCoords(const utils::Point32f &pix, const geom::Camera &cam) const;

    /// adapts the stiffness of a given fold (optionally memorizes deformation)
    void adaptFoldStiffness(const LinkCoords &coords, float stiffness, bool memorize=false);

    /// highlights all folds colliniear with line ab in paper space
    utils::VisualizationDescription getFoldLineHighlight(const LinkCoords &coords,
                                                         const geom::Camera &currCam) const;

    /// sets whether folds are always straightened
    void setStraightenFolds(bool enabled);

    /// sets whether folds are automatically inserted doubled
    void setDoubleFolds(bool enabled);


    struct Structure{
      struct Face { int a,b,c; };
      std::vector<utils::Point32f> texCoords;
      std::vector<geom::Vec> vertices;
      std::vector<Face> faces;
      geom::Vec interpolatePosition(const utils::Point32f &p) const;

      void deserializeFrom(std::istream &str);
      void updateToSceneObject(SceneObject *obj);
      utils::Point32f hit(const geom::ViewRay &ray) const;
    };

    void serializeStructureTo(std::ostream &str);



    protected:




    void updateCollisionClusters();

    void updateNodeAreas();
    //void updateConstants();

    bool hitLink(btSoftBody::Link *l, const utils::Point32f &a, const utils::Point32f &b);

    bool hitTriangle(btSoftBody::Face *f, const utils::Point32f &a, const utils::Point32f &b);

    void addTriangle(int a, int b, int c);

    void addLink(int a, int b, float stiffness=1.0f, const LinkState &state=LinkState());

    void addVertexOrReuseOldOne(utils::Point32f &t, btVector3 &v, int &idx);

    bool replaceTriangle(btSoftBody::Face *f, const utils::Point32f &a, const utils::Point32f &b);

    void lockWorld();

    void unlockWorld();

    void updateSmoothNormalGraph();

    void computeSmoothNormals();
  };
} // end of namespace physics
} // end of namespace icl
