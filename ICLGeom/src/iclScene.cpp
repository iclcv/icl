#include <iclScene.h>
#include <iclStackTimer.h>

namespace icl{
  Scene::Scene(const Rect &viewPort,const Camera &cam):
    m_oViewPort(viewPort),m_oCam(cam),m_oTransMat(Mat::id()){}
  
  Scene::~Scene(){
    for(unsigned int i=0;i<m_vecObjs.size();++i){
      delete m_vecObjs[i];
    }
  }
  
  Scene &Scene::operator=(const Scene &other){
    m_oViewPort = other.m_oViewPort;
    m_oCam = other.m_oCam;

    for(unsigned int i=0;i<m_vecObjs.size();++i){
      delete m_vecObjs[i];
    }
    m_vecObjs.clear();
    m_oTransMat = other.m_oTransMat;
    return *this;
  }

  Mat Scene::getViewPortMatrix() const{
    //float dx = m_oViewPort.width/2;
    //float dy = m_oViewPort.height/2;

    float dx = (m_oViewPort.left()+m_oViewPort.right())/2;
    float dy = (m_oViewPort.top()+m_oViewPort.bottom())/2;
    float slope = iclMin(m_oViewPort.width/2,m_oViewPort.height/2);
    return  Mat ( slope , 0     , 0 , dx,
                  0     , slope , 0 , dy,
                  0     , 0     , 0 , 0 ,
                  0     , 0     , 0 , 1 );
  }

  // old     Mat C = m_oCam.getTransformationMatrix();
  void Scene::update(){
    Mat C = m_oCam.getCoordinateSystemTransformationMatrix();
    Mat P = m_oCam.getProjectionMatrix();
    Mat V = getViewPortMatrix();
    Mat T = m_oTransMat;

    Mat A = V*P*C*T;
    
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->project(A);
    }
  }
  
  void Scene::render(ICLDrawWidget *w) const{
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->render(w);
    }
  }
  void Scene::render(Img32f *image) const{
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->render(image);
    }            
  }
  
  void Scene::add(Object *obj){
    m_vecObjs.push_back(obj);
  }
  
  void Scene::remove(Object *obj){
    std::vector<Object*>::iterator it = find(m_vecObjs.begin(),m_vecObjs.end(),obj);
    if(it != m_vecObjs.end()){
      delete *it;
      m_vecObjs.erase(it);
    }
  }
  
  void Scene::transformAllObjs(const Mat &m){
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->transform(m);
    }
  }

  void Scene::showMatrices(const std::string &title) const{
    Mat C = m_oCam.getCoordinateSystemTransformationMatrix();
    Mat P = m_oCam.getProjectionMatrix();
    Mat V = getViewPortMatrix();
    Mat T = m_oTransMat;
    
    printf("ICLGeom::Scene \"%s\" \n",title.c_str());
    // C.show("camera coordinate system transformation matrix (C)");
    //m_oTransMat.show("transformationmatrix (T)");
    //V.show("view port matrix (V)");
    //printf("--\n");
    std::cout << "modelview-matrix:" << std::endl << (C*T) << std::endl;
    std::cout << "projection-matrix:" << std::endl << (V*P) << std::endl;
    printf("----------------------------------\n");
  }
}


