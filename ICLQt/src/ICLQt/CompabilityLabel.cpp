/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/CompabilityLabel.cpp                   **
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

#include <ICLQt/CompabilityLabel.h>
#include <QPainter>

namespace icl{
  namespace qt{

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


  } // namespace qt
}


