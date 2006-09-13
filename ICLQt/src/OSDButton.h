#ifndef OSDBUTTON_H
#define OSDBUTTON_H

#include "OSDWidget.h"

namespace icl{
  /// implementation of an ordinary OSD-button
  class OSDButton : public OSDWidget{
    public:
    OSDButton(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent, string sText, int iToggable=0);
    virtual void drawSelf(GLPaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    virtual void mousePressed(int _x, int _y, int button);
    void setText(string sText);
    void setToggled(int iToggled);
    
    protected:
    string m_sText;
    int m_iToggable;
    int m_iIsToggled;
  };
}

#endif
