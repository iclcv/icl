/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/MultiDrawHandle.h                      **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#include <ICLQt/GUIHandle.h>
#include <QtGui/QTabBar>
#include <vector>
#include <map>
#include <QtCore/QObject>

namespace icl{

  /** \cond */
  namespace core{ class ImgBase; }
  /** \endcond */

  namespace qt{
    
    /** \cond */
    class ICLDrawWidget;
    /** \endcond */
  
    /// Handle class for image components \ingroup HANDLES
    class MultiDrawHandle : public QObject, public GUIHandle<ICLDrawWidget>{
      Q_OBJECT
  
      public:
      
      /// Create an empty draw handle
      ICL_QT_API MultiDrawHandle();
  
      /// create a new ImageHandel
      ICL_QT_API MultiDrawHandle(ICLDrawWidget *w, QTabBar *t, std::vector<core::ImgBase*> *imageBuffer, bool bufferAll, bool bufferDeeply, GUIWidget *guiw);
  
      /// explicit copy constructor
      ICL_QT_API MultiDrawHandle(const MultiDrawHandle &other);
  
      ICL_QT_API ~MultiDrawHandle();
  
      class Assigner{
        public:
        MultiDrawHandle *d;
        int idx;
  
        ICL_QT_API void setImage(const core::ImgBase *image);
        void operator=(const core::ImgBase *image){ setImage(image); }
        void operator=(const core::ImgBase &image){ setImage(&image); }
      };
      
      ICL_QT_API Assigner operator[](int idx);
      ICL_QT_API Assigner operator[](const std::string &name);
      
      /// calles updated internally
      ICL_QT_API void render();
      ICL_QT_API int getSelectedIndex();
      ICL_QT_API int getNumTabs();
      ICL_QT_API std::string getSelected();
      ICL_QT_API bool isSelected(const std::string &text);
  
      public slots:
      ICL_QT_API void tabChanged(int idx);
      
  
      private:
  
      std::vector<core::ImgBase*> *m_imageBuffer;
      QTabBar *m_tabBar;
      std::map<std::string,int> m_map;
      bool m_bufferAll;
      bool m_bufferDeeply;
    };
    
  } // namespace qt
}

