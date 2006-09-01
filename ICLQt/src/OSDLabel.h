#ifndef OSDLABEL_H
#define OSDLABEL_H
#include "OSDWidget.h"

#include <vector>
#include <QString>

namespace icl{
  
  class OSDLabel : public OSDWidget{
    public:
    OSDLabel(int id, QRect r,ImageWidget* poIW , OSDWidget *poParent, QString sText);
    virtual void drawSelf(QPainter *poPainter,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    void setMultiText(std::vector<QString> smultitext);
    
    protected:
    QString m_sText;

    int m_iContainsMultiText;
    std::vector<QString> m_vecMultiText;
  };
} // namespace

#endif
