#include <ICLPhysics/PhysicsScene.h>
#include <ICLPhysics/PhysicsObject.h>
namespace icl{
  namespace physics{
      void PhysicsScene::addObject(PhysicsObject *object, bool passOwnerShip) {
        Scene::addObject((geom::SceneObject*)object, passOwnerShip);
        PhysicsWorld::addObject(object);
      }
    
      void PhysicsScene::removeObject(int idx) {
        PhysicsObject* object = dynamic_cast<PhysicsObject*>(Scene::getObject(idx));
        if(!object) {
          PhysicsWorld::addObject(object);
        }
        Scene::removeObject(idx);
      }
    
      void PhysicsScene::removeObject(PhysicsObject *obj) {
        PhysicsWorld::removeObject(obj);
        Scene::removeObject((geom::SceneObject*)obj);
      }

      void PhysicsScene::removeObjects(int startIndex, int endIndex) {
        if(endIndex < 0) endIndex = m_objects.size();
        ICLASSERT_RETURN(startIndex >= 0 && startIndex < (int)m_objects.size());
        ICLASSERT_RETURN(endIndex >= 0 && endIndex <= (int)m_objects.size());
        ICLASSERT_RETURN(endIndex > startIndex);

        int pos = startIndex;
        while(startIndex++ < endIndex){
          removeObject(pos);
        }
      }
  }
}
