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

#ifndef ICLBORDERBOX_H
#define ICLBORDERBOX_H

#include <QGroupBox>
#include <QVBoxLayout>
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
