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

#ifndef ICL_Unicap_GRAB_ENGINE_H
#define ICL_Unicap_GRAB_ENGINE_H
#include <ICLCore/Types.h>
#include <string>
#include <unicap.h>
#include <ICLCore/ImgParams.h>

namespace icl{
  /// Interface class for UnicapGrabEngines \ingroup UNICAP_G
  /** Base implemetation is the UnicapDefaultGrabEngine */
  struct UnicapGrabEngine{
    virtual ~UnicapGrabEngine(){}
    // locks the grabbers image
    virtual void lockGrabber() = 0;

    // unlocks the grabber image
    virtual void unlockGrabber() =0;

    /// returns a converted frame
    virtual void getCurrentFrameConverted(const ImgParams &desiredParams, 
                                          depth desiredDepth,
                                          ImgBase **ppoDst) = 0;

    /// returns an unconverted frame
    virtual const icl8u *getCurrentFrameUnconverted() = 0;

    /// returns whether the grab engine is able to provide given image params
    virtual bool isAbleToProvideParams(const ImgParams &desiredParams, 
                                       depth desiredDepth) const = 0;
    
    /// retruns whether this engine is able to provide converted frames or not
    virtual bool needsConversion() const = 0;
  };
}

#endif
