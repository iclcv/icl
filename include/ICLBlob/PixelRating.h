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

#ifndef PIXEL_RATING_H
#define PIXEL_RATING_H

#include <stdio.h>


namespace icl{
  
  /// Function object interface that produces a reference-color-based pixel rating \ingroup G_CBS
  template<class PixelType, class RatingType>
  class PixelRating{
    public:
    /// virtual destructor
    virtual ~PixelRating(){}
    virtual  RatingType rate(PixelType t0, PixelType t1, PixelType t2)=0;
  };
  
}

#endif
