#ifndef ICL_IMG_PARAM_WIDGET_H
#define ICL_IMG_PARAM_WIDGET_H

#include <QWidget>
#include <QString>
#include <iclTypes.h>

/** \cond **/
class QComboBox;
class QHBoxLayout;
/** \endcond **/

namespace icl{
  
  /// Internally used widget to define image params \ingroup UNCOMMON
  class ImgParamWidget : public QWidget{
    Q_OBJECT
    public:
    ImgParamWidget(QWidget *parent);
    void doEmitState();
    void getParams(int &width, int &height, int &d, int &fmt) const;
    
    private slots:
    void sizeChanged(const QString &val);
    void formatChanged(const QString &val);
    void depthChanged(const QString &val);

    void setup(int width, int height, int d, int format);
    
    signals:
    void somethingChanged(int width, int height, int d, int format);

    
    private:
    QComboBox *m_poSizeCombo;
    QComboBox *m_poDepthCombo;
    QComboBox *m_poFormatCombo;
    QHBoxLayout *m_poLayout;
    int m_iWidth,m_iHeight,m_iDepth, m_iFormat;
  };
}

#endif
