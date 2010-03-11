/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
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

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
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

#include <ICLUtils/FixedMatrix.h>
#include <ICLUtils/StackTimer.h>
using namespace icl;

#define N 1000000



void part_based_bench(){
  FixedMatrix<float,4,4> a( 0,1,2,3,
                            4,5,6,7,
                            8,9,10,11,
                            12,13,14,15);
  FixedMatrix<float,4,4> b(0.0);
  
  {
    BENCHMARK_THIS_FUNCTION;
    for(int i=0;i<N;++i){
      a.part<0,0,4,1>() = b.part<1,0,1,4>(); 
    }
  }
}

void row_col_based_bench(){
  FixedMatrix<float,4,4> a( 0,1,2,3,
                            4,5,6,7,
                            8,9,10,11,
                            12,13,14,15);
  FixedMatrix<float,4,4> b(0.0);
  
  {
    BENCHMARK_THIS_FUNCTION;
    for(int i=0;i<N;++i){
      a.row(0) = b.col(1);
    }
  }
}

void raw_based_bench(){
  FixedMatrix<float,4,4> a( 0,1,2,3,
                            4,5,6,7,
                            8,9,10,11,
                            12,13,14,15);
  FixedMatrix<float,4,4> b(0.0);
  
  {
    BENCHMARK_THIS_FUNCTION;
    for(int i=0;i<N;++i){
      for(int j=0;j<4;++j){
        a.begin()[j] = b.begin()[1+4*j];
      }
    }
  }
}


int main(){
  FixedMatrix<float,4,4> a( -1,1,2,3,
                            4,5,6,7,
                            8,9,10,11,
                            12,13,14,15);
  
  FixedMatrix<float,3,3> b = a.part<1,1,3,3>();
  
  FixedMatrix<int,10,10> c(0.0);
  c.part<0,2,1,4>() = a.row(0);
  

  DEBUG_LOG("1");
  part_based_bench();

  DEBUG_LOG("2");
  raw_based_bench();

  DEBUG_LOG("3");
  row_col_based_bench();
  
  
  
  DEBUG_LOG("b is: \n" << b << "\n");
  DEBUG_LOG("c is: \n" << c << "\n");
  
  
}
