#include <iclGLTextureMapBaseImage.h>


namespace icl{

  namespace{
    template<class T>
    inline void SAVE_DEL(T *&p){
      if(p){
        delete p;
        p = 0;
      }
    }

    int getCellSize(const Size &imageSize){
      if(imageSize.getDim() < 100) return 16;
      if(imageSize.getDim() < 2500) return 32;
      if(imageSize.getDim() < 10000) return 64;
      if(imageSize.getDim() < 3000000) return 128;
      return 256;
    }
    void append(const ImgBase *src, ImgBase *dst){
      switch(src->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: dst->asImg<icl##D>()->append(dst->asImg<icl##D>()); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
      }
    }
  }
  

  GLTextureMapBaseImage::~GLTextureMapBaseImage(){
    SAVE_DEL(m_po8u);
    SAVE_DEL(m_po16s);
    SAVE_DEL(m_po32s);
    SAVE_DEL(m_po32f);
    SAVE_DEL(m_poChannelBuf);    
  }
  
  const ImgBase *GLTextureMapBaseImage::adaptChannels(const ImgBase *image){
    ICLASSERT_RETURN_VAL(image && image->getChannels(), 0);
    switch(image->getChannels()){
      case 1: 
      case 3: 
      case 4: return image;
      case 2:  /// add an empty channel
        ensureCompatible(&m_poChannelBuf,image->getDepth(),image->getSize(),1);
        append(image,m_poChannelBuf);
        m_poChannelBuf->swapChannels(0,2);
        return m_poChannelBuf;
      default: // use first 3 channels
        std::vector<int> idxs; idxs.push_back(0);idxs.push_back(1);idxs.push_back(2);
        return const_cast<ImgBase*>(image)->selectChannels(idxs,&m_poChannelBuf);
    }
  }  


  void GLTextureMapBaseImage::updateTextures(const ImgBase *imageIn){
    const ImgBase *image = adaptChannels(imageIn);
    ICLASSERT_RETURN(image);
    
#define APPLY_FOR(D)                                                                                                      \
        if(m_po##D && !(m_po##D->compatible(image->asImg<icl##D>()))){                                                    \
          SAVE_DEL(m_po##D);                                                                                              \
        }                                                                                                                 \
        if(!m_po##D){                                                                                                     \
          m_po##D = new GLTextureMapImage<icl##D>(image->getSize(),image->getChannels(), getCellSize(image->getSize()));  \
          m_po##D->bci(m_aiBCI[0],m_aiBCI[1],m_aiBCI[2]);                                                                 \
        }                                                                                                                 \
        m_po##D->updateTextures(image->asImg<icl##D>());                                                                  \
        break;                                                                                                                  
    
    switch(image->getDepth()){
      case depth8u:
        SAVE_DEL(m_po16s);
        SAVE_DEL(m_po32s);
        SAVE_DEL(m_po32f);
        APPLY_FOR(8u);
      case depth16s:
        SAVE_DEL(m_po8u);
        SAVE_DEL(m_po32s);
        SAVE_DEL(m_po32f);
        APPLY_FOR(16s);
      case depth32s:
        SAVE_DEL(m_po8u);
        SAVE_DEL(m_po16s);
        SAVE_DEL(m_po32f);
        APPLY_FOR(32s);
      case depth32f:
        SAVE_DEL(m_po8u);
        SAVE_DEL(m_po16s);
        SAVE_DEL(m_po32s);
        APPLY_FOR(32f);
      case depth64f:{ // fallback!!
        Img<icl32f> *tmp = image->convert(depth32f)->asImg<icl32f>();
        updateTextures(tmp);
        delete tmp;
      }default:
        ICL_INVALID_DEPTH;
        break;
    }    
  }
  
  Size GLTextureMapBaseImage::getImageSize(){
    if(m_po8u)return Size (m_po8u->getImageWidth(), m_po8u->getImageHeight());
    if(m_po16s)return Size (m_po16s->getImageWidth(), m_po16s->getImageHeight());
    if(m_po32s)return Size (m_po32s->getImageWidth(), m_po32s->getImageHeight());
    if(m_po32f)return Size (m_po32f->getImageWidth(), m_po32f->getImageHeight());
    return Size::null;
  }

  void GLTextureMapBaseImage::bci(int b, int c, int i){
    m_aiBCI[0]= b;
    m_aiBCI[1]= c;
    m_aiBCI[2]= i;
    if(m_po8u) { m_po8u->bci(b,c,i); return; }
    if(m_po16s) { m_po16s->bci(b,c,i); return; }
    if(m_po32s) { m_po32s->bci(b,c,i); return; }
    if(m_po32f) { m_po32f->bci(b,c,i); return; }
  }

  void GLTextureMapBaseImage::drawTo(const Rect &rect, const Size &windowSize){
    if(m_po8u){
      m_po8u->drawTo(rect,windowSize);
      return;
    }
    if(m_po32f){
      m_po32f->drawTo(rect,windowSize);
      return;
    }
    if(m_po16s){
      m_po16s->drawTo(rect,windowSize);
      return;
    }
    if(m_po32s){
      m_po32s->drawTo(rect,windowSize);
      return;
    }
  }


}
