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

#ifndef ICL_IMG_PARAM_WIDGET_H
#define ICL_IMG_PARAM_WIDGET_H

#include <QWidget>
#include <QString>
#include <ICLCore/Types.h>

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
