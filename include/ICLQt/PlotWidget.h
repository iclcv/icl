/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/PlotWidget.h                             **
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

#ifndef ICL_PLOT_WIDGET_H
#define ICL_PLOT_WIDGET_H

#include <ICLQt/LowLevelPlotWidget.h>

namespace icl{

  class PlotWidget : public LowLevelPlotWidget{
    template<class T> T *getBuf(int minLen);
    
    public:
    PlotWidget(QWidget *parent=0);
    
    inline void lock() { LowLevelPlotWidget::lock(); }
    inline void unlock() { LowLevelPlotWidget::unlock(); }

    inline void reset() { clear(); }
    
    void color(int r, int g, int b, int a=255);
    void fill(int r, int g, int b, int a=255);
    void sym(char s);
    
    void nocolor();
    void nofill();
    void nosym();
    
    void linewidth(float width);
    void symsize(float size);
    void pointsize(float size);
    
    void scatter(float *xs, float *ys, int num, int xStride=1, int yStride=1);

    inline void scatter(const std::vector<Point32f> &ps){
      if(!ps.size()) return;
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2);
    }

    inline void scatter(const std::vector<Point32f> &ps){
      if(!ps.size()) return;
      scatter(&ps[0].x, &ps[0].y, ps.size(), 2, 2);
    }
    
    template<class T>
    void scatter(T *xs, T *ys, int num, 
    
    

  };

};

#endif
