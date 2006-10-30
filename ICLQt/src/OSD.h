#ifndef ______OSD_H
#define ______OSD_H

#include "OSDWidget.h"
#include "OSDButton.h"
#include "OSDLabel.h"
#include "OSDSlider.h"
#include "OSDNavBar.h"

namespace icl{
 
  /// Complex Widgets, that is used in the ICLWidget
  class OSD : public OSDWidget{
    typedef std::map<std::string,OSDWidget*> pmap;
    public:
    OSD(int id, Rect r,ImageWidget* poIW , OSDWidget *poParent);

    int getCurrID();
    void setCurrID(int id);

    protected:
    virtual void childChanged(int id, void *val);
    void setActive(std::string sName);
    void setImageInfo(std::vector<std::string> info);

    /* {{{ help functions */

    Rect getNavRect();
    Rect getMainRect();
    Rect getSubRect(int i, int iMax);
    Rect getBottomRectMain();
    Rect getBottomRectExit();

    /* }}} */
    /* {{{ members */

    OSDNavBar *m_poNavBar;
    OSDButton *m_poMain, *m_poExit;
    pmap m_mapPanels;
    int m_iCurrID;

    /* }}} */
    public:
    /* {{{ NAV constants */

    static const int NAV_ID = 100;
    static const int NAV_MAIN_ID = 101;
    static const int NAV_ADJUST_ID = 102;  
    static const int NAV_FITMODE_ID = 103;  
    static const int NAV_CHANNELS_ID = 104;
    static const int NAV_CAPTURE_ID = 105;
    static const int NAV_INFO_ID = 106;
    static const int NAV_MENU_ID = 107;

    /* }}} */
    /* {{{ MAIN constants */

    static const int MAIN_ID = 200;
    static const int MAIN_ADJUST_ID = 201;
    static const int MAIN_FITMODE_ID = 202;
    static const int MAIN_CHANNELS_ID = 203;
    static const int MAIN_CAPTURE_ID = 204;
    static const int MAIN_INFO_ID = 205;
    static const int MAIN_MENU_ID = 206;

    /* }}} */
    /* {{{ ADJUST constants */

    static const int ADJUST_ID = 300;
    static const int ADJUST_MODE_ID = 301;
    static const int ADJUST_MODE_NONE_ID = 302;
    static const int ADJUST_MODE_MANUAL_ID = 303;
    static const int ADJUST_MODE_AUTO_ID = 304;
    static const int ADJUST_BRIGHTNESS_LABEL_ID = 310;
    static const int ADJUST_BRIGHTNESS_SLIDER_ID = 311;
    static const int ADJUST_CONTRAST_LABEL_ID = 312;
    static const int ADJUST_CONTRAST_SLIDER_ID = 313;
    static const int ADJUST_INTENSITY_LABEL_ID = 314;
    static const int ADJUST_INTENSITY_SLIDER_ID = 315;

    /* }}} */
    /* {{{ FITMODE constants */

    static const int FITMODE_ID = 400;
    static const int FITMODE_NOSCALE_ID = 401;
    static const int FITMODE_HOLDAR_ID = 402;
    static const int FITMODE_FIT_ID = 403;

    /* }}} */

    static const int CHANNELS_ID = 500;
    static const int CHANNELS_SLIDER_ID = 501;
    static const int CHANNELS_LABEL_ID = 502;
    
    
    static const int CAPTURE_ID = 600;
    static const int CAPTURE_LABEL_ID = 600;
    static const int INFO_ID = 700;
    
    /* {{{ MENU constants */

    static const int MENU_ID = 800;
    static const int MENU_FILL_RGB_LABEL_ID = 801;
    static const int MENU_FILL_R_SLIDER_ID = 802;
    static const int MENU_FILL_G_SLIDER_ID = 803;
    static const int MENU_FILL_B_SLIDER_ID = 804;
    
    static const int MENU_BORDER_RGB_LABEL_ID = 805;
    static const int MENU_BORDER_R_SLIDER_ID = 806;
    static const int MENU_BORDER_G_SLIDER_ID = 807;
    static const int MENU_BORDER_B_SLIDER_ID = 808;
    
    static const int MENU_ALPHA_LABEL_ID = 809;
    static const int MENU_ALPHA_SLIDER_ID = 810;
    /* }}} */
    /* {{{ BOTTOM constants */
    static const int BOTTOM_MAIN_ID = 1000;
    static const int BOTTOM_EXIT_ID = 1001;
    /* }}} */
    /* {{{ OTHER constants */

    static const int GAP = 2;
    static const int MARGIN = 2;
    static const int NAV_H = 18;
    static const int BOTTOM_H = 18;
    static const int BOTTOM_W = 50;

    /* }}} */
  };
} // namespace icl

#endif


/*
channels
  [single|rgb]

  [0|1|2|...N]

info
  [full|light|none]

  "info info info"
  "info info info"
  "info info info"
  "info info info"
*/
