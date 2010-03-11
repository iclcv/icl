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

#ifndef ICL_PROXY_LAYOUT_H
#define ICL_PROXY_LAYOUT_H

#include <ICLQt/GUIWidget.h>


namespace icl{
  /// just a helper class for GUI Layouting \ingroup UNCOMMON
  /** This class shall help to implement GUI container components,
      that do not use a QLayout for layouting e.g. QTabWidgets or
      QSplitters */
  struct ProxyLayout{
    /// defines how to add widges
    virtual void addWidget(GUIWidget *widget)=0;
  };  
}



#endif
