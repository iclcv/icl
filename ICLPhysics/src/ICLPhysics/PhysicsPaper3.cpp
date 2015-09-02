/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsPaper3.cpp            **
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
#include <ICLPhysics/PhysicsPaper3.h>
#include <ICLQt/Quick.h>
#include <ICLPhysics/GeometricTools.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <ICLPhysics/PhysicsDefs.h>
#include <ICLUtils/ConsoleProgress.h>
#include <ICLMath/StraightLine2D.h>
#include <ICLGeom/ShaderUtil.h>

#ifdef ICL_SYSTEM_APPLE
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif

#include <ICLPhysics/FoldMap.h>

namespace icl{
  namespace physics{
    using namespace utils;
    using namespace math;
    using namespace core;
    using namespace qt;
    using namespace geom;

    bool line_segment_full_intersect(const Point32f &a, const Point32f &b,
                                     const Point32f &c, const Point32f &d){
      float s(-1),t(-1);
      bool intersect = line_segment_intersect(a,b,c,d,0,&s,&t);
      if(intersect){
        static const float dMin = 1.e-6, dMax = 1.-dMin;
        if(s<dMin || s>dMax || t<dMin || t>dMax) return false;
        return true;
      }
      return false;
    }

    struct PhysicsPaper3::Data{
      qt::GLImg tex[2];

      bool haveTexture;
      float initialStiffness;
      std::vector<utils::Point32f> texCoords;
      std::vector<utils::Point32f> projectedPoints;

      int numPointsBeforeUpdate;
      bool softBodyUpdatesEnabled;
      PhysicsWorld *physicsWorld;
      float faceAlpha;

      mutable int lastUsedFaceIdx;

      bool visLinks;
      bool visFaces;

      mutable Mutex mutex;

      FoldMap fm;
      utils::Function<void,const Img32f &> fmCallback;

      bool enableSelfCollision;

      float linkStiffness;


      bool straightenFolds;
      bool doubleFolds;

      std::vector<std::vector<btSoftBody::Face*> > smoothNormalGraph;
      std::vector<Vec> smoothNormals;
      bool useSmoothNormals;
      
      struct LinkColors{
        GeomColor original;
        GeomColor inserted;
        GeomColor creases;
        GeomColor bendingConstraints;
      } linkColors;

      Data():mutex(Mutex::mutexTypeRecursive),fm(Size(200,300),1.0f){
        visLinks = true;
        visFaces = true;
        linkStiffness = 1.e-5;

        faceAlpha = 1;
        straightenFolds = true;
        doubleFolds = true;
        useSmoothNormals = true;

        haveTexture = false;

        linkColors.original = geom_green(255)*(1./255);
        linkColors.inserted = GeomColor(255,0,255,255)*(1./255);
        linkColors.creases = GeomColor(255,255,0,255)*(1./255);
        linkColors.bendingConstraints = GeomColor(255,0,0,255)*(1./255);

        lastUsedFaceIdx = 0;
      }

      std::map<std::pair<int,int>,float> rlMap;

      void clearMemorizedRestDistnaces(){
        rlMap.clear();
      }
      void addMemorizedRestDistance(int a, int b, float rl){
        rlMap[std::make_pair(a,b)] = rl;
      }
      float getMemorizedRestDistance(int a, int b){
        std::map<std::pair<int,int>,float>::const_iterator it = rlMap.find(std::make_pair(a,b));
        if(it == rlMap.end()) return 0;
        else return it->second;
      }
    };

    template<class T> inline bool is_face() { return false; }
    template<> inline bool is_face<btSoftBody::Face>() { return true; }

    template<class T>
    void free_bullet_array_instance(T &){ }

    template<> void free_bullet_array_instance<btSoftBody::Link>(btSoftBody::Link &l){
      PhysicsPaper3::free_link_state(l.m_tag);
    }

    template<class T>
    void remove_indices_from_bullet_array(btAlignedObjectArray<T> &array, std::vector<int> &indices){
      std::sort(indices.begin(),indices.end());
      btAlignedObjectArray<T> tmp;
      tmp.resize(array.size()-indices.size());

      const int *is = indices.data(), *isEnd = is+indices.size();

      for(int si=0,di=0;di<tmp.size();++si){
        if(is < isEnd && si == *is){
          free_bullet_array_instance<T>(array[si]);
          ++is;
        }else{
          tmp[di++] = array[si];
        }
      }
      array = tmp;

    }

    PhysicsPaper3 *PhysicsPaper3::clone(PhysicsWorld *world) const{
      return new PhysicsPaper3(world, *this);
    }


    PhysicsPaper3::PhysicsPaper3(PhysicsWorld *world, const PhysicsPaper3 &other) : m_data(new Data){
      m_data->enableSelfCollision = other.m_data->enableSelfCollision;
      m_data->visLinks = other.m_data->visLinks;
      m_data->physicsWorld = world;
      m_data->initialStiffness = other.m_data->initialStiffness;
      m_data->texCoords = other.m_data->texCoords;
      m_data->projectedPoints = other.m_data->projectedPoints;
      
      setLockingEnabled(true);
      if(other.m_data->haveTexture){
        m_data->haveTexture = true;
        m_data->tex[0].update(other.m_data->tex[0].extractImage());
        m_data->tex[1].update(other.m_data->tex[1].extractImage());
      }
      const btSoftBody *sOrig = other.getSoftBody();
      std::vector<btVector3> ns(sOrig->m_nodes.size());
      for(size_t i=0;i<ns.size();++i){
        ns[i] = sOrig->m_nodes[i].m_x;
      }
      //      DEBUG_LOG("created copy of btSoftBody object with " << ns.size() << " nodes");
      btSoftBody *s = new btSoftBody(const_cast<btSoftBodyWorldInfo*>(world->getWorldInfo()),
                                     ns.size(), ns.data(), 0);

      s->m_cfg = sOrig->m_cfg;
      s->getCollisionShape()->setMargin(icl2bullet(2));

      s->appendMaterial();
      s->m_materials[0]->m_kLST = 1.0;
      s->m_materials[0]->m_kAST = 1.0;
      s->m_materials[0]->m_kVST = 1.0;
      
      
      s->setTotalMass(ns.size()*0.01,false);
      
      setPhysicalObject(s);      
      setCurrentPhysicsWorld(world);
      
      // copy triangles
      const btSoftBody::Node *n0 = (const btSoftBody::Node*)&sOrig->m_nodes[0];
      for(int i=0;i<sOrig->m_faces.size();++i){
        const btSoftBody::Face &f = sOrig->m_faces[i];
        addTriangle((int)(f.m_n[0]-n0),(int)(f.m_n[1]-n0),(int)(f.m_n[2]-n0));
        s->m_faces[i].m_normal = f.m_normal;
        s->m_faces[i].m_ra = f.m_ra;
      }

      // copy constraints
      for(int i=0;i<sOrig->m_links.size();++i){
        const btSoftBody::Link &l = sOrig->m_links[i];
        addLink((int)(l.m_n[0]-n0),(int)(l.m_n[1]-n0),
                l.m_material->m_kLST);
        s->m_links[i].m_tag = ((LinkState*)l.m_tag)->p();
        s->m_links[i].m_bbending = l.m_bbending;
        s->m_links[i].m_c0 = l.m_c0;
        s->m_links[i].m_c1 = l.m_c1;
        s->m_links[i].m_c2 = l.m_c2;
        s->m_links[i].m_c3 = l.m_c3;
      }

      m_data->fm = FoldMap(other.getFoldMap());
      
    }



