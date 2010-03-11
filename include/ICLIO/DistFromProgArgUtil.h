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

#ifndef ICL_DIST_FROM_PROGARG_UTIL_H
#define ICL_DIST_FROM_PROGARG_UTIL_H


#include <ICLUtils/ProgArg.h>

namespace icl{

  /// utility class for getting distortion parameters from progarg environtment
  /** example usage (using additional cpp-define DIST_FROM_PROGARG:
      \code
      GenerigGrabber g(...)
      if(pa("-dist")){
         g.enableDistortion(DIST_FROM_PROGARG("-dist"),g.getDesiredSize());
      }
      
      
      int main(int n, char **ppc){
        painit(n,ppc,"-dist(float,float,float,float) ...");
      }
      \endcode
  */
  struct DistFromProgArgUtil{
    static double p[4];
    /// Construktor
    inline DistFromProgArgUtil(const std::string &s){
      p[0] = pa(s,0);
      p[1] = pa(s,1);
      p[2] = pa(s,2);
      p[3] = pa(s,3);
    }
    /// converts to double* (passable to Grabber::enableDistortion)
    operator double*(){
      return p;
    }
  };

#define DIST_FROM_PROGARG(S) DistFromProgArgUtil(S)

}

#endif
