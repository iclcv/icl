#ifndef ICL_DOUBLE_SLIDER_H
#define ICL_DOUBLE_SLIDER_H

#include <QWidget>

class QSlider;
class QLabel;
class QGridLayout;

namespace icl{
  class DoubleSlider : public QWidget{
    Q_OBJECT
    public:
    DoubleSlider(QWidget *parent, const QString &id);
    
    public slots:
    void receiveValueChanged(int i);
    
    signals:
    void doubleValueChanged(const QString &id, double doubleValue);
    
    public:
    void setMinDouble(double dmin);
    void setMaxDouble(double dmax);
    double getDoubleValue();
    void setDoubleValue(double d);
    void setDoubleStepping(double s);
          
    private:
    QSlider *m_poSlider;
    QLabel *m_poLabel;
    QGridLayout *m_poLayout;
    double m_dMin, m_dMax, m_dStepping;
    QString m_qsID;

  };
}

#endif
