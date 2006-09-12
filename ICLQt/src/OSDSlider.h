#ifndef OSDSLIDER_H
#define OSDSLIDER_H

#include "OSDWidget.h"

namespace icl{
  
  class OSDSlider : public OSDWidget{
    public:
    OSDSlider(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent,int min, int max, int curr);
    virtual ~OSDSlider(){}
    virtual void drawSelf(GLPaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    virtual void mouseMoved(int _x, int _y, int downmask[3]);
    virtual void mousePressed(int _x, int _y, int button);
    virtual void mouseReleased(int x, int y, int button);    
    int mxb(int x, int x_min, int x_max, int y_min, int y_max);
    int valToPos(int val);
    int posToVal(int pos);
    
    protected:
    int m_iMin, m_iMax,m_iCurr,m_iGripY,m_iGripH;
    string m_sText;
    
    Rect m_oBar,m_oText,m_oGrip;
    
    static const float s_fTextFrac;
    static const float s_13;
    static const int s_iGripW;
       
    bool m_iSliderHovered;
  };
  
} // namespace icl

#endif