    PhysicsPaper3::PhysicsPaper3(PhysicsWorld *world, bool enableSelfCollision, const Size &cellsInit, const Vec corners[4],
                                 const Img8u *front_texture, const Img8u *back_texture, float initialStiffness, float initialMaxLinkDistnace):
      m_data(new Data){


      const float rx = 210./2, ry = 297./2;
      const float init_z = 40;
      static const Vec def_corners[] = {
        Vec(-rx,-ry,init_z,1),  Vec(rx,-ry,init_z,1),
        Vec(-rx,ry,init_z,1),  Vec(rx,ry,init_z,1)
      };
      const Vec *cs = corners ? corners : def_corners;

      m_data->initialStiffness = initialStiffness;
      m_data->enableSelfCollision = enableSelfCollision;
      m_data->visLinks = false;
      m_data->physicsWorld = world;
      setLockingEnabled(true);

      if(front_texture || back_texture){
        m_data->haveTexture = true;
        m_data->tex[0].update(front_texture ? front_texture : back_texture);
        m_data->tex[1].update(back_texture ? back_texture : front_texture);
      }

      const int nx = cellsInit.width, ny = cellsInit.height;
      const float dx = 1./(nx-1), dy = 1./(ny-1);

      std::vector<btVector3> nodes1,nodes2,normals1,normals2;
      std::vector<Point32f> texCoords1,texCoords2;


      for(int y=0;y<ny;++y){
        for(int x=0;x<nx;++x){
          Point32f p(x*dx,y*dy);
          texCoords1.push_back(p);
          const Vec interpolated = bilinear_interpolate(cs,p.x,p.y);
          btVector3 bulletVec = icl2bullet_scaled(interpolated);
          nodes1.push_back(bulletVec);
          normals1.push_back(btVector3(0,0,1));
          if(x && y){
            Point32f q((x-.5)*dx, (y-.5)*dy);
            texCoords2.push_back(q);
            const Vec interpolated2 = bilinear_interpolate(cs,q.x,q.y);
            nodes2.push_back(icl2bullet_scaled(interpolated2));
            normals2.push_back(btVector3(0,0,1));
          }
        }
      }

      std::copy(nodes2.begin(),nodes2.end(),std::back_inserter(nodes1));
      std::copy(normals2.begin(),normals2.end(),std::back_inserter(normals1));

      m_data->texCoords.assign(texCoords1.begin(),texCoords1.end());
      std::copy(texCoords2.begin(),texCoords2.end(),std::back_inserter(m_data->texCoords));

   
      btSoftBody *s = new btSoftBody(const_cast<btSoftBodyWorldInfo*>(world->getWorldInfo()),
                                     nodes1.size(), nodes1.data(),0);
      s->setTotalMass(nodes1.size()*0.01,false);

      s->m_cfg.kLF = 0;  // lift coefficient
      s->m_cfg.kDG = 0;  // drag coefficient

      s->m_cfg.kMT = 0.7;
      s->m_cfg.kDP = 0.1; // damping
      s->m_cfg.kDF = 0.4;  // dynamic friction

      //s->m_cfg.kSRHR_CL    = 0.1;
      //s->m_cfg.kSKHR_CL    = 1;
      //s->m_cfg.kSSHR_CL    = 0.01; //0.5;
      //s->m_cfg.kSR_SPLT_CL = 0.5;
      //s->m_cfg.kSK_SPLT_CL = 0.5;
      //s->m_cfg.kSS_SPLT_CL = 0.01; //0.5;

      s->getCollisionShape()->setMargin(icl2bullet(2));

      s->appendMaterial();
      s->m_materials[0]->m_kLST = 1.0;
      s->m_materials[0]->m_kAST = 1.0;
      s->m_materials[0]->m_kVST = 1.0;

      if(enableSelfCollision){
        //s->m_cfg.collisions	= btSoftBody::fCollision::CL_SS+btSoftBody::fCollision::CL_RS + btSoftBody::fCollision::CL_SELF;
        s->m_cfg.collisions	= btSoftBody::fCollision::CL_SS+btSoftBody::fCollision::SDF_RS + btSoftBody::fCollision::CL_SELF;
      }else{
        //s->m_cfg.collisions += btSoftBody::fCollision::CL_SS;
        //s->m_cfg.collisions	= btSoftBody::fCollision::CL_RS + btSoftBody::fCollision::CL_SS;
      }

      setPhysicalObject(s);
      setCurrentPhysicsWorld(world);
      // center vertex/tex-coords offset
      int o = nx * ny;

  #define IDX(x,y) ((x)+nx*(y))
  #define IDXO(x,y) (o+(x)+(nx-1)*(y))

      for(int y=1;y<ny;++y){
        for(int x=1;x<nx;++x){
          int a = IDX(x-1,y-1), b=IDX(x,y-1), c = IDX(x-1,y), d = IDX(x,y), e = IDXO(x-1,y-1);
          addTriangle(e,a,b);
          addTriangle(e,b,d);
          addTriangle(e,d,c);
          addTriangle(e,c,a);

          LinkState orig;
          orig.isOriginal = true;
          addLink(a, b, 1, orig);
          addLink(a, c, 1, orig);
          addLink(a, e, 1, orig);
          addLink(b, e, 1, orig);
          addLink(c, e, 1, orig);
          addLink(d, e, 1, orig);
          if(x == (nx-1) || y == (ny-1)){
            addLink(b, d, 1, orig);
            addLink(c, d, 1, orig);
          }
        }
      }

      m_data->lastUsedFaceIdx = -1;

      createBendingConstraints(initialMaxLinkDistnace,initialStiffness);
    }
    
    void PhysicsPaper3::takeSoftBodyFrom(PhysicsPaper3 *other){
      lock();
      other->lock();
      
      btSoftBody *sCur = getSoftBody();
      m_data->physicsWorld->removeObject(this);
      
      btSoftBody *sNew = other->getSoftBody();
      sNew->m_cfg = sCur->m_cfg;
    
      setPhysicalObject(sNew);      
      other->forgetPhysicalObject(true);
      
      m_data->physicsWorld->addObject(this);
      sNew->setUserPointer(this);
      sNew->m_worldInfo = m_data->physicsWorld->getWorldInfo();
      
      m_data->fm = FoldMap(other->getFoldMap());
      m_data->texCoords = other->m_data->texCoords;
      other->unlock();
      unlock();
    }

    PhysicsPaper3::~PhysicsPaper3(){
      delete m_data;
    }

    const std::vector<Point32f> PhysicsPaper3::getTexCoords() const{
      return m_data->texCoords;
    }


    void PhysicsPaper3::lock(){
      m_data->mutex.lock();
    }

    void PhysicsPaper3::unlock(){
      m_data->mutex.unlock();
    }

    void PhysicsPaper3::updateSceneObject(btSoftBody *soft){
      (void) soft;
    }

    void PhysicsPaper3::setFaceAlpha(float alpha01){
      m_data->faceAlpha = alpha01;
    }

    float PhysicsPaper3::getFaceAlpha() const{
      return m_data->faceAlpha;
    }


    Vec PhysicsPaper3::interpolatePosition(const Point32f &pp) const{
      Point32f p = pp;
      if(p.x <= 0) p.x = 1.e-7f;
      if(p.y <= 0) p.y = 1.e-7f;
      if(p.x >= 1) p.x = 1.f - 1.e-7f;
      if(p.y >= 1) p.y = 1.f - 1.e-7f;

      // find-triangle that contains the point (could be sped up later)
      const btSoftBody *s = getSoftBody();

      int hit_idx = -1, ia(0),ib(0),ic(0);

      if(m_data->lastUsedFaceIdx >= 0){
        const btSoftBody::Face &f = s->m_faces[m_data->lastUsedFaceIdx];
        ia = (int)(f.m_n[0] - &s->m_nodes[0]);
        ib = (int)(f.m_n[1] - &s->m_nodes[0]);
        ic = (int)(f.m_n[2] - &s->m_nodes[0]);
        if(point_in_triangle(p,m_data->texCoords[ia],m_data->texCoords[ib],m_data->texCoords[ic])){
          hit_idx = m_data->lastUsedFaceIdx;
        }
      }

      if(hit_idx == -1){
        for(int i=0;i<s->m_faces.size();++i){
          const btSoftBody::Face &f = s->m_faces[i];
          ia = (int)(f.m_n[0] - &s->m_nodes[0]);
          ib = (int)(f.m_n[1] - &s->m_nodes[0]);
          ic = (int)(f.m_n[2] - &s->m_nodes[0]);
          if(point_in_triangle(p,m_data->texCoords[ia],m_data->texCoords[ib],m_data->texCoords[ic])){
            hit_idx = i;
            break;
          }
        }
      }
      if(hit_idx == -1) throw ICLException("PhysicsPaper3::interpolatePosition: no triangle on position " +str(pp)+" found");
      m_data->lastUsedFaceIdx = hit_idx;

      const btSoftBody::Face &f = s->m_faces[hit_idx];

      const Vec a = bullet2icl_scaled(f.m_n[0]->m_x);
      const Vec b = bullet2icl_scaled(f.m_n[1]->m_x);
      const Vec c = bullet2icl_scaled(f.m_n[2]->m_x);


      const Point32f &ta = m_data->texCoords[ia];
      const Point32f &tb = m_data->texCoords[ib];
      const Point32f &tc = m_data->texCoords[ic];

      /*
        a@x1y1        b@x2y2

              (pxpy)

             c@x3y3

          alpha (c-a)  +  beta (b-a) = p-a

          |cx-ax  bx-ax|       |px-ax|
          |cy-ay  by-ay| * x = |py-ay|

          A x = b

          x = a.solve(b,"inv"); // inv is faster and more stable for (cx-ax = 0)
      */

      FixedMatrix<float,2,2> M(tc.x - ta.x, tb.x - ta.x,
                              tc.y - ta.y, tb.y - ta.y);

      FixedMatrix<float,1,2> ff = M.inv()*FixedMatrix<float,1,2>( p.x - ta.x, p.y - ta.y);

      Vec res = a + (c - a)*ff[0] + (b-a)*ff[1];
      res[3] = 1;

      return res;
    }

    void PhysicsPaper3::setStraightenFolds(bool enabled){
      m_data->straightenFolds = enabled;
    }

