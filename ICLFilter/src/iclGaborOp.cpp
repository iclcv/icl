#include "iclGaborOp.h"
#include "iclConvolutionOp.h"
#include <iclImgChannel.h>
#include <math.h>

using namespace std;

namespace icl{
  GaborOp::GaborOp(){}
  GaborOp::GaborOp(const Size &kernelSize,
                   std::vector<icl32f> lambdas,
                   std::vector<icl32f> thetas,
                   std::vector<icl32f> psis,
                   std::vector<icl32f> sigmas,
                   std::vector<icl32f> gammas){
    m_vecLambdas = lambdas;
    m_vecThetas = thetas;
    m_vecPsis = psis;
    m_vecSigmas = sigmas;
    m_vecGammas = gammas;
    m_oKernelSize = kernelSize;
    
    updateKernels();
  }
  
  GaborOp::~GaborOp(){}
  
  void GaborOp::setKernelSize(const Size &size){
    m_oKernelSize = size;
   
    updateKernels();
  }
  
  void GaborOp::addLambda(float lambda){ m_vecLambdas.push_back(lambda); }
  void GaborOp::addTheta(float theta){ m_vecThetas.push_back(theta); }
  void GaborOp::addPsi(float psi){ m_vecPsis.push_back(psi); }
  void GaborOp::addSigma(float sigma){ m_vecSigmas.push_back(sigma); }
  void GaborOp::addGamma(float gamma){ m_vecGammas.push_back(gamma); }
  
  void GaborOp::updateKernels(){
    m_vecKernels.clear();
    for(unsigned int i=0;i<m_vecResults.size();i++){
      delete m_vecResults[i];
    }
    m_vecResults.clear();
    
    ICLASSERT_RETURN( m_oKernelSize != Size::null );

    for(unsigned int l = 0;l<m_vecLambdas.size();l++){
      for(unsigned int t = 0;t<m_vecThetas.size();t++){
        for(unsigned int p = 0;p<m_vecPsis.size();p++){
          for(unsigned int s = 0;s<m_vecSigmas.size();s++){
            for(unsigned int g = 0;g<m_vecGammas.size();g++){
              Img32f *k = createKernel(m_oKernelSize,
                                       m_vecLambdas[l],
                                       m_vecThetas[t],
                                       m_vecPsis[p],
                                       m_vecSigmas[s],
                                       m_vecGammas[g]);
              m_vecKernels.push_back(*k);
              m_vecResults.push_back(0);
              delete k;
            }
          }
        }
      }
    }
  }

  void GaborOp::apply(const ImgBase *poSrc, ImgBase **ppoDst){
    ICLASSERT_RETURN( poSrc && ppoDst );

    if(!*ppoDst){
      *ppoDst = new Img32f(Size::null,0);
    }
    ImgBase *poDst = *ppoDst;
    poDst->setChannels(0);
    poDst->setSize(Size::null);
    
    for(unsigned int i=0;i<m_vecKernels.size();i++){
      ConvolutionOp co(m_vecKernels[i].getData(0),m_oKernelSize, false);
      co.setCheckOnly(false);
      co.setClipToROI(true);

      co.apply(poSrc,&(m_vecResults[i]));

      poDst->setSize(m_vecResults[i]->getSize());
      poDst->asImg<icl32f>()->append(m_vecResults[i]->asImg<icl32f>());
    }
  }
  
  vector<icl32f> GaborOp::apply(const ImgBase *poSrc, const Point &p){
    ICLASSERT_RETURN_VAL( poSrc && poSrc->getChannels() && poSrc->getSize() != Size::null, vector<icl32f>() );
    vector<icl32f> v;
    
    
    Img32f resPix(Size(1,1),poSrc->getChannels());
    ImgBase *resPixBase  = &resPix;
    
    const ImgBase *poSrcROIPix = poSrc->shallowCopy(Rect(p,Size(1,1)));

    for(unsigned int i=0;i<m_vecKernels.size();i++){
      ConvolutionOp co(m_vecKernels[i].getData(0),m_oKernelSize, false);
      co.setCheckOnly(false);
      co.setClipToROI(true);

      co.apply(poSrcROIPix,&resPixBase);
      for(int c=0;c<resPix.getChannels();c++){
        v.push_back(resPix(0,0,c));
      }
    }   
    return v;
  }
 

  Img32f *GaborOp::createKernel(const Size &size, float lambda, float theta, float psi, float sigma, float gamma){
    Img32f *poKernelImage = new Img32f (size,1);
    
    int xCenter = size.width/2;
    int yCenter = size.height/2;
    
    ImgChannel32f k = pickChannel(poKernelImage,0);
    
    gamma *=gamma;
    sigma *=sigma*2;
    
    for(int x=0;x<k.getWidth();++x){
      for(int y=0;y<k.getHeight();++y){
        float xTrans = xCenter-x;
        float yTrans = yCenter-y;
        float x2 = xTrans*cos(theta) + yTrans*sin(theta);
        float y2 = -xTrans*sin(theta) + yTrans*cos(theta);
        
        k(x,y) = exp( -(x2*x2+gamma*y2*y2)/sigma) * cos( (2.0*M_PI*x2)/lambda  + psi );
      }
    }
    
    return poKernelImage;
  }
}
