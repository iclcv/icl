/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/LLM.cpp                                    **
** Module : ICLMath                                                **
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

#include <ICLMath/LLM.h>
#include <ICLCore/Mathematics.h>
#include <ICLUtils/StringUtils.h>
namespace icl{
  namespace math{
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
    
    void LLM::Kernel::set(const float *w_in, const float *w_out, const float *A){
      std::copy(w_in, w_in+this->inputDim, this->w_in );
      std::copy(w_out, w_out+this->outputDim,this->w_out);
      std::copy(w_in, w_in+this->inputDim*this->outputDim, this->A);
    }
  
  
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
  
  
    void LLM::init_private(unsigned int inputDim, unsigned int outputDim){
      m_inputDim = inputDim;
      m_outputDim = outputDim;
      // m_bUseSoftMax = true;
  
      m_outBuf.resize(outputDim);
      m_gBuf.resize(outputDim);
      m_errorBuf.resize(outputDim);
  
      /*
          m_epsilonIn = 0.01;
          m_epsilonOut = 0.01;
          m_epsilonA = 0;//0.000001;
          m_epsilonSigma = 0.001;
      */
  
      addProperty("epsilon In","range","[0,0.1]",0.01,0,"input weight learning rate");
      addProperty("epsilon Out","range","[0,0.5]",0.01,0,"output weight learning rate");
      addProperty("epsilon A","range","[0,0.1]",0.001,0,"slope matrix learning rate");
      addProperty("epsilon Sigma","range","[0,0.1]",0.0,0,"kernel variance learning rate");
      addProperty("soft max enabled","flag","",true,0,"enables the soft-max interpolation");
    }
    
    LLM::LLM(unsigned int inputDim, unsigned int outputDim){
      // {{{ open
      init_private(inputDim,outputDim);
      
    }
  
    // }}}
  