    void PhysicsPaper3::setDoubleFolds(bool enabled){
      m_data->doubleFolds = enabled;
    }


    void PhysicsPaper3::setLinksVisible(bool visible){
      m_data->visLinks = visible;
    }

    void PhysicsPaper3::setFacesVisible(bool visible){
      m_data->visFaces = visible;
    }

    void PhysicsPaper3::createBendingConstraints(float maxDistance, float fixedStiffness){
      m_data->physicsWorld->lock();
      lock();

      /// remove all non-first-order-links
      btSoftBody *s = getSoftBody();
      btAlignedObjectArray<btSoftBody::Link> &ls = s->m_links;
      std::vector<int> rmLinks;
      m_data->clearMemorizedRestDistnaces();
      for(int i=0;i<ls.size();++i){
        if(!LinkState::is_first_order(ls[i].m_tag)){
          if(LinkState::has_memorized_rest_dist(ls[i].m_tag)){
            btSoftBody::Node *o = &s->m_nodes[0];
            m_data->addMemorizedRestDistance((int)(ls[i].m_n[0]-o),
                                             (int)(ls[i].m_n[1]-o),
                                             ls[i].m_rl);
          }
          rmLinks.push_back(i);
        }
      }
      remove_indices_from_bullet_array(ls,rmLinks);

      maxDistance *= maxDistance;


      std::vector<Point32f> &tex = m_data->texCoords;


      const int num = tex.size();
      for(int i=0;i<num;++i){
        for(int j=0;j<num;++j){
          if(i == j) continue;
          float d = sqr(tex[i].x-tex[j].x) + sqr(tex[i].y-tex[j].y);
          if(d < maxDistance){
            float stiffness = (fixedStiffness > 0) ? fixedStiffness : m_data->fm.getFoldValue(tex[i],tex[j]);
            addLink(i,j,fabs(stiffness),LinkState(false,false,stiffness<0));
          }
        }
      }
      unlock();


      updateNodeAreas();
      s->randomizeConstraints();

      //s->generateClusters(0);
      if(m_data->enableSelfCollision){
        updateCollisionClusters();
      }

      m_data->physicsWorld->unlock();
    }

    void PhysicsPaper3::updateCollisionClusters(){
      btSoftBody *s = getSoftBody();
  #if 1
      s->generateClusters(0);
  #else
      s->releaseClusters();
      std::vector<btSoftBody::Face*> clusterFaces; // todo: create
      // add faces that have no "fold"-edge
      std::set<btSoftBody::Node*> foldNodes;
      for(int i=0;i<s->m_links.size();++i){
        btSoftBody::Link &l = s->m_links[i];
        if(LinkState::is_fold(l.m_tag)){
          foldNodes.insert(l.m_n[0]);
          foldNodes.insert(l.m_n[1]);
        }
      }

      btSoftBody::Node *o = &s->m_nodes[0];

      for(int i=0;i<s->m_faces.size();++i){
        btSoftBody::Face &f = s->m_faces[i];
        if(!foldNodes.count(f.m_n[0]) &&
           !foldNodes.count(f.m_n[1]) &&
           !foldNodes.count(f.m_n[2]) ){

          int ia = (int)(f.m_n[0]-o);
          int ib = (int)(f.m_n[1]-o);
          int ic = (int)(f.m_n[2]-o);

          const Point32f A = m_data->texCoords[ia].transform(210,294);
          const Point32f B = m_data->texCoords[ib].transform(210,294);
          const Point32f C = m_data->texCoords[ic].transform(210,294);

          float area = 0.5 * FixedMatrix<float,3,3>(A.x, B.x, C.x,
                                                    A.y, B.y, C.y,
                                                    1,   1,   1).det();
          // This does not help, all collision faces seem to be of size 1029 mm^2
          if(area > 200){
            clusterFaces.push_back(&f);
          }
          // TODO: also filter out too small faces! (but how?)
        }
      }

      int n = (int)clusterFaces.size();
      //s->releaseClusters();

      btAlignedObjectArray<btSoftBody::Cluster*> &cl = s->m_clusters;

      cl.resize(n);
      for(int i=0;i<n;++i){
        cl[i] = new(btAlignedAlloc(sizeof(btSoftBody::Cluster),16)) btSoftBody::Cluster();
        cl[i]->m_collide = true;
        for(int j=0;j<3;++j){
          cl[i]->m_nodes.push_back(clusterFaces[i]->m_n[j]);
        }
      }

      s->initializeClusters();
      s->updateClusters();

      //for self-collision
      s->m_clusterConnectivity.resize(n*n);
      for(int c0=0;c0<n;c0++){
        cl[c0]->m_clusterIndex=c0;
        for(int c1=0;c1<n;c1++){
          bool connected=false;
          btSoftBody::Cluster* cla = cl[c0];
          btSoftBody::Cluster* clb = cl[c1];
          for(int i=0;!connected&&i<cla->m_nodes.size();i++){
            for(int j=0;j<clb->m_nodes.size();j++){
              if(cla->m_nodes[i] == clb->m_nodes[j]){
                connected=true;
                break;
              }
            }
          }
          s->m_clusterConnectivity[c0+c1*n]=connected;
        }
      }
  #endif
    }

    void PhysicsPaper3::movePosition(const Point32f &coords, const Vec &target, float streangth, float radius,
                                     std::vector<NodeMovement> *dst){
      m_data->physicsWorld->lock();
      Vec pW =  interpolatePosition(coords);
      btVector3 offset = icl2bullet_scaled(target - pW);

      // find all nodes within radius:
      std::vector<int> closeIndices;
      std::vector<float> distances;

      float sumDist = 0;
      btSoftBody *s = getSoftBody();
      if(dst) dst->clear();

      // TODO_LOG("add gaussian based movement strategy .. maybe this could have been one of the issues with our kinect based tracking ?");

      for(int i=0;i<s->m_nodes.size();++i){
        float d = coords.distanceTo(m_data->texCoords[i]);
        if(d < 3*radius){
          closeIndices.push_back(i);
          distances.push_back(d);
          sumDist += d;
        }
      }

      if(closeIndices.size() == 1){
        int idx = closeIndices[0];
        btSoftBody::Node &n = s->m_nodes[idx];
        n.m_v = offset * streangth;
      }else if(closeIndices.size()){
        float N = 1.0/(::sqrt(2*M_PI)*radius);
        for(size_t i=0;i<closeIndices.size();++i){
          int idx = closeIndices[i];
          btSoftBody::Node &n = s->m_nodes[idx];
          float d = distances[i];

          //        float alpha = (((sumDist-d)/sumDist) * streangth);
          float alpha = N*::exp(-d*d/(radius*radius)) * streangth;
          n.m_v = offset * alpha;
          if(dst){
            NodeMovement m = { bullet2icl_scaled( n.m_x ),
                               bullet2icl_scaled( n.m_x + n.m_v),
                               alpha };
            dst->push_back(m);
          }
        }
      }
      m_data->physicsWorld->unlock();
    }

    void PhysicsPaper3::lockWorld(){
      m_data->physicsWorld->lock();
    }

    void PhysicsPaper3::unlockWorld(){
      m_data->physicsWorld->unlock();
    }

    void PhysicsPaper3::setLinkColors(const geom::GeomColor &originalLinks, 
                                      const geom::GeomColor &insertedLinks,
                                      const geom::GeomColor &creaseLines,
                                      const geom::GeomColor &bendingConstraints){
      m_data->linkColors.original = originalLinks*(1./255);
      m_data->linkColors.inserted = insertedLinks*(1./255);
      m_data->linkColors.creases = creaseLines*(1./255);
      m_data->linkColors.bendingConstraints = bendingConstraints*(1./255);
    }


    void PhysicsPaper3::addTriangle(int a, int b, int c){
      btSoftBody *s = getSoftBody();
      s->appendFace(a,b,c);

      btSoftBody::Face &f = s->m_faces[s->m_faces.size()-1];

      Point32f t0 = m_data->texCoords[a].transform(210,297);
      Point32f t1 = m_data->texCoords[b].transform(210,297);
      Point32f t2 = m_data->texCoords[b].transform(210,297);

      float la = t0.distanceTo(t1);
      float lb = t1.distanceTo(t2);
      float lc = t2.distanceTo(t0);

      float s_tmp = 0.5*(la+lb+lc);

      /// face rest area
      f.m_ra = ::sqrt( s_tmp * (s_tmp-la) * (s_tmp-lb) * (s_tmp-lc) );
    }

    void PhysicsPaper3::updateNodeAreas(){
      btSoftBody *s = getSoftBody();
      std::vector<int> counts(s->m_nodes.size(),0);
      for(size_t i=0;i<counts.size();++i){
        s->m_nodes[i].m_area = 0;
      }
      for(int i=0;i<s->m_faces.size();++i){
        btSoftBody::Face&	f=s->m_faces[i];
        for(int j=0;j<3;++j){
          const int index=(int)(f.m_n[j]-&s->m_nodes[0]);
          counts[index]++;
          f.m_n[j]->m_area+=fabs(f.m_ra);
        }
      }
      for(size_t i=0;i<counts.size();++i){
        s->m_nodes[i].m_area /= (counts[i] ? counts[i] : 1);
      }

      s->m_fdbvt.clear();
      if(s->m_cfg.collisions&btSoftBody::fCollision::VF_SS){
        s->initializeFaceTree();
      }

      s->m_bUpdateRtCst = false;
    }

