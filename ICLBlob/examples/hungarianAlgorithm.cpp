#include "Img.h"
#include <algorithm>
#include <Mathematics.h>
#include <SimpleMatrix.h>
#include <Timer.h>

using namespace icl;
using namespace std;

typedef icl32f real;
typedef vector<int> vec;
//typedef vector<vector<real> > mat;


typedef SimpleMatrix<real> mat;
typedef SimpleMatrix<int> imat;

/**
Benchmark: 41ms for a 100² matrix
           0.0375ms for 10² matrix
           12s for  a 500² matrix
*/
class HungarianAlgorithm {
public:
  static void genereateRandomArray(mat &m, string method){
    // {{{ open

    for(int i=0;i<m.w(); i++){
      for(int j=0;j<m.h(); j++){
        m[i][j] = method == "random" ? icl::random(1.0) : icl::gaussRandom(1.0)+0.5;
      }
    }
  }

  // }}}
  static real findLargest(mat &m){
    // {{{ open
    real largest = 0;
    for(int i=0;i<m.w(); i++){
      for(int j=0;j<m.h(); j++){
        largest = std::max(largest,m[i][j]);
      }
    }
    return largest;
  }

  // }}}
  
  static imat hgAlgorithm(mat &m, const string &sumType){
    // {{{ open

    mat cost = m.deepCopy();
    if(sumType == "max"){
      real maxWeight = findLargest(cost); 
      for(int i=0;i<cost.w(); i++){
        for(int j=0;j<cost.h(); j++){
          cost[i][j] = maxWeight - cost[i][j];
        }
      }
    }
    real maxCost = findLargest(cost); 
    imat mask(m.w(),m.h());
    vec rowCover(m.h());
    vec colCover(m.w());
    vec zero_RC(2);
    int step = 1;
    bool done = false;
    while(!done){
      switch(step){
        case 1: step = hg_step1(step,cost); break;
        case 2: step = hg_step2(step,cost,mask,rowCover,colCover); break;
        case 3: step = hg_step3(step,mask,colCover); break;
        case 4: step = hg_step4(step, cost, mask, rowCover, colCover, zero_RC); break;
        case 5: step = hg_step5(step, mask, rowCover, colCover, zero_RC); break;
        case 6: step = hg_step6(step, cost, rowCover, colCover, maxCost); break;
        case 7: done = true; break;
      }
    }
    imat assignment(m.w(),2);
    for(int i=0;i<mask.w();i++){
      for(int j=0;j<mask.h();j++){
        if(mask[i][j] == 1){
          assignment[i][0] = i;
          assignment[i][1] = j;
        }
      }
    }
    return assignment;
  }

  // }}}
  static int hg_step1(int step, mat &cost){
    // {{{ open

    //What STEP 1 does:
    //For each row of the cost matrix, find the smallest element
    //and subtract it from from every other element in its row. 
    real minval;
    for(int i=0; i<cost.h(); i++){									
      minval=cost[i][0];
      for(int j=0; j<cost.w(); j++){
        minval = std::min(minval,cost[i][j]);
      }
      for (int j=0; j<cost.h(); j++){
        cost[i][j]-=minval;
      }
    }
    step=2;
    return step;
  }

  // }}}
  static int hg_step2(int step, mat &cost, imat &mask, vec &rowCover, vec &colCover){
    // {{{ open

    //What STEP 2 does:
    //Marks uncovered zeros as starred and covers their row and column.
    for (int i=0; i<cost.w(); i++){
      for (int j=0; j<cost.h(); j++){
        if ((cost[i][j]==0) && (colCover[j]==0) && (rowCover[i]==0)) {
          mask[i][j]=1;
          colCover[j]=1;
          rowCover[i]=1;
        }
      }
    }
    clearCovers(rowCover, colCover);	//Reset cover vectors.
    
    step=3;
    return step;
  }

  // }}}
  static int hg_step3(int step, imat &mask, vec &colCover){
    // {{{ open

    //What STEP 3 does:
    //Cover columns of starred zeros. Check if all columns are covered.
		
    for (int i=0; i<mask.w(); i++){	//Cover columns of starred zeros.
      for (int j=0; j<mask.h(); j++){
        if (mask[i][j] == 1){
          colCover[j]=1;
        }
      }
    }
    int count=0;						
    for (unsigned int j=0; j<colCover.size(); j++){//Check if all columns are covered.
      count+=colCover[j];
    }
    if (count>=mask.w()){//Should be cost.length but ok, because mask has same dimensions.	
      step=7;
    }else {
      step=4;
    }
    return step;
  }

