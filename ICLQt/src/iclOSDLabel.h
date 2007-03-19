#ifndef OSDLABEL_H
#define OSDLABEL_H

#include <iclOSDWidget.h>
#include <vector>
#include <QString>

namespace icl{
  
  /// Implementation of a ordinary label widget
  class OSDLabel : public OSDWidget{
    public:
    OSDLabel(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent, std::string sText);
    virtual void drawSelf(PaintEngine *e,int x, int y,int mouseOver,int mouseOverChild, int downmask[3]);
    void setMultiText(std::vector<std::string> smultitext);
    
    protected:
    std::string m_sText;

    int m_iContainsMultiText;
    std::vector<std::string> m_vecMultiText;
  };
} // namespace

#endif
