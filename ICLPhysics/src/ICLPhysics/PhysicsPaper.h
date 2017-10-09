/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsPaper.h               **
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
#include <ICLUtils/Array2D.h>
#include <ICLPhysics/BendingConstraint.h>
#include <ICLGeom/PlaneEquation.h>
#include <map>

/** \cond */
class btSoftBody;
/** \endcond */

namespace icl{
namespace physics{

  /** \cond */
  class PhysicsWorld;
  /** \endcond */

  /// This class represents cloth or paper like soft-body objects
  class ICLPhysics_API PhysicsPaper : public SoftObject{

    friend class PaperTexturePrimitive;

    SceneObject *so;
    utils::Size cells;
    bool hasBackfaceTexture;

    protected:
    std::vector<BendingConstraint> constraints;
    std::vector<float> originalRestLengths;


    public:
    PhysicsPaper(const PhysicsWorld &scene,
                 int nxCells, int nyCells,
                 const geom::Vec *init, bool initWithCorners,
                 const core::Img8u *texture = 0,
                 const core::Img8u *backfaceTexture=0);

    /// adapted version of update scene object that does already get the special version of the object
    virtual void updateSceneObject();

    /// moves a vertex
    void moveVertex(const utils::Point &xy, const geom::Vec &pos, float factor);

    /// moves an arbitrary paper position towards pos
    void movePosition(const utils::Point32f &paperPos, const geom::Vec &pos, float factor);

    utils::Point getNodeIndex(const geom::Vec &v);
    void setDraggedNode(const utils::Point &xy);

    inline const utils::Size &getDimensions() const { return cells; }
    geom::Vec getNodePosition(int x, int y) const;
    geom::Vec getNodePosition(const utils::Point &p) const{
      return getNodePosition(p.x,p.y);
    }
    void setNodeMass(const utils::Point &xy, float mass);
    void setNodeMass(const geom::Vec &v, float mass);
    void setTotalMass(float mass);

    core::Img32f getVelocityMap() const;

    /// a row is shorter than a column
    /** +---------+
        |         |
        |         |
        |         |   <--- rows !!!
        |         |
        |         |
        |         |
        |         |
        |         |
        +---------+
    */
    virtual void adaptRowStiffness(float val, int row);
    virtual void adaptColStiffness(float val, int col);
    virtual void adaptGlobalStiffness(float val);

    /// softens all constraints that intersect the given plane equation
    void adaptStiffnessAlongIntersection(const geom::PlaneEquation &plane,
                                         float val);

    virtual void memorizeDeformation();
    virtual void resetDeformation();

    /// finds the optimal paper coordinates for a given point in the world
    utils::Point32f getPaperCoordinates(const geom::ViewRay &ray);

    /// returns the world position for a given paper position
    geom::Vec getInterpolatedPosition(const utils::Point32f &paperPos);

    /// returns node position (using the physics model, translated to ICL-units)
    geom::Vec getPosFromPhysics(int x, int y) const;

    /// returns node normal (using the physics model, translated to ICL-units)
    geom::Vec getNormalFromPhysics(int x, int y);


    protected:
    void randomizeLinks();

  };
}
}
