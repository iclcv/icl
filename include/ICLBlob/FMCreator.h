/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLBlob module of ICL                         **
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

#ifndef FMCREATOR_H
#define FMCREATOR_H

#include <ICLUtils/Size.h>
#include <vector>
#include <ICLCore/Types.h>

namespace icl{
  /// interface for Feature Map creators \ingroup G_RBBS
  /** see RegionBasedBlobSearcher for more details */
  struct FMCreator{

    /// Destructor
    virtual ~FMCreator(){};

    /// returns the demanded image size
    /** @return size of the source image of the feature-map**/
    virtual Size getSize()=0;


    /// returns the demaned image format
    /** @return format for the source image of the feature-map**/
    virtual format getFormat()=0;
    
    /// create a featur-map from the given image (always 8u)
    /** @param image source image
        @return new feature map
    **/
    virtual Img8u* getFM(Img8u *image)=0;
    
    /// returns the last calculated feature map
    virtual const ImgBase *getLastFM() const{ return 0; }

    /// static function, that creates a default FMCreator with given parameters
    /** The new FMCreator will create the feature map using the following algorithm:
        \code
        I = input-image converted to size "imageSize" and format "imageFormat"
        {the example is written for a 3-channel rgb image it is generalizable for abitrary formats}
        [r,g,b] = "refcolor"       
        [tr,tg,tb] = thresholds  
        for all pixel p=(x,y) I do
           dr = abs( I(x,y,0) - r )
           dg = abs( I(x,y,1) - g )
           db = abs( I(x,y,2) - b )
           RESULT(x,y) = (dr<tr && dg<tg && db<tb) * 255;
        done

        \endcode

        @param size image size that is used by the new FMCreator
        @param fmt image format that is used by the new FMCreator
        @param refcolor reference color
        @param thresholds array of color component thresholds
        @return new default FMCreator object
    */
    static FMCreator *getDefaultFMCreator(const Size &size, 
                                          format fmt,
                                          std::vector<icl8u> refcolor,
                                          std::vector<icl8u> thresholds);
  };
}

#endif
