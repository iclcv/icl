#include <iclMEx.h>
#include <iclTypes.h>
#include <iclMathematics.h>

namespace icl{
  
  //int ___tmp;
  //#define IPP_CALL(X,S) ___tmp = X; if(___tmp) printf(S)
#define IPP_CALL(X,S) X


  template<>
  void fitModel<icl64f>(icl64f *xs, icl64f *ys, unsigned int n, GeneralModel<icl64f> &model){
    // {{{ open

#ifdef WITH_IPP_OPTIMIZATION    
    ICLASSERT_RETURN( n>0 && (int)n>model.dim() );  
    int dim = model.dim();
    
    static Array<icl64f> D, S, invS, buf, invS_C, EV, Eval;
    D.resize(dim*n);
    S.resize(dim*dim);
    invS.resize(dim*dim);
    buf.resize(dim*(dim+1));
    invS_C.resize(dim*dim);
    EV.resize(dim*dim);
    Eval.resize(dim);
    
    
    int b = sizeof(icl64f);
    int a = dim*b;
    
    // calculating data matrix D
    for(unsigned int i=0;i<n;i++){
      model.features(xs[i],ys[i],*D+dim*i);
    }
    

    // calculating scattermatrix S
    IPP_CALL( ippmMul_tm_64f(*D,a,b,dim,n,*D,a,b,dim,n,*S,a,b) , "mul_tm" );
    
    // inv(S)
    IPP_CALL( ippmInvert_m_64f(*S,a,b,*buf,*invS,a,b,dim) , "invert");
    
    // inv(S)*C 
    IPP_CALL( ippmMul_mm_64f(*invS,a,b,dim,dim,model.constraints(),a,b,dim,dim,*invS_C,a,b), "mul_mm" );
    
    // eigenvectors and eigenvalues of inv(S)*C
    IPP_CALL( ippmEigenValuesVectorsSym_m_64f(*invS_C,a,b,*buf,*EV,a,b,*Eval,dim), "eigen" );
    
    // optimize using ipps-max-index
    icl64f max_elem(0);
    int max_index(0);
    IPP_CALL( ippsMaxIndx_64f(*Eval,dim,&max_elem, &max_index), "max index" );

    // memcpy(dstParams,(*EV)+max_index*dim,dim*sizeof(icl64f));

    for(int i=0;i<dim;i++){
      model[i] = EV[dim*i+max_index];
    }
    
#else
#warning "ICLMEx::fitEllipse is not implemented without ipp optimization"
#endif    
  }

// }}}
  
  template<>
  void fitModel<icl32f>(icl32f *xs, icl32f *ys, unsigned int n, GeneralModel<icl32f> &model){
    // {{{ open

#ifdef WITH_IPP_OPTIMIZATION    
    ICLASSERT_RETURN( n>0 && (int)n>model.dim() );  
    int dim = model.dim();
    
    static Array<icl32f> D, S, invS, buf, invS_C, EV, Eval;
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
      model.features(xs[i],ys[i],*D+dim*i);
    }
    

    // calculating scattermatrix S
    IPP_CALL( ippmMul_tm_32f(*D,a,b,dim,n,*D,a,b,dim,n,*S,a,b) , "mul_tm" );
    
    // inv(S)
    IPP_CALL( ippmInvert_m_32f(*S,a,b,*buf,*invS,a,b,dim) , "invert");
    
    // inv(S)*C 
    IPP_CALL( ippmMul_mm_32f(*invS,a,b,dim,dim,model.constraints(),a,b,dim,dim,*invS_C,a,b), "mul_mm" );
    
    // eigenvectors and eigenvalues of inv(S)*C
    IPP_CALL( ippmEigenValuesVectorsSym_m_32f(*invS_C,a,b,*buf,*EV,a,b,*Eval,dim), "eigen" );
    
    // optimize using ipps-max-index
    icl32f max_elem(0);
    int max_index(0);
    IPP_CALL( ippsMaxIndx_32f(*Eval,dim,&max_elem, &max_index), "max index" );


    for(int i=0;i<dim;i++){
      model[i] = EV[dim*i+max_index];
    }
    
   
#else
#warning "ICLMEx::fitEllipse is not implemented without ipp optimization"
#endif        

  }

