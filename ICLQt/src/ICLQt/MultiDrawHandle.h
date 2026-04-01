// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <ICLQt/GUIHandle.h>
#include <QTabBar>
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
      ICLQt_API MultiDrawHandle();

      /// create a new ImageHandel
      ICLQt_API MultiDrawHandle(ICLDrawWidget *w, QTabBar *t, std::vector<core::ImgBase*> *imageBuffer, bool bufferAll, bool bufferDeeply, GUIWidget *guiw);

      /// explicit copy constructor
      ICLQt_API MultiDrawHandle(const MultiDrawHandle &other);

      ICLQt_API ~MultiDrawHandle();

      class Assigner{
        public:
        MultiDrawHandle *d;
        int idx;

        ICLQt_API void setImage(const core::ImgBase *image);
        void operator=(const core::ImgBase *image){ setImage(image); }
        void operator=(const core::ImgBase &image){ setImage(&image); }
      };

      ICLQt_API Assigner operator[](int idx);
      ICLQt_API Assigner operator[](const std::string &name);

      /// calles updated internally
      ICLQt_API void render();
      ICLQt_API int getSelectedIndex();
      ICLQt_API int getNumTabs();
      ICLQt_API std::string getSelected();
      ICLQt_API bool isSelected(const std::string &text);

      public Q_SLOTS:
      ICLQt_API void tabChanged(int idx);


      private:

      std::vector<core::ImgBase*> *m_imageBuffer;
      QTabBar *m_tabBar;
      std::map<std::string,int> m_map;
      bool m_bufferAll;
      bool m_bufferDeeply;
    };

  } // namespace qt
}
