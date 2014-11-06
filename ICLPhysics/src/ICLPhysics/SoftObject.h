#pragma once

#include <ICLPhysics/PhysicsObject.h>
#include <ICLUtils/Configurable.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletSoftBody/btSoftBody.h>

namespace icl{
namespace physics{
  /// This class combines a soft-body physics object and it's graphical representation in ICL
  /** Actually, this sub-class is just an explicit restriction of the general PhysicalObject 
      class, that can represent:
      - collision objects 
      - rigid object
      - soft body objects 
  */
  class ICLPhysics_API SoftObject : public PhysicsObject, public utils::Configurable{
    /// internal property wrapped from bullet
    bool m_usePoseMatching;

    /// internal property wrapped from bullet
    bool m_useVolumeConversion;

    protected:
    
    /// protected constructor ..
    SoftObject();
    
    /// this MUST be called by the real implementation class AFTER the real softbody instance is created
    void createAllProperties();

    public:
    /// constructor that uses an obj-file to create a softbody
    SoftObject(const std::string &objFileName, PhysicsWorld *world) throw (utils::ICLException);
    /// returns internal physical object as softBody
    /** Soft bodys are simple soft-body physical objects */
    virtual btSoftBody *getSoftBody();

    /// returns internal physical object as softBody (const)
    /** Soft bodys are simple soft-body physical objects */
    virtual const btSoftBody *getSoftBody() const;

    /// this is now linked via register callback
    void propertyChanged(const Configurable::Property &prop);

  };
}
}
