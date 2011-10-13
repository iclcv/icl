/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/src/LLM.cpp                              **
** Module : ICLAlgorithms                                          **
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

#include <ICLAlgorithms/LLM.h>
#include <ICLCore/Mathematics.h>
#include <ICLUtils/StringUtils.h>
namespace icl{
  namespace{
    inline float square_vec(const float *a,const  float *b,unsigned  int dim){
      // {{{ open

      float sum = 0;
      for(unsigned int i=0;i<dim;++i){
        sum+=pow(a[i]-b[i],2);
      }
      return sum;
    }

    // }}}
    inline float squared_pearson_dist(const float *a,const  float *b,const float *var, unsigned  int dim){
      // {{{ open
      float sum = 0;
      for(unsigned int i=0;i<dim;++i){
        sum+=pow(a[i]-b[i],2)/(2*var[i]);
      }
      return sum;
    }

    // }}}

    inline float eucl_dist(const float *a,const  float *b,unsigned  int dim){
      // {{{ open

      return sqrt(square_vec(a,b,dim));
    }

    // }}}
    /// scalar product of row r of the w x ? matrix M with w-dim vector a
    inline float mult_mat_row(const float *M, unsigned int w, int r,const  float *a){
      // {{{ open

      M += r*w;
      float sum = 0;
      for(unsigned int i=0;i<w;++i){
        sum += M[i]*a[i];
      }
      return sum;
    }

    // }}}
 
    std::string vecToStr(const float *v, unsigned int dim){
      std::string s("{");
      for(unsigned int i=0;i<dim;++i){
        s+=str<float>(v[i])+ ((i<dim-1) ? "," : "}");
      }
      return s;
    }
  }
  
  
  LLM::Kernel::Kernel():
    // {{{ open

    w_in(0),w_out(0),A(0),dw_in(0),var(0),inputDim(0),outputDim(0){
  }

  // }}}
  LLM::Kernel::Kernel(unsigned int inputDim, unsigned int outputDim):
    // {{{ open

    inputDim(inputDim),outputDim(outputDim){
    w_in = new float [inputDim];
    w_out = new float [outputDim];
    A = new float [inputDim*outputDim];
    dw_in = new float[inputDim];
    var = new float[inputDim];
  }

  // }}}
  LLM::Kernel::~Kernel(){
    // {{{ open

    if(w_in) delete [] w_in;
    if(w_out) delete [] w_out;
    if(A) delete [] A;
    if(dw_in) delete [] dw_in;
    if(var)delete [] var;
  }

  // }}}
  LLM::Kernel &LLM::Kernel::operator=(const LLM::Kernel &k){
    // {{{ open

    inputDim = k.inputDim;
    outputDim = k.outputDim;
    if(inputDim){
      if(w_in) delete [] w_in;
      if(dw_in) delete [] dw_in;
      if(var) delete [] var;
      w_in = new float [inputDim];
      dw_in = new float [inputDim];
      var = new float [inputDim];
      memcpy(w_in,k.w_in,inputDim*sizeof(float));
      memcpy(dw_in,k.dw_in,inputDim*sizeof(float));
      memcpy(var,k.var,inputDim*sizeof(float));
    }else{
      w_in = 0;
      dw_in = 0;
      var = 0;
    }
    
    if(outputDim){
      if(w_out)delete [] w_out;
      w_out = new float [outputDim];
      memcpy(w_out,k.w_out,outputDim*sizeof(float));
    }else{
      w_out = 0;
    }
    if(inputDim && outputDim){
      if(A) delete [] A;
      A = new float [inputDim*outputDim];
      memcpy(A,k.A,inputDim*outputDim*sizeof(float));
    }else{
      A = 0;
    }
    return *this;
  }

  // }}}
  LLM::Kernel::Kernel(const LLM::Kernel &k):
    // {{{ open

    inputDim(k.inputDim),outputDim(k.outputDim){
    
    if(inputDim){
      w_in = new float [inputDim];
      dw_in = new float [inputDim];
      var = new float [inputDim];
      memcpy(w_in,k.w_in,inputDim*sizeof(float));
      memcpy(dw_in,k.dw_in,inputDim*sizeof(float));
      memcpy(var,k.var,inputDim*sizeof(float));
    }else{
      w_in = 0;
      dw_in = 0;
      var = 0;
    }
    
    if(outputDim){
      w_out = new float [outputDim];
      memcpy(w_out,k.w_out,outputDim*sizeof(float));
    }else{
      w_out = 0;
    }
    if(inputDim && outputDim){
      A = new float [inputDim*outputDim];
      memcpy(A,k.A,inputDim*outputDim*sizeof(float));
    }else{
      A = 0;
    }
  }

