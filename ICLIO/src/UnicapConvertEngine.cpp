/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/UnicapConvertEngine.cpp                      **
** Module : ICLIO                                                  **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/UnicapConvertEngine.h>
#include <ICLIO/UnicapFormat.h>
#include <ICLIO/UnicapDevice.h>
#include <ICLCore/CoreFunctions.h>

namespace icl{

  void UnicapConvertEngine::cvtNative(const icl8u *rawData, ImgBase **ppoDst){
    depth d = depth8u; // maybe we can try to estimate depth from m_poDevice
    UnicapFormat f = m_poDevice->getCurrentUnicapFormat();
    ImgParams p(f.getSize(),formatRGB); // here we use RGB as default
    ensureCompatible(ppoDst,d,p);
    cvt(rawData,p,d,ppoDst);
  }

}
