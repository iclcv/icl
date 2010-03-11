/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_MULTI_DRAW_HANDLE_H
#define ICL_MULTI_DRAW_HANDLE_H

#include <ICLQt/GUIHandle.h>
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