  // }}}
  void LLM::Kernel::show(unsigned int idx) const{
    // {{{ open

    printf("K:%d (%d>%d) w_in=%s w_out=%s A=%s sigma² = %s\n",
           idx,inputDim,outputDim,
           vecToStr(w_in,inputDim).c_str(),
           vecToStr(w_out,outputDim).c_str(),
           vecToStr(A,inputDim*outputDim).c_str(),
           vecToStr(var,inputDim).c_str());
  }

  // }}}
  
  LLM::LLM(unsigned int inputDim, unsigned int outputDim):
    // {{{ open

    m_uiInputDim(inputDim),m_uiOutputDim(outputDim),m_bUseSoftMax(true){
    m_pfOut = new float[outputDim];
    m_pfGs = 0;
    m_pfDy = new float[outputDim];


    m_fEpsilonIn = 0.01;
    m_fEpsilonOut = 0.01;
    m_fEpsilonA = 0;//0.000001;
    m_fEpsilonSigma = 0.001;
  }

  // }}}

  LLM::LLM(const LLM &llm):
    // {{{ open

    m_uiInputDim(llm.m_uiInputDim),m_uiOutputDim(llm.m_uiOutputDim),m_bUseSoftMax(llm.m_bUseSoftMax){

    m_vecKernels = llm.m_vecKernels;
    
    m_pfOut = new float[m_uiOutputDim];
    m_pfGs = llm.m_pfGs ? new float[llm.m_vecKernels.size()] : 0;
    m_pfDy = new float[m_uiOutputDim];
    
    std::copy(llm.m_pfOut,llm.m_pfOut+m_uiOutputDim,m_pfOut);
    std::copy(llm.m_pfDy,llm.m_pfDy+m_uiOutputDim,m_pfDy);
    if(m_pfGs){
      std::copy(llm.m_pfGs,llm.m_pfGs+m_vecKernels.size(),m_pfGs);
    }

    m_fEpsilonIn = llm.m_fEpsilonIn;
    m_fEpsilonOut = llm.m_fEpsilonOut;
    m_fEpsilonA = llm.m_fEpsilonA;
    m_fEpsilonSigma = llm.m_fEpsilonSigma;
    
  }

  // }}}
  LLM &LLM::operator=(const LLM &llm){
    // {{{ open

    m_uiInputDim = llm.m_uiInputDim;
    m_uiOutputDim = llm.m_uiOutputDim;
    m_bUseSoftMax = llm.m_bUseSoftMax;
 
    if(m_pfOut) delete [] m_pfOut;
    if(m_pfGs) delete [] m_pfGs;
    if(m_pfDy) delete [] m_pfDy;

    m_vecKernels = llm.m_vecKernels;
    
    m_pfOut = new float[m_uiOutputDim];
    m_pfGs = llm.m_pfGs ? new float[llm.m_vecKernels.size()] : 0;
    m_pfDy = new float[m_uiOutputDim];
    
    std::copy(llm.m_pfOut,llm.m_pfOut+m_uiOutputDim,m_pfOut);
    std::copy(llm.m_pfDy,llm.m_pfDy+m_uiOutputDim,m_pfDy);
    if(m_pfGs){
      std::copy(llm.m_pfGs,llm.m_pfGs+m_vecKernels.size(),m_pfGs);
    }

    m_fEpsilonIn = llm.m_fEpsilonIn;
    m_fEpsilonOut = llm.m_fEpsilonOut;
    m_fEpsilonA = llm.m_fEpsilonA;
    m_fEpsilonSigma = llm.m_fEpsilonSigma;
    
    return *this;
  }

  // }}}
  

  void LLM::init(unsigned int numCenters, const std::vector<Range<icl32f> > &ranges,const std::vector<float> &var){
    // {{{ open
    ICLASSERT_RETURN(ranges.size() == m_uiInputDim);
    
    std::vector<float*> centers(numCenters);
    for(unsigned int i=0;i<numCenters;++i){
      centers[i] = new float[m_uiInputDim];
      for(unsigned int j=0;j<m_uiInputDim;++j){
        centers[i][j] = icl::random((double)ranges[j].minVal, (double)ranges[j].maxVal);
      }
    }    
    init(centers,var);
    
    for(unsigned int i=0;i<numCenters;++i){
      delete [] centers[i];
    }
  }