    const Img32f &PhysicsPaper3::getFoldMap() const{
      return m_data->fm.getImage();
    }

    void PhysicsPaper3::setFoldMapChangedCallback(utils::Function<void,const Img32f &> cb){
      m_data->fmCallback = cb;
    }



    void PhysicsPaper3::addLink(int a, int b, float stiffness, const PhysicsPaper3::LinkState &state){
      if(a == b) return;
      if(a > b) std::swap(a,b);
      btSoftBody *s = getSoftBody();
      int n = s->m_links.size();
      s->appendLink(a,b,0,true);
      if(s->m_links.size() > n){
        btSoftBody::Link &l = s->m_links[s->m_links.size()-1];
        if(state.isFirstOrder){
          l.m_bbending = false;
        }else{
          l.m_bbending = true;
        }
        const Point32f &ta = m_data->texCoords[a];
        const Point32f &tb = m_data->texCoords[b];

        if(false || state.hasMemorizedRestDist){
          float rd = m_data->getMemorizedRestDistance(a,b);
          if(rd != 0){
            l.m_rl = rd;
          }else{
            btVector3 d = s->m_nodes[a].m_x - s->m_nodes[b].m_x;
            l.m_rl = ::sqrt( utils::sqr(d[0]) + utils::sqr(d[1]) + utils::sqr(d[2]) );
          }
        }else{
          l.m_rl = icl2bullet( ::sqrt (::sqr((ta.x-tb.x)*210) + ::sqr((ta.y-tb.y)*297) )  );
        }
        l.m_c1 = l.m_rl*l.m_rl;

        btSoftBody::Material *m = s->appendMaterial();
        m->m_kLST = stiffness;
        l.m_material = m;
        l.m_c0 = (l.m_n[0]->m_im+l.m_n[1]->m_im)/m->m_kLST;
        l.m_tag = state.p();
        if(state.isFold){
          m_data->fm.addFold(ta,tb,stiffness);
        }
      }else{
        // update fold state:
        if(state.isFold){
          for(int i=0;i<s->m_links.size();++i){
            btSoftBody::Link &l = s->m_links[i];
            l.m_material->m_kLST = stiffness;
            const btSoftBody::Node *na  = &s->m_nodes[a];
            const btSoftBody::Node *nb  = &s->m_nodes[b];
            if( ((l.m_n[0] == na) && (l.m_n[1] == nb)) ||
                ((l.m_n[0] == nb) && (l.m_n[1] == na)) ){
              if(!LinkState::is_fold(l.m_tag)){
                delete (LinkState*)(l.m_tag);
                l.m_tag = state.p();
                const Point32f &ta = m_data->texCoords[a];
                const Point32f &tb = m_data->texCoords[b];
                m_data->fm.addFold(ta,tb,stiffness);
                break;
              }
            }
          }
        }
      }
      s->m_bUpdateRtCst = false;
    }

    namespace{
      struct cmp_point_32f{
        const Point32f &a;
        cmp_point_32f(const Point32f &a):a(a){}
        bool operator()(const Point32f &b) const {
          return (sqr(a.x-b.x) + sqr(a.y-b.y)) < 0.000001;
        }
      };
    }

    void PhysicsPaper3::addVertexOrReuseOldOne(Point32f &t, btVector3 &v, int &idx){
      std::vector<Point32f>::iterator it;
      it = std::find_if(m_data->texCoords.begin()+m_data->numPointsBeforeUpdate, m_data->texCoords.end(), cmp_point_32f(t));
      btSoftBody *s = getSoftBody();
      if(it != m_data->texCoords.end()){
        t = *it;
        idx = (int)(it - m_data->texCoords.begin());
        v = s->m_nodes[idx].m_x;

      }else{
        idx = m_data->texCoords.size();
        m_data->texCoords.push_back(t);
        s->appendNode(v,0.01);
      }
    }

    bool PhysicsPaper3::hitLink(btSoftBody::Link *l, const Point32f &a, const Point32f &b){
      btSoftBody *s = getSoftBody();
      int idx_la = (int)(l->m_n[0] - &s->m_nodes[0]);
      int idx_lb = (int)(l->m_n[1] - &s->m_nodes[0]);

      return line_segment_intersect(a,b,m_data->projectedPoints[idx_la],m_data->projectedPoints[idx_lb]);
    }


    bool PhysicsPaper3::hitTriangle(btSoftBody::Face *f, const Point32f &a, const Point32f &b){
      btSoftBody *s = getSoftBody();
      int idx_la = (int)(f->m_n[0] - &s->m_nodes[0]);
      int idx_lb = (int)(f->m_n[1] - &s->m_nodes[0]);
      int idx_lc = (int)(f->m_n[2] - &s->m_nodes[0]);

      Point32f ta = m_data->projectedPoints[idx_la];
      Point32f tb = m_data->projectedPoints[idx_lb];
      Point32f tc = m_data->projectedPoints[idx_lc];

      return ( line_segment_intersect(a,b,ta,tb) ||
               line_segment_intersect(a,b,tb,tc) ||
               line_segment_intersect(a,b,tc,ta) );
    }

    template<class T>
    inline void shift_back_3(T &a, T &b, T &c){
      T tmp = a;
      a = b;
      b = c;
      c = tmp;
    }

