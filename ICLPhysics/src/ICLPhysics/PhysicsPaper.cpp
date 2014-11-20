/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/src/ICLPhysics/PhysicsPaper.cpp             **
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
#include <ICLPhysics/PhysicsPaper.h>
#include <ICLPhysics/PhysicsWorld.h>
#include <ICLPhysics/PhysicsDefs.h>

#include <ICLPhysics/TriangleIntersectionEstimator.h>

#include <BulletSoftBody/btSoftBody.h>
#include <BulletSoftBody/btSoftBodyHelpers.h>
#include <BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h>
#include <BulletSoftBody/btSoftRigidDynamicsWorld.h>

#include <ICLGeom/Primitive.h>

#include <ICLMarkers/FiducialDetector.h>
#include <ICLQt/Quick.h>


namespace icl{
  using namespace utils;
  using namespace geom;
namespace physics{


  PhysicsPaper::PhysicsPaper(const PhysicsWorld &world,
                             int nxCells, int nyCells, const Vec *init,
                             bool initWithCorners,
                             const Img8u *texture,const Img8u *backfaceTexture):cells(nxCells,nyCells){
    //    const int dim = nxCells * nyCells;

    static const Vec stdcs[4]={
      Vec(-1,-1,0,1),Vec(1 ,-1,0,1),
      Vec(1 , 1,0,1),Vec(-1, 1,0,1)
    };
    const Vec *cs = initWithCorners ? init : stdcs;
    btVector3 bt[4];
    for(int i=0;i<4;++i){
      for(int j=0;j<3;++j){
        bt[i][j] = icl2bullet(cs[i][j]);
      }
    }
    
    btSoftBody *s=btSoftBodyHelpers::CreatePatch(*const_cast<btSoftBodyWorldInfo*>(world.getWorldInfo()),
                                                 bt[0],bt[1],bt[3],bt[2],nxCells,nyCells,0,true);
    setPhysicalObject(s);
    
    btSoftBody::tNodeArray & nodes = s->m_nodes;
    if(!initWithCorners){
      for(int i=0;i<nodes.size();++i){
        nodes[i].m_x = btVector3(icl2bullet(init[i][0]),icl2bullet(init[i][1]),icl2bullet(init[i][2]));
      }
    }
    
    //    s->getCollisionShape()->setMargin(0.18);
    s->getCollisionShape()->setMargin(icl2bullet(2));

    s->appendMaterial();
    s->m_materials[0]->m_kLST = 1.0;
    s->m_materials[0]->m_kAST = 1.0;
    s->m_materials[0]->m_kVST = 1.0;
    
    const Rect r(0,0,nxCells,nyCells);
    const int d = 4;

#define IDX(x,y) x+nxCells*y
    
    for(int y=0;y<nyCells;++y){
      for(int x=0;x<nxCells;++x){
        for(int i=-d;i<=d;++i){
          for(int j=-d;j<=d;++j){
            int tx = i+x, ty = j+y;
            //            if( !(i==0) && !(j==0) ) continue;

            if(r.contains(tx,ty) && (::abs(i)+::abs(j) >= 3)){ // changed
              btSoftBody::Material *m = s->appendMaterial();

              m->m_kVST = m->m_kAST = m->m_kLST = 0.9;

              int sizeBefore = s->m_links.size();
              s->appendLink(IDX(x,y),IDX(tx,ty), m, true); 
              if(s->m_links.size() == sizeBefore) continue; // link did already exist
              s->m_links[s->m_links.size()-1].m_bbending=1;
              
              constraints.push_back(BendingConstraint(&s->m_links[s->m_links.size()-1],
                                                      Point(x,y),Point(tx,ty)));


            }
          }
        }
      }
    }
    
    //std::cout << "created " << s->m_links.size() << " bending constraints" << std::endl;

    //s->randomizeConstraints();
    randomizeLinks();
    
    s->generateClusters(0);


    s->m_cfg.kLF = 0;
    s->m_cfg.kDG = 0;

    s->m_cfg.kMT = 10;
    s->m_cfg.kDP = 0.1;
    s->m_cfg.kDF = 1.0;

    //s->m_cfg.aeromodel = btSoftBody::eAeroModel::V_TwoSided;   
    //s->m_cfg.collisions	= btSoftBody::fCollision::CL_SS+btSoftBody::fCollision::CL_RS + btSoftBody::fCollision::CL_SELF;
    s->m_cfg.collisions	= (btSoftBody::fCollision::CL_SS | btSoftBody::fCollision::CL_RS | btSoftBody::fCollision::CL_SELF);
                           //btSoftBody::fCollision::SDF_RS + 
    //btSoftBody::fCollision::CL_SELF);
    
    /// creating the scene object
    for(int i=0;i<nxCells*nyCells;++i){
      addVertex(Vec(bullet2icl(nodes[i].m_x[0]),
                    bullet2icl(nodes[i].m_x[1]),
                    bullet2icl(nodes[i].m_x[2]),1),
                GeomColor(0,100,255,255));
      addNormal(Vec(nodes[i].m_n[0],nodes[i].m_n[1],nodes[i].m_n[2],1));
    }
   
    
    //for(int x=0;x<nxCells-1;++x){
    //  for(int y=0;y<nyCells-1;++y){
    //    int i = x+nxCells * y;
    //    addQuad(i,i+1,i+1+nxCells,i+nxCells,
    //            i,i+1,i+1+nxCells,i+nxCells,
    //            GeomColor(0,100,255,255));
    //          addLine(i,i+1,GeomColor(255,0,0,255));
    //    addLine(i+1,i+1+nxCells,GeomColor(255,0,0,255));
    //    addLine(i+1+nxCells,i+nxCells,GeomColor(255,0,0,255));
    //    addLine(i+nxCells,i,GeomColor(255,0,0,255));
    //  }
    //}
    addTwoSidedTGrid(nxCells, nyCells, m_vertices.data(), m_normals.data(),
                     GeomColor(0,100,255,255), GeomColor(255,0,100,255), 
                     GeomColor(0,255,100,255), false, true);
    
    hasBackfaceTexture = !! backfaceTexture;
    
    if(texture){
      setVisible(Primitive::quad,false);
      setVisible(Primitive::texture,false);
      if(backfaceTexture){
        SmartPtr<ImgBase> cpy = backfaceTexture->deepCopy();
#ifdef FLIP_MARKERS_ONLY
        FiducialDetector fd("bch","[0,4095]","size=1x1");
        std::vector<Fiducial> fids = fd.detect(cpy.get());
        for(size_t i=0;i<fids.size();++i){
          std::vector<Point32f> corners = fids[i].getCorners2D();
          Range32s rx(corners[0].x,corners[0].x), ry(corners[0].y,corners[0].y);
          for(int j=1;j<4;++j){
            rx.extend(corners[j].x);
            ry.extend(corners[j].y);
          }
          cpy->setROI(Rect(rx.minVal, ry.minVal, rx.getLength()+1, ry.getLength()+1));
          cpy->mirror(axisVert, true);
        }
        cpy->setFullROI();
        //        icl::show( (cvt(backfaceTexture), cvt(cpy.get()) ) );
#else
        cpy->mirror(axisVert, true);
#endif
        
        //struct DepthTestOnOffPrimitive : public Primitive{
        //  bool on;
        //  DepthTestOnOffPrimitive(bool on):Primitive(Primitive::texture),on(on){}
        //  virtual void render(const Primitive::RenderContext &ctx){
        //    if(on)glEnable(GL_DEPTH_TEST);
        //    else glDisable(GL_DEPTH_TEST);
        //  }
        //  virtual Primitive *copy() const { return new DepthTestOnOffPrimitive(*this); }
        //};

        
        //addCustomPrimitive(new DepthTestOnOffPrimitive(false));
        addTwoSidedTextureGrid(cells.width,cells.height,
                               texture,
                               cpy.get(),
                               &m_vertices[0][0], 
                               &m_vertices[0][1], 
                               &m_vertices[0][2],
                               &m_normals[0][0],
                               &m_normals[0][1],
                               &m_normals[0][2],4);
        TwoSidedTextureGridPrimitive * p = (TwoSidedTextureGridPrimitive*)(m_primitives.back());
        p->alphaFunc = (int)GL_ALWAYS;
        //addCustomPrimitive(new DepthTestOnOffPrimitive(true));
      }else{
        addTextureGrid(cells.width,cells.height,
                       texture,
                       &m_vertices[0][0], 
                       &m_vertices[0][1], 
                       &m_vertices[0][2],
                       &m_normals[0][0],
                       &m_normals[0][1],
                       &m_normals[0][2],
                       4);
      }

    }
    setVisible(Primitive::line,false);
    setColorsFromVertices(Primitive::quad,true);
    createAllProperties();

    s->setTotalMass( 0.01 ); // does not affect anything?

    for(int i=0;i<s->m_links.size();++i){
      originalRestLengths.push_back(s->m_links[i].m_rl);
    }

  }


