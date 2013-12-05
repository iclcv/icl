/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/BorderBox.h                            **
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

#include <QtGui/QGroupBox>
#include <QtGui/QVBoxLayout>
/// The icl namespace
namespace icl{
  namespace qt{
    /// A simple utility class (QWidget with titled border) \ingroup UNCOMMON
    struct BorderBox : public QGroupBox{
  
      /// Create a new BorderBox Object
      /** @param label title of the border
          @param content child widget
          @param parent optional parent widget*/
      BorderBox(const QString &label, QWidget *content, QWidget *parent) : 
        QGroupBox(label,parent), m_poContent(content){
        m_poLayout = new QVBoxLayout;
        m_poLayout->setMargin(3);
        m_poLayout->addWidget(content);
        setLayout(m_poLayout);
      }
      
      /// returns the content of this widget (the child)
      QWidget *content() { return m_poContent; }
      
      private:
      QVBoxLayout *m_poLayout; //<! layout
      QWidget *m_poContent;    //<! content widget
    };
  } // namespace qt
}


