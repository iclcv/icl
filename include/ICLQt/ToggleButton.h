/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLQt/ToggleButton.h                           **
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
*********************************************************************/

#ifndef ICL_TOGGLE_BUTTON_H
#define ICL_TOGGLE_BUTTON_H

#include <QPushButton>

namespace icl{

  class ToggleButton : public QPushButton{
    Q_OBJECT;
    public:
    ToggleButton(const std::string &untoggledText, 
                 const std::string &toggledText,
                 QWidget *parent = 0,
                 bool *stateRef=0);
    ~ToggleButton();

    private slots:
    void toggleStateChanged(bool toggled);

    private:
    std::string m_text[2];
    bool *m_stateRef;
  };  
}

#endif
