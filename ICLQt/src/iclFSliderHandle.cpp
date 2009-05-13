#include <iclFSliderHandle.h>
#include <QSlider>

namespace icl{
  FSliderHandle::FSliderHandle():
    GUIHandle<QSlider>(),m_fMin(0),m_fMax(0),m_fM(0),m_fB(0),m_iSliderRange(0){
  }
  FSliderHandle::FSliderHandle(QSlider *sl,float *minV, float *maxV, float *M, float *B,int range, GUIWidget *w):
    GUIHandle<QSlider>(sl,w),m_fMin(minV),m_fMax(maxV),m_fM(M),m_fB(B),m_iSliderRange(range){
    updateMB();
  }
  void FSliderHandle::setMin(float min){
    *m_fMin = min;
    updateMB();
  }
  void FSliderHandle::setMax(float max){
    *m_fMax = max;
    updateMB();
  }

  void FSliderHandle::setValue(float val){
    (**this)->setValue(f2i(val));
  }

  void FSliderHandle::updateMB(){
    *m_fM = (*m_fMax-*m_fMin)/m_iSliderRange;
    *m_fB = *m_fMin;
  }
  
  float FSliderHandle::getMin() const{
    return *m_fMin;
  }
  float FSliderHandle::getMax() const{
    return *m_fMax;
  }
  float FSliderHandle::getValue() const{
    return i2f((**this)->value());
  }
}
