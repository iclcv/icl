/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/HungarianAlgorithm.cpp                 **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter, Michael Goetting                  **
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

#include <ICLCore/Types.h>
#include <ICLUtils/Point.h>
#include <ICLCV/HungarianAlgorithm.h>
#include <ICLUtils/Timer.h>
#include <ICLUtils/StackTimer.h>
#include <list>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace cv{

    typedef Array2D<int> imat;
    typedef std::vector<int> vec;
     
    //#define BEGIN_METHOD(X) counter_##X++; MethodTimer __timer(&timer_##X)
    
    void clearCovers(vec &rowCover, vec &colCover){
      // {{{ open
      for (unsigned int i=0; i<rowCover.size(); i++){
        rowCover[i] = 0;
      }	
      for (unsigned int j=0; j<colCover.size(); j++){
        colCover[j] = 0;
      }
    }
  
    // }}}
    
    template<class real>
    int hg_step1(int step, Array2D<real> &cost){
      // {{{ open
  
      //What STEP 1 does:
      //For each row of the cost matrix, find the smallest element
      //and subtract it from from every other element in its row. 
      real minval;
      for(int i=0; i<cost.getHeight(); i++){									
        minval=cost(i,0);
        for(int j=0; j<cost.getWidth(); j++){
          minval = iclMin(minval,cost(i,j));
        }
        for (int j=0; j<cost.getHeight(); j++){
          cost(i,j)-=minval;
        }
      }
      step=2;
      return step;
    }
  
    // }}}
    
    template<class real>
    int hg_step2(int step, Array2D<real> &cost, imat &mask, vec &rowCover, vec &colCover){
      // {{{ open
  
      //What STEP 2 does:
      //Marks uncovered zeros as starred and covers their row and column.
      for (int i=0; i<cost.getWidth(); i++){
        for (int j=0; j<cost.getHeight(); j++){
          if ((cost(i,j)==0) && (colCover[j]==0) && (rowCover[i]==0)) {
            mask(i,j)=1;
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
    int hg_step3(int step, imat &mask, vec &colCover){
      // {{{ open
  
      //What STEP 3 does:
      //Cover columns of starred zeros. Check if all columns are covered.
  		
      for (int i=0; i<mask.getWidth(); i++){	//Cover columns of starred zeros.
        for (int j=0; j<mask.getHeight(); j++){
          if (mask(i,j) == 1){
            colCover[j]=1;
          }
        }
      }
      int count=0;						
      for (unsigned int j=0; j<colCover.size(); j++){//Check if all columns are covered.
        count+=colCover[j];
      }
      if (count>=mask.getWidth()){//Should be cost.length but ok, because mask has same dimensions.	
        step=7;
      }else {
        step=4;
      }
      return step;
    }
  
    // }}}
  
    /* orig:: working, but slow !!!
    template<class real>
    int hg_step4(int step, Array2D<real> &cost, imat &mask, vec &rowCover, vec &colCover, vec &zero_RC){
      BEGIN_METHOD(hg_step4);
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
          for (int j=0; j<mask.getHeight(); j++){ 
            if (mask[row_col[0]][j]==1){ //If there is a star in the same row...
              starInRow = true;
              row_col[1] = j;//remember its column.
            }
          }
          if (starInRow==true){
            rowCover[row_col[0]] = 1;	//Cover the star's row.
            colCover[row_col[1]] = 0;	//Uncover its column.
          }else{
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
    */
   
    template<class real>
    void findZeroPositions(std::vector<Point> &dst, Array2D<real> &cost){
       // {{{ open
  
      for(int i=0;i<cost.getWidth();++i){
        for(int j=0;j<cost.getHeight();++j){
          if(!cost(i,j)){
            dst.push_back(Point(i,j));
          }
        }
      } 
    }
  
    // }}}
  
    
    
    /*
    original function, 5x slower than ***Lite
    */
    template<class real>
    vec findUncoveredZero(vec &row_col, Array2D<real> &cost, vec &rowCover, vec &colCover){
      // {{{ open
      for(int i=0;i<cost.getWidth();++i){
        for(int j=0;j<cost.getHeight();++j){
          if(!cost[i][j] && !rowCover[i] && !colCover[j]){
            row_col[0] = i;
            row_col[1] = j;
            return row_col;
          }
        }
      }
      row_col[0] = -1;//Just a check value. Not a real index.
      row_col[1] = 0;
      return row_col;
  
      /***************************
      unsigned int i = 0; 
      bool done = false;
      while (!done){
        int j = 0;
        while (j < cost.getHeight()){
          if (cost[i][j]==0 && rowCover[i]==0 && colCover[j]==0){
            row_col[0] = i;
            row_col[1] = j;
            done = true;
          }
          j++;
        }//end inner while
        i++;
        if ((int)i >= cost.getWidth()){
          done = true;
        }
      }//end outer while
      return row_col;
      ****************************/
    }
  
    // }}}
    
    /*
    Best function, 5x faster then original, and twice as fast as ***Fast
    */
    vec findUncoveredZeroLite(vec &row_col, const std::vector<Point> &indices, vec &rowCover, vec &colCover){
      // {{{ open
  
      //    x  = col;
      for(unsigned int i=0;i<indices.size();i++){
        if(!rowCover[indices[i].x] && !colCover[indices[i].y]){
          row_col[0] = indices[i].x;
          row_col[1] = indices[i].y;
          return row_col;
        }
      }
      row_col[0] = -1;//Just a check value. Not a real index.
      row_col[1] = 0;
      return row_col;
    }
    // }}}
  
  
    template<class real>
    int hg_step4(int step, Array2D<real> &cost, imat &mask, vec &rowCover, vec &colCover, vec &zero_RC){
      // {{{ open
      //What STEP 4 does:
      //Find an uncovered zero in cost and prime it (if none go to step 6). Check for star in same row:
      //if yes, cover the row and uncover the star's column. Repeat until no uncovered zeros are left
      //and go to step 6. If not, save location of primed zero and go to step 5.
      vec row_col(2); //Holds row and col of uncovered zero.
      bool done = false;
      std::vector<Point> zeroPositions;
      findZeroPositions(zeroPositions, cost);
      
      if(!zeroPositions.size()){
        step = 6;
        return step;
      }
      
      while (!done){ 
        findUncoveredZeroLite(row_col, zeroPositions, rowCover, colCover);
        if (row_col[0] == -1){
          done = true;
          step = 6;
        }else{
          mask(row_col[0],row_col[1]) = 2;	//Prime the found uncovered zero.
          bool starInRow = false;
          for (int j=0; j<mask.getHeight(); j++){ 
            if (mask(row_col[0],j)==1){ //If there is a star in the same row...
              starInRow = true;
              row_col[1] = j;//remember its column.
            }
          }
          if (starInRow==true){
            rowCover[row_col[0]] = 1;	//Cover the star's row.
            colCover[row_col[1]] = 0;	//Uncover its column.
          }else{
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
    
    int findStarInCol(imat &mask, int col){
      // {{{ open
      // int r=-1;	//Again this is a check value.
      for (int i=0; i<mask.getWidth(); i++){
        if (mask(i,col)==1){
            return i;
            //          r = i;
          }
      }
      //    return r;
      return -1;
    }
  
    // }}}
  
    int findPrimeInRow(imat &mask, int row){
      // {{{ open
      //    int c = -1;
     
      for (int j=0; j<mask.getHeight(); j++){
        if (mask(row,j)==2){
          return j;
          //        c = j;
        }
      }    
      //    return c;
      return -1;
    }
  
    // }}}
    void convertPath(imat &mask, imat &path, int count){
      // {{{ open
  
      for (int i=0; i<=count; i++){
        int &m = mask((int)path(i,0),(int)path(i,1));
        if (m  == 1){
          m = 0;
        }else{
          m = 1;
        }
      }
    }
  
    // }}}
    void erasePrimes(imat &mask){
      // {{{ open
  
      for (int i=0; i<mask.getWidth(); i++){
        for (int j=0; j<mask.getHeight(); j++){
          if (mask(i,j)==2){
            mask(i,j)= 0;
          }
        }
      }
    }
  
    // }}}
    
    int hg_step5(int step, imat &mask, vec &rowCover, vec &colCover, vec &zero_RC){
      // {{{ open
  
      //What STEP 5 does:	
      //Construct series of alternating primes and stars. Start with prime from step 4.
      //Take star in the same column. Next take prime in the same row as the star. Finish
      //at a prime with no star in its column. Unstar all stars and star the primes of the
      //series. Erase any other primes. Reset covers. Go to step 3.
      
      int count = 0; //Counts rows of the path matrix.
      // printf("use the new representation as it scaled better with dynamic size here \n");
      // printf("The correct formula for the hg_step5 was ... path(mask.getHeight()*2,2) todo: \n");
      // printf("TEST TEST TEST .. HungarianAlgorithm.cppp hg_step5  line 228 ........ \n");
      // orig, unstable, as first index becomes too large sometimes ???:
      // imat path(mask.getHeight()+2,2);
  
      std::vector<Point> vecPath;
      
      vecPath.push_back(Point(zero_RC[0],zero_RC[1]));
  
      //path[count][0] = zero_RC[0];//Row of last prime.
      //path[count][1] = zero_RC[1];//Column of last prime.
      
      bool done = false;
     
      while (!done){
        //      int r = findStarInCol(mask,path[count][1]);
        int r = findStarInCol(mask,vecPath[count].y);
        if (r>=0){
          count++;
          
          vecPath.push_back(Point(r,vecPath[count-1].y));
          // path[count][0] = r;               //Row of starred zero.
          // path[count][1] = path[count-1][1];//Column of starred zero.
        }else{
  	done = true;
        }
        if(!done){
          //int c = findPrimeInRow(mask, path[count][0]);
          int c = findPrimeInRow(mask, vecPath[count].x);
          count++;
          vecPath.push_back(Point(vecPath[count-1].x,c));
          // path[count][0] = path [count-1][0]; //Row of primed zero.
          // path[count][1] = c;		    //Col of primed zero.
        }
      }//end while
  		
      imat path((int)vecPath.size(),2);
      for(unsigned int i=0;i<vecPath.size();i++){
        path(i,0) = vecPath[i].x;
        path(i,1) = vecPath[i].y;
      }
      convertPath(mask, path, count);
      clearCovers(rowCover, colCover);
      erasePrimes(mask);
  	
      // printf("need %d more lines at dim %d  2xOK=%s\n", mask.getHeight()-vecPath.size(),mask.getHeight(),(int)((int)mask.getHeight()-(int)vecPath.size())<2*mask.getHeight() ? "OK" : "ERROR" );
      
      step = 3;
      return step;
    }
  
    // }}}
   
    template<class real>
    real findSmallest(Array2D<real> &cost, vec &rowCover, vec &colCover, real maxCost){
      // {{{ open
  
      real minval = maxCost;	   //There cannot be a larger cost than this.
      for (int i=0; i<cost.getWidth(); i++){ //Now find the smallest uncovered value.
        for (int j=0; j<cost.getHeight(); j++){
          if (rowCover[i]==0 && colCover[j]==0 && (minval > cost(i,j))){
            minval = cost(i,j);
          }
        }
      }
      return minval;
    }
  
    // }}}
 
    template<class real>
    int hg_step6(int step, Array2D<real> &cost, vec &rowCover, vec &colCover, real maxCost){
      // {{{ open
  
      //What STEP 6 does:
      //Find smallest uncovered value in cost: a. Add it to every element of covered rows
      //b. Subtract it from every element of uncovered columns. Go to step 4.
      real minval = findSmallest(cost, rowCover, colCover, maxCost);
      
      for (unsigned int i=0; i<rowCover.size(); i++){
        for (unsigned int j=0; j<colCover.size(); j++){
          if (rowCover[i]==1){
            cost(i,j) += minval;
          }
          if (colCover[j]==0){
            cost(i,j) -= minval;
          }
        }
      }
      step = 4;
      return step;
    }
  
    // }}}
   
     
    template<class real>
    vec HungarianAlgorithm<real>::apply(const Array2D<real> &m, bool isCostMatrix){
      // {{{ open
      if(!m.getDim()) return vec();
      mat cost = m.deepCopy();
      if(!isCostMatrix){
        real maxWeight = cost.maxElem();
        for(int i=0;i<cost.getWidth(); i++){
          for(int j=0;j<cost.getHeight(); j++){
            cost(i,j) = maxWeight - cost(i,j);
          }
        }
      }
      real maxCost = cost.maxElem();
      imat mask(m.getWidth(),m.getHeight(),(int)0);
      vec rowCover(m.getHeight());
      vec colCover(m.getWidth());
      vec zero_RC(2);
      int step = 1;
      bool done = false;
      while(!done){
        switch(step){
          case 1: step = hg_step1(step, cost); break;
          case 2: step = hg_step2(step, cost, mask, rowCover, colCover); break;
          case 3: step = hg_step3(step, mask, colCover); break;
          case 4: step = hg_step4(step, cost, mask, rowCover, colCover, zero_RC); break;
          case 5: step = hg_step5(step, mask, rowCover, colCover, zero_RC); break;
          case 6: step = hg_step6(step, cost, rowCover, colCover, maxCost); break;
          case 7: done = true; break;
        }
      }
      
      vec assignment; //(m.getWidth(),2);
      for(int i=0;i<mask.getWidth();i++){
        for(int j=0;j<mask.getHeight();j++){
          if(mask(i,j) == 1){
            assignment.push_back(j);
            // assignment[i][0] = i;
            // assignment[i][1] = j;
          }
        }
      }
      return assignment;
    }
  
    // }}}
  
    template<class real>
    void HungarianAlgorithm<real>::visualizeAssignment(const Array2D<real> &cost, const vec &a){
      // {{{ open
      mat v(cost.getWidth(),cost.getHeight());
      v.fill(0);
      for(unsigned int i=0;i<a.size();i++){
        v(i,a[i]) = 1;
      }
      
      for(int j=0;j<cost.getWidth();j++){
        for(int i=0;i<cost.getHeight();i++){
          printf("%3.3f%s ",(float)cost(i,j), v(i,j) ?  "* " : "  ");
        }
        printf("\n");
      }
      printf("\n");
    }
  
  // }}}
     
  
    template class ICLCV_API HungarianAlgorithm<icl32s>;
    template class ICLCV_API HungarianAlgorithm<icl32f>;
    template class ICLCV_API HungarianAlgorithm<icl64f>;
  
  } // namespace cv
}