  // }}}
  static int hg_step4(int step, mat &cost, imat &mask, vec &rowCover, vec &colCover, vec &zero_RC){
    // {{{ open

    //What STEP 4 does:
    //Find an uncovered zero in cost and prime it (if none go to step 6). Check for star in same row:
    //if yes, cover the row and uncover the star's column. Repeat until no uncovered zeros are left
    //and go to step 6. If not, save location of primed zero and go to step 5.
    vec row_col(2); //Holds row and col of uncovered zero.
    bool done = false;
    while (!done){
      row_col = findUncoveredZero(row_col, cost, rowCover, colCover);
      if (row_col[0] == -1){
        done = true;
        step = 6;
      }else{
        mask[row_col[0]][row_col[1]] = 2;	//Prime the found uncovered zero.
        bool starInRow = false;
        for (int j=0; j<mask.h(); j++){ // TODO why not mask.h()
          if (mask[row_col[0]][j]==1){ //If there is a star in the same row...
            starInRow = true;
            row_col[1] = j;//remember its column.
          }
        }
        if (starInRow==true){
          rowCover[row_col[0]] = 1;	//Cover the star's row.
          colCover[row_col[1]] = 0;	//Uncover its column.
        }else{
          // TODO zero_RC = row_col ??
          zero_RC[0] = row_col[0];	//Save row of primed zero.
          zero_RC[1] = row_col[1];	//Save column of primed zero.
          done = true;
          step = 5;
        }
      }
    }
    return step;
  }

  // }}}
  static vec findUncoveredZero(vec &row_col, mat &cost, vec &rowCover, vec &colCover){
    // {{{ open

    row_col[0] = -1;	//Just a check value. Not a real index.
    row_col[1] = 0;
    unsigned int i = 0; 
    bool done = false;
    while (!done){
      int j = 0;
      while (j < cost.h()){
        if (cost[i][j]==0 && rowCover[i]==0 && colCover[j]==0){
          row_col[0] = i;
          row_col[1] = j;
          done = true;
        }
        j++;
      }//end inner while
      i++;
      if ((int)i >= cost.w()){
        done = true;
      }
    }//end outer while
    return row_col;
  }

  // }}}
  static int hg_step5(int step, imat &mask, vec &rowCover, vec &colCover, vec &zero_RC){
    // {{{ open

    //What STEP 5 does:	
    //Construct series of alternating primes and stars. Start with prime from step 4.
    //Take star in the same column. Next take prime in the same row as the star. Finish
    //at a prime with no star in its column. Unstar all stars and star the primes of the
    //series. Erase any other primes. Reset covers. Go to step 3.
    
    int count = 0; //Counts rows of the path matrix.
    //imat path(mask.h()+2,2);
    imat path(mask.h()+20,2);
    path[count][0] = zero_RC[0];//Row of last prime.
    path[count][1] = zero_RC[1];//Column of last prime.
    
    bool done = false;
   
    while (!done){
      int r = findStarInCol(mask,path[count][1]);
      if (r>=0){
        count++;
        //        printf("count is %d ---> max index is %d\n",count,mask.h()+2-1);
        path[count][0] = r;               //Row of starred zero.
        path[count][1] = path[count-1][1];//Column of starred zero.
      }else{
	done = true;
      }
      if(!done){
        int c = findPrimeInRow(mask, path[count][0]);
        count++;
        path[count][0] = path [count-1][0]; //Row of primed zero.
        path[count][1] = c;		    //Col of primed zero.
      }
    }//end while
		
    convertPath(mask, path, count);
    clearCovers(rowCover, colCover);
    erasePrimes(mask);
		
    step = 3;
    return step;
  }

  // }}}
  static int findStarInCol(imat &mask, int col){
    // {{{ open

    int r=-1;	//Again this is a check value.
    for (int i=0; i<mask.w(); i++){
        if (mask[i][col]==1){
          r = i;
        }
    }
    return r;
  }

  // }}}
  static int findPrimeInRow(imat &mask, int row){
    // {{{ open

    int c = -1;
    for (int j=0; j<mask.h(); j++){
      if (mask[row][j]==2){
        c = j;
      }
    }    
    return c;
  }

  // }}}
  static void convertPath(imat &mask, imat &path, int count){
    // {{{ open

    for (int i=0; i<=count; i++){
      if (mask[(int)path[i][0]][(int)path[i][1]]==1){
        mask[(int)path[i][0]][(int)path[i][1]] = 0;
      }else{
        mask[(int)path[i][0]][(int)path[i][1]] = 1;
      }
    }
  }

  // }}}
  static void erasePrimes(imat &mask){
    // {{{ open

    for (int i=0; i<mask.w(); i++){
      for (int j=0; j<mask.h(); j++){
        if (mask[i][j]==2){
          mask[i][j] = 0;
        }
      }
    }
  }

