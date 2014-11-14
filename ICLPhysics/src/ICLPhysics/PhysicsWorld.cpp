#include <ICLPhysics/PhysicsWorld.h>
#include <ICLUtils/ConfigFile.h>

#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>
#include <BulletCollision/BroadphaseCollision/btAxisSweep3.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>

#include <BulletCollision/CollisionShapes/btBoxShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <LinearMath/btDefaultMotionState.h>

#include <ICLPhysics/PhysicsDefs.h>
#include <ICLPhysics/PhysicsObject.h>
#include <ICLPhysics/RigidObject.h>
#include <ICLPhysics/SoftObject.h>
#include <ICLGeom/ViewRay.h>
#include <ICLPhysics/Constraint.h>

#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

#include <BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.h>
#include <ICLPhysics/Constraint.h>

using namespace std;

namespace icl{
  namespace physics{
    
    struct PhysicsWorld::Data{
      btAlignedObjectArray<btCollisionShape*> m_collisionShapes;
      btBroadphaseInterface* m_broadphase;
      btCollisionDispatcher* m_dispatcher;
      btConstraintSolver* m_solver;
      btDefaultCollisionConfiguration* m_collisionConfiguration;
      btSoftBodyWorldInfo *m_worldInfo;
      btSoftRigidDynamicsWorld *m_dynamicsWorld;
      btOverlapFilterCallback* m_filterCallback;
      math::DynMatrix<bool>* m_collisionMatrix;
      
      std::vector<PhysicsObject*> m_objects;

      std::vector<Constraint*> m_ownedConstraints;
      
      utils::Time lastTime;
    };

