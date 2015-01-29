/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsPaper3.h              **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
  namespace physics {

    /** \cond */
    class PhysicsWorld;
    /** \endcond */

    class ICLPhysics_API PhysicsPaper3 : public SoftObject{
      protected:
      struct Data;
      Data *m_data;

      struct ICLPhysics_API LinkState{
        LinkState(bool isFirstOrder=true, bool isFold=false,
                  bool hasMemorizedRestDist=false, bool isOriginal=false):
          isFirstOrder(isFirstOrder),isFold(isFold),
          hasMemorizedRestDist(hasMemorizedRestDist),
          isOriginal(isOriginal){}

        bool isFirstOrder;
        bool isFold;
        bool hasMemorizedRestDist;
        bool isOriginal;

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
        static bool is_original(void *tag){
          return ((LinkState*)tag)->isOriginal;
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

      PhysicsPaper3(PhysicsWorld *world, const PhysicsPaper3 &other);
      
      virtual ~PhysicsPaper3();
      
      PhysicsPaper3 *clone(PhysicsWorld *world) const;

      /// takes the softbody from the given other physics paper 
      /** The current own soft-body is deleted, the current other 
          softbody is moved (by pointer) into this class. Most additional
          state variables (e.g. texture coords) are copied over aswell
          (right now the physics world and the corresponding config from
          other is not copied over)*/
      void takeSoftBodyFrom(PhysicsPaper3 *other);

      void saveState(const std::string &filename);
      
      void restoreState(const std::string &filename);
      
      void setFaceAlpha(float alpha01);

      float getFaceAlpha() const;
      
      int getNumNodes() const;

      void splitAlongLine(const utils::Point32f &a, const utils::Point32f &b, const geom::Camera &currCam);

      void splitAlongLineInPaperCoords(const utils::Point32f &a, const utils::Point32f &b, bool autoExtendLineToEdges=true);


      geom::Vec interpolatePosition(const utils::Point32f &p) const;

      /// returns paper coordinates of given (or Point(-1,-1) in case of no hit)
      utils::Point32f hit(const geom::ViewRay &ray) const;


      struct ICLPhysics_API NodeMovement{
        geom::Vec curr;
        geom::Vec target;
        float alpha;
      };

      void movePosition(const utils::Point32f &coords, const geom::Vec &target, float streangth=1, float radius=0.1,
                        std::vector<NodeMovement> *dst=0);



      virtual void complexCustomRender(icl::geom::ShaderUtil* util);


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


      struct ICLPhysics_API Structure{
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

      void setLinkColors(const geom::GeomColor &originalLinks, 
                         const geom::GeomColor &insertedLinks,
                         const geom::GeomColor &creaseLines,
                         const geom::GeomColor &bendingConstraints);

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
