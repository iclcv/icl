#include <iclSzene.h>
#include <iclStackTimer.h>

namespace icl{
  Szene::Szene(const Size &imageSize){
    static const float f = -1; // default focal length
    static const Vec pos( 0,0,10,0);
    static const Vec norm( 0,0,-1,0);
    static const Vec up( 1,0,0,0);
    
    cam = Camera( pos, norm,up, f, imageSize);
  }

  void Szene::update(){
    Mat T = cam.getTransformationMatrix();
    for(unsigned int i=0;i<objs.size();i++){
      objs[i]->project(T);

    }
  }
  
  void Szene::render(ICLDrawWidget *w) const{
    for(unsigned int i=0;i<objs.size();i++){
      objs[i]->render(w);
    }
  }
  void Szene::render(Img32f *image) const{
    for(unsigned int i=0;i<objs.size();i++){
      objs[i]->render(image);
    }            
  }
  
  void Szene::add(Object *obj){
    objs.push_back(obj);
  }

  void Szene::transformAllObjs(const Mat &m){
    for(unsigned int i=0;i<objs.size();i++){
      objs[i]->transform(m);
    }
  }
}


