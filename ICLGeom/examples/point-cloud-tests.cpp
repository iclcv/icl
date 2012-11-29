#include <ICLQt/Common.h>
#include <ICLGeom/Geom.h>
#include <ICLUtils/Random.h>

struct RandPos{
  URand r;
  RandPos():r(-1000,1000){}
  operator Vec() const{
    return Vec(r,r,r,1);
  }
};

struct RandNormal{
  URand r;
  RandNormal():r(0,2*M_PI){}
  operator Vec() const{
    Mat m = create_hom_4x4<float>(r,r,r);
    Vec c = m * Vec(1,0,0,1);
    return c;
  }
};

std::vector<PointCloudObjectBase*> create_clouds(){
  Size size(20,10);
  std::vector<PointCloudObjectBase*> clouds(13);
  
  clouds[0] = new PCLPointCloudObject<pcl::PointXYZ>(size.width,size.height);
  clouds[1] = new PCLPointCloudObject<pcl::PointXYZI>(size.width,size.height);
  clouds[2] = new PCLPointCloudObject<pcl::PointXYZL>(size.width,size.height);
  clouds[3] = new PCLPointCloudObject<pcl::PointXYZRGB>(size.width,size.height);
  clouds[4] = new PCLPointCloudObject<pcl::PointXYZRGBA>(size.width,size.height);
  clouds[5] = new PCLPointCloudObject<pcl::PointXYZRGBL>(size.width,size.height);
  clouds[6] = new PCLPointCloudObject<pcl::InterestPoint>(size.width,size.height);
  clouds[7] = new PCLPointCloudObject<pcl::PointXYZRGBNormal>(size.width,size.height);
  clouds[8] = new PCLPointCloudObject<pcl::PointXYZINormal>(size.width,size.height);
  clouds[9] = new PointCloudObject(size.width,size.height,true,false,false);
  clouds[10] = new PointCloudObject(size.width,size.height,true,true,false);
  clouds[11] = new PointCloudObject(size.width,size.height,true,false,true);
  clouds[12] = new PointCloudObject(size.width,size.height,true,true,true);

  PointCloudObjectBase *copies[13];
  URand ra(0,255),rb(0,1),rc(-1000,1000);
  RandPos rp;
  RandNormal rn;
 
  for(int i=0;i<13;++i){
    if(clouds[i]->supports(PointCloudObjectBase::XYZH)) clouds[i]->selectXYZH().fill(rp);
    else if(clouds[i]->supports(PointCloudObjectBase::XYZ)) clouds[i]->selectXYZ().fillScalar(rc);

    if(clouds[i]->supports(PointCloudObjectBase::Normal)) clouds[i]->selectNormal().fill(rn);
    
    if(clouds[i]->supports(PointCloudObjectBase::Label)) {
      clouds[i]->selectLabel().fillScalar(ra);
      clouds[i]->selectLabel().fill(ra); // should be equal
    }

    if(clouds[i]->supports(PointCloudObjectBase::Intensity)) {
      clouds[i]->selectIntensity().fillScalar(ra);
      clouds[i]->selectIntensity().fill(ra); // should be equal
    }
    
    if(clouds[i]->supports(PointCloudObjectBase::BGRA)) clouds[i]->selectBGRA().fillScalar(ra);
    else if(clouds[i]->supports(PointCloudObjectBase::BGR)) clouds[i]->selectBGR().fillScalar(ra);
    else if(clouds[i]->supports(PointCloudObjectBase::RGBA32f)) clouds[i]->selectRGBA32f().fillScalar(rb);
  }
  return clouds;
}
void free_clouds(std::vector<PointCloudObjectBase*> &clouds){
  for(size_t i=0;i<clouds.size();++i){
    ICL_DELETE(clouds[i]);
  }
}

void test_point_cloud_deep_copy(){
  std::vector<PointCloudObjectBase*> clouds = create_clouds();
  std::vector<PointCloudObjectBase*> copies(clouds.size());

  for(size_t i=0;i<clouds.size();++i){
    copies[i] = clouds[i]->copy();
    std::cout << "  checking point cloud " << i << ".... copy(): ";
    if(clouds[i]->equals(*copies[i])){
      std::cout << "OK!";
    }else{
      std::cout << "ERROR!";
    }

    clouds[i]->deepCopy(*clouds[i]);
    std::cout << "  ... deepCopy(dst) ...";
    if(clouds[i]->equals(*copies[i])){
      std::cout << "OK!" << std::endl;
    }else{
      std::cout << "ERROR!" << std::endl;
    }
  }
  free_clouds(clouds);
  free_clouds(copies);
}

void test_point_cloud_cross_copy(){
  std::vector<PointCloudObjectBase*> clouds = create_clouds();
  std::vector<PointCloudObjectBase*> copies = create_clouds();
  for(int i=0;i<clouds.size();++i){
    for(int j=0;j<copies.size();++j){
      std::cout << "  checking cross copy " << i << "-->" << j <<  " ...";
      clouds[i]->deepCopy(*copies[j]);
      if(clouds[i]->equals(*copies[j],true,true,1)){
        std::cout << "OK!" << std::endl;
      }else{
        std::cout << "ERROR!" << std::endl;
      }
    }
  }
  // compare(true,true)
    
}

int main(){
  test_point_cloud_deep_copy();
  test_point_cloud_cross_copy();
}
