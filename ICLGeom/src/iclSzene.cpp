#include <iclSzene.h>
#include <iclStackTimer.h>

namespace icl{
  Szene::Szene(const Rect &viewPort,const Camera &cam):
    m_oViewPort(viewPort),m_oCam(cam),m_oTransMat(Mat::id()){}
  
  Mat Szene::getViewPortMatrix() const{
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
  void Szene::update(){
    Mat C = m_oCam.getCoordinateSystemTransformationMatrix();
    Mat P = m_oCam.getProjectionMatrix();
    Mat V = getViewPortMatrix();
    Mat T = m_oTransMat;

    Mat A = V*P*C*T;
    
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->project(A);
    }
  }
  
  void Szene::render(ICLDrawWidget *w) const{
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->render(w);
    }
  }
  void Szene::render(Img32f *image) const{
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->render(image);
    }            
  }
  
  void Szene::add(Object *obj){
    m_vecObjs.push_back(obj);
  }
  
  void Szene::remove(Object *obj){
    std::vector<Object*>::iterator it = find(m_vecObjs.begin(),m_vecObjs.end(),obj);
    if(it != m_vecObjs.end()){
      m_vecObjs.erase(it);
    }
  }
  
  void Szene::transformAllObjs(const Mat &m){
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->transform(m);
    }
  }

  void Szene::showMatrices(const std::string &title) const{
    Mat C = m_oCam.getCoordinateSystemTransformationMatrix();
    Mat P = m_oCam.getProjectionMatrix();
    Mat V = getViewPortMatrix();
    Mat T = m_oTransMat;
    
    printf("ICLGeom::Szene \"%s\" \n",title.c_str());
    // C.show("camera coordinate system transformation matrix (C)");
    //m_oTransMat.show("transformationmatrix (T)");
    //V.show("view port matrix (V)");
    //printf("--\n");
    (C*T).show("modelview-matrix");
    (V*P).show("projection-matrix");
    printf("----------------------------------\n");
  }
}


