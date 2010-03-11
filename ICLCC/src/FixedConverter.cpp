/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCC module of ICL                           **
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

#include <ICLCC/FixedConverter.h>


namespace icl{

  FixedConverter::FixedConverter(const ImgParams &p, depth d, bool applyToROIOnly):
    m_oParams(p),m_oConverter(applyToROIOnly),m_eDepth(d) { }
  
  void FixedConverter::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poSrc );
    ICLASSERT_RETURN( ppoDst );
    ensureCompatible(ppoDst,m_eDepth,m_oParams);
    m_oConverter.apply(poSrc,*ppoDst);
  }  
}
