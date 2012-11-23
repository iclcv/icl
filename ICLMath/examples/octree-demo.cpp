#include <ICLQt/Common.h>
#include <ICLMath/Octree.h>
#include <ICLGeom/Scene.h>
#include <ICLGeom/PointCloudObject.h>
#include <ICLUtils/Random.h>
#include <ICLGeom/PCLPointCloudObject.h>
#include <ICLGeom/OctreeObject.h>

#include <boost/shared_ptr.hpp>

#define TRY_PCL_OCTREE

#ifdef TRY_PCL_OCTREE
#include <pcl/search/octree.h>
typedef pcl::search::Octree<pcl::PointXYZ> PCL_OT;
//#include <pcl/octree/octree_search.h>
//typedef pcl::octree::OctreePointCloudSearch<pcl::PointXYZ> PCL_OT;

#else
#include <pcl/kdtree/kdtree_flann.h>
typedef pcl::KdTreeFLANN<pcl::PointXYZ> PCL_OT;
#endif

typedef PCLPointCloudObject<pcl::PointXYZ> PCL_PC;

GUI gui;
PointCloudObject obj(500*1000,1,false);
Scene scene;
OctreeObject<icl32s,16> ot(-500,-500,-500,1000,1000,1000);


void init(){
  
  ot.setRenderPoints(true);
  
  GRandClip r(0,100,Range64f(-500,500));

  DataSegment<float,3> ps = obj.selectXYZ();
  DataSegment<float,4> cs = obj.selectRGBA32f();
  
  std::vector<FixedColVector<int,4> > nn(1000),nnres(1000);
  for(int i=0;i<obj.getDim();++i){
    ps[i] = Vec3(r,r,r);
    cs[i] = GeomColor(0,100,255,255);
    
    if(i < (int)nn.size()){
      nn[i] = FixedColVector<int,4>(r,r,r);
    }
  }
  
  Time t = Time::now();
  for(int i=0;i<obj.getDim();++i){
    FixedColVector<float,3> p = ps[i];
    ot.insert(FixedColVector<int,3>(p[0],p[1],p[2]));
  }
  t.showAge("insertion time");
  
  std::vector<FixedColVector<int,4> > q = ot.query(0,-500,-500,500,1000,1000);
  static PointCloudObject res(q.size(),1,false);
  ps = res.selectXYZ();
  cs = res.selectRGBA32f();
  for(size_t i=0;i<q.size();++i){
    ps[i] = Vec3(q[i][0],q[i][1],q[i][2],1);
    cs[i] = GeomColor(255,0,0,255);
  }

  PCL_PC pcl_pc(obj.getDim(),1);
  DataSegment<float,3> pcl_ps = pcl_pc.selectXYZ();
  for(int i=0;i<obj.getDim();++i){
    pcl_ps[i] = ps[i];
  }

  t = Time::now();
  PCL_OT pcl_ot(16);
  t.showAge("create pcl octree");
  static boost::shared_ptr<pcl::PointCloud<pcl::PointXYZ> > ptr(&pcl_pc.pcl());
  
  t = Time::now();
  pcl_ot.setInputCloud(ptr);

  t.showAge("set input cloud to pcl tree"); // 3 times faster!
  

  
  t = Time::now();
  for(size_t i=0;i<nn.size();++i){
    nnres[i] = ot.nn(nn[i]);
  }
  t.showAge("icl nn search");

  t = Time::now();
  for(size_t i=0;i<nn.size();++i){
    std::vector<int> dstIdx;
    std::vector<float> dstDist;
    pcl_ot.nearestKSearch(pcl::PointXYZ(nn[i][0],nn[i][1],nn[i][2]), 1, dstIdx,dstDist);
  }
  t.showAge("pcl nn search");



  t = Time::now();
  for(size_t i=0;i<nn.size();++i){
    nnres[i] = ot.nn_approx(nn[i]);
  }
  t.showAge("icl nn approx search");

#ifdef TRY_PCL_OCTREE
  t = Time::now();
  for(size_t i=0;i<nn.size();++i){
    int idx(0); float dist(0);
    pcl_ot.approxNearestSearch(pcl::PointXYZ(nn[i][0],nn[i][1],nn[i][2]), idx,dist);
  }
  t.showAge("pcl nn approx search");
#endif

  
  
  scene.addObject(&res);  
  //scene.addObject(&obj);
  scene.addObject(&ot);

  scene.addCamera(Camera(Vec(0,0,1500,1)));
  
  gui << Draw3D().handle("draw3D").minSize(32,24) << Show();
  
  gui["draw3D"].link(scene.getGLCallback(0));
  gui["draw3D"].install(scene.getMouseHandler(0));
}

void run(){
  gui["draw3D"].render();
}

int main(int n, char **v){
  return ICLApp(n,v,"",init,run).exec();
}
