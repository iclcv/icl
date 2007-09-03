#ifndef ICL_COMPABILITY_LABEL_H
#define ICL_COMPABILITY_LABEL_H

#include <QLabel>
#include <QString>
#include <QMutex>

namespace icl{
  
  /// Utility class to avoid Qt warning when accesing QLabels from differnt Threads
  /** QLabels can not be used from different Threads. So if a QLabel is created in 
      in the main thread, it might not be set up to show another text/number from
      the working thread.
      As a workaround, the "label" component of the ICL GUI API uses not the 
      original QLabel but this thread-save reimplementation called
      CompabilityLabel.
  */
  class CompabilityLabel : public QLabel{
    public:
    /// Create a new label with given text and given parent widget
    CompabilityLabel(const QString &text, QWidget *parent=0);

    /// reimplemented drawin function (draw the current text centered)
    virtual void paintEvent(QPaintEvent *evt);

    /// make the label show an integer value
    void setNum(int i);

    /// make the label show a float value
    void setNum(float f);

    /// make the lable show a given string
    void setText(const QString &text);

    /// returns the current text (also thread save)
    QString text() const;
    
    private:
    /// current text (protected by the mutex)
    QString m_sText;
    
    /// Thread-safety mutex
    QMutex m_oMutex;
  };
}

#endif