    LLM::LLM(unsigned int inputDim, unsigned int outputDim, unsigned int numCenters, 
             const std::vector<Range<icl32f> > &ranges, 
             const std::vector<float> &var){
      init_private(inputDim,outputDim);
      init(numCenters, ranges, var);
    }
    
  
    void LLM::init(unsigned int numCenters, const std::vector<Range<icl32f> > &ranges,const std::vector<float> &var){
      // {{{ open
      ICLASSERT_RETURN(ranges.size() == m_inputDim);
      
      std::vector<float*> centers(numCenters);
      for(unsigned int i=0;i<numCenters;++i){
        centers[i] = new float[m_inputDim];
        for(unsigned int j=0;j<m_inputDim;++j){
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
  
      m_gBuf.resize(centers.size());
      m_kernels.resize(centers.size());
      
      for(unsigned int i=0;i<centers.size();++i){
        m_kernels[i] = Kernel(m_inputDim, m_outputDim);
        Kernel &k = m_kernels[i];
        std::fill(k.w_out, k.w_out+m_outputDim,0);
        std::copy(centers[i],centers[i]+m_inputDim,k.w_in);
        std::fill(k.A, k.A+m_inputDim*m_outputDim,0); // of course: A=0 --> no "steigung?"
        std::fill(k.dw_in,k.dw_in+m_inputDim,0);
        std::copy(var.begin(),var.end(), k.var);
      }
    }
  
    // }}}
  
    void LLM::showKernels() const{
      // {{{ open
  
      printf("llm kernels: \n");
      for(unsigned int i=0;i<m_kernels.size();++i){
        m_kernels[i].show(i);
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
      unsigned int N = m_kernels.size();
      unsigned int ID = m_inputDim;
      unsigned int OD = m_outputDim;
      float *out = m_outBuf.data();
      float *inbuf = new float[ID];
      float *outbuf = new float [OD];
  
      for(unsigned int d=0;d<OD;++d){
        out[d]=0;
        for(unsigned int i=0;i<N;++i){
          Kernel &k = m_kernels[i];
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
      const float eIn = getPropertyValue("epsilon In");
  
      const float *dy = 0;
      if( (trainflags >> 2) & 1){
        dy = getErrorVecIntern(y,applyIntern(x, g));
        trainOutputsIntern(x,y,g,dy,((trainflags&1)&&eIn)?true:false);
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
  
      float *g = m_gBuf.data();
      if(getPropertyValue("soft max enabled")){
        //e/ calculate g_i(x) = (exp(-beta*|x-w_i^in|)) / (sum_j exp(-beta*|x-w_j^in|))
        float sum_gi = 0;
        for(unsigned int i=0;i<m_kernels.size();++i){
          g[i] = exp(-squared_pearson_dist(x,m_kernels[i].w_in,m_kernels[i].var,m_inputDim));
          sum_gi += g[i];
        }
        if(sum_gi){ // if softmax is off do this not!
          for(unsigned int i=0;i<m_kernels.size();++i){
            g[i] /= sum_gi;
          }
        }
      }else{
        for(unsigned int i=0;i<m_kernels.size();++i){
          
          g[i] = exp(-squared_pearson_dist(x,m_kernels[i].w_in,m_kernels[i].var,m_inputDim));
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
  
      for(unsigned int i=0;i<m_outputDim;++i){
        m_errorBuf[i] = y[i]-ynet[i];
      }
      return m_errorBuf.data();
    }
  
    // }}}
    void LLM::trainCentersIntern(const float *x,const float *g){
      // {{{ open
      const float eIn = getPropertyValue("epsilon In"); 
      if(!eIn) return;
      //    printf("training of centers g=%s \n",vecToStr(g,m_kernels.size()).c_str());
      for(unsigned int i=0;i<m_kernels.size();++i){
        for(unsigned int d=0;d<m_inputDim;++d){
          Kernel &k = m_kernels[i];
          k.dw_in[d] = eIn*(x[d]-k.w_in[d])*g[i];
          k.w_in[d] += k.dw_in[d];
        } 
      }
    }
  
    // }}}
    void LLM::trainSigmasIntern(const float *x,const float *g){
      // {{{ open
      const float eS = getPropertyValue("epsilon Sigma");
      if(!eS) return;
      for(unsigned int i=0;i<m_kernels.size();++i){
        Kernel &k = m_kernels[i];
        //      m_kernels[i].var += eS * square_vec(x,m_kernels[i].w_in,m_inputDim) - m_kernels[i].var*g[i];
        for(unsigned int j=0;j<m_inputDim;++j){
          k.var[j] += eS * pow(x[j]-k.w_in[j],2) - k.var[j]*g[i]; 
        }
      }
    }
  
    // }}}
    void LLM::trainOutputsIntern(const float *x,const float *y, const float *g, const float *dy, bool useDeltaWin){
      // {{{ open
      const float eO = getPropertyValue("epsilon Out");
      if(!eO) return;
      if(useDeltaWin){
        for(unsigned int i=0;i<m_kernels.size();++i){
          for(unsigned int d=0;d<m_outputDim;++d){
            m_kernels[i].w_out[d] += eO * g[i] * dy[d] + mult_mat_row(m_kernels[i].A,m_inputDim,d,m_kernels[i].dw_in);
          } 
        }    
      }else{
        for(unsigned int i=0;i<m_kernels.size();++i){
          for(unsigned int d=0;d<m_outputDim;++d){
            m_kernels[i].w_out[d] += eO * g[i] * dy[d];
          } 
        }       
      }
    }
  
    // }}}
    void LLM::trainMatricesIntern(const float *x,const float *y, const float *g, const float *dy){
      // {{{ open
      const float eA = getPropertyValue("epsilon A");
      if(!eA) return;
      for(unsigned int i=0;i<m_kernels.size();++i){
        //float fNorm = square_vec(x,m_kernels[i].w_in,m_inputDim);
        float fNorm = sqrt(square_vec(x,m_kernels[i].w_in,m_inputDim)); /// hack!!
        
        if(!fNorm) continue;
        for(unsigned int xx=0;xx<m_inputDim;++xx){
          for(unsigned int yy=0;yy<m_outputDim;++yy){
            float &a = m_kernels[i].A[yy*m_inputDim+xx];
            a += eA * g[i] * dy[yy] * (x[xx]-m_kernels[i].w_in[xx])/fNorm;
          }
        } 
      }   
    }
  
    // }}}
    
    REGISTER_CONFIGURABLE(LLM, return new LLM(1,1));
  } // namespace math
}

