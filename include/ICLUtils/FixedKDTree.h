/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : include/ICLUtils/FixedKDTree.h                         **
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
#ifndef ICL_FIXED_KDTREE_H
#define ICL_FIXED_KDTREE_H

#include <ICLUtils/FixedColVector.h>
namespace icl{
  
  template<class T, int K>
  class FixedKDTree{
    public:
    typedef FixedColVector<T,K> Vector;

    private:
    struct sort_by{
      const int dim;
      inline sort_by(int dim):dim(dim){}
      inline bool operator()(const Vector &a, const Vector &b){
        return a[dim] < b[dim];
      }
    };
    
    
    struct Node{
      Node *left;
      Node *right;
      T median;
      Node(const Vector *v, int num, int level):left(0),right(0){
        const int dim = level%K
        if(num == 1){
          median = (*v)[dim];
        }else{
          std::sort(v,v+num,sort_by(dim));
          const int m = dim>>1;
          median = v[m];
          left = new Node(v,m,level+1);
          right = new Node(v+m,num-m,level+1);
        }
      }
      ~Node(){
        if(left) delete left;
        if(right) delete right;
      }
    }*root;

    FixedKDTree(const Vector *v, int num){
      std::vector<Vector> cpy(v,v+num);
      root = new Node(cpy.data(), num, 0);
    }
    
        
    
    
    
    
  };

}