    bool PhysicsPaper3::replaceTriangle(btSoftBody::Face *f, const Point32f &lineA, const Point32f &lineB){
      btSoftBody *s = getSoftBody();
      int idx_a = (int)(f->m_n[0] - &s->m_nodes[0]);
      int idx_b = (int)(f->m_n[1] - &s->m_nodes[0]);
      int idx_c = (int)(f->m_n[2] - &s->m_nodes[0]);

      btVector3 a = s->m_nodes[idx_a].m_x;
      btVector3 b = s->m_nodes[idx_b].m_x;
      btVector3 c = s->m_nodes[idx_c].m_x;

      Point32f ta = m_data->projectedPoints[idx_a];
      Point32f tb = m_data->projectedPoints[idx_b];
      Point32f tc = m_data->projectedPoints[idx_c];

      float rA=0,rB=0,rC=0;
      bool iA = line_segment_intersect(lineA,lineB,ta,tb,0,0,&rA);
      bool iB = line_segment_intersect(lineA,lineB,tb,tc,0,0,&rB);
      bool iC = line_segment_intersect(lineA,lineB,tc,ta,0,0,&rC);

      rA = 1.f-rA;
      rB = 1.f-rB;
      rC = 1.f-rC;

      if( (!!iA + !!iB + !! iC) != 2){
        // do nothing
        addLink(idx_a,idx_b);
        addLink(idx_b,idx_c);
        addLink(idx_c,idx_a);
        return false;
      }

      if(iA && iB){
        //DEBUG_LOG("no shift");
        // keep it!
      }else if(iB && iC){
        //DEBUG_LOG("shifting once");
        shift_back_3(idx_a,idx_b,idx_c);
        shift_back_3(ta,tb,tc);
        shift_back_3(a,b,c);
        shift_back_3(rA,rB,rC);
        shift_back_3(iA,iB,iC);
      }else{
        //DEBUG_LOG("shifting twice");
        for(int i=0;i<2;++i){
          shift_back_3(idx_a,idx_b,idx_c);
          shift_back_3(ta,tb,tc);
          shift_back_3(a,b,c);
          shift_back_3(rA,rB,rC);
          shift_back_3(iA,iB,iC);
        }
      }
      /** Configuration:
            A
      a    tta     b
       +----|-----+
       |    |    /
       |    |   /
       |    |  /
       |    | /
     C |    |/  B
       |    |ttb
       |   /|
       |  / |
       | /  |
       |/
       +
      c

      */

      Point32f tta = linear_interpolate(m_data->texCoords[idx_a],m_data->texCoords[idx_b],rA);
      Point32f ttb = linear_interpolate(m_data->texCoords[idx_b],m_data->texCoords[idx_c],rB);

      const float MIN_VEC_DIST  = m_data-> enableSelfCollision ? 0.002 : 0.002;

      bool too_close_tta_ta = m_data->texCoords[idx_a].distanceTo(tta) < MIN_VEC_DIST;
      bool too_close_tta_tb = m_data->texCoords[idx_b].distanceTo(tta) < MIN_VEC_DIST;
      bool too_close_tta_tc = m_data->texCoords[idx_c].distanceTo(tta) < MIN_VEC_DIST;
      bool too_close_ttb_ta = m_data->texCoords[idx_a].distanceTo(ttb) < MIN_VEC_DIST;
      bool too_close_ttb_tb = m_data->texCoords[idx_b].distanceTo(ttb) < MIN_VEC_DIST;
      bool too_close_ttb_tc = m_data->texCoords[idx_c].distanceTo(ttb) < MIN_VEC_DIST;

      bool split = true;

      Point *newSoftLink = 0;

      if(too_close_tta_ta){
        if(too_close_ttb_ta || too_close_ttb_tb  || too_close_ttb_tc){
          split = false;
          if(too_close_ttb_tb){
            newSoftLink  = new Point(idx_a,idx_b);
          }else if(too_close_ttb_tc){
            newSoftLink  = new Point(idx_a,idx_c);
          }
        }else{
          int idx_new = -1;
          btVector3 vNew = linear_interpolate(b,c,rB);
          addVertexOrReuseOldOne(ttb,vNew,idx_new);
          addLink(idx_a,idx_b);
          addLink(idx_b,idx_new);
          addLink(idx_new,idx_a,m_data->linkStiffness,LinkState(true,true));
          addLink(idx_new,idx_c);
          addLink(idx_c,idx_a);

          addTriangle(idx_a,idx_b,idx_new);
          addTriangle(idx_a,idx_new,idx_c);
        }
      }else if(too_close_ttb_tc){
        if(too_close_tta_ta || too_close_tta_tb  || too_close_tta_tc){
          split = false;
          if(too_close_ttb_ta){
            newSoftLink  = new Point(idx_a,idx_c);
          }else if(too_close_ttb_tb){
            newSoftLink  = new Point(idx_b,idx_c);
          }
        }else{
          int idx_new = -1;
          btVector3 vNew = linear_interpolate(a,b,rA);
          addVertexOrReuseOldOne(tta,vNew,idx_new);
          addLink(idx_a,idx_new);
          addLink(idx_new,idx_c,m_data->linkStiffness, LinkState(true,true));
          addLink(idx_c,idx_a);
          addLink(idx_new,idx_b);
          addLink(idx_b,idx_c);

          addTriangle(idx_a,idx_new,idx_c);
          addTriangle(idx_new,idx_b,idx_c);
        }
      }else if(!too_close_tta_ta && !too_close_tta_tb && !too_close_tta_tc &&
               !too_close_ttb_ta && !too_close_ttb_tb && !too_close_ttb_tc){
        int idx_new_a = -1;
        int idx_new_b = -1;
        btVector3 vNew_a = linear_interpolate(a,b,rA);
        btVector3 vNew_b = linear_interpolate(b,c,rB);
        addVertexOrReuseOldOne(tta,vNew_a,idx_new_a);
        addVertexOrReuseOldOne(ttb,vNew_b,idx_new_b);

        addLink(idx_new_a,idx_b);
        addLink(idx_b,idx_new_b);
        addLink(idx_new_b,idx_new_a,m_data->linkStiffness, LinkState(true,true));

        addLink(idx_a,idx_new_a);
        addLink(idx_new_b,idx_a);
        addLink(idx_new_b,idx_c);
        addLink(idx_c,idx_a);

        addTriangle(idx_new_a,idx_b,idx_new_b);
        addTriangle(idx_a,idx_new_a,idx_new_b);
        addTriangle(idx_a,idx_new_b,idx_c);
      }else{
        split=false;
      }

      if(!split){
        int k = newSoftLink ? newSoftLink->x : -1;
        int l = newSoftLink ? newSoftLink->y : -1;
        if(newSoftLink) delete newSoftLink;
        if(k > l) std::swap(k,l);

        bool b_a = (iclMin(idx_a,idx_b)==k && iclMax(idx_a,idx_b)==l);
        bool b_b = (iclMin(idx_b,idx_c)==k && iclMax(idx_b,idx_c)==l);
        bool b_c = (iclMin(idx_c,idx_a)==k && iclMax(idx_c,idx_a)==l);

        addLink(idx_a,idx_b, b_a ? m_data->linkStiffness : 1, b_a ? LinkState(true,true) : LinkState());
        addLink(idx_b,idx_c, b_b ? m_data->linkStiffness : 1, b_b ? LinkState(true,true) : LinkState());
        addLink(idx_c,idx_a, b_c ? m_data->linkStiffness : 1, b_c ? LinkState(true,true) : LinkState());

        return false;
      }else{
        return true;
      }
    }

    void PhysicsPaper3::splitAlongLine(const Point32f &a, const Point32f &b, const Camera &currCam){
      lock();
      m_data->physicsWorld->lock();

      std::vector<int> delLinks,delTriangles;
      btSoftBody *s = getSoftBody();
      std::vector<Vec> vertices(s->m_nodes.size());
      for(size_t i=0;i<vertices.size();++i){
        vertices[i] = bullet2icl_scaled(s->m_nodes[i].m_x);
      }

      m_data->projectedPoints = currCam.project(vertices);
      //Img32f tmp(currCam.getResolution(),1);
      //      SHOW(currCam);
      //color(255,255,255);
      //pix(tmp,std::vector<Point>(m_data->projectedPoints.begin(), m_data->projectedPoints.end()));
      //qt::show(tmp);


      m_data->numPointsBeforeUpdate = vertices.size();

      for(int i=0;i<s->m_links.size();++i){
        if(hitLink(&s->m_links[i],a,b)) delLinks.push_back(i);
      }
      remove_indices_from_bullet_array(s->m_links, delLinks);

      int n_faces = s->m_faces.size();
      for(int i=0;i<n_faces;++i){
        if(hitTriangle(&s->m_faces[i],a,b)){
          if(replaceTriangle(&s->m_faces[i],a,b)) delTriangles.push_back(i);
        }
      }

      // delete old triangles and links ...

      remove_indices_from_bullet_array(s->m_faces, delTriangles);

      m_data->lastUsedFaceIdx = -1;

      createBendingConstraints(m_data->initialStiffness);

      if(m_data->fmCallback){
        m_data->fmCallback(m_data->fm.getImage());
      }

      m_data->physicsWorld->unlock();

      unlock();
    }

    void PhysicsPaper3::splitAlongLineInPaperCoords(const utils::Point32f &aIn, const utils::Point32f &bIn, bool extendLineToEdges){
      Point32f a = aIn, b = bIn;
      Point32f e = (b-a)*100; // elongate each side by 100 % to avoid 
      b += e;
      a -= e;
      std::vector<Point32f> wheres;
      if(extendLineToEdges){
        Point32f where;
        utils::Point32f edges[5] = {Point32f(0,0), Point32f(1,0), Point32f(1,1), Point32f(0,1), Point32f(0,0) };
        for(int i=0;i<4;++i){
          if(line_segment_intersect(a,b,edges[i],edges[i+1], &where)){
            //            DEBUG_LOG("a:" << a << " b:" << b << "  intersected at " << where);
            wheres.push_back(where);
          }
          if(wheres.size() >= 2) break;
        }
        if(wheres.size() != 2){
          ERROR_LOG("the given line aIn - bIn should definitely intersect the unitiy"
                    " rect of the paper frame twice ??");
        }else{
          a = wheres[0];
          b = wheres[1];
          Point32f e = (b-a); // elongate each side by 100 % to avoid 
          b += e;
          a -= e;
        }
      }
      
      lock();
      m_data->physicsWorld->lock();

      std::vector<int> delLinks,delTriangles;
      btSoftBody *s = getSoftBody();

      int nNodes = s->m_nodes.size();
      m_data->projectedPoints = m_data->texCoords;

      m_data->numPointsBeforeUpdate = nNodes;

      for(int i=0;i<s->m_links.size();++i){
        if(hitLink(&s->m_links[i],a,b)) delLinks.push_back(i);
      }
      remove_indices_from_bullet_array(s->m_links, delLinks);

      int n_faces = s->m_faces.size();
      for(int i=0;i<n_faces;++i){
        if(hitTriangle(&s->m_faces[i],a,b)){
          if(replaceTriangle(&s->m_faces[i],a,b)) delTriangles.push_back(i);
        }
      }

      // delete old triangles and links ...

      remove_indices_from_bullet_array(s->m_faces, delTriangles);

      m_data->lastUsedFaceIdx = -1;

      createBendingConstraints(m_data->initialStiffness);

      if(m_data->fmCallback){
        m_data->fmCallback(m_data->fm.getImage());
      }

      m_data->physicsWorld->unlock();

      unlock();
    
    }


    void PhysicsPaper3::updateSmoothNormalGraph(){
      btSoftBody *s = getSoftBody();

      m_data->smoothNormalGraph.resize(s->m_nodes.size());
      for(int i=0;i<s->m_nodes.size();++i){
        m_data->smoothNormalGraph[i].clear();
      }
      btSoftBody::Node *o = &s->m_nodes[0];
      for(int i=0;i<s->m_faces.size();++i){
        btSoftBody::Face *f = &s->m_faces[i];
        for(int j=0;j<3;++j){
          m_data->smoothNormalGraph[(int)(f->m_n[j]-o)].push_back(f);
        }
      }
    }

