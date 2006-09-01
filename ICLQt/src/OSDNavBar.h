#ifndef OSDNAVBAR_H
#define OSDNAVBAR_H

#include "OSDWidget.h"
#include "OSDButton.h"

namespace icl{

  class OSDNavBar : public OSDWidget{
    typedef std::vector<QString> svec;
    typedef std::vector<int> ivec;

    public:
    OSDNavBar(int id, QRect r,ImageWidget* poIW , OSDWidget *poParent,svec vecNames, svec vecShortNames, ivec vecIDs, int selectedIndex=0);
    void selectIndex(int index);
    virtual void mousePressed(int _x, int _y, int button);
      
    protected:

    static const int s_iGap = 2;
    static const int s_iMargin = 0;
    //    static const int s_iSmallW = 32;
    
    int m_iCurrIndex;
    int m_iSmallW;
    svec m_vecNames,m_vecShortNames;
    
  };

}// namespace icl

#endif
