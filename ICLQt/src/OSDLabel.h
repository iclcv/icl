#ifndef OSDLABEL_H
#define OSDLABEL_H
#include "OSDWidget.h"

#include <vector>
#include <QString>

namespace icl{
  
  /// Implementation of a ordinary label widget
  class OSDLabel : public OSDWidget{
    public:
    OSDLabel(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent, string sText);
    virtual void drawSelf(GLPaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    void setMultiText(std::vector<string> smultitext);
    
    protected:
    string m_sText;

    int m_iContainsMultiText;
    std::vector<string> m_vecMultiText;
  };
} // namespace

#endif
