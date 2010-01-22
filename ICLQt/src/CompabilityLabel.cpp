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


