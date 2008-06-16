#ifndef ICL_MULTI_DRAW_HANDLE
#define ICL_MULTI_DRAW_HANDLE

#include <iclGUIHandle.h>
#include <QTabBar>
#include <vector>
#include <map>

namespace icl{
  
  /** \cond */
  class ICLDrawWidget;
  class ImgBase;
  /** \endcond */

  /// Handle class for image components \ingroup HANDLES
  class MultiDrawHandle : public GUIHandle<ICLDrawWidget>{
    public:
    /// create a new ImageHandel
    MultiDrawHandle(ICLDrawWidget *w=0, QTabBar *t=0);
    
    class Assigner{
      public:
      MultiDrawHandle *d;
      int idx;

      void setImage(const ImgBase *image);
      void operator=(const ImgBase *image){ setImage(image); }
      void operator=(const ImgBase &image){ setImage(&image); }
    };
    
    Assigner operator[](int idx);
    Assigner operator[](const std::string &name);
    
    /// calles updated internally
    void update();
    int getSelectedIndex();
    int getNumTabs();
    std::string getSelected();
    bool isSelected(const std::string &text);

    private:

    QTabBar *m_tabBar;
    std::map<std::string,int> m_map;
  };
  
}

#endif