  void PhysicsPaper::resetDeformation(){
    btSoftBody *s = getSoftBody();
    for(int i=0;i<s->m_links.size();++i){
      btSoftBody::Link &l = s->m_links[i];
      float o = originalRestLengths[i];
      l.m_rl = o;
      l.m_c1 = o*o;
    }
  }

 
  void PhysicsPaper::moveVertex(const Point &xy, const Vec &pos, float factor){
    btSoftBody::tNodeArray & nodes = getSoftBody()->m_nodes;
    int i = xy.x+cells.width*xy.y;
    btVector3 t(icl2bullet(pos[0]),icl2bullet(pos[1]),icl2bullet(pos[2]));
    
    nodes[i].m_v = (t-nodes[i].m_x)*factor;
  }

  void PhysicsPaper::movePosition(const Point32f &paperPos, const Vec &pos, float factor){
    const Vec currPos = getInterpolatedPosition(paperPos);

    const float x = paperPos.x, y = paperPos.y;
    const int x0 = floor(x), y0 = floor(y);
    const int x1 = ceil(x),  y1 = ceil(y);
    
    const Point32f ps[4] = { Point(x0,y0), Point(x1,y0), Point(x0,y1), Point(x1,y1) };
    for(int i=0;i<4;++i){
      float contribution = iclMax(1.0 - ::sqrt( ::sqr(ps[i].x-x) + ::sqr(ps[i].y-y) ), 0.0);
      moveVertex( ps[i], pos + (getNodePosition(ps[i])-currPos),factor * contribution);
    }
  }