      static inline Vec compute_normal(const Vec &va, const Vec &vb, const Vec &vc, bool *ok=0){
        Vec ac = va - vc;
        Vec bc = vb - vc;

        const float lac = ::sqrt (sqr(ac[0]) + sqr(ac[1]) +  sqr(ac[2]) );
        const float lbc = ::sqrt (sqr(bc[0]) + sqr(bc[1]) +  sqr(bc[2]) );

        if(lac == 0 || lbc == 0){
          if(ok) *ok = false;
          return Vec(0,0,0,0);
        }

        const float ilac = 1.0f/lac;
        const float ilbc = 1.0f/lbc;

        for(int i=0;i<3;++i){
          ac[i] *= ilac;
          bc[i] *= ilbc;
        }

        return  cross(ac,bc);
      }


    static Vec get_face_normal(const btSoftBody::Face &f){
      return compute_normal(bullet2icl_unscaled(f.m_n[0]->m_x),
                            bullet2icl_unscaled(f.m_n[1]->m_x),
                            bullet2icl_unscaled(f.m_n[2]->m_x));
    }

    void PhysicsPaper3::computeSmoothNormals(){
      btSoftBody *s = getSoftBody();
      m_data->smoothNormals.resize(s->m_nodes.size());
      for(int i=0;i<s->m_nodes.size();++i){
        const std::vector<btSoftBody::Face*> &fs = m_data->smoothNormalGraph[i];
        std::vector<Vec> ns(fs.size());
        bool isFlat = true;
        for(size_t j=0;j<fs.size();++j){
          Vec &a = ns[j];
          a = get_face_normal(*fs[j]);
          float l = ::sqrt( sqr(a[0]) + sqr(a[1]) + sqr(a[2]) );
          if(l){
            float il = 1.0/l;
            a[0] *= il;
            a[1] *= il;
            a[2] *= il;
          }
          for(size_t k=0;k<j;++k){
            if(sprod3(a,ns[k]) < 0.6){
              isFlat = false;
            }
          }
        }
        if(isFlat){
          Vec aS = ns[0];
          for(size_t j=1;j<ns.size();++j){
            aS += ns[j];
          }
          m_data->smoothNormals[i] = normalize3(aS);
        }else{
          m_data->smoothNormals[i] = Vec(0,0,0,0);
        }
      }
    }

    namespace {

      struct RenderedTriangleImpl{
        Vec nodes[3];
        Vec normals[3];
        Point32f texCoords[3];
        float distToCam;
      };
      
      struct RenderedTriangle{
        utils::SmartPtr<RenderedTriangleImpl> impl;
        RenderedTriangle():
          impl(new RenderedTriangleImpl){}

        bool operator<(const RenderedTriangle &other) const {
          return impl->distToCam < other.impl->distToCam;
        }
      };
    }

    void PhysicsPaper3::complexCustomRender(icl::geom::ShaderUtil* util){
      m_data->physicsWorld->lock();
      util->activateShader(Primitive::triangle, true);


      if(m_data->useSmoothNormals){
        updateSmoothNormalGraph(); // todo: if too slow: only do this if something is changed
        computeSmoothNormals();
      }
      
      btSoftBody *s = getSoftBody();

      btSoftBody::Node *o = &s->m_nodes[0];

      if(m_data->visFaces){
        Vec cc(0,0,0,1);
        try{ 
          cc = util->getCurrentCamera().getPosition(); 
        } catch(...){
          // this is the shadow case .. which simply uses un-sorted rendering!
        }
          
        std::vector<RenderedTriangle> ts(s->m_faces.size());
        for(size_t i=0;i<ts.size();++i){
          ts[i] = RenderedTriangle();
        }
        for(int i=0;i<s->m_faces.size();++i){
          btSoftBody::Face &f = s->m_faces[i];
          RenderedTriangleImpl &t = *ts[i].impl;
          
          bool isFlat = true;
          for(int j=0;j<3;++j){
            t.nodes[j] = bullet2icl_scaled(f.m_n[j]->m_x);
            if(m_data->useSmoothNormals){
              t.normals[j] = m_data->smoothNormals[(int)(f.m_n[j]-o)];
              if(!t.normals[j][3]) isFlat = false;
            }else{
              t.normals[j] = bullet2icl_unscaled(f.m_n[j]->m_n);
            }
            t.texCoords[j] = m_data->texCoords[(int)(f.m_n[j] - &s->m_nodes[0])];
          }

          if(!isFlat){
            int good = -1;
            for(int j=0;j<3;++j){
              if(t.normals[j][3]) good = j; break;
            }
            if(good == -1) {
              t.normals[0] = t.normals[1] = t.normals[2] = compute_normal(t.nodes[0], t.nodes[1], t.nodes[2]);
            }else{
              for(int j=0;j<3;++j){
                if(!t.normals[j][3]) t.normals[j] = t.normals[good];
              }
            }
          }
          
          Vec p = (t.nodes[0] + t.nodes[1] + t.nodes[2]) * (1./3);
          t.distToCam = sqr(p[0]-cc[0]) + sqr(p[1]-cc[1]) + sqr(p[2]-cc[2]);
        }
        
        std::sort(ts.begin(), ts.end());
        
        glEnable(GL_CULL_FACE);
        for(int i=0;i<s->m_faces.size();++i){
          /*
          btSoftBody::Face &f = s->m_faces[i];
          Vec c[3],n[3];
          Point32f t[3];

          bool isFlat = true;
          for(int j=0;j<3;++j){
            c[j] = bullet2icl_scaled(f.m_n[j]->m_x);
            if(m_data->useSmoothNormals){
              n[j] = m_data->smoothNormals[(int)(f.m_n[j]-o)];
              if(!n[j][3]) isFlat = false;
            }else{
              n[j] = bullet2icl_unscaled(f.m_n[j]->m_n);
            }
            t[j] = m_data->texCoords[(int)(f.m_n[j] - &s->m_nodes[0])];
          }

          if(!isFlat){
            int good = -1;
            for(int j=0;j<3;++j){
              if(n[j][3]) good = j; break;
            }
            if(good == -1) {
              n[2] = n[1] = n[0] = compute_normal(c[0], c[1], c[2]);
            }else{
              for(int j=0;j<3;++j){
                if(!n[j][3]) n[j] = n[good];
              }
            }
          }
              */
          RenderedTriangleImpl &ti = *ts[i].impl;
          const Vec *c = ti.nodes;
          const Vec *n = ti.normals;
          const Point32f *t = ti.texCoords;

          glFrontFace(GL_FRONT);
          glCullFace(GL_FRONT);

          if(m_data->haveTexture){
              m_data->tex[0].draw3DGeneric(3, &c[0][0], &c[0][1], &c[0][2], 4, t,&n[0][0],&n[0][1],&n[0][2],4);
          }else{
            glColor4f(0.9,.2,.6,getFaceAlpha());
            glBegin(GL_TRIANGLES);
            for(int j=0;j<3;++j){
              glNormal3f(n[j][0],n[j][1],n[j][2]);
              glVertex3fv(&c[j][0]);
            }
            glEnd();
          }

          glFrontFace(GL_BACK);
          glCullFace(GL_BACK);

          if(m_data->haveTexture){
            m_data->tex[1].draw3DGeneric(3, &c[0][0], &c[0][1], &c[0][2], 4, t,&n[0][0],&n[0][1],&n[0][2],4);
          }else{
            glColor4f(0.1,0.6,1,getFaceAlpha());
            glBegin(GL_TRIANGLES);
            for(int j=0;j<3;++j){
              glNormal3f(n[j][0],n[j][1],n[j][2]);
              glVertex3fv(&c[j][0]);
            }
            glEnd();
          }
        }
        glDisable(GL_CULL_FACE);
      }

      glLineWidth(3);

      //glDisable(GL_DEPTH_TEST);
      if(isVisible(Primitive::line)){
        util->deactivateShaders();
        glDisable(GL_LIGHTING);
        glBegin(GL_LINES);
        for(int i=0;i<s->m_links.size();++i){
          btSoftBody::Link &l = s->m_links[i];
          if(LinkState::is_first_order(l.m_tag) || m_data->visLinks){
            if(LinkState::is_first_order(l.m_tag)){
              if(LinkState::is_fold(l.m_tag)){
                glColor4fv(&m_data->linkColors.creases[0]);
              }else if(LinkState::is_original(l.m_tag)){
                glColor4fv(&m_data->linkColors.original[0]);
              }else{
                glColor4fv(&m_data->linkColors.inserted[0]);
              }
            }else{
              // xxxx
              if(LinkState::is_first_order(l.m_tag)){
                glColor4fv(&m_data->linkColors.original[0]);
              }else{
                float stiffness = l.m_material->m_kLST;
                if(stiffness < 0.001){
                  continue; // dont render link at all
                }else if(stiffness < 0.1){
                  glColor4f(m_data->linkColors.bendingConstraints[0],
                            m_data->linkColors.bendingConstraints[1],
                            m_data->linkColors.bendingConstraints[2], 0.5);
                }else{
                  glColor4f(m_data->linkColors.bendingConstraints[0],
                            m_data->linkColors.bendingConstraints[1],
                            m_data->linkColors.bendingConstraints[2], 0.8);
                }
              }
            }

            glVertex3f(bullet2icl(l.m_n[0]->m_x[0]),
                       bullet2icl(l.m_n[0]->m_x[1]),
                       bullet2icl(l.m_n[0]->m_x[2]) );

            glVertex3f(bullet2icl(l.m_n[1]->m_x[0]),
                       bullet2icl(l.m_n[1]->m_x[1]),
                       bullet2icl(l.m_n[1]->m_x[2]) );
          }
        }
        glEnd();
        glEnable(GL_LIGHTING);
      }
      //glEnable(GL_DEPTH_TEST);

      m_data->physicsWorld->unlock();
    }

