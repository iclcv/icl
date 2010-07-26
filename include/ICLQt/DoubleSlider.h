/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/DoubleSlider.h                           **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_DOUBLE_SLIDER_H
#define ICL_DOUBLE_SLIDER_H

#include <QtGui/QWidget>
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
