/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLUtils/LeastSquareModelFitting.h             **
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

#ifndef ICL_LEAST_SQUARE_MODEL_FITTING_H
#define ICL_LEAST_SQUARE_MODEL_FITTING_H

#include <ICLUtils/DynMatrix.h>

namespace icl{
  class LeastSquareModelFitting{
    
    
  };
}


#endif


#if 0
 template<>
  void fitModel<icl32f>(icl32f *xs, icl32f *ys, unsigned int n, GeneralModel<icl32f> &model){
    // {{{ open

#ifdef HAVE_IPP
    ICLASSERT_RETURN( n>0 && (int)n>model.dim() );  
    int dim = model.dim();
    
    static std::vector<icl32f> D, S, invS, buf, invS_C, EV, Eval;
    D.resize(dim*n);
    S.resize(dim*dim);
    invS.resize(dim*dim);
    buf.resize(dim*(dim+1));
    invS_C.resize(dim*dim);
    EV.resize(dim*dim);
    Eval.resize(dim);
    
    
    int b = sizeof(icl32f);
    int a = dim*b;
    
    // calculating data matrix D
    for(unsigned int i=0;i<n;i++){
      model.features(xs[i],ys[i],D.data()+dim*i);
    }
    

    // calculating scattermatrix S
    IPP_CALL( ippmMul_tm_32f(D.data(),a,b,dim,n,D.data(),a,b,dim,n,S.data(),a,b) , "mul_tm" );
    
    // inv(S)
    IPP_CALL( ippmInvert_m_32f(S.data(),a,b,buf.data(),invS.data(),a,b,dim) , "invert");
    
    // inv(S)*C 
    IPP_CALL( ippmMul_mm_32f(invS.data(),a,b,dim,dim,model.constraints(),a,b,dim,dim,invS_C.data(),a,b), "mul_mm" );
    
    // eigenvectors and eigenvalues of inv(S)*C
    IPP_CALL( ippmEigenValuesVectorsSym_m_32f(invS_C.data(),a,b,buf.data(),EV.data(),a,b,Eval.data(),dim), "eigen" );
    
    // optimize using ipps-max-index
    icl32f max_elem(0);
    int max_index(0);
    IPP_CALL( ippsMaxIndx_32f(Eval.data(),dim,&max_elem, &max_index), "max index" );


    for(int i=0;i<dim;i++){
      model[i] = EV[dim*i+max_index];
    }
#endfi
