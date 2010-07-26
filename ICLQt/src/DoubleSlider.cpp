/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/DoubleSlider.cpp                             **
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

#include <ICLQt/DoubleSlider.h>

#include <QtGui/QSlider>
#include <QtGui/QLabel>
#include <QtGui/QGridLayout>
#include <math.h>

using namespace std;

namespace icl{
  DoubleSlider::DoubleSlider(QWidget *parent, const QString &id): QWidget(parent),m_qsID(id){
    m_poSlider = new QSlider(this);
    m_poSlider->setMinimum(0);
    m_poSlider->setMaximum(10000);    
    m_poSlider->setOrientation(Qt::Horizontal);
    m_poLabel = new QLabel(this);
    
    m_poLayout = new QGridLayout(this);
    m_poLayout->addWidget(m_poSlider,0,0,1,5); // y,x ??
    m_poLayout->addWidget(m_poLabel,0,5,1,1); // y,x ??

    m_dStepping = 0;
    
    connect(m_poSlider,SIGNAL(valueChanged(int)),this,SLOT(receiveValueChanged(int)));
    m_poLayout->setMargin(1);
    setLayout(m_poLayout);
    setMinimumWidth(300);


  }
  
  double DoubleSlider::getDoubleValue(){
    int i = m_poSlider->value(); 
    double srcRange = 10000;
    double dstRange = m_dMax-m_dMin;
    double val = (dstRange/srcRange)*i + m_dMin;
    
    if(m_dStepping){
      int stepIdx = (int)round((val-m_dMin)/m_dStepping);
      val = m_dMin+stepIdx*m_dStepping;
    } 
    
    m_poLabel->setNum(val);
    return val;
  }
  
  void DoubleSlider::receiveValueChanged(int i){
    emit doubleValueChanged(m_qsID,getDoubleValue());  
  }
  
  void DoubleSlider::setDoubleValue(double d){
    double r = m_dMax-m_dMin;
    double m = 10000.0/r;
    double b = -m*m_dMin;
    m_poLabel->setNum(d);
    m_poSlider->setValue((int)(m*d+b));;    
  }

  void DoubleSlider::setDoubleStepping(double s){
    double r = m_dMax-m_dMin;
    static const double ri = 10000;
    
    int interval =   (int)round(r/ri *s);
    if(interval < 100) interval *=10;
    if(interval < 100) interval *=10;
    
    m_poSlider->setTickInterval(interval);
    m_poSlider->setSingleStep(interval);
    m_poSlider->setTickPosition(QSlider::TicksBelow);

    m_dStepping = s;
  }
  
  void DoubleSlider::setMinDouble(double dmin){
    m_dMin = dmin;    
  }
  void DoubleSlider::setMaxDouble(double dmax){
    m_dMax = dmax;    
  }
}

