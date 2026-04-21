// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/filter/GaborOp.h>
#include <icl/filter/ConvolutionOp.h>
#include <icl/core/Image.h>
#include <cmath>

using namespace icl::utils;
using namespace icl::core;

namespace icl::filter {
  // Refresh the "kernel preview" image property payload from the currently
  // configured kernels. Called at the end of updateKernels(); the Prop GUI
  // widget polls the payload via its volatileness timer.
  void GaborOp::updatePreview(){
    if(m_vecKernels.empty()) return;
    Img32f k = m_vecKernels[0].detached();
    k.normalizeAllChannels(Range<float>(0, 255));
    setPropertyPayload("kernel preview", std::any(core::Image(k)));
  }

  // Rebuild kernel bank from the scalar properties — collapses the 5
  // multi-value bank slots into single-element vectors so the playground
  // shows one kernel per knob-set. The multi-value bank API (addLambda/
  // addTheta/...) still exists for direct users.
  void GaborOp::rebuildFromProperties(){
    m_oKernelSize = Size(parse<int>(prop("size.w").value),
                         parse<int>(prop("size.h").value));
    m_vecLambdas = {parse<float>(prop("lambda").value)};
    m_vecThetas  = {parse<float>(prop("theta").value)};
    m_vecPsis    = {parse<float>(prop("psi").value)};
    m_vecSigmas  = {parse<float>(prop("sigma").value)};
    m_vecGammas  = {parse<float>(prop("gamma").value)};
    updateKernels();
  }

  void GaborOp::addGaborProperties(){
    addProperty("size.w","range:spinbox","[3,50]",str(m_oKernelSize.width  > 0 ? m_oKernelSize.width  : 10));
    addProperty("size.h","range:spinbox","[3,50]",str(m_oKernelSize.height > 0 ? m_oKernelSize.height : 10));
    addProperty("lambda","range:slider","[0.1,100]:0.1",
                str(m_vecLambdas.empty() ? 20.f : m_vecLambdas.front()));
    addProperty("theta","range:slider","[0,3.15]:0.01",
                str(m_vecThetas.empty() ? 0.f : m_vecThetas.front()));
    addProperty("psi","range:slider","[0,50]:0.1",
                str(m_vecPsis.empty() ? 0.f : m_vecPsis.front()));
    addProperty("sigma","range:slider","[0.1,30]:0.1",
                str(m_vecSigmas.empty() ? 5.f : m_vecSigmas.front()));
    addProperty("gamma","range:slider","[0.01,10]:0.01",
                str(m_vecGammas.empty() ? 0.5f : m_vecGammas.front()));
    addProperty("kernel preview","image","", Any(), /*volatileness=*/100);
    registerCallback([this](const Property &p){
      static const std::string knobs[] =
        {"size.w","size.h","lambda","theta","psi","sigma","gamma"};
      for(const auto &k : knobs){
        if(p.name == k){ rebuildFromProperties(); return; }
      }
    });
  }

  GaborOp::GaborOp(){
    addGaborProperties();
    // Seed the kernel bank from the property defaults so the very first
    // apply() produces a non-empty result (and the preview payload is set).
    rebuildFromProperties();
  }
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

    // Properties must exist before updateKernels runs, since updateKernels
    // calls updatePreview → setPropertyPayload("kernel preview", ...).
    addGaborProperties();
    updateKernels();
  }

  GaborOp::~GaborOp(){}

  REGISTER_CONFIGURABLE_DEFAULT(GaborOp);

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
    // Explicit lock covers direct callers (ctors) — the callback-driven path
    // is already auto-wrapped by UnaryOp::registerCallback. Recursive so the
    // nested acquire is harmless.
    std::scoped_lock lock(m_applyMutex);
    m_vecKernels.clear();
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
              m_vecResults.push_back(Image());
              delete k;
            }
          }
        }
      }
    }
    updatePreview();
  }

  void GaborOp::apply(const Image &src, Image &dst) {
    ICLASSERT_RETURN(!src.isNull());
    // Reader-side lock — the callback side is auto-wrapped by UnaryOp.
    std::scoped_lock lock(m_applyMutex);

    // Gabor kernels are real-valued (sub-unit coefficients). Convolving them
    // as "custom" kernels against an integer source forces a lossy int cast
    // that zeroes the kernel — and produces an integer result that Img32f
    // cannot append. Normalize to 32f once up front, reusing an internal
    // buffer across frames.
    const Image *src32f = &src;
    if(src.getDepth() != depth32f){
      if(m_src32fBuffer.isNull()
         || m_src32fBuffer.getSize()      != src.getSize()
         || m_src32fBuffer.getChannels()  != src.getChannels()
         || m_src32fBuffer.getFormat()    != src.getFormat()){
        m_src32fBuffer = Image(src.getSize(), depth32f, src.getChannels(), src.getFormat());
      }
      src.convertTo(m_src32fBuffer);
      src32f = &m_src32fBuffer;
    }

    Img32f result(Size::null, 0);

    for(unsigned int i = 0; i < m_vecKernels.size(); i++){
      ConvolutionOp co(ConvolutionKernel(m_vecKernels[i].getData(0), m_oKernelSize, false));
      co.setCheckOnly(false);
      co.setClipToROI(true);

      co.apply(*src32f, m_vecResults[i]);

      result.setSize(m_vecResults[i].getSize());
      result.append(&m_vecResults[i].as32f());
    }

    dst = Image(result);
  }

  std::vector<icl32f> GaborOp::apply(const ImgBase *poSrc, const Point &p){
    ICLASSERT_RETURN_VAL( poSrc && poSrc->getChannels() && poSrc->getSize() != Size::null, std::vector<icl32f>() );
    std::vector<icl32f> v;

    Img32f resPix(Size(1,1),poSrc->getChannels());
    ImgBase *resPixBase  = &resPix;

    const ImgBase *poSrcROIPix = poSrc->shallowCopy(Rect(p,Size(1,1)));

    for(unsigned int i=0;i<m_vecKernels.size();i++){
      ConvolutionOp co(ConvolutionKernel(m_vecKernels[i].getData(0),m_oKernelSize, false));
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

    Channel32f k = (*poKernelImage)[0];

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

  } // namespace icl::filter