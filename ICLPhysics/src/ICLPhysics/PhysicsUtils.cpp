#include <ICLPhysics/PhysicsUtils.h>
#include <ICLPhysics/RigidObject.h>
#include <ICLPhysics/RigidBoxObject.h>
#include <ICLPhysics/RigidCylinderObject.h>
#include <ICLPhysics/RigidSphereObject.h>
#include <ICLPhysics/RigidConvexHullObject.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h>
#include <BulletCollision/NarrowPhaseCollision/btPointCollector.h>
#include <BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btConvexPenetrationDepthSolver.h>
#include <BulletCollision/CollisionDispatch/btCollisionObject.h>
#include <BulletCollision/CollisionShapes/btConvexShape.h>
#include <ICLPhysics/PhysicsDefs.h>

#include <ICLUtils/Random.h>

#include <ICLGeom/Scene.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <ICLPhysics/PhysicsObject.h>

namespace icl{
  namespace physics{
    
    void remove_fallen_objects(geom::Scene *scene, PhysicsWorld *world, float minZ){
      utils::Mutex::Locker lock(scene);
      for(int i=scene->getObjectCount()-1;i>=0;--i){
        RigidObject *pobj = dynamic_cast<RigidObject*>(scene->getObject(i));
        if(pobj && pobj->getTransformation()(3,2) < minZ){
          scene->removeObject(i);
          world->removeObject(pobj);
        }
      }
    }
    void add_clutter(geom::Scene *scene, PhysicsWorld *world, int num){
      utils::randomSeed();
      for(int i=0;i<num;++i){
        static utils::URand rand0(0,1);
        static utils::URand rand1(-40,40);
        static utils::URand rand2(60 ,90);
        static utils::URand rand3(2,40);
        static utils::URand rand4(0,255);
        static utils::URand rand5(1,8);
        
        RigidObject *cube = ((float)rand0) < 0.5 ?
        (RigidObject *) new RigidBoxObject(rand1,rand1,rand2, rand3,rand3,rand3,10):
        (RigidObject *) new RigidSphereObject(rand1,rand1,rand2, rand5,10);
        cube->setVisible(geom::Primitive::vertex,false);
        cube->setVisible(geom::Primitive::line,false);
        cube->setColor(geom::Primitive::quad,geom::GeomColor(rand4,rand4,rand4,255));
        cube->setCollisionMargin(1);
        scene->addObject(cube);
        world->addObject(cube);
      }
    }

    void calcDistance(PhysicsObject* a, PhysicsObject* b, geom::Mat transA, geom::Mat transB, geom::Vec &pointOnA, geom::Vec &pointOnB, float &distance) {
      btVoronoiSimplexSolver sGjkSimplexSolver;
      btGjkEpaPenetrationDepthSolver epa;
      btGjkPairDetector	convexConvex(dynamic_cast<btConvexShape*>(a->getCollisionObject()->getCollisionShape()),
                                     dynamic_cast<btConvexShape*>(b->getCollisionObject()->getCollisionShape()),
                                     &sGjkSimplexSolver,&epa);

			btPointCollector gjkOutput;
			btGjkPairDetector::ClosestPointInput input;
			input.m_transformA = icl2bullet_scaled_mat(transA);
			input.m_transformB = icl2bullet_scaled_mat(transB);

			convexConvex.getClosestPoints(input ,gjkOutput,0);

			pointOnB = bullet2icl_scaled(gjkOutput.m_pointInWorld);
			pointOnA = bullet2icl_scaled(gjkOutput.m_normalOnBInWorld * gjkOutput.m_distance + gjkOutput.m_pointInWorld);
			distance = bullet2icl(gjkOutput.m_distance);
		}

