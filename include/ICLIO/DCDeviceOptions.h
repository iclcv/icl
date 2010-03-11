/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
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

#ifndef ICL_DCDEVICE_OPTIONS_H
#define ICL_DCDEVICE_OPTIONS_H

#include <ICLIO/DC.h>

namespace icl{
  
  /// Utility struct for DC Camera device options \ingroup DC_G
  struct DCDeviceOptions{
    
    /// bayer method
    dc1394bayer_method_t bayermethod;

    /// framerate
    dc1394framerate_t framerate;

    /// video mode
    dc1394video_mode_t videomode;

    /// flag whether images should be labeled or not
    bool enable_image_labeling;

    /// iso MBits
    int isoMBits;
    
    /// if set, each frame can be grabbed only once
    bool suppressDoubledImages;
  };
}

#endif
