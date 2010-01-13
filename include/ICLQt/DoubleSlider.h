#ifndef ICL_DOUBLE_SLIDER_H
#define ICL_DOUBLE_SLIDER_H

#include <QWidget>
#include <ICLUtils/CompatMacros.h>

/** \cond */
class QSlider;
class QLabel;
class QGridLayout;
/** \endcond */

// The icl namespace
namespace icl{
  
  /// A utiltiy class which implements a labeld, double valued QSlider \ingroup UNCOMMON
  class DoubleSlider : public QWidget{
    Q_OBJECT
    public:
    /// Create a new QSlider object
    /** @param parent parent widget
        @param id label
    */
    DoubleSlider(QWidget *parent, const QString &id);
    
    public slots:
    /// internally used slot to process when the corresponding QSlider is moved
    /** @param i value */
    void receiveValueChanged(int i);
    
    signals:
    /// emitted when the slider values is changed
    void doubleValueChanged(const QString &id, double doubleValue);
    
    public:
    /// sets the min. double value
    void setMinDouble(double dmin);

    /// sets the max. double value
    void setMaxDouble(double dmax);

    /// returns the current double value
    double getDoubleValue();

    /// sets the current double value
    void setDoubleValue(double d);

    /// sets the current double stepping
    void setDoubleStepping(double s);
          
    private:
    QSlider *m_poSlider;        //!< wrapped QSlider (int-valued)
    QLabel *m_poLabel;          //!< label
    QGridLayout *m_poLayout;    //!< layout
    double m_dMin;              //!< current min value
    double m_dMax;              //!< current max value
    double m_dStepping;         //!< current stepping
    QString m_qsID;             //!< current label text

  };
}

#endif