  // }}}
  void LLM::init(const std::vector<float*> &centers,const std::vector<float> &var){
    // {{{ open

    if(m_pfGs){
      delete [] m_pfGs;
    }
    m_pfGs = new float[centers.size()];
    m_vecKernels.resize(centers.size());
    
    for(unsigned int i=0;i<centers.size();++i){
      m_vecKernels[i] = Kernel(m_uiInputDim, m_uiOutputDim);
      Kernel &k = m_vecKernels[i];
      std::fill(k.w_out, k.w_out+m_uiOutputDim,0);
      std::copy(centers[i],centers[i]+m_uiInputDim,k.w_in);
      std::fill(k.A, k.A+m_uiInputDim*m_uiOutputDim,0); // of course: A=0 --> no "steigung?"
      std::fill(k.dw_in,k.dw_in+m_uiInputDim,0);
      std::copy(var.begin(),var.end(), k.var);
    }
  }

  // }}}
  LLM::~LLM(){
    // {{{ open

    if(m_pfOut) delete [] m_pfOut;
    if(m_pfGs) delete [] m_pfGs;
    if(m_pfDy) delete [] m_pfDy;
  }

  // }}}
  void LLM::showKernels() const{
    // {{{ open

    printf("llm kernels: \n");
    for(unsigned int i=0;i<m_vecKernels.size();++i){
      m_vecKernels[i].show(i);
    }
    printf("------------\n");
  }

  // }}}
  
  
  const float *LLM::apply(const float *x){
    // {{{ open
    return applyIntern(x,updateGs(x));
  }

  // }}}
  const float *LLM::applyIntern(const float *x, const float *g){
    // {{{ open

    // y_net(x) = sum_i  (w_i^out + Ai*(x-w_i^in))*g_i(x)  
    unsigned int N = m_vecKernels.size();
    unsigned int ID = m_uiInputDim;
    unsigned int OD = m_uiOutputDim;
    float *out = m_pfOut;
    float *inbuf = new float[ID];
    float *outbuf = new float [OD];

    for(unsigned int d=0;d<OD;++d){
      out[d]=0;
      for(unsigned int i=0;i<N;++i){
        Kernel &k = m_vecKernels[i];
        for(unsigned int j=0;j<ID;++j){
          inbuf[j] = x[j] - k.w_in[j];
        }
        out[d] += (k.w_out[d] + mult_mat_row(k.A,ID,d,inbuf)) * g[i];
      }
    }
    
    delete [] outbuf;
    delete [] inbuf;
    return out;
  }

  // }}}
  void LLM::train(const float *x,const float *y, int trainflags){
    // {{{ open
    const float *g = updateGs(x);
    if(trainflags & 1){
      trainCentersIntern(x,g);
    }
    if( (trainflags >> 1) & 1){
      trainSigmasIntern(x,g);
    }

    const float *dy = 0;
    if( (trainflags >> 2) & 1){
      dy = getErrorVecIntern(y,applyIntern(x, g));
      trainOutputsIntern(x,y,g,dy,((trainflags&1)&&m_fEpsilonIn)?true:false);
    }
    if( (trainflags >> 3) & 1){
      if(!dy) dy = getErrorVecIntern(y,applyIntern(x, g));
      trainMatricesIntern(x,y,g,dy);
    }    
  }

  // }}}
  
  void LLM::trainCenters(const float *x){
    // {{{ open

    trainCentersIntern(x,updateGs(x));
  }

  // }}}
  void LLM::trainSigmas(const float *x){
    // {{{ open

    trainSigmasIntern(x,updateGs(x));
  }

  // }}}
  void LLM::trainOutputs(const float *x,const float *y){
    // {{{ open
    trainOutputsIntern(x,y,updateGs(x),getErrorVec(x,y),false);
  }

  // }}}
  void LLM::trainMatrices(const float *x,const float *y){
    // {{{ open

    trainMatricesIntern(x,y,updateGs(x),getErrorVec(x,y));
  }

  // }}}
  
