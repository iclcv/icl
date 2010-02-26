#include <ICLAlgorithms/SOM.h>
#include <ICLUtils/Macros.h>
#include <ICLUtils/Exception.h>
#include <cstring>

using namespace std;

namespace icl{

  namespace som{
    
    static inline float distance2(const float *a,const float *b, unsigned int dim){
      // {{{ open

      float sum = 0;
      for(unsigned int i=0;i<dim;++i){
        sum+=::pow(a[i]-b[i],2);
      }
      return sum;
    }    

    // }}}
    
    static inline float def_h_func(const float *r,const float *s, float sigma, unsigned int dim){
      // {{{ open

      return ::exp( -distance2(r,s,dim)/(2*sigma*sigma) );
    }

    // }}}

  }
  
 
  SOM::Neuron::Neuron(SOM::Neuron::vector_type gridpos,SOM::Neuron::vector_type prototype,unsigned int griddim, unsigned int datadim):
    // {{{ open
    gridpos(gridpos),prototype(prototype),griddim(griddim),datadim(datadim){
  }

  // }}}

  SOM::SOM(unsigned int dataDim, const std::vector<unsigned int> &dims,const std::vector<Range<float> > &prototypeBounds, float epsilon, float sigma){
    // {{{ open

    ICLASSERT_THROW(dataDim>0,ICLException("SOM data dimension must be > 0"));
    ICLASSERT_THROW(dims.size()>0,ICLException("SOM grid dimension must be > 0"));
    m_uiDataDim = dataDim;
    m_uiSomDim = dims.size();
    m_vecDimensions = dims;
    m_vecPrototypeBounds = prototypeBounds;
    m_fEpsilon = epsilon;
    m_fSigma = sigma;

    // count neuron count
    unsigned int dim = dims[0];
    for(unsigned int i=1;i<m_uiSomDim;++i){
      dim *= dims[i];
    }
    ICLASSERT_THROW(dim > 0,ICLException("Product of SOM dimensions must be > 0"));
    
    m_vecNeurons.resize(dim);

    // calculate offsets for each dimension in the planar neurons array
    m_vecDimOffsets.resize(m_uiSomDim);
    m_vecDimOffsets[0] = 1;

    for(unsigned int i=1;i<m_uiSomDim;i++){
      m_vecDimOffsets[i] = i>1 ? m_vecDimOffsets[i-1]*dims[i-1] : dims[i-1];
    }
    
    // Create the neurons
    for(unsigned int i=0;i<dim;++i){
      float *gridpos = new float[m_uiSomDim];
      float *prototype = new float[m_uiDataDim];
      
      // calculate the corresponding grid location ( TODO check check check! )
      int iRest = i;
      for(int d=m_uiSomDim-1;d>=0;--d){
        gridpos[d] = iRest ? iRest/m_vecDimOffsets[d] : 0;
        iRest -= gridpos[d]*m_vecDimOffsets[d];
      }
      ICLASSERT_THROW(iRest == 0,ICLException("Somethings going wrong here! [code 1240/B.l]") );
      
      // create some randomly initialized prototypes (using the given ranges for each dimension)
      for(unsigned int d=0;d<m_uiDataDim;++d){
        prototype[d] = random((double)m_vecPrototypeBounds[d].minVal,(double)m_vecPrototypeBounds[d].maxVal);
      }
      
      // set up new neuron
      m_vecNeurons[i] = Neuron(gridpos,prototype,m_uiSomDim,m_uiDataDim);
    }
  }

  // }}}
 
    SOM::~SOM(){
    // {{{ open

    m_uiDataDim = 0;
    m_vecNeurons.clear();
    m_fEpsilon = 0;
  }

  // }}}

  void SOM::train(const float *input){
    // {{{ open

    const Neuron &s = getWinner(input);
    
    for(unsigned int i=0;i<m_vecNeurons.size();++i){
      Neuron &r = m_vecNeurons[i];
      for(unsigned int d=0;d<m_uiDataDim;++d){
        r.prototype.get()[d] += m_fEpsilon * som::def_h_func(s.gridpos.get(),r.gridpos.get(),m_fSigma,m_uiSomDim) * ( input[d]-r.prototype.get()[d] );
      }
    }
  }

  // }}}
  const vector<SOM::Neuron> &SOM::getNeurons() const{
    // {{{ open

    return m_vecNeurons;
  }

  // }}}
  vector<SOM::Neuron> &SOM::getNeurons(){
    // {{{ open
    return m_vecNeurons;
  }

  // }}}


  const SOM::Neuron &SOM::getWinner(const float *input) const{
    // {{{ open

    static Neuron dummy;
    ICLASSERT_RETURN_VAL(m_vecNeurons.size(),dummy);
    
    unsigned int minIdx = 0;
    float minDist = som::distance2(m_vecNeurons[0].prototype.get(),input,m_uiDataDim);
    
    for(unsigned int i=0;i<m_vecNeurons.size();++i){
      float dist = som::distance2(m_vecNeurons[i].prototype.get(),input,m_uiDataDim);
      if(dist < minDist){
        minDist = dist;
        minIdx = i;
      }
    }  
    return m_vecNeurons[minIdx];
  }

  // }}}
  SOM::Neuron &SOM::getWinner(const float *input){
    // {{{ open
    return const_cast<Neuron&>(const_cast<const SOM*>(this)->getWinner(input));
  }

  // }}}

  const SOM::Neuron &SOM::getNeuron(const std::vector<int> &dims) const{
    // {{{ open

    static Neuron dummy;
    ICLASSERT_RETURN_VAL(dims.size() == m_uiSomDim, dummy);
    unsigned int idx = 0;
    for(unsigned int i=0;i<m_uiSomDim;++i){
      idx+=m_vecDimOffsets[i]*dims[i];
    }
    ICLASSERT_RETURN_VAL(idx < m_vecNeurons.size(),dummy);
    return m_vecNeurons[idx];
  }

  // }}}
  SOM::Neuron &SOM::getNeuron(const std::vector<int> &dims){
    // {{{ open
    return const_cast<Neuron&>(const_cast<const SOM*>(this)->getNeuron(dims));
  }

  // }}}

  void SOM::setEpsilon(float epsilon){
    // {{{ open

    m_fEpsilon = epsilon;
  }

  // }}}
  void SOM::setSigma(float sigma){
    // {{{ open

    m_fSigma = sigma;
  }

  // }}}

}
