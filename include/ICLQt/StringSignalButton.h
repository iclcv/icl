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

#ifndef ICL_STRING_SIGNAL_BUTTON_H
#define ICL_STRING_SIGNAL_BUTTON_H

#include <QPushButton>

namespace icl{
  /// internally used button that emits a signal with its text \ingroup UNCOMMON
  class StringSignalButton : public QPushButton{
    Q_OBJECT
    public:
    /// Create a new StringSignalButton with given text and parent widget
    StringSignalButton(const QString &text,QWidget *parent);

    signals:
    /// the clicked signal (with the buttons text)
    void clicked(const QString &text);
    
    private slots:
    /// internally used slot (connected to the parent buttons clicked() signal)
    void receiveClick(bool checked);
    
  };
}


#endif