// }}}

  
  template<class T>
  void fitModel(T *xs,T *ys, unsigned int n, 
                GeneralModel<T> &model, int nSubSets, int subSetSize){
    // {{{ open
    int dim = model.dim();
    ICLASSERT_RETURN(dim > 0);
    ICLASSERT_RETURN(subSetSize < (int)n && subSetSize >= dim);
   

    subSetSize = iclMax(dim,subSetSize);

    static Array<T> XBuf, YBuf;
    static Array<Array<T> > PBuf;    
    XBuf.resize(subSetSize);
    YBuf.resize(subSetSize);
    PBuf.resize(dim);
    for(int i=0;i<dim;i++){
      PBuf[i].resize(nSubSets);
    }
    
    static MathematicsRandomSeedInitializer s_initRandom;

    
    for(int i=0;i<nSubSets;i++){
      for(int j=0;j<subSetSize;j++){
        int idx = random(n);
        XBuf[j] = xs[idx];
        YBuf[j] = ys[idx];
      }  
      
      fitModel(*XBuf, *YBuf,subSetSize, model);

      for(int j=0;j<dim;j++){
        PBuf[j][i] = model[j];
      }
    }
    
    // taking the median param of the param vec
    for(int j=0;j<dim;j++){
      sort(PBuf[j].begin(),PBuf[j].end());
      model[j] = PBuf[j][nSubSets/2];
    } 
  }
  
  // }}}

  template<class T, class X>
  void drawModel(GeneralModel<T> &model,Img<X> *im, X *color){
    // {{{ open
    ICLASSERT_RETURN( im );
    Img<X> &image = *im;
    int w = image.getWidth();
    int h = image.getHeight();
    int ch = image.getChannels();
    for(int px=0;px<w;px++){
      for(int c=0;c<ch;c++){
        std::vector<T> pr = model.y(px);
        for(unsigned int p=0;p<pr.size();p++){
          int y = (int)pr[p];
          if(y>=0 && y<h){
            image(px,y,c)=color[c];
          }
        }
      }
    }
    for(int py=0;py<h;py++){
      for(int c=0;c<ch;c++){
        std::vector<T> pr = model.x(py);
        for(unsigned int p=0;p<pr.size();p++){
          int x = (int)pr[p];
          if(x>=0 && x<w){ 
            image(x,py,c)=color[c];
          }
        }
      }
    }
  }

// }}}
  
  template<class T, class X>
  void drawQuadraticModel(QuadraticModel<T> &model, Img<X> *image, X *color){
    drawModel(model,image,color);
    
    // draw the center
    T tx(0),ty(0);
    int w=image->getWidth(),h=image->getHeight();
    model.center(tx,ty);
    const int SIZE=10;
    int x = (int)tx;
    int y = (int)ty;
    for(int i=x-SIZE/2;i<x+SIZE/2;i++){
      for(int j=y-SIZE/2;j<y+SIZE/2;j++){
        if(x>=0 && x<w && y>=0 && y<h){ 
          for(int c=0;c<image->getChannels();c++){
            (*image)(i,j,c)=color[c];
          }
        }
      }
    }
  }


  // explicit template declarations
  template void fitModel<icl64f>(icl64f*,icl64f*,unsigned int,GeneralModel<icl64f>&,int,int);
  template void fitModel<icl32f>(icl32f*,icl32f*,unsigned int,GeneralModel<icl32f>&,int,int);

  template void drawModel<icl32f,icl8u>(GeneralModel<icl32f>&,Img8u*,icl8u*);
  template void drawModel<icl64f,icl8u>(GeneralModel<icl64f>&,Img8u*,icl8u*);
  template void drawModel<icl32f,icl32f>(GeneralModel<icl32f>&,Img32f*,icl32f*);
  template void drawModel<icl64f,icl32f>(GeneralModel<icl64f>&,Img32f*,icl32f*);

  template void drawQuadraticModel<icl32f,icl8u>(QuadraticModel<icl32f>&,Img8u*,icl8u*);
  template void drawQuadraticModel<icl64f,icl8u>(QuadraticModel<icl64f>&,Img8u*,icl8u*);
  template void drawQuadraticModel<icl32f,icl32f>(QuadraticModel<icl32f>&,Img32f*,icl32f*);
  template void drawQuadraticModel<icl64f,icl32f>(QuadraticModel<icl64f>&,Img32f*,icl32f*);
}
