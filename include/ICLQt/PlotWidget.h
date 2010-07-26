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

#include <QtOpenGL/QGLWidget>
#include <QtGui/QColor>
#include <vector>

#include <ICLUtils/Range.h>
#include <ICLUtils/SteppingRange.h>
#include <ICLQt/GUI.h>
#include <vector>

namespace icl{
  class PlotWidget : public QGLWidget{
    Q_OBJECT
    public:
    /// internal hidden data structure
    struct Data;
    
    /// Funtion type
    struct Function{
      virtual inline Range32f yrange(){ return Range32f(0,0); }
      virtual float operator()(float x)=0;
      
      std::vector<float> operator()(const std::vector<float> &xs);
      std::vector<float> &operator()(const std::vector<float> &xs, std::vector<float> &dst);
      
      virtual inline QColor getColor() { return QColor(255,0,0,0); }
    };
    
    
    PlotWidget(bool withControlGUI=true, QWidget *parent=0);
    ~PlotWidget();
    
    virtual void paintEvent(QPaintEvent *e);

    void addFunction(Function *f);
    void deleteFunction(Function *f);
    void removeFunction(Function *f);
    
    void deleteAllFunctions();
    void removeAllFunctions();

    /// by default 5 pixels
    void setMargins(int left, int top, int right, int bottom);
    void setXTicsSpacing(float val);
    void setYTicsSpacing(float val);
    void setXRange(const SteppingRange<icl32f> &xs);
    void setXValues(const std::vector<icl32f> &xs);
    
    public slots:

    void setFillModeEnabled(bool enabled);
    void setLogModeEnabled(bool enabled);
    void setAccuModeEnalbed(bool enabled);
    void setAAMode(bool enabled);
    
    /// how to blur functions (none, mean3, mean5, median)
    void setBlurModeEnabled(const QString &mode);
    
    void setControlGUIEnabled(bool enabled);

    void selectFunction(int index);
    void deselectFunction();
    
    void setXTicsEnabled(bool enabled);
    void setYTicsEnabled(bool enabled);
    void setTicLablesEnabled(bool enabled);
    
    private:
    Data *m_data;
  };
}

#endif
