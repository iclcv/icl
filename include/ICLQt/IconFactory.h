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

#ifndef ICL_ICON_FACTORY_H
#define ICL_ICON_FACTORY_H

#include <QPixmap>
#include <QIcon>
#include <ICLCore/Img.h>

namespace icl{

  /// Simple utility class providing static functions to create some icons
  class IconFactory{
    public:
    static const QPixmap &create_icl_window_icon_as_qpixmap();
    static const QIcon &create_icl_window_icon_as_qicon();

    static const QIcon &create_icon(const std::string &id);
    static const Img8u &create_image(const std::string &id);
  };
  
}
#endif
