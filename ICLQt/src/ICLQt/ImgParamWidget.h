/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/ImgParamWidget.h                       **
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

#pragma once

#include <QWidget>
#include <QtCore/QString>
#include <ICLCore/Types.h>

/** \cond **/
class QComboBox;
class QHBoxLayout;
/** \endcond **/

namespace icl{
  namespace qt{

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
  } // namespace qt
}