  Point PhysicsPaper::getNodeIndex(const Vec &v){
    btSoftBody::tNodeArray & nodes = getSoftBody()->m_nodes;
    std::vector<float> ds(nodes.size());
    
    float x=icl2bullet(v[0]),y=icl2bullet(v[1]),z=icl2bullet(v[2]);
    for(int i=0;i<nodes.size();++i){
      ds[i] = sqr(nodes[i].m_x[0]-x) + sqr(nodes[i].m_x[1]-y) + sqr(nodes[i].m_x[2]-z); 
    }
    int minDistIdx = (int)(std::min_element(ds.begin(),ds.end())-ds.begin());
    return Point(minDistIdx%cells.width,minDistIdx/cells.width);
  }

  void PhysicsPaper::updateSceneObject(){
    btSoftBody::tNodeArray & nodes = getSoftBody()->m_nodes;
    const int &nxCells = cells.width;
    const int &nyCells = cells.height;
    const int dim = nxCells * nyCells;
    
    for(int i=0;i<dim;++i){
      for(int j=0;j<3;++j){
        m_vertices[i][j] = bullet2icl(nodes[i].m_x[j]);
        m_normals[i][j] = nodes[i].m_n[j];
      }
    }

#if 0
    if(hasBackfaceTexture){
#if 0
      for(int i=0;i<dim;++i){
        m_normals[i+dim] = m_normals[i];
        m_vertices[i+dim] = m_vertices[i] - m_normals[i]*-1;
      }
#else
      const int W = getDimensions().width, H = getDimensions().height;
      for(int y=0;y<H;++y){
        for(int x=0;x<W;++x){
          const int iSrc = x + W*y;
          const int iDst = dim + (W-1-x) + W*y;
          m_normals[iDst] = -m_normals[iSrc];
          m_vertices[iDst] = m_vertices[iSrc] - m_normals[iSrc]*-1;
        }
      }
#endif
    }
#endif
    
  }

