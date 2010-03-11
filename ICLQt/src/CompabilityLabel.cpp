/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLQt module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

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


