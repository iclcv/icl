/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/HistogrammWidget.h                       **
** Module : ICLGeom                                                **
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

#ifndef ICL_HISTOGRAMM_WIDGET_H
#define ICL_HISTOGRAMM_WIDGET_H

#include <ICLQt/PlotWidget.h>
#include <ICLQt/ImageStatistics.h>

namespace icl{
  namespace qt{
    
    /// Utility class used within the ICLWidget class
    /** The HistogrammWidget is used in the 'info' tab of the ICLWidget's on screen display */
    struct HistogrammWidget : public PlotWidget{
      /// logarithmic mode
      bool logOn;
      
      /// mean averaging
      bool meanOn;
      
      /// median filtering
      bool medianOn;
      
      /// selected channel (or -1 for all)
      int selChannel;
  
      std::vector<float> buf;
  
      /// Base constructor with given parent widget
      HistogrammWidget(QWidget *parent);
      
      /// sets all features
      void setFeatures(bool logOn, bool meanOn, bool medianOn, int selChannel);
      
      /// updates the histogramms
      void updateData(const ImageStatistics &s);
    };
  
  
  } // namespace qt
}


#endif
