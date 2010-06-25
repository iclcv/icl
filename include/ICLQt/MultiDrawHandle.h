/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/MultiDrawHandle.h                        **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

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
