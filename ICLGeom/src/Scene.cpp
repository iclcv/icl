#include <ICLGeom/Scene.h>
#include <ICLUtils/StackTimer.h>

namespace icl{
  Scene::Scene(const Camera &cam):
    m_cam(cam),m_transMat(Mat::id()){}
  
  Scene::~Scene(){
    for(unsigned int i=0;i<m_objs.size();++i){
      delete m_objs[i];
    }
  }
  
  Scene &Scene::operator=(const Scene &other){
    m_cam = other.m_cam;

    for(unsigned int i=0;i<m_objs.size();++i){
      delete m_objs[i];
    }
    m_objs.clear();
    m_transMat = other.m_transMat;
    return *this;
  }

 

  // old     Mat C = m_cam.getTransformationMatrix();
  void Scene::update(){
    Mat C = m_cam.getCoordinateSystemTransformationMatrix();
    Mat P = m_cam.getProjectionMatrix();
    Mat V = m_cam.getViewPortMatrix();
    Mat T = m_transMat;

    Mat A = V*P*C*T;
    
    for(unsigned int i=0;i<m_objs.size();i++){
      m_objs[i]->project(A);
    }
  }
  
#ifdef HAVE_QT
  void Scene::render(ICLDrawWidget *w) const{
    for(unsigned int i=0;i<m_objs.size();i++){
      m_objs[i]->render(w);
    }
  }
#endif

  void Scene::render(Img32f *image) const{
    for(unsigned int i=0;i<m_objs.size();i++){
      m_objs[i]->render(image);
    }            
  }
  
  void Scene::add(Object *obj){
    m_objs.push_back(obj);
  }
  
  void Scene::remove(Object *obj){
    std::vector<Object*>::iterator it = find(m_objs.begin(),m_objs.end(),obj);
    if(it != m_objs.end()){
      delete *it;
      m_objs.erase(it);
    }
  }
  
  void Scene::transformAllObjs(const Mat &m){
    for(unsigned int i=0;i<m_objs.size();i++){
      m_objs[i]->transform(m);
    }
  }

  void Scene::showMatrices(const std::string &title) const{
    Mat C = m_cam.getCoordinateSystemTransformationMatrix();
    Mat P = m_cam.getProjectionMatrix();
    Mat V = m_cam.getViewPortMatrix();
    Mat T = m_transMat;
    
    printf("ICLGeom::Scene \"%s\" \n",title.c_str());
    // C.show("camera coordinate system transformation matrix (C)");
    //m_transMat.show("transformationmatrix (T)");
    //V.show("view port matrix (V)");
    //printf("--\n");
    std::cout << "modelview-matrix:" << std::endl << (C*T) << std::endl;
    std::cout << "projection-matrix:" << std::endl << (V*P) << std::endl;
    printf("----------------------------------\n");
  }
}


