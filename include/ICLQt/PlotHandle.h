/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/PlotHandle.h                             **
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

#ifndef ICL_PLOT_HANDLE_H
#define ICL_PLOT_HANDLE_H

#include <ICLQt/GUIHandle.h>
#include <ICLQt/PlotWidget.h>

namespace icl{
  
  /// Handle class for image components \ingroup HANDLES
  class PlotHandle : public GUIHandle<PlotWidget>{
    public:
    /// Create an empty handle
    PlotHandle(){}
    
    /// create a new ImageHandel
    PlotHandle(PlotWidget *w, GUIWidget *guiw):GUIHandle<PlotWidget>(w,guiw){}
    
    /// calles updated internally
    void update();

    // todo: implement several set data method for more convenience

    /// callback registration is not supported for this compoment
    virtual void registerCallback(const GUI::Callback &cb, const std::string &events="all"){
      throw ICLException("PlotHandle::registerCallback: you cannot register" 
                         " Callbacks to this component");
    }

    /// complex callbacks are not allowed for image-components (this method will throw an exception)
    virtual void registerCallback(const GUI::ComplexCallback&, const std::string &){
      throw ICLException("PlotHandle::registerCallback: you cannot register "
                         "GUI::ComplexCallback instances to an image GUI component");
    }
  };
  
}

#endif
