/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLCore/src/Random.cpp                                 **
** Module : ICLCore                                                **
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
*********************************************************************/

#include <ICLCore/Random.h>
namespace icl{
  
  double gaussRandom(double mu, double sigma){
    static bool haveNextGaussian = false;
    static double nextGaussian = 0;
    if(haveNextGaussian){
      haveNextGaussian = false;
      return nextGaussian*sigma + mu;
    } else{
      double v1(0), v2(0), s(0);
      do{
        v1 = 2 * random(1.0)-1;
        v2 = 2 * random(1.0)-1;
        s = v1*v1 + v2*v2;
      }while(s>=1 || s == 0);
      double fac = sqrt(-2.0*log(s)/s);
      nextGaussian = v2 * fac;
      haveNextGaussian = true;
      return v1 * fac * sigma + mu;
    }
  }   

  
}
