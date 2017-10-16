/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLUtils/src/ICLUtils/ConsoleProgress.cpp              **
** Module : ICLUtils                                               **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLUtils/ConsoleProgress.h>
#include <cstdio>
using namespace std;
using namespace icl;

namespace{

  char BACK_LINE[] = { char(27), char(91), char(65), char(0)};// "\E[A\0"

  int BAR_LEN = 30;

  string PROGRESS_STR;
}
namespace icl{
  namespace utils{
    void progress_init(const std::string &text){
      // {{{ open

      printf("\n");
      printf("%s%s\n",BACK_LINE,text.c_str());
      PROGRESS_STR = text;
    }

    // }}}

    void progress_finish(){
      // {{{ open

      printf("%s%s:100%s[##############################]\n",BACK_LINE,PROGRESS_STR.c_str(),"%");

    }

    // }}}

    void progress(int curr, int max, const std::string &t){
      // {{{ open

      printf("%s%s:",BACK_LINE,PROGRESS_STR.c_str());
      float frac = (float)curr/(float)max;
      printf("%3d%s",(int)(frac*100),"%");

      int N1 = (int)(frac*BAR_LEN);
      int N2 = BAR_LEN - N1;

      printf("[");
      for(int i=0;i<N1;i++){
        printf("#");
      }
      for(int i=0;i<N2;i++){
        printf("=");
      }
      if(t.length()){
        printf("] %s\n",t.c_str());
      }else{
        printf("]\n");
      }

    }
  } // namespace utils
}