    int collisions = 0;
    PhysicsWorld::PhysicsWorld():utils::Lockable(true),data(new Data){

      //const float WORLD_AABB_MIN_X = icl2bullet(-1000);
      //const float WORLD_AABB_MIN_Y = icl2bullet(-1000);
      //const float WORLD_AABB_MIN_Z = icl2bullet(-1000);

      //const float WORLD_AABB_MAX_X = icl2bullet(1000);
      //const float WORLD_AABB_MAX_Y = icl2bullet(1000);
      //const float WORLD_AABB_MAX_Z = icl2bullet(1000);
      
      const float WORLD_GRAVITY_X = 0;
      const float WORLD_GRAVITY_Y = 0;
      const float WORLD_GRAVITY_Z = -9.81 / METER_TO_BULLET_UNIT;

      const bool DISPATCHER_ENABLE_SPU = true;

      const float WORLD_AIR_DENSITY = 1.2;

      //btVector3 aabbmin(WORLD_AABB_MIN_X,WORLD_AABB_MIN_Y,WORLD_AABB_MIN_Z);
      //btVector3 aabbmax(WORLD_AABB_MAX_X,WORLD_AABB_MAX_Y,WORLD_AABB_MAX_Z);
      //data->m_broadphase = new btAxisSweep3(aabbmin,aabbmax,33000);
      data->m_broadphase = new btDbvtBroadphase();
      data->m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration;
      data->m_dispatcher = new btCollisionDispatcher(data->m_collisionConfiguration);
      data->m_solver = new btSequentialImpulseConstraintSolver;


      data->m_dynamicsWorld = new btSoftRigidDynamicsWorld(data->m_dispatcher,
                                                     data->m_broadphase,
                                                     data->m_solver,
                                                     data->m_collisionConfiguration);
      data->m_dynamicsWorld->setGravity(btVector3(WORLD_GRAVITY_X,WORLD_GRAVITY_Y,WORLD_GRAVITY_Z));
      data->m_dynamicsWorld->getDispatchInfo().m_enableSPU = DISPATCHER_ENABLE_SPU;
    
      data->m_worldInfo = new btSoftBodyWorldInfo;
      data->m_worldInfo->air_density = (btScalar)WORLD_AIR_DENSITY;
      data->m_worldInfo->water_density = 0;
      data->m_worldInfo->water_offset = 0;
      data->m_worldInfo->water_normal = btVector3(0,0,0);
      data->m_worldInfo->m_gravity.setValue(WORLD_GRAVITY_X,WORLD_GRAVITY_Y,WORLD_GRAVITY_Z);


      data->m_worldInfo->m_broadphase = data->m_broadphase;
      data->m_worldInfo->m_dispatcher = data->m_dispatcher;
      data->m_worldInfo->m_sparsesdf.Initialize();
      //create collisionmatrix
      data->m_collisionMatrix = new math::DynMatrix<bool>(100,100,true);
      
      //define collision callback using the collisionmatrix and check constraints
      struct customFilterCallback : public btOverlapFilterCallback
      {
        math::DynMatrix<bool> *collisionMatrix;
        customFilterCallback(math::DynMatrix<bool>* mat):collisionMatrix(mat){}
	      // return true when pairs need collision
	      virtual bool	needBroadphaseCollision(btBroadphaseProxy* proxy0,btBroadphaseProxy* proxy1) const
	      {
		      bool collides = (proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0;
		      collides = collides && (proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask);
		      //stop here if collision masks already determined that no collision is needed
		      if(!collides)
		        return false;
	        
		      //check collisionmatrix
		      PhysicsObject* obj0 = static_cast<PhysicsObject*>(static_cast<btCollisionObject*>(proxy0->m_clientObject)->getUserPointer());
		      PhysicsObject* obj1 = static_cast<PhysicsObject*>(static_cast<btCollisionObject*>(proxy1->m_clientObject)->getUserPointer());
		      if(obj0 && obj1)
		      {
		        int col = min(obj0->getCollisionGroup(), obj1->getCollisionGroup());
		        int row = max(obj0->getCollisionGroup(), obj1->getCollisionGroup());
		        collides = collides && (*collisionMatrix)(col,row);
		      }
		      //stop here if collision masks already determined that no collision is needed
		      if(!collides)
		        return false;
		      return true;
	      }
      };
      
      //add the callback
      data->m_filterCallback = new customFilterCallback(data->m_collisionMatrix);
      data->m_dynamicsWorld->getPairCache()->setOverlapFilterCallback(data->m_filterCallback);

      //define collision tick callback that checks for collisions and calls each objects callback
      struct tickCallback {
        static void callback(btDynamicsWorld *world, btScalar timeStep) {
          int numManifolds = world->getDispatcher()->getNumManifolds();
          for (int i=0;i<numManifolds;i++)
          {
            btPersistentManifold* contactManifold =  world->getDispatcher()->getManifoldByIndexInternal(i);

            int numContacts = contactManifold->getNumContacts();
            if(numContacts) {
              PhysicsObject* obj0 = static_cast<PhysicsObject*>(static_cast<const btCollisionObject*>(contactManifold->getBody0())->getUserPointer());
              PhysicsObject* obj1 = static_cast<PhysicsObject*>(static_cast<const btCollisionObject*>(contactManifold->getBody1())->getUserPointer());
              geom::Vec pos = scaleBullet2icl(contactManifold->getContactPoint(0).getPositionWorldOnA());
              obj0->collisionCallback(obj0,obj1,pos);
              obj1->collisionCallback(obj1,obj0,pos);
            }
          }
        }
      };

      //add the callback to the world
      data->m_dynamicsWorld->setInternalTickCallback(tickCallback::callback);
    }
    
    PhysicsWorld::~PhysicsWorld() {
      //removing constraints from the world
      while(data->m_dynamicsWorld->getNumConstraints() > 0) {
        Constraint *cons = static_cast<Constraint*>(data->m_dynamicsWorld->getConstraint(0)->getUserConstraintPtr());
        removeConstraint(cons);
      }
      //destroying the world
      delete data->m_filterCallback;
      delete data->m_collisionMatrix;
      delete data->m_worldInfo;
      delete data->m_dynamicsWorld;
      delete data->m_dispatcher;
      delete data->m_collisionConfiguration;
      delete data->m_broadphase;
      delete data;
      
    }
    
    void PhysicsWorld::addObject(PhysicsObject *obj){
      utils::Mutex::Locker lock(this);
      //check for the type of physics object and remove it in the right way
      RigidObject* rigid_obj = dynamic_cast<RigidObject*>(obj);
      if(rigid_obj) {
        if(rigid_obj->getRigidBody()) {
          data->m_dynamicsWorld->addRigidBody(rigid_obj->getRigidBody());
          data->m_objects.push_back(obj);
          obj->setCurrentPhysicsWorld(this);
        }
      } else {
        SoftObject* soft_obj = dynamic_cast<SoftObject*>(obj);
        if(soft_obj) {
          if(soft_obj->getSoftBody()) {
            data->m_dynamicsWorld->addSoftBody(soft_obj->getSoftBody());
            data->m_objects.push_back(obj);
            obj->setCurrentPhysicsWorld(this);
          }
        }else{
          ERROR_LOG("given object has an unsupport type or is null");
          return;
        }
      }
    }

    
    void PhysicsWorld::removeObject(PhysicsObject *obj){
      utils::Mutex::Locker lock(this);
      //check for the type of physics object and remove it in the right way
      RigidObject* rigid_obj = dynamic_cast<RigidObject*>(obj);
      if(rigid_obj) {
        if(rigid_obj->getRigidBody()) {
          data->m_dynamicsWorld->removeRigidBody(rigid_obj->getRigidBody());
        }
      } else {
        SoftObject* soft_obj = dynamic_cast<SoftObject*>(obj);
        if(soft_obj) {
          if(soft_obj->getSoftBody()) {
            data->m_dynamicsWorld->removeSoftBody(soft_obj->getSoftBody());
          }
        }else{
          ERROR_LOG("given object has an unsupport type or is null");
          return;
        }
      }
      //remove it from the objectslist
      std::vector<PhysicsObject*>::iterator it = std::find(data->m_objects.begin(),data->m_objects.end(),obj);
      if(it != data->m_objects.end()){
        data->m_objects.erase(it);
        obj->setCurrentPhysicsWorld(0);
      }
    }
      
    void PhysicsWorld::setGravity(const geom::Vec &gravity){
      data->m_dynamicsWorld->setGravity(scaleIcl2bullet(gravity));
      data->m_worldInfo->m_gravity = scaleIcl2bullet(gravity);
    }
    
    void PhysicsWorld::setGravityEnabled(bool on, const geom::Vec *useThisGravityIfOn){
      if(on){
        setGravity(useThisGravityIfOn ? *useThisGravityIfOn : geom::Vec(0,0,-9810));
      }else{
        setGravity(geom::Vec(0,0,0));
      }
    }
    
    void PhysicsWorld::splitImpulseEnabled(bool enable){
      btContactSolverInfo& info = data->m_dynamicsWorld->getSolverInfo();
      info.m_splitImpulse = (int)enable;
      info.m_splitImpulsePenetrationThreshold = -0.02;
    }
    
    void PhysicsWorld::step(float dtSecs, int maxSubSteps, float fixedTimeStep){
      utils::Mutex::Locker lock(this);
        
      //step simulation
      if(dtSecs < 0){
        if(data->lastTime == utils::Time::null){
          data->lastTime = utils::Time::now();
        }else{
          utils::Time now = utils::Time::now();
          data->m_dynamicsWorld->stepSimulation((now-data->lastTime).toSecondsDouble(),maxSubSteps, fixedTimeStep); // default is 1/60
          data->lastTime = now;
        }
      }else{
        data->m_dynamicsWorld->stepSimulation(dtSecs,maxSubSteps, fixedTimeStep); // default is 1/60
      }
    }
    
    bool PhysicsWorld::collideWithWorld(RigidObject* obj, bool ignoreJoints)
    {
      
      struct ContactSensorCallback : public btCollisionWorld::ContactResultCallback {
      
	      btRigidBody& body;
	      bool& ccol;
	      bool ignore;
	      
	      ContactSensorCallback(btRigidBody& tgtBody , bool& col, bool ignoreJoints)
		      : btCollisionWorld::ContactResultCallback(), body(tgtBody), ccol(col), ignore(ignoreJoints) { }

	      virtual bool needsCollision(btBroadphaseProxy* proxy) const {
		      // superclass will check m_collisionFilterGroup and m_collisionFilterMask
		      if(!btCollisionWorld::ContactResultCallback::needsCollision(proxy))
			      return false;
		      // if passed filters, may also want to avoid contacts between constraints if ignoring joints is activated
		      if(ignore)
		        return body.checkCollideWithOverride(static_cast<btCollisionObject*>(proxy->m_clientObject));
		      //return true since the mask check true and the joints are not ignored, allowing us to reach this part of the code
		      return true;
	      }
	
	      //Called with each contact for your own processing (e.g. test if contacts fall in within sensor parameters)
	      virtual btScalar addSingleResult(btManifoldPoint& cp,	
	        const btCollisionObjectWrapper* colObj0Wrap,int partId0,int index0,
	        const btCollisionObjectWrapper* colObj1Wrap,int partId1,int index1)
	      {
	        ICLASSERT((colObj0Wrap->m_collisionObject==&body || colObj1Wrap->m_collisionObject==&body) && "body does not match either collision object");
		      ccol = true;
		      return 0;
	      }
      };

      bool collision = false;
      ContactSensorCallback callback(*(obj->getRigidBody()), collision, false);
      data->m_dynamicsWorld->contactTest(obj->getRigidBody(),callback);
      return collision;
    }
    
    void PhysicsWorld::setGroupCollision(int group0, int group1, bool collides)
    {
      int col = min(group0, group1);
      int row = max(group0, group1);
      (*data->m_collisionMatrix)(col,row) = collides;
    }
    
    bool PhysicsWorld::getGroupCollision(int group0, int group1)
    {
      int col = min(group0, group1);
      int row = max(group0, group1);
      return (*data->m_collisionMatrix)(col,row);
    }
    
    bool PhysicsWorld::rayCast(const geom::ViewRay& ray, float rayLength, PhysicsObject*& obj, geom::Vec &normal, geom::Vec &hitPoint) {
      btVector3 from = btVector3(icl2bullet(ray.offset[0]),icl2bullet(ray.offset[1]),icl2bullet(ray.offset[2]));
      btVector3 to = from + icl2bullet(rayLength) * btVector3(ray.direction[0],ray.direction[1],ray.direction[2]);
      btCollisionWorld::ClosestRayResultCallback callback(from, to);
      data->m_dynamicsWorld->rayTest(from, to, callback);
      if(callback.m_collisionObject)
        obj = static_cast<PhysicsObject*>(callback.m_collisionObject->getUserPointer());
      if(callback.hasHit()){
        normal = geom::Vec(callback.m_hitNormalWorld[0],callback.m_hitNormalWorld[1],callback.m_hitNormalWorld[2], 0);
        hitPoint = geom::Vec(bullet2icl(callback.m_hitPointWorld[0]),bullet2icl(callback.m_hitPointWorld[1]),bullet2icl(callback.m_hitPointWorld[2]), 1);
        return true;
      }
      return false;
    }
    
    void PhysicsWorld::addConstraint(Constraint* constraint, bool disableCollisionWithLinkedBodies, bool passOwnerShip){
      if(!constraint->getConstraint()) throw utils::ICLException("PhysicsWorld::addConstraint: constraint was null");
      data->m_dynamicsWorld->addConstraint(constraint->getConstraint(),disableCollisionWithLinkedBodies);
      if(passOwnerShip) data->m_ownedConstraints.push_back(constraint);
    }
    
    void PhysicsWorld::removeConstraint(Constraint* constraint){
      if(!constraint->getConstraint()) throw utils::ICLException("PhysicsWorld::removeConstraint: constraint was null");
      data->m_dynamicsWorld->removeConstraint(constraint->getConstraint());
      vector<Constraint*>::iterator found = find(data->m_ownedConstraints.begin(),data->m_ownedConstraints.end(),constraint);
      if(found != data->m_ownedConstraints.end())delete *found;
    }
    
    void PhysicsWorld::removeContactPoints(PhysicsObject *obj) {
      data->m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->
        cleanProxyFromPairs(obj->getCollisionObject()->getBroadphaseHandle(), data->m_dynamicsWorld->getDispatcher());
    }

    const btSoftBodyWorldInfo *PhysicsWorld::getWorldInfo() const{
      return data->m_worldInfo;
    }

    btSoftBodyWorldInfo *PhysicsWorld::getWorldInfo(){
      return data->m_worldInfo;
    }

  }
}
