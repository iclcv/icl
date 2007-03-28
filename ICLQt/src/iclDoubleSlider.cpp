#include "iclDoubleSlider.h"

#include <QSlider>
#include <QLabel>
#include <QGridLayout>


using namespace std;

namespace icl{
  DoubleSlider::DoubleSlider(QWidget *parent, const QString &id): QWidget(parent),m_qsID(id){
    m_poSlider = new QSlider(this);
    m_poSlider->setMinimum(0);
    m_poSlider->setMaximum(1000);    
    m_poSlider->setOrientation(Qt::Horizontal);
    m_poLabel = new QLabel(this);
    
    m_poLayout = new QGridLayout(this);
    m_poLayout->addWidget(m_poSlider,0,0,1,5); // y,x ??
    m_poLayout->addWidget(m_poLabel,0,5,1,1); // y,x ??
    
    connect(m_poSlider,SIGNAL(sliderMoved(int)),this,SLOT(receiveValueChanged(int)));
    m_poLayout->setMargin(1);
    setLayout(m_poLayout);
    setMinimumWidth(300);
  }
  
  double DoubleSlider::getDoubleValue(){
    int i = m_poSlider->value(); 
    double srcRange = 1000;
    double dstRange = m_dMax-m_dMin;
    double val = (dstRange/srcRange)*i + m_dMin;
    m_poLabel->setNum(val);
    return val;
  }
  
  void DoubleSlider::receiveValueChanged(int i){
    emit doubleValueChanged(m_qsID,getDoubleValue());  
  }
  
  void DoubleSlider::setDoubleValue(double d){
    double r = m_dMax-m_dMin;
    double m = 1000.0/r;
    double b = -m*m_dMin;
    m_poLabel->setNum(d);
    m_poSlider->setValue((int)(m*d+b));;    
  }
  
  void DoubleSlider::setMinDouble(double dmin){
    m_dMin = dmin;    
  }
  void DoubleSlider::setMaxDouble(double dmax){
    m_dMax = dmax;    
  }
}