  const float *LLM::updateGs(const float *x){
    // {{{ open

    float *g = m_pfGs;
    if(m_bUseSoftMax){
      //e/ calculate g_i(x) = (exp(-beta*|x-w_i^in|)) / (sum_j exp(-beta*|x-w_j^in|))
      float sum_gi = 0;
      for(unsigned int i=0;i<m_vecKernels.size();++i){
        g[i] = exp(-squared_pearson_dist(x,m_vecKernels[i].w_in,m_vecKernels[i].var,m_uiInputDim));
        sum_gi += g[i];
      }
      if(sum_gi){ // if softmax is off do this not!
        for(unsigned int i=0;i<m_vecKernels.size();++i){
          g[i] /= sum_gi;
        }
      }
    }else{
      for(unsigned int i=0;i<m_vecKernels.size();++i){
        
        g[i] = exp(-squared_pearson_dist(x,m_vecKernels[i].w_in,m_vecKernels[i].var,m_uiInputDim));
      }
      
    }
    return g;
  }

  // }}}
  const float *LLM::getErrorVec(const float *x, const float *y){
    // {{{ open

    return getErrorVecIntern(y,apply(x));
  }

  // }}}
  const float *LLM::getErrorVecIntern(const float *y, const float *ynet){
    // {{{ open

    for(unsigned int i=0;i<m_uiOutputDim;++i){
      m_pfDy[i] = y[i]-ynet[i];
    }
    return m_pfDy;
  }

  // }}}
  void LLM::trainCentersIntern(const float *x,const float *g){
    // {{{ open

    if(!m_fEpsilonIn) return;
    //    printf("training of centers g=%s \n",vecToStr(g,m_vecKernels.size()).c_str());
    for(unsigned int i=0;i<m_vecKernels.size();++i){
      for(unsigned int d=0;d<m_uiInputDim;++d){
        Kernel &k = m_vecKernels[i];
        k.dw_in[d] = m_fEpsilonIn*(x[d]-k.w_in[d])*g[i];
        k.w_in[d] += k.dw_in[d];
      } 
    }
  }

  // }}}
  void LLM::trainSigmasIntern(const float *x,const float *g){
    // {{{ open

    if(!m_fEpsilonSigma) return;
    for(unsigned int i=0;i<m_vecKernels.size();++i){
      Kernel &k = m_vecKernels[i];
      //      m_vecKernels[i].var += m_fEpsilonSigma * square_vec(x,m_vecKernels[i].w_in,m_uiInputDim) - m_vecKernels[i].var*g[i];
      for(unsigned int j=0;j<m_uiInputDim;++j){
        k.var[j] += m_fEpsilonSigma * pow(x[j]-k.w_in[j],2) - k.var[j]*g[i]; 
      }
    }
  }

  // }}}
  void LLM::trainOutputsIntern(const float *x,const float *y, const float *g, const float *dy, bool useDeltaWin){
    // {{{ open
    if(!m_fEpsilonOut) return;
    if(useDeltaWin){
      for(unsigned int i=0;i<m_vecKernels.size();++i){
        for(unsigned int d=0;d<m_uiOutputDim;++d){
          m_vecKernels[i].w_out[d] += m_fEpsilonOut * g[i] * dy[d] + mult_mat_row(m_vecKernels[i].A,m_uiInputDim,d,m_vecKernels[i].dw_in);
        } 
      }    
    }else{
      for(unsigned int i=0;i<m_vecKernels.size();++i){
        for(unsigned int d=0;d<m_uiOutputDim;++d){
          m_vecKernels[i].w_out[d] += m_fEpsilonOut * g[i] * dy[d];
        } 
      }       
    }
  }

  // }}}
  void LLM::trainMatricesIntern(const float *x,const float *y, const float *g, const float *dy){
    // {{{ open
    if(!m_fEpsilonA) return;
    for(unsigned int i=0;i<m_vecKernels.size();++i){
      //float fNorm = square_vec(x,m_vecKernels[i].w_in,m_uiInputDim);
      float fNorm = sqrt(square_vec(x,m_vecKernels[i].w_in,m_uiInputDim)); /// hack!!
      
      if(!fNorm) continue;
      for(unsigned int xx=0;xx<m_uiInputDim;++xx){
        for(unsigned int yy=0;yy<m_uiOutputDim;++yy){
          float &a = m_vecKernels[i].A[yy*m_uiInputDim+xx];
          a += m_fEpsilonA * g[i] * dy[yy] * (x[xx]-m_vecKernels[i].w_in[xx])/fNorm;
        }
      } 
    }   
  }

  // }}}
}

