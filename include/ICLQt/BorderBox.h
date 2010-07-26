/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/BorderBox.h                              **
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

#ifndef ICLBORDERBOX_H
#define ICLBORDERBOX_H

#include <QtGui/QGroupBox>
#include <QtGui/QVBoxLayout>
/// The icl namespace
namespace icl{
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
}


#endif