  void PhysicsPaper::setDraggedNode(const Point &xy){
    if(xy.x < 0  && xy.y < 0 ){
      std::fill(m_vertexColors.begin(),m_vertexColors.end(), GeomColor(0,100,255,255)/255.);
    }else{
      m_vertexColors[xy.x+cells.width*xy.y] = GeomColor(1,0,0,1);
    }
  }
  
  
  Vec PhysicsPaper::getNodePosition(int x, int y) const{
    const btSoftBody::tNodeArray &nodes = getSoftBody()->m_nodes;
    const btVector3 &p = nodes[x+cells.width*y].m_x;
    return Vec(bullet2icl(p[0]),bullet2icl(p[1]),bullet2icl(p[2]),1);
  }

  void PhysicsPaper::setNodeMass(const Point &xy, float mass){
    getSoftBody()->setMass(xy.x+cells.width*xy.y,mass);
  }

  void PhysicsPaper::setNodeMass(const Vec &v, float mass){ 
    setNodeMass(getNodeIndex(v),mass); 
  }
  
  void PhysicsPaper::setTotalMass(float mass){
    btSoftBody::tNodeArray & nodes = getSoftBody()->m_nodes;
    for(int i=0;i<nodes.size();++i){
      nodes[i].m_im = mass/nodes.size();
    }
  }
  
  Img32f PhysicsPaper::getVelocityMap() const{
    Img32f m(cells,1);
    Channel32f c = m[0];
    const btSoftBody::tNodeArray & nodes = getSoftBody()->m_nodes;
    for(int i=0;i<nodes.size();++i){
      c[i] = ::sqrt( sqr(nodes[i].m_v[0]) + sqr(nodes[i].m_v[1]) + sqr(nodes[i].m_v[2]) );
    }
    return m;
  }

  void PhysicsPaper::randomizeLinks(){
    getSoftBody()->randomizeConstraints();
    btSoftBody::tLinkArray &links = getSoftBody()->m_links;

    std::map<BendingConstraint::Material*,BendingConstraint::Link*> lookup;

    for(int i=0;i<links.size();++i){
      lookup[links[i].m_material] = &links[i];
    }
    
    for(size_t i=0;i<constraints.size();++i){
      constraints[i].updateLinkPointer(lookup);
    }
  }
  
  void PhysicsPaper::adaptRowStiffness(float val, int row){
    for(size_t i=0;i<constraints.size();++i){
      BendingConstraint &c = constraints[i];
      const float y=c.a.y,ty=c.b.y;
      if( (y>row && ty<row) || (y<row && ty>row) ){
        c.setStiffness(val);
      }
    }
  }
  
  void PhysicsPaper::adaptColStiffness(float val, int col){
    for(size_t i=0;i<constraints.size();++i){
      BendingConstraint &c = constraints[i];
      const float x=c.a.x,tx=c.b.x;
      if( (x>col && tx<col) || (x<col && tx>col) ){
        c.setStiffness(val);
      }
    }
  }

  void PhysicsPaper::adaptGlobalStiffness(float val){
    btSoftBody::tLinkArray &links = getSoftBody()->m_links;
    for(int i=0;i<links.size();++i){
      BendingConstraint c(&links[i]);
      c.setStiffness(val);
    }
  }
  
  void PhysicsPaper::memorizeDeformation(){
    lock();
    getSoftBody()->updateConstants();
    unlock();
  }

  static int sign(float x){ return x > 0 ? 1 : -1; }

  void PhysicsPaper::adaptStiffnessAlongIntersection(const PlaneEquation &plane, float val){
    for(size_t i=0;i<constraints.size();++i){
      BendingConstraint &c = constraints[i];

      Vec a = getNodePosition(c.a.x,c.a.y) - plane.offset;
      Vec b = getNodePosition(c.b.x,c.b.y) - plane.offset;
      
      if( sign(sprod3(a,plane.normal)) != sign(sprod3(b,plane.normal)) ){
        c.setStiffness(val);
      }      
    }
  }