  // }}}
  static void clearCovers(vec &rowCover, vec &colCover){
    // {{{ open

    for (unsigned int i=0; i<rowCover.size(); i++){
      rowCover[i] = 0;
    }	
    for (unsigned int j=0; j<colCover.size(); j++){
      colCover[j] = 0;
    }
  }

  // }}}
  static int hg_step6(int step, mat &cost, vec &rowCover, vec &colCover, real maxCost){
    // {{{ open

    //What STEP 6 does:
    //Find smallest uncovered value in cost: a. Add it to every element of covered rows
    //b. Subtract it from every element of uncovered columns. Go to step 4.
    real minval = findSmallest(cost, rowCover, colCover, maxCost);
    
    for (unsigned int i=0; i<rowCover.size(); i++){
      for (unsigned int j=0; j<colCover.size(); j++){
        if (rowCover[i]==1){
          cost[i][j] = cost[i][j] + minval;
        }
        if (colCover[j]==0){
          cost[i][j] = cost[i][j] - minval;
        }
      }
    }
    step = 4;
    return step;
  }

  // }}}
  static double findSmallest(mat &cost, vec &rowCover, vec &colCover, real maxCost){
    // {{{ open

    real minval = maxCost;	   //There cannot be a larger cost than this.
    for (int i=0; i<cost.w(); i++){ //Now find the smallest uncovered value.
      for (int j=0; j<cost.h(); j++){
        if (rowCover[i]==0 && colCover[j]==0 && (minval > cost[i][j])){
          minval = cost[i][j];
        }
      }
    }
    return minval;
  }

  // }}}
};

void show_mat(mat &m){
  // {{{ open

  for(int j=0;j<m.w();j++){
    for(int i=0;i<m.h();i++){
      printf("%3d ",(int)m[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

// }}}
void show_mat(imat &m){
  // {{{ open

  for(int j=0;j<m.w();j++){
    for(int i=0;i<m.h();i++){
      printf("%3d ",m[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

// }}}
void show_result(mat &m, imat &a, int d){
  // {{{ open

  mat v(d,d);
  
  for(int i=0;i<d;i++){
    v[a[i][0]][a[i][1]] = 1;
  }
  
  for(int j=0;j<d;j++){
    for(int i=0;i<d;i++){
      printf("%3d%s ",(int)m[i][j], v[i][j] ?  "* " : "  ");
    }
    printf("\n");
  }
  printf("\n");

}

// }}}

mat random_mat(int d){
  // {{{ open

  mat m(d,d);
  for(int j=0;j<d;j++){
    for(int i=0;i<d;i++){     
      m[i][j] = icl::random(100);
    }
  }
  return m;
}

// }}}

real calculate_err(imat &a, mat &cost){
  // {{{ open

  real err = 0;
  for(int i=0;i<cost.w();i++){
    err += cost[a[i][0]][a[i][1]];
  }
  return err;
}

// }}}

imat random_assignment(int d){
  // {{{ open

  imat a(d,2);
  
  static vec randPos;
  if(!randPos.size()){
    for(int i=0;i<d;i++){
      randPos.push_back(i);
    }
  }
  std::next_permutation(randPos.begin(), randPos.end());
  
  //printf("new assignment : ");
  for(int i=0;i<d;i++){
    a[i][0] = i;
    a[i][1] = randPos[i];
    // printf("%d ",(int)randPos[i]);
  }
  // printf("\n");
  return a;
}

// }}}

void test(int ntests,int nSubTests, int dim){
  // {{{ open

  icl::randomSeed();
  for(int i=0;i<ntests;i++){
    mat m = random_mat(dim);
    // show_mat(m);
    imat aOpt = HungarianAlgorithm::hgAlgorithm(m,"min");
    real errOpt = calculate_err(aOpt, m);
    printf("first test = err = %f \n",errOpt);
    //show_result(m,aOpt,dim);
    /*   for(int j=0;j<nSubTests;j++){
      imat aOther = random_assignment(dim);
      real errOther = calculate_err(aOther,m);
      if(errOther < errOpt){
        printf("****** found smaller error %f\n",errOther);
      }
    }
    */
  }
} 

// }}}

int main(int n, char **ppc){
  /*  real matrix[] = { 90,  75,  75,  80,
                    35,  85,  55,  65,
                    125,  95,  90, 105,  
  45, 110,  95, 115   }; */

  // test(1000,1000,7);

  Timer t;
  t.start();
  mat m = random_mat(500);
  for(int i=0;i<1;i++){
    imat aOpt = HungarianAlgorithm::hgAlgorithm(m,"min");
    aOpt[0][1] = 0;
    //    if(!(i%10)) printf("%d \n",i/10);
  }
  t.stop();
  
  printf("done ----------------- \n");
  return 0;

}

