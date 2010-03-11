/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
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

#include <ICLFilter/WeightChannelsOp.h>
#include <ICLFilter/UnaryArithmeticalOp.h>

namespace icl {
  
  using namespace std;
  
  void WeightChannelsOp::apply (const ImgBase *poSrc, ImgBase **ppoDst) {
    ICLASSERT_RETURN(poSrc);
    ICLASSERT_RETURN( (int) m_vecWeights.size() == poSrc->getChannels() );
    
    const ImgBase *oTmpSrcImg;
    ImgBase *oTmpDstImg;

    if(!UnaryOp::prepare(ppoDst,poSrc)) return;
    
    UnaryArithmeticalOp op = UnaryArithmeticalOp(UnaryArithmeticalOp::mulOp);
    
    for (int c=0;c<poSrc->getChannels();c++) {
      oTmpSrcImg = poSrc->selectChannel(c);
      oTmpDstImg = (*ppoDst)->selectChannel(c);
      op.setValue(m_vecWeights[c]);
      op.apply(oTmpSrcImg, &oTmpDstImg);
      delete oTmpSrcImg;
      delete oTmpDstImg;
    }
  }
} // namespace icl