    SceneObject *PhysicsPaper3::approximateSurface(int nx, int ny) const{
      m_data->physicsWorld->lock();
      SceneObject *o = new SceneObject;
      std::vector<Vec> &vs = o->getVertices(), cs = o->getVertexColors();
      vs.resize(nx*ny);
      cs.resize(nx*ny,GeomColor(1,0,0,1));

      for(int y=0;y<ny;++y){
        for(int x=0;x<nx;++x){
          const Point32f p(float(x)/(nx-1),float(y)/(ny-1));
          try{
            vs[x + nx*y] = interpolatePosition(p);
          }catch(ICLException &e){
            DEBUG_LOG(e.what() << " point was " << p);
          }
        }
      }
      m_data->physicsWorld->unlock();
      return o;
    }


    namespace {
      struct GetPaperCoordinatesHit{
        Point32f coords;
        float distToCamera;
        GetPaperCoordinatesHit(){}
        GetPaperCoordinatesHit(const Point32f &coords,
                               const Vec &a, const Vec &b):coords(coords){
          distToCamera = sqr(a[0]-b[0]) + sqr(a[1]-b[1]) + sqr(a[2]-b[2]);
        }
        bool operator<(const GetPaperCoordinatesHit &o) const{
          return distToCamera < o.distToCamera;
        }
      };
    }

    Point32f PhysicsPaper3::hit(const geom::ViewRay &ray) const{
      m_data->physicsWorld->lock();
      std::vector<GetPaperCoordinatesHit> hits;
      const btSoftBody *s = getSoftBody();



      for(int i=0;i<s->m_faces.size();++i){
        const btSoftBody::Face &f = s->m_faces[i];
        Vec a = bullet2icl_scaled(f.m_n[0]->m_x);
        Vec b = bullet2icl_scaled(f.m_n[1]->m_x);
        Vec c = bullet2icl_scaled(f.m_n[2]->m_x);

        Vec pW;
        Point32f coords;

        ViewRay::TriangleIntersection inter = ray.getIntersectionWithTriangle(a,b,c,&pW,&coords);

        if(inter == ViewRay::foundIntersection){
          const Point32f &ta = m_data->texCoords[ (int)(f.m_n[0] - &s->m_nodes[0]) ];
          const Point32f &tb = m_data->texCoords[ (int)(f.m_n[1] - &s->m_nodes[0]) ];
          const Point32f &tc = m_data->texCoords[ (int)(f.m_n[2] - &s->m_nodes[0]) ];

          Point32f pPaper = ta + (tb-ta)*coords.x + (tc-ta)*coords.y;

          hits.push_back(GetPaperCoordinatesHit(pPaper,pW,ray.offset));
        }
      }
      m_data->physicsWorld->unlock();

      if(!hits.size()) return Point32f(-1,-1);
      return std::min_element(hits.begin(),hits.end())->coords;
    }

    Img32f PhysicsPaper3::paperCoordinateTest(const Camera &cam) const{
      m_data->physicsWorld->lock();
      Img32f image(Size::QVGA,formatRGB);

      Array2D<ViewRay> vrs = cam.getAllViewRays();
      ICLASSERT_THROW(vrs.getWidth() == 640 &&
                      vrs.getHeight() == 480, ICLException("PhysicsPaper3D::paperCoordinateTest: this feature is only supported for VGA cameras"));

      float *r = image.begin(0);
      float *g = image.begin(1);
      float *b = image.begin(2);

      progress_init("creating image");
      progress(0,vrs.getHeight()/2);
      for(int y=0;y<vrs.getHeight()/2;++y){
        progress(y,vrs.getHeight()/2);
        for(int x=0;x<vrs.getWidth()/2;++x,++r,++g,++b){
          Point32f p = this->hit(vrs(2*x,2*y));
          if(p == Point32f(-1,-1)){
            *r = *g = *b = 0;
          }else{
            *r = 255 * p.x;
            *g = 0;
            *b = 255 * p.y;
          }
        }
      }
      progress_finish();
      m_data->physicsWorld->unlock();
      return image;
    }


    inline float sqr_dist(const btVector3 &a, const btVector3 &b){
      return sqr(a[0]-b[0]) + sqr(a[1]-b[1]) + sqr(a[2]-b[2]);
    }

    inline float max_3(float a, float b, float c){
      if(a > b) return iclMax(a,c);
      else return iclMax(b,c);
    }

    inline const Vec &vec_cast(const btVector3 &v){
      return *reinterpret_cast<const Vec*>(&v);
    }

    inline Vec &vec_cast_unconst(btVector3 &v){
      return *reinterpret_cast<Vec*>(&v);
    }

    void PhysicsPaper3::simulateSelfCollision(){
      //TODO LOOK AT THIS AGAIN
      //PhysicsWorld::Locker lock(*m_data->physicsWorld);

      static const float TRIANGLE_RADIUS_MARGIN = icl2bullet(5); // 5mm
      static const float SELF_COLLISION_MARGIN = icl2bullet(10); // 5mm todo: make smaller
      static const float MIN_NUMERICAL_DIST = 1.e-8;

      // move away nodes from triangles that are not connected
      btSoftBody *s = getSoftBody();

      btAlignedObjectArray<btSoftBody::Node> &ns = s->m_nodes;
      btAlignedObjectArray<btSoftBody::Face> &ts = s->m_faces;

      std::vector<bool> nodesMoved(ns.size(),false);

      for(int t=0;t<ts.size();++t){
        btSoftBody::Face &f = ts[t];
        const btVector3 a = f.m_n[0]->m_x, b = f.m_n[1]->m_x, c = f.m_n[2]->m_x;
        btVector3 mean = (a+b+c)* 0.33333333f;
        float r = max_3( sqr_dist(a,mean),
                         sqr_dist(a,mean),
                         sqr_dist(a,mean) ) + TRIANGLE_RADIUS_MARGIN;
        for(int i=0;i<ns.size();++i){
          if(nodesMoved[i]) continue;
          btSoftBody::Node *n = &ns[i];
          if(sqr_dist(n->m_x,mean) < r){ // broad-phase#
            if( (n != f.m_n[0]) && (n != f.m_n[1]) && (n != f.m_n[2]) ){ // avoid self repulsion
              btVector3 p;

              float d = dist_point_triangle(vec_cast(n->m_x), vec_cast(a), vec_cast(b), vec_cast(c),
                                            &vec_cast_unconst(p));
              if(d < SELF_COLLISION_MARGIN){
                btVector3 dVec = n->m_x - p;
                float l = sqr(dVec[0]) + sqr(dVec[1]) + sqr(dVec[2]);
                if(l > MIN_NUMERICAL_DIST){
                  nodesMoved[i] = true;
                  n->m_v = dVec *(SELF_COLLISION_MARGIN-d)/l; // normalized
                }
              }
            }
          }
        }
      }
    }


    utils::VisualizationDescription PhysicsPaper3::getFoldLineHighlight(const LinkCoords &coords, const Camera &cam) const{
      Point32f a = coords.first, b = coords.second;
      VisualizationDescription d;
      d.linewidth(6);
      d.color(255,255,0,150);

      const btSoftBody *s = getSoftBody();

      StraightLine2D ab(a, b-a);

      const btSoftBody::Node *o = &s->m_nodes[0];
      for(int i=0;i<s->m_links.size();++i){
        if(!LinkState::is_fold(s->m_links[i].m_tag)) continue;
        int ia = (s->m_links[i].m_n[0]-o);
        int ib = (s->m_links[i].m_n[1]-o);
        Point32f la = m_data->texCoords[ia];
        Point32f lb = m_data->texCoords[ib];
        if((ab.distance(la) < 0.05) && (ab.distance(lb) < 0.05)){

          Vec va = bullet2icl_scaled(s->m_nodes[ia].m_x);
          Vec vb = bullet2icl_scaled(s->m_nodes[ib].m_x);

          Point32f pa = cam.project(va);
          Point32f pb = cam.project(vb);

          d.line(pa,pb);
        }
      }
      return d;
    }

