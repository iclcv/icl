/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/DrawHandle3D.h                         **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLQt/GUIHandle.h>

namespace icl{
  /** \cond */
  namespace core{ class ImgBase; }
  /** \endcond */

  namespace qt{

    /** \cond */
    class ICLDrawWidget3D;
    /** \endcond */

    /// Handle class for image components \ingroup HANDLES
    class ICLQt_API DrawHandle3D : public GUIHandle<ICLDrawWidget3D>{
      public:

      /// Create an empty handle
      DrawHandle3D(){}

      /// create a new ImageHandel
      DrawHandle3D(ICLDrawWidget3D *w, GUIWidget *guiw):GUIHandle<ICLDrawWidget3D>(w,guiw){}

      /// make the wrapped ICLWidget show a given image
      void setImage(const core::ImgBase *image);

      /// make the wrapped ICLWidget show a given image (as set Image)
      void operator=(const core::ImgBase *image) { setImage(image); }

      /// re-renders the widget
      void render();

      /// passes callback registration to wrapped ICLWidget instance)
      virtual void registerCallback(const GUI::Callback &cb, const std::string &events="all");

      /// complex callbacks are not allowed for image-components (this method will throw an exception)
      virtual void registerCallback(const GUI::ComplexCallback&, const std::string &){
        throw utils::ICLException("ImageHandle::registerCallback: you cannot register "
                           "GUI::ComplexCallback instances to an image GUI component");
      }

      /// passes callback registration to wrapped ICLWidget instance)
      virtual void removeCallbacks();

    };

  } // namespace qt
}

