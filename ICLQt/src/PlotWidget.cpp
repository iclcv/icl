/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/PlotWidget.cpp                               **
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

#include <ICLQt/PlotWidget.h>

namespace icl{
  struct PlotWidget::Data{

  };
  
  PlotWidget::PlotWidget(QWidget *parent):
    LowLevelPlotWidget(parent),m_data(new Data){
  }
  
  PlotWidget::~PlotWidget(){
    delete m_data;
  }
  
  void PlotWidget::color(int r, int g, int b, int a=255){
    
  }
  
  void PlotWidget::fill(int r, int g, int b, int a=255){
  
  }

  void PlotWidget::sym(char s){
  
  }
  
  void PlotWidget::linewidth(float width){
  
  }
  
  void PlotWidget::symsize(float size){
  
  }
  
  void PlotWidget::pointsize(float size){
  
  }
  
  template<class T>
  void PlotWidget::scatter(const T *xs, const T *ys, int num, int xStride, int yStride){
    
  }
  /// TODO instantiate!

  template<class T>
  void PlotWidget::series(const T *data, int num, int stride){
  
  }

  template<class T>
  void PlotWidget::bars(const T *data, int num, int stride){
  
  }

  void PlotWidget::point(){
  
  }
  void PlotWidget::points(){}
  void PlotWidget::line(){}
  void PlotWidget::lines(){}
  void PlotWidget::linestrip(){}
  void PlotWidget::triangle(){}
  void PlotWidget::rect(){}
  void PlotWidget::text(){}
  void PlotWidget::fontsize(){}
  void PlotWidget::ellipse(){}
  void PlotWidget::circle(){}
  void PlotWidget::arrow(){}

}
