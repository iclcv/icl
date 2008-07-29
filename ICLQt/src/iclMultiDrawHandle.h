#ifndef ICL_MULTI_DRAW_HANDLE_H
#define ICL_MULTI_DRAW_HANDLE_H

#include <iclGUIHandle.h>
#include <QTabBar>
#include <vector>
#include <map>
#include <QObject>

namespace icl{
  
  /** \cond */
  class ICLDrawWidget;
  class ImgBase;
  /** \endcond */

  /// Handle class for image components \ingroup HANDLES
  class MultiDrawHandle : public QObject, public GUIHandle<ICLDrawWidget>{
    Q_OBJECT

    public:
    
    /// Create an empty draw handle
    MultiDrawHandle();

    /// create a new ImageHandel
    MultiDrawHandle(ICLDrawWidget *w, QTabBar *t,std::vector<ImgBase*> *imageBuffer,  bool bufferAll, bool bufferDeeply, GUIWidget *guiw);

    /// explicit copy constructor
    MultiDrawHandle(const MultiDrawHandle &other);

    ~MultiDrawHandle();

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

    public slots:
    void tabChanged(int idx);
    

    private:

    std::vector<ImgBase*> *m_imageBuffer;
    QTabBar *m_tabBar;
    std::map<std::string,int> m_map;
    bool m_bufferAll;
    bool m_bufferDeeply;
  };
  
}

#endif
