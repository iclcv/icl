#ifndef OSDBUTTON_H
#define OSDBUTTON_H

#include "OSDWidget.h"

namespace icl{
  class OSDButton : public OSDWidget{
    public:
    OSDButton(int id, QRect r,ImageWidget* poIW , OSDWidget *poParent, QString sText, int iToggable=0);
    virtual void drawSelf(QPainter *poPainter,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    virtual void mousePressed(int _x, int _y, int button);
    void setText(QString sText);
    void setToggled(int iToggled);
    
    protected:
    QString m_sText;
    int m_iToggable;
    int m_iIsToggled;
  };
}

#endif
