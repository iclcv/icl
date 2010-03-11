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

#include <ICLQuick/Common.h>

int main(int nArgs, char **ppcArg){

  ImgQ a = scale(create("flowers"),320,240);
  ImgQ b = scale(create("windows"),320,240);

  /// this will implicitly use the arithmetical op  
  ImgQ apb = norm(a+b);
  ImgQ amb = norm(a-b);
  ImgQ am100 = norm(a-100);

  ImgQ x = (label(a,"A"), label(b,"B")) %
  (label(apb,"norm(A+B)"), label(amb,"norm(A-B)"), label(am100,"norm(a-100)"));
  
  show(x);
  
}
