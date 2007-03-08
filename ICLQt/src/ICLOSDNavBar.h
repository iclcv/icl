#include <ICLOSDWidget.h>
#include <ICLOSDButton.h>
#ifndef OSDNAVBAR_H
#define OSDNAVBAR_H


namespace icl{
  /// Implementation of a so called "Nav-Bar" widget
  /** The nav-bar widget contains a count of buttons alligned on a
      horizontal line. One of these buttons is selected (like a
      group of radio buttons), all others are deselected. The selected
      Button is expaned, and its text is replace by a "full-text" and
      visualized <em>glowing</em>. Each contained button has
      an own ID, short-text and expaned-text.
  */
  class OSDNavBar : public OSDWidget{
    typedef std::vector<std::string> svec;
    typedef std::vector<int> ivec;

    public:
    OSDNavBar(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent,svec vecNames, svec vecShortNames, ivec vecIDs, int selectedIndex=0);
    void selectIndex(int index);
    virtual void mousePressed(int _x, int _y, int button);
      
    protected:

    static const int s_iGap = 2;
    static const int s_iMargin = 0;

    int m_iCurrIndex;
    int m_iSmallW;
    svec m_vecNames,m_vecShortNames;
    
  };

}// namespace icl

#endif