  namespace {
    struct GetPaperCoordinatesHit{
      Point32f coords;
      float distToCamera;
      bool operator<(const GetPaperCoordinatesHit &o) const{
        return distToCamera < o.distToCamera;
      }
    };
    
    static float squared_norm(const Vec &v){
      return sqr(v[0]) + sqr(v[1]) + sqr(v[2]);
    }
  }
  
  /// finds the optimal paper coordinates for a given point in the world
  Point32f PhysicsPaper::getPaperCoordinates(const geom::ViewRay &ray){
    typedef TriangleIntersectionEstimator TIE;
    
    std::vector<GetPaperCoordinatesHit> hits;
    
    for(int x=1;x<cells.width;++x){
      for(int y=1;y<cells.height;++y){
        Vec a = getNodePosition(x-1,y-1);
        Vec b = getNodePosition(x,y-1);
        Vec c = getNodePosition(x,y);
        Vec d = getNodePosition(x-1,y);
        
        TIE::Intersection i1 = TIE::find(TIE::Triangle(a,d,b),ray);
        TIE::Intersection i2 = TIE::find(TIE::Triangle(c,b,d),ray);
        
        if(i1){
          const float &ix = i1.trianglePosition.x, &iy = i1.trianglePosition.y;
          GetPaperCoordinatesHit h = {
            // orig: just wrong Point32f(x - iy,y - ix), 
            Point32f(x-1 + iy,y-1 + ix), 
            squared_norm(i1.position-ray.offset)
          };
          hits.push_back(h);
        }else if(i2){
          const float &ix = i2.trianglePosition.x, &iy = i2.trianglePosition.y;
          GetPaperCoordinatesHit h = {
            // orig: just wrong! Point32f(x-1 + iy, y-1 + ix), 
            Point32f(x - iy, y - ix), 
            squared_norm(i2.position-ray.offset)
          };
          hits.push_back(h);
        }
      }
    }
    if(!hits.size()) return Point32f(-1,-1);
    return std::min_element(hits.begin(),hits.end())->coords;
  }

  static inline Vec lin_interpolate(const Vec &a, const Vec &b, float f){
    const float f1 = 1.0-f;
    return  Vec (a[0] * f1 + b[0] * f,
                 a[1] * f1 + b[1] * f,
                 a[2] * f1 + b[2] * f,1);
                 //a * (1-f) + b * f; 
  }
  
  
  Vec PhysicsPaper::getInterpolatedPosition(const Point32f &p){
    float x = p.x, y = p.y;
    int x0 = floor(x), y0 = floor(y);
    int x1 = ceil(x),  y1 = ceil(y);
    
    Vec a = getNodePosition(x0,y0);
    Vec b = getNodePosition(x1,y0);
    Vec c = getNodePosition(x0,y1);
    Vec d = getNodePosition(x1,y1);
   
    //also wrong orig: float fx = x1 - p.x, fy = y1 - p.y;

    float fx = 1.0f - (x1 - p.x), fy = 1.0f - (y1 - p.y);
    return lin_interpolate( lin_interpolate(a,b,fx), lin_interpolate(c,d,fx), fy);
  }
  
  Vec PhysicsPaper::getPosFromPhysics(int x, int y) const{
    const btSoftBody::tNodeArray &nodes = getSoftBody()->m_nodes;
    int idx = x + getDimensions().width * y;
    return Vec(bullet2icl(nodes[idx].m_x[0]),
               bullet2icl(nodes[idx].m_x[1]),
               bullet2icl(nodes[idx].m_x[2]),1);
  }

  Vec PhysicsPaper::getNormalFromPhysics(int x, int y){
    const btSoftBody::tNodeArray &nodes = getSoftBody()->m_nodes;
    int idx = x + getDimensions().width * y;
    return Vec(nodes[idx].m_n[0],
               nodes[idx].m_n[1],
               nodes[idx].m_n[2],1);
  }


}
}