    SmartPtr<PhysicsPaper3::LinkCoords> PhysicsPaper3::getLinkCoords(const Point32f &pix,
                                                                     const Camera &cam) const {
      ViewRay v = cam.getViewRay(pix);
      Point32f p = hit(v);

      const btSoftBody *s = getSoftBody();
      const btSoftBody::Node *o = &s->m_nodes[0];

      int bestIA(-1),bestIB(-1), bestD = -1;
      for(int i=0;i<s->m_links.size();++i){
        if(!LinkState::is_fold(s->m_links[i].m_tag)) continue;
        int ia = (s->m_links[i].m_n[0]-o);
        int ib = (s->m_links[i].m_n[1]-o);
        Point32f la = m_data->texCoords[ia];
        Point32f lb = m_data->texCoords[ib];

        StraightLine2D line(la,lb-la);
        float d = line.distance(p);
        if(d < 0.005){
          if(bestIA < 0 || d < bestD){
            bestIA = ia;
            bestIB = ib;
            bestD = d;
          }
        }
      }
      if(bestIA != -1){
        return new LinkCoords(m_data->texCoords[bestIA],m_data->texCoords[bestIB]);
      }else{
        return SmartPtr<LinkCoords>();
      }
    }

    void PhysicsPaper3::adaptFoldStiffness(const LinkCoords &coords, float stiffness, bool memorize){
      Point32f a = coords.first, b = coords.second;
      btSoftBody *s = getSoftBody();

      StraightLine2D ab(a, b-a);
      std::vector<btSoftBody::Link*> links;

      btSoftBody::Node *o = &s->m_nodes[0];
      for(int i=0;i<s->m_links.size();++i){
        btSoftBody::Link &l = s->m_links[i];
        int ia = (l.m_n[0]-o);
        int ib = (l.m_n[1]-o);
        Point32f la = m_data->texCoords[ia];
        Point32f lb = m_data->texCoords[ib];
        if((ab.distance(la) < 0.01) && (ab.distance(lb) < 0.01)){
          m_data->fm.addFold(la,lb,memorize ? -stiffness : stiffness);
          //if(memorize){
          //  Vec d = bullet2icl(s->m_nodes[ia].m_x) - bullet2icl(s->m_nodes[ib].m_x);
          //  l.m_rl = icl2bullet_scaled( norm3(d) );
          //  LinkState *fs = (LinkState*)l.m_tag;
          //  fs->hasMemorizedRestDist = true;
          //}else{
          //  l.m_rl = icl2bullet_scaled( ::sqrt (::sqr((la.x-lb.x)*210) + ::sqr((la.y-lb.y)*297) )  );
          //  LinkState *fs = (LinkState*)l.m_tag;
          //  fs->hasMemorizedRestDist = false;
          //}
          //l.m_c1 = l.m_rl*l.m_rl;
        }
      }

      createBendingConstraints(m_data->initialStiffness);
      // all links that cross one of the toBeMemorizedLinks must not update their rest-distances
      // createBendingConstraints2(0.5, toBeMemorizedLinks);
    }

    void PhysicsPaper3::serializeStructureTo(std::ostream &str){
      btSoftBody *s = getSoftBody();
      str << s->m_nodes.size() << ' ';
      str << s->m_faces.size() << ' ';
      for(int i=0;i<s->m_nodes.size();++i){
        str << bullet2icl(s->m_nodes[i].m_x[0]) << ' ';
        str << bullet2icl(s->m_nodes[i].m_x[1]) << ' ';
        str << bullet2icl(s->m_nodes[i].m_x[2]) << ' ';
        str << m_data->texCoords[i].x << ' ';
        str << m_data->texCoords[i].y << ' ';
      }

      btSoftBody::Node *offs = &s->m_nodes[0];
      for(int i=0;i<s->m_faces.size();++i){
        str << (int)(s->m_faces[i].m_n[0] - offs) << ' ';
        str << (int)(s->m_faces[i].m_n[1] - offs) << ' ';
        str << (int)(s->m_faces[i].m_n[2] - offs) << ' ';
      }
    }

    void PhysicsPaper3::Structure::deserializeFrom(std::istream &str){
      Structure &s = *this;
      int n(0),m(0);
      str >> n >> m;
      s.texCoords.resize(n);
      s.vertices.resize(n);
      s.faces.resize(m);
      for(int i=0;i<n;++i){
        str >> s.vertices[i][0] >> s.vertices[i][1] >> s.vertices[i][2];
        s.vertices[i][3] = 1;
        str >> s.texCoords[i].x >> s.texCoords[i].y;
      }
      for(int i=0;i<m;++i){
        str >> s.faces[i].a >> s.faces[i].b >> s.faces[i].c;
      }
    }

    void PhysicsPaper3::Structure::updateToSceneObject(SceneObject *obj){
      obj->setLockingEnabled(true);
      obj->lock();

      obj->getVertices().resize(vertices.size());
      std::copy(vertices.begin(),vertices.end(), obj->getVertices().begin());
      //    DEBUG_LOG("copied " << vertices.size() << " vertices" )
      obj->getVertexColors().resize(vertices.size(), geom_green(255));
      for(size_t i=0;i<obj->getPrimitives().size();++i){
        delete obj->getPrimitives()[i];
      }

      obj->getPrimitives().clear();
      for(size_t i=0;i<faces.size();++i){
        obj->addTriangle(faces[i].a,faces[i].b,faces[i].c,geom_green(100));
        obj->addLine(faces[i].a,faces[i].b, geom_green(255));
        obj->addLine(faces[i].b,faces[i].c, geom_green(255));
        obj->addLine(faces[i].c,faces[i].a, geom_green(255));
      }
      obj->unlock();
    }

    geom::Vec PhysicsPaper3::Structure::interpolatePosition(const utils::Point32f &pp) const{
      Point32f p = pp;
      if(p.x <= 0) p.x = 1.e-7f;
      if(p.y <= 0) p.y = 1.e-7f;
      if(p.x >= 1) p.x = 1.f - 1.e-7f;
      if(p.y >= 1) p.y = 1.f - 1.e-7f;

      int hit_idx=-1;
      for(size_t i=0;i<faces.size();++i){
        const Face &f = faces[i];
        if(point_in_triangle(p,texCoords[f.a], texCoords[f.b], texCoords[f.c])){
          hit_idx = i;
          break;
        }
      }
      if(hit_idx == -1) throw ICLException("PhysicsPaper3::Structure:interpolatePosition: no triangle on position " +str(pp)+" found");
      const Face &f = faces[hit_idx];

      const Vec a = vertices[f.a];
      const Vec b = vertices[f.b];
      const Vec c = vertices[f.c];

      const Point32f ta = texCoords[f.a];
      const Point32f tb = texCoords[f.b];
      const Point32f tc = texCoords[f.c];


      /*
        a@x1y1        b@x2y2

              (pxpy)

             c@x3y3

          alpha (c-a)  +  beta (b-a) = p-a

          |cx-ax  bx-ax|       |px-ax|
          |cy-ay  by-ay| * x = |py-ay|

          A x = b

          x = a.solve(b,"inv"); // inv is faster and more stable for (cx-ax = 0)
      */

      FixedMatrix<float,2,2> M(tc.x - ta.x, tb.x - ta.x,
                              tc.y - ta.y, tb.y - ta.y);

      FixedMatrix<float,1,2> ff = M.inv()*FixedMatrix<float,1,2>( p.x - ta.x, p.y - ta.y);

      Vec res = a + (c - a)*ff[0] + (b-a)*ff[1];
      res[3] = 1;

      return res;
    }

    Point32f PhysicsPaper3::Structure::hit(const geom::ViewRay &ray) const{
      std::vector<GetPaperCoordinatesHit> hits;

      for(size_t i=0;i<faces.size();++i){
        const Face &f = faces[i];
        const Vec &a = vertices[f.a];
        const Vec &b = vertices[f.b];
        const Vec &c = vertices[f.c];

        Vec pW;
        Point32f coords;

        ViewRay::TriangleIntersection inter = ray.getIntersectionWithTriangle(a,b,c,&pW,&coords);

        if(inter == ViewRay::foundIntersection){
          const Point32f &ta = texCoords[f.a];
          const Point32f &tb = texCoords[f.b];
          const Point32f &tc = texCoords[f.c];

          Point32f pPaper = ta + (tb-ta)*coords.x + (tc-ta)*coords.y;

          hits.push_back(GetPaperCoordinatesHit(pPaper,pW,ray.offset));
        }
      }
      if(!hits.size()) return Point32f(-1,-1);
      return std::min_element(hits.begin(),hits.end())->coords;
    }


    int PhysicsPaper3::getNumNodes() const{
      const btSoftBody *s = getSoftBody();
      return s->m_nodes.size();
    }
  }
}
