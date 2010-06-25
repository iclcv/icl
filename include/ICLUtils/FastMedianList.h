/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/FastMedianList.h                      **
** Module : ICLUtils                                               **
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

#ifndef ICL_FASTMEDIANLIST_H
#define ICL_FASTMEDIANLIST_H

#include <string.h>
#include <cmath>
namespace icl{
  /// Utility class for fast calculation of a median (calculating a median in O(N)) \ingroup G_UTILS
  /** The median of a set S is defined by the element of sorted S at indes |S|/2
      To avoid the expensive sorting procedure of the list with runs in O(N*log(N)),
      the FastMedianList can be used.\n
      It exploits the knowledge of the maximum expected value to accelerate median
      calculation. 
      Naive approach: 
      <pre>
      given: set S of length n
      
      1.  sort(S)                                             O(n*log(n))
      return S[n/2]
      </pre>
      
      FastMedianList approach:
      <pre>
      given: an empty set S with max element M
      Create counter array A of size M initialized with 0     O(M)
      Create overall counter C = 0

      Set creation (iteratively) n times:                          
      
      given: next elem e                                      O(n)
      A[e]++
      C++
      
      Accessing median:                                       O(M)
      mass=0;
      HALF_C = C/2;
      for i=0..M
        mass+=A[i];
        if mass > HALF_C
          return i
        endif
      endfor

      Overall complexity                                       O(M+n)
      </pre>
      
      
      The fast median list is faster if O(M+n) < O(n*log(n)). E.g. if we try
      to compute the median pixel x-position of an image of width 640.  We have
      640+n < n*log(n) --> 640 < n*(log(n)+1) which is true if n is larger than
      about 120.
      
  */
  class FastMedianList{
    public:
    /// Create a new fast median list with given max size
    /** t2,t3 and t4 are deprecated! */
    inline FastMedianList(int size=0,int t2=-1, int t3=-1, int t4=-1):
      m_iSize(size),m_iT2(t2),m_iT3(t3),m_iT4(t4), m_piTable(size ? new int[size]: 0){
      clear();
    }
    
    /// Copy cosntructor
    inline FastMedianList(const FastMedianList &l):
      m_iSize(l.m_iSize),m_iT2(l.m_iT2),m_iT3(l.m_iT3),m_iT4(l.m_iT4){
      m_piTable = m_iSize ? new int[m_iSize] : 0;
      clear();
    }
    
    /// Assign operator
    inline FastMedianList &operator=(const FastMedianList &l){
      m_iSize = l.m_iSize;
      m_iT2   = l.m_iT2;
      m_iT3   = l.m_iT3;
      m_iT4   = l.m_iT4;
      m_piTable = m_iSize ? new int[m_iSize] : 0;
      clear();
      return *this;
    }

    /// destructor
    inline ~FastMedianList(){
      if(m_piTable) delete [] m_piTable;
    }
    
    /// initialization function
    inline void init(int size, int t2=-1, int t3=-1, int t4=-1){
      m_iSize = size;
      m_iT2 = t2;
      m_iT3 = t3;
      m_iT4 = t4;
      if(m_piTable)delete [] m_piTable;
      m_piTable = new int[size];
    }
    
    /// add a new datum
    inline int add(int index){
      m_piTable[index]++;
      m_iCount++;
      return m_iCount;
    }

    /// add a new datum (deprecated)
    inline void add(int index, int val){
      m_piTable[index]++;
      m_iCount++;
      if(val > m_iT2)return;
      m_piTable[index]++;
      m_iCount++;
      if(val > m_iT3)return;
      m_piTable[index]++;
      m_iCount++;
      if(val > m_iT4)return;
      m_piTable[index]++;
      m_iCount++;
    }
    
    /// calculates the median value
    inline int median(){
      if(m_iCount == 0)return -1;
      int mass=0;
      int count2 = m_iCount/2;
      for(int i=0;i<m_iSize;i++){
        mass+=m_piTable[i];
        if(mass > count2){
          return i;
        }
      }
      return 0;
    }
    
    /// resets the internal lookup table
    inline void clear(){
      memset(m_piTable,0,m_iSize*sizeof(int));
      m_iCount=0;
    }
    
    /// calculates the mean value (not accelerated)
    inline int mean(){
      int m=0;
      
      if (!m_iCount) return 0;
      
      for(int i=0;i<m_iSize;i++){
        m+=m_piTable[i]*i;
      }
      return m/m_iCount;
      
    }
    
    /// calculates the variance of contained elements
    inline int variance(){
      int var=0;
      int m = mean();
      for(int i=0;i<m_iSize;i++){
        var+=( m_piTable[i]*(int)pow(float(i-m),2) );
      }
      return var/m_iCount;
      
    }
    /// returns the current count of elements
    inline int getSize(){
      return m_iCount;
    }
    
    protected:
    /// internal element count
    int m_iCount;

    /// internal storage of max size
    int m_iSize;

    /// deprecated variable
    int m_iT2;

    /// deprecated variable
    int m_iT3;

    /// deprecated variable
    int m_iT4;

    /// internal counter array
    int *m_piTable;
  };
  
}

#endif
