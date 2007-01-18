#include "Extrapolator.h"
#include <PositionTracker.h>
#include <HungarianAlgorithm.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <Img.h>
#include <Array.h>
#include <Mathematics.h>


using namespace std;
using namespace icl;

typedef Array<int> vec;


void showRes(const Array<int> &dataXs,const Array<int> &dataYs,PositionTracker<int> &pt){
  printf("-----------------------------------------------\n");
  printf("current tracking result: \n");
  for(unsigned int i=0;i<dataXs.size();i++){
    printf("datapoint %d %d  --> index %d \n",dataXs[i],dataYs[i],pt.getID(dataXs[i],dataYs[i]));
  }
  printf("-----------------------------------------------\n");
}


int main(int n, char  **ppc){
  PositionTracker<int> pt;
  randomSeed();
  
  int N = 1000000;

  int pi[100],pj[100];
  for(int i=0;i<N;i++){
    int dim = randomi(30)+2;
    printf("dim = %d \n",dim);
    for(int k=0;k<dim;k++){
      pi[k] = randomi(100);
      pj[k] = randomi(100);
    }
    pt.pushData(vec(pi,dim),vec(pj,dim));
    if(!(i%10000)){
      printf("%f \n",float(i)/N);
    }
  }
  return 0;
}

/*
  int cc[D*D] = { 0,1,1,1,1,
                  1,2,0,1,1,
                  3,0,1,6,1,
                  1,1,6,1,0,
                  1,1,1,0,1 };

  SimpleMatrix<int> m(D,D,cc);
  ivec ass = HungarianAlgorithm<int>::apply(m);
  HungarianAlgorithm<int>::visualizeAssignment(m,ass);
  return 0;
  */

  /*
  27|              |               |              |              |
  26|              |               |              |             (o)
  25+--------------+---------------+--------------+-------------.+-
  24|              |               |              |            . |
  23|              |               |              |           .  |
  22|              |               |              |          .   |
  21|              |               |              |        ..    |
  20+--------------+---------------+--------------+-------.------+-
  19|              |               |              |     ..       |
  18|              |               |              |   ..         |
  17|              |               |              | ..           |
  16|              |    ,,,,,,,,,,(#),            |.            (X)
  15+-----------,,(#),,,-----------+--,,,,,-----.(o)-------------+-
  14|         ,,   |               |       ,,,..  |              |
  13|       ,,     |               |        ..,,  |              |
  12|     ,,       |               |      ..    , |              |
  11|   ,,         |               |   ...       (#)             |
  10+-(#)----------+---------------+-..----------(X),,-----------+-
  9 |              |             .(o)             |   ,          |
  8 |              |          ...  |              |    ,,        |
  7 |              |      ....     |              |      ,       |
  6 |              |   ...        (X)             |       ,,     |
  5-+--------------+-..------------+--------------+---------,----+-
  4 |          ...(o)              |              |          ,   |
  3 |       ...   (X)              |              |           ,  | 
  2 | (o)...       |               |              |            , |
  1 | (X)          |               |              |             (#)
  0-+--------------+---------------+--------------+--------------+-
  t=0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
  */


 /*
  22|              |               |              |              |
  21|              |               |              |              |
  20+--------------+---------------+--------------+--------------+-
  19|              |               |              |              |
  18|              |               |              |              |
  17|              |               |              |              |
  16|              |               |              |              |
  15+--------------+---------------+--------------+--------------+-
  14|              |               |              |              |
  13|              |               |              |              |
  12|              |               |              |              |
  11|              |               |              |              |
  10+--------------+---------------+--------------+--------------+-
  9 |              |               |              |              |
  8 |              |               |              |              |
  7 |              |               |              |              |
  6 |              |               |              |              |
  5-+--------------+---------------+--------------+--------------+-
  4 |              |               |              |              |
  3 |              |               |              |              |
  2 |              |               |              |              |
  1 |              |               |              |              |
  0-+--------------+---------------+--------------+--------------+-
  t=0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19 20
  */
