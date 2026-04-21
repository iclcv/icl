// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#include <QWidget>
#include <QtCore/QString>
#include <icl/core/Types.h>

/** \cond **/
class QComboBox;
class QHBoxLayout;
/** \endcond **/

namespace icl::qt {
  /// Internally used widget to define image params \ingroup UNCOMMON
  class ICLQt_API ImgParamWidget : public QWidget{
    Q_OBJECT
    public:
    ImgParamWidget(QWidget *parent);
    void doEmitState();
    void getParams(int &width, int &height, int &d, int &fmt) const;

    private Q_SLOTS:
    void sizeChanged(const QString &val);
    void formatChanged(const QString &val);
    void depthChanged(const QString &val);

    void setup(int width, int height, int d, int format);

    Q_SIGNALS:
    void somethingChanged(int width, int height, int d, int format);


    private:
    QComboBox *m_poSizeCombo;
    QComboBox *m_poDepthCombo;
    QComboBox *m_poFormatCombo;
    QHBoxLayout *m_poLayout;
    int m_iWidth,m_iHeight,m_iDepth, m_iFormat;
  };
  } // namespace icl::qt