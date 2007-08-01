#include <iclSzene.h>
#include <iclStackTimer.h>

namespace icl{
  Szene::Szene(const Rect &viewPort,const Camera &cam):
    m_oViewPort(viewPort),m_oCam(cam){}
  
  inline Mat Szene::getViewPortMatrix() const{
    //float dx = m_oViewPort.width/2;
    //float dy = m_oViewPort.height/2;

    float dx = (m_oViewPort.left()+m_oViewPort.right())/2;
    float dy = (m_oViewPort.top()+m_oViewPort.bottom())/2;
    float slope = std::min(m_oViewPort.width/2,m_oViewPort.height/2);
    return  Mat ( slope , 0     , 0 , dx,
                  0     , slope , 0 , dy,
                  0     , 0     , 0 , 0 ,
                  0     , 0     , 0 , 1 );
  }
  
  void Szene::update(){
    Mat C = m_oCam.getTransformationMatrix();
    Mat V = getViewPortMatrix();
    Mat T = V*C;

    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->project(T);
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

  void Szene::transformAllObjs(const Mat &m){
    for(unsigned int i=0;i<m_vecObjs.size();i++){
      m_vecObjs[i]->transform(m);
    }
  }
}


