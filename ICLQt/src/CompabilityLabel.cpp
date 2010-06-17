/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/CompabilityLabel.cpp                         **
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

#include <ICLQt/CompabilityLabel.h>
#include <QPainter>

namespace icl{

  CompabilityLabel::CompabilityLabel(const QString &text, QWidget *parent):
    ThreadedUpdatableWidget(parent),m_sText(text),m_alignment(Qt::AlignCenter){
  }
  
  
  void CompabilityLabel::paintEvent(QPaintEvent *evt){
    m_oMutex.lock();
    QString t = m_sText;
    m_oMutex.unlock();

    //    QWidget::paintEvent(evt); do i need this explicitly ??
    QPainter p(this);
    p.setPen(QColor(0,0,0));
    p.drawText(QRect(0,0,width(),height()),m_alignment,t);
  }
  void CompabilityLabel::setNum(int i){
    m_oMutex.lock();
    m_sText = QString::number(i);
    m_oMutex.unlock();
  }
  void CompabilityLabel::setNum(float f){
    m_oMutex.lock();
    m_sText = QString::number(f);
    m_oMutex.unlock();
  }
  void CompabilityLabel::setText(const QString &text){
    m_oMutex.lock();
    m_sText = text;
    m_oMutex.unlock();
  }

  QString CompabilityLabel::text() const{
    QString retVal;
    m_oMutex.lock();
    retVal = m_sText;
    m_oMutex.unlock();
    return retVal;
  }

  Qt::Alignment CompabilityLabel::getAlignment() const{
    return m_alignment;
  }
  
  void CompabilityLabel::setAlignment(Qt::Alignment a){
    m_alignment = a;
  }


}