    void calcDistance(PhysicsObject* a, PhysicsObject* b, geom::Vec &pointOnA, geom::Vec &pointOnB, float &distance)
    {
      calcDistance(a, b, a->getTransformation(), b->getTransformation(), pointOnA, pointOnB, distance);
    }
    
//    template<unsigned int N>
//    static math::FixedMatrix<float,N,N> covariance_matrix(const std::vector<math::FixedColVector<float, N> > &vs, math::FixedColVector<float, N> &mean)
//    {
//        uint size = vs.size();
//        mean = math::FixedColVector<float, N>();
//        for(uint i = 0; i < size; ++i)
//            mean += vs[i];
//        mean /= size;
//       
//        math::FixedMatrix<float, N, N> cov; //stack allocated
//        for(uint i = 0; i < N; ++i)
//            for(uint j = 0; j < N; ++j)
//        {
//            float tmp = 0;
//            for(uint n = 0; n < size; ++n)
//                tmp += (vs[n][i] - mean[i]) * (vs[n][j] - mean[j]);
//            tmp /= (size - 1);
//            cov(i, j) = tmp;
//        }
//       
//        return cov;
//    }
    

//  void createObjectsFromBlobs(const std::vector< std::vector<int> > &indices, const std::vector<Vec> &vertices, std::vector<PhysicsObject*> &objects)
//  {
//    //create python embedder
//    EmbedPython embedder;
//    //embedder = new EmbedPython();
//    char path[] = "./pyscripts/fit_adapter.py";
//    char func[] = "run";
//    embedder.init(path,func);
//    
//    for(unsigned int i = 0; i < indices.size() - 1; i++){
//      //fit super quadric with python
//      std::vector<float> sqParams;
//      sqParams = embedder.fitSQ(vertices, indices[i]);

//      float x1=0,x2=0,x3=0,x4=0,x5=0,x6=0,x7=0,x8=0,x9=0,x10=0,x11=0;

//      x1=sqParams.at(0);
//      x2=sqParams.at(1);
//      x3=sqParams.at(2);
//      x4=sqParams.at(3);
//      x5=sqParams.at(4);
//      x6=sqParams.at(5);
//      x7=sqParams.at(6);
//      x8=sqParams.at(7);
//      x9=sqParams.at(8);
//      x10=sqParams.at(9);
//      x11=sqParams.at(10);
//      
//      //create transform
//      float a1, a2, a3, e1, e2, rx, ry, rz, xx, yy, zz;
//      a1=x1, a2=x2, a3=x3, e1=x4, e2=x5, rx=x6, ry=x7, rz=x8, xx=x9, yy=x10,zz=x11;
//      FixedMatrix<float,4,4> tm  = FixedMatrix<float,4,4> ::id();
//      float a  = cos(rx);
//      float b  = sin(rx);
//      float c  = cos(ry);
//      float d  = sin(ry);
//      float e  = cos(rz);
//      float ff  = sin(rz);
//      float ad = a*d;
//      float bd = b*d;
//      tm.at(0,0)=c*e;
//      tm.at(0,1)=-bd*e+a*ff;
//      tm.at(0,2)=ad*e+b*ff;
//      tm.at(0,3)=0;
//      tm.at(1,0)=-c*ff;
//      tm.at(1,1)=bd*ff+a*e;
//      tm.at(1,2)=-ad*ff+b*e;
//      tm.at(1,3)=0;
//      tm.at(2,0)=-d;
//      tm.at(2,1)=-b*c;
//      tm.at(2,2)=a*c;
//      tm.at(2,3)=0;
//      tm.at(3,0)=xx;
//      tm.at(3,1)=yy;
//      tm.at(3,2)=zz;
//      tm.at(3,3)=1;
//      
//      
//      //estimate a fitting primitve based on the "roundness" of the super quadrics
//      PhysicsObject* obj;
//      if((e1 < 0.6) && (e2 < 0.6))
//      {
//        obj = (PhysicsObject*)new RigidBoxObject(0.0,0.0,0.0, a1 * 2.0, a2 * 2.0, a3 * 2.0, 10);
//      }else if((a1 > (a2 * 1.5)) || (a1 > (a3 * 1.5)) || (a2 > (a1 * 1.5)) || (a2 > (a3 * 1.5)) || (a3 > (a1 * 1.5)) || (a3 > (a2 * 1.5)))
//      {
//        obj = (PhysicsObject*)new RigidCylinderObject(0.0,0.0,0.0, max(a2, a3) * 2.0, a1 * 2.0, 20,10);
//        geom::Mat trans  = geom::Mat::id();
//        trans.at(0,0) = 0.0;
//        trans.at(0,1) = 1.0;
//        trans.at(0,2) = 0.0;
//        
//        trans.at(1,0) = 0.0;
//        trans.at(1,1) = 0.0;
//        trans.at(1,2) = 1.0;
//        
//        trans.at(2,0) = 1.0;
//        trans.at(2,1) = 0.0;
//        trans.at(2,2) = 0.0;
//        obj->transform(trans);
//        
//      }else
//      {
//        obj = (PhysicsObject*)new RigidSphereObject(0.0,0.0,0.0, max(max(a1,a2),a2), 20, 20, 10);
//      }
//      
//      obj->transform(tm);
//      objects.push_back(obj);
//    }
//    
//    //create the table
//    FixedColVector<float, 3> table_dim(1500.0,1500.0,10.0);
//    FixedColVector<float, 3> mean;
//    std::vector<FixedColVector<float, 3> > vs;
//    for(uint i=0; i < indices.back().size();i++)
//    {
//      const Vec &vertex = vertices[indices.back()[i]];
//      FixedColVector<float, 3> trim = FixedColVector<float, 3>(vertex[0],vertex[1],vertex[2]);
//      vs.push_back(trim);
//    }
//    FixedMatrix<float,3,3> cov = covariance_matrix<3>(vs,mean);
//    FixedMatrix<float,3,3> eigenvectors;
//    FixedMatrix<float,1,3> eigenvalues;
//    cov.eigen(eigenvectors, eigenvalues);
//    //normalized eigenvectors
//    for(unsigned int j=0; j < 3; j++){
//      float length = std::sqrt(eigenvectors(j,0) * eigenvectors(j,0) 
//                                + eigenvectors(j,1) * eigenvectors(j,1)
//                                + eigenvectors(j,2) * eigenvectors(j,2));
//      for(unsigned int k = 0; k < 3; k++){
//        eigenvectors(j,k) /= length;
//      }
//    }
//    //eigenvectors.setBounds(4,4,true,0.0);
//    geom::Mat trans = eigenvectors.resize<4,4>(0.f);
//    trans(3,0) = mean[0];
//    trans(3,1) = mean[1];
//    trans(3,2) = mean[2];
//    trans(3,3) = 1.0;
//    
//    PhysicsObject* obj = (PhysicsObject*)new RigidBoxObject(0.0,0.0,0.0, table_dim[0], table_dim[1], table_dim[2], 0.0);
//    //Mat trans(eigenvectors.begin(), eigenvectors.end());
//    obj->setTransformation(trans);
//    obj->setFriction(0.3);
//    obj->setColor(icl::Primitive::quad, GeomColor(255,0,0,127), true);
//    objects.push_back(obj);
//  }
  }
}

