/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/GLImg.cpp                                    **
** Module : ICLQt                                                  **
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

#include <ICLCore/Img.h>
#include <ICLQt/GLImg.h>
#include <ICLQt/GLContext.h>
#include <ICLCore/CCFunctions.h>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <ICLUtils/Array2D.h>

#include <ICLQuick/Quick.h>
#include <ICLUtils/Rect32f.h>
#include <ICLUtils/Lockable.h>

#ifdef HAVE_QT
#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QMutex>
#include <QtOpenGL/QGLContext>
#endif

namespace icl{
  struct WhiteTexture{
    Img8u image;
    GLImg gli;
    WhiteTexture() : image(Size(1,1),1){
      image(0,0,0) = 255;
      gli.update(&image);
    }
  };
  
  void bindWhiteTexture(){
    static WhiteTexture wt;
    glTexCoord2f(0,0);
    wt.gli.bind();
  }

#ifdef HAVE_QT  
  struct AsynchronousTextureDeleter : public QObject{
    struct Event : public QEvent{
      Event(const std::vector<GLuint> &del):
        QEvent((QEvent::Type)QEvent::registerEventType()),del(del){
        ctx = QGLContext::currentContext();
      }
      std::vector<GLuint> del;
      const QGLContext *ctx;
    };
    virtual bool event(QEvent *eIn){
      Event *e = dynamic_cast<Event*>(eIn);
      if(!e) return false;
      const_cast<QGLContext*>(e->ctx)->makeCurrent();
      glDeleteTextures(e->del.size(), e->del.data());
      return true;
    }
  };
  
  void freeTextures(const std::vector<GLuint> &del){
    static SmartPtr<AsynchronousTextureDeleter> deleter(new AsynchronousTextureDeleter);
    QCoreApplication::postEvent(deleter.get(), new AsynchronousTextureDeleter::Event(del));
  }
#endif

  inline float static winToDraw(float x, float w) { return (2/w) * x -1; }  
  inline float static drawToWin(float x, float w) { return (w/2) * x + (w/2); } 
  
  struct TextureElement{
    TextureElement(const Point &offset, const Size &size, int pixelSize, const Size &imageSize):
      tex(0),offset(offset), size(size), data(size.getDim()*pixelSize){
      
      float maxX  = imageSize.width, maxY = imageSize.height;
      relUL.x = maxX ? float(offset.x)/maxX : 0;
      relUL.y = maxY ? float(offset.y)/maxY : 0;
      relLR.x = maxX ? float(offset.x+size.width)/maxX : 1;
      relLR.y = maxY ? float(offset.y+size.height)/maxY : 1;
    }
    GLuint tex;
    Point offset;
    Size size;
    Point32f relUL, relLR;

    std::vector<icl8u> data;

    template<class T, int C>
    std::vector<Range64f> findMinMax() const{
      const T *p = (const T*) data.data();
      const int dim = size.getDim();
      Range64f rs[C];
      for(int c=0;c<C;++c){
        rs[c] = Range64f::limits();
        std::swap(rs[c].minVal, rs[c].maxVal);
        const T *pc = p+c;
        for(int i=0;i<dim;++i){
          T t = pc[i*C];
          if(t < rs[c].minVal) rs[c].minVal = t;
          if(t > rs[c].maxVal) rs[c].maxVal = t;
        }
      }
      return std::vector<Range64f>(rs,rs+C);
    }
  };
  typedef SmartPtr<TextureElement> TextureElementPtr;

#ifdef HAVE_IPP

  // ipp optimization for 1 channel data
  template<> std::vector<Range64f> TextureElement::findMinMax<icl8u,1>() const{
    icl8u mm[2];
    ippiMinMax_8u_C1R(data.data(), size.width, size, mm, mm+1);
    return std::vector<Range64f>(1,Range64f(mm[0],mm[1]));
  }

  template<> std::vector<Range64f> TextureElement::findMinMax<icl16s,1>() const{
    icl16s mm[2];
    ippiMinMax_16s_C1R(reinterpret_cast<const icl16s*>(data.data()), size.width*sizeof(icl16s), size, mm, mm+1);
    return std::vector<Range64f>(1,Range64f(mm[0],mm[1]));
  }

  template<> std::vector<Range64f> TextureElement::findMinMax<icl32f,1>() const{
    icl32f mm[2];
    ippiMinMax_32f_C1R(reinterpret_cast<const icl32f*>(data.data()), size.width*sizeof(icl32f), size, mm, mm+1);
    return std::vector<Range64f>(1,Range64f(mm[0],mm[1]));
  }

  /// ipp optimization for 3 channel data
  template<> std::vector<Range64f> TextureElement::findMinMax<icl8u,3>() const{
    icl8u mins[3],maxs[3];
    ippiMinMax_8u_C3R(data.data(), size.width, size, mins, maxs);
    std::vector<Range64f> r(3); 
    for(int i=0;i<3;++i) { r[i].minVal = mins[i]; r[i].maxVal = maxs[i]; }
    return r;
  }

  template<> std::vector<Range64f> TextureElement::findMinMax<icl16s,3>() const{
    icl16s mins[3],maxs[3];
    ippiMinMax_16s_C3R(reinterpret_cast<const icl16s*>(data.data()), size.width*sizeof(icl16s), size, mins, maxs);
    std::vector<Range64f> r(3); 
    for(int i=0;i<3;++i) { r[i].minVal = mins[i]; r[i].maxVal = maxs[i]; }
    return r;
  }

  template<> std::vector<Range64f> TextureElement::findMinMax<icl32f,3>() const{
    icl32f mins[3],maxs[3];
    ippiMinMax_32f_C3R(reinterpret_cast<const icl32f*>(data.data()), size.width*sizeof(icl32f), size, mins, maxs);
    std::vector<Range64f> r(3); 
    for(int i=0;i<3;++i) { r[i].minVal = mins[i]; r[i].maxVal = maxs[i]; }
    return r;
  }

  /// ipp optimization for 4 channel data
  template<> std::vector<Range64f> TextureElement::findMinMax<icl8u,4>() const{
    icl8u mins[4],maxs[4];
    ippiMinMax_8u_C4R(data.data(), size.width, size, mins, maxs);
    std::vector<Range64f> r(4); 
    for(int i=0;i<4;++i) { r[i].minVal = mins[i]; r[i].maxVal = maxs[i]; }
    return r;
  }

  template<> std::vector<Range64f> TextureElement::findMinMax<icl16s,4>() const{
    icl16s mins[4],maxs[4];
    ippiMinMax_16s_C4R(reinterpret_cast<const icl16s*>(data.data()), size.width*sizeof(icl16s), size, mins, maxs);
    std::vector<Range64f> r(4); 
    for(int i=0;i<4;++i) { r[i].minVal = mins[i]; r[i].maxVal = maxs[i]; }
    return r;
  }

  template<> std::vector<Range64f> TextureElement::findMinMax<icl32f,4>() const{
    icl32f mins[4],maxs[4];
    ippiMinMax_32f_C4R(reinterpret_cast<const icl32f*>(data.data()), size.width*sizeof(icl32f), size, mins, maxs);
    std::vector<Range64f> r(4); 
    for(int i=0;i<4;++i) { r[i].minVal = mins[i]; r[i].maxVal = maxs[i]; }
    return r;
  }

#endif
  
  template<class T>
  static inline void histo_entry(T v, double m, vector<int> &h, unsigned int n, double r){
    // todo check 1000 times +5 times (3Times done!)
    ++h[ floor( n*(v-m)/(r+1)) ];
  }
  template<class T>
  static void histo_interleaved(const void *vdata,
                                const int dim,
                                const int channels, 
                                const std::vector<Range64f> &ranges,
                                std::vector< std::vector<int> > &histos){
    const T * data = reinterpret_cast<const T*>(vdata);
    double mins[4],ls[4];
    for(unsigned int i=0;i<ranges.size();++i){
      mins[i] = ranges[i].minVal;
      ls[i] = ranges[i].getLength();
    }
    for(int i=0;i<dim;++i){
      for(int c=0;c<channels;++c){
        const T &val = data[channels * i + c];
        histo_entry(val,mins[c],histos[c],256,ls[c]);
      }
    }
  }
  
  template<>
  void histo_interleaved<icl8u>(const void *vdata,
                                const int dim,
                                const int channels, 
                                const std::vector<Range64f> &ranges,
                                std::vector< std::vector<int> > &histos){
    const icl8u *data = reinterpret_cast<const icl8u*>(vdata);
    switch(channels){
      case 1:{
          int *h = histos[0].data();
          for(int i=0;i<dim;++i){
            ++h[ data[i] ];
          }
          break;
      }
      case 2:{
        int *h0 = histos[0].data();
        int *h1 = histos[1].data();
        for(int i=0;i<dim;++i){
          ++h0[ data[2*i] ];
          ++h1[ data[2*i+1] ];
        }
        break;
      }
      case 3:{
        int *h0 = histos[0].data();
        int *h1 = histos[1].data();
        int *h2 = histos[2].data();
        for(int i=0;i<dim;++i){
          ++h0[ data[3*i] ];
          ++h1[ data[3*i+1] ];
          ++h2[ data[3*i+2] ];
        }
        break;
      }
      case 4:{
        int *h0 = histos[0].data();
        int *h1 = histos[1].data();
        int *h2 = histos[2].data();
        int *h3 = histos[3].data();
        for(int i=0;i<dim;++i){
          ++h0[ data[4*i] ];
          ++h1[ data[4*i+1] ];
          ++h2[ data[4*i+2] ];
          ++h3[ data[4*i+3] ];
        }
        break;
      }
      default:
        for(int i=0;i<dim;++i){
          for(int c=0;c<channels;++c){
            ++ histos[c][ data[i * channels + c] ];
          }
        }  
        break;
    }
  }

  struct GLImg::Data{
    static bool useDirtyFlag;
    
    scalemode sm;
    bool dirty;
    
    depth imageDepth;
    depth origImageDepth;
    Size imageSize;
    Rect imageROI;
    int imageChannels;
    int maxCellSize;
    int bci[3];
    
    struct TextureInfo{
      inline TextureInfo():threadID(0){}
      inline TextureInfo(unsigned threadID, const GLContext &ctx):
        threadID(threadID),ctx(ctx){}
      
      unsigned int threadID;
      GLContext ctx;
      
      static inline TextureInfo getCurrentTextureInfo(){
        return TextureInfo(pthread_self(), GLContext::currentContext());
      }
      inline bool operator==(const TextureInfo &other) const{
        return threadID == other.threadID && ctx == other.ctx;
      }
    };
      
    TextureInfo textureInfo;
    
    std::vector<GLuint> textures;
    Array2D<TextureElementPtr> data;
    mutable ImgBase *extractedImageBuffer;

    std::vector<GLImg::Vec3> gridBuffer, gridNormalBuffer;

    float gridColor[4];
    bool drawGrid;
    format imageFormat;
    Time timeStamp;
    mutable ImageStatistics stats;
    mutable QMutex textureBufferMutex;

    bool isImageNull;

    Data():textureBufferMutex(QMutex::Recursive){
    }
    
    ~Data(){
      releaseTextures();
      ICL_DELETE(extractedImageBuffer);
    }
    
    inline bool isDirty(){
      return useDirtyFlag ? dirty : true;
    }
    
    void releaseTextures(){

      if(textures.size()){
#ifdef HAVE_QT
        if(QCoreApplication::instance()){
          if(textureInfo == TextureInfo::getCurrentTextureInfo()){
            glDeleteTextures(textures.size(),textures.data());
          }else{
            freeTextures(textures);
          }
        }else{
            glDeleteTextures(textures.size(),textures.data());
        }
        textures.clear();
#else
        glDeleteTextures(textures.size(),textures.data());
#endif

      }
    }


    template<class ExternalType, class InternalType>
    void bufferTextureData(const Img<ExternalType> &src, int maxCellSize){
      textureBufferMutex.lock();
      timeStamp = src.getTime();
      imageROI = src.getROI();
      
      const int w = src.getWidth(), h = src.getHeight(), c = src.getChannels();
      const int M = maxCellSize, nx = ceil(float(w)/M), ny = ceil(float(h)/M);
      
      if(imageSize != Size(nx,ny) || imageChannels != c || imageDepth != icl::getDepth<InternalType>()
         || maxCellSize != this->maxCellSize){
        this->maxCellSize = maxCellSize;
        imageSize = Size(w,h);
        imageDepth = icl::getDepth<InternalType>();
        imageChannels  = c;
        imageFormat = src.getFormat();
        
        data = Array2D<TextureElementPtr>(nx,ny);

        for(int y=0;y<ny;++y){
          for(int x=0;x<nx;++x){
            Point offs(x*M, y*M);
            Size size(iclMin(M,w-x*M),iclMin(M,h-y*M));
            data(x,y) = new TextureElement(offs,size, c*sizeof(InternalType), imageSize);
          }
        }
      }
      for(int y=0;y<ny;++y){
        for(int x=0;x<nx;++x){
          TextureElement &t = *data(x,y);
          SmartPtr<const Img<ExternalType> > roi = src.shallowCopy(Rect(t.offset,t.size));
          planarToInterleaved(roi.get(), reinterpret_cast<InternalType*>(t.data.data()), 
                              t.size.width*imageChannels*sizeof(InternalType));
        }
      }
      
      dirty = true;
      textureBufferMutex.unlock();
    }

    
    const ImageStatistics &updateStats() const {
      textureBufferMutex.lock();
      stats.params = ImgParams(imageSize,imageChannels);
      stats.time = timeStamp;
      stats.d = origImageDepth;
      stats.ranges = findMinMaxGeneric();
      stats.histos.resize(imageChannels);
      stats.isNull = false;
      for(int i=0;i<imageChannels;++i){ 
        stats.histos[i].resize(256);
        std::fill(stats.histos[i].begin(), stats.histos[i].end(), 0);
      }
      for(int x=0;x<data.getWidth();++x){
        for(int y=0;y<data.getHeight();++y){
          const TextureElement &t = *data(x,y);
          if(imageDepth == depth8u){
            histo_interleaved<icl8u>(t.data.data(),t.size.getDim(),
                                     imageChannels, stats.ranges,
                                     stats.histos);
          }else if(imageDepth == depth16s){
            histo_interleaved<icl16s>(t.data.data(),t.size.getDim(),
                                      imageChannels, stats.ranges,
                                      stats.histos);
          }else{
            histo_interleaved<icl32f>(t.data.data(),t.size.getDim(),
                                      imageChannels, stats.ranges,
                                      stats.histos);
          }
        }
      }
      
      
      textureBufferMutex.unlock();
      return stats;
    }

    template<class T, int C>
    std::vector<Range64f> findMinMax() const{
      textureBufferMutex.lock();
      std::vector<Range64f> all;
      for(int y=0;y<data.getHeight();++y){
        for(int x=0;x<data.getWidth();++x){
          std::vector<Range64f> rs = data(x,y)->findMinMax<T,C>();
          if(!x && !y) all = rs;
          else{
            for(int i=0;i<C;++i){
              if(rs[i].minVal < all[i].minVal) all[i].minVal = rs[i].minVal;
              if(rs[i].maxVal > all[i].maxVal) all[i].maxVal = rs[i].maxVal;
            }
          }
        }
      }
      textureBufferMutex.unlock();
      return all;
    }
    std::vector<Range64f> findMinMaxGeneric() const{
      const int c = imageChannels;
      if(imageDepth == depth8u){
        switch(c){
          case 1: return findMinMax<icl8u,1>();
          case 2: return findMinMax<icl8u,2>();
          case 3: return findMinMax<icl8u,3>();
          case 4: return findMinMax<icl8u,4>();
        }
      }else if(imageDepth == depth16s){
        switch(c){
          case 1: return findMinMax<icl16s,1>();
          case 2: return findMinMax<icl16s,2>();
          case 3: return findMinMax<icl16s,3>();
          case 4: return findMinMax<icl16s,4>();
        }
      }else{
        switch(c){
          case 1: return findMinMax<icl32f,1>();
          case 2: return findMinMax<icl32f,2>();
          case 3: return findMinMax<icl32f,3>();
          case 4: return findMinMax<icl32f,4>();
        }
      }
      return std::vector<Range64f>();
    }
    
    void setupUnpackAllignment( int w){
      int wBytes = w * iclMin(4u,getSizeOf(imageDepth));
      for(int i=3;i>=0;++i){
        if( !((wBytes)%(1<<i)) ){
          glPixelStorei(GL_UNPACK_ALIGNMENT,1<<i);
          return;
        }
      }
    }

    static inline void setup_pixel_transfer(float sa, float sr, float sg, float sb,
                                            float a, float r, float g, float b){
      glPixelTransferf(GL_ALPHA_SCALE,sa);
      glPixelTransferf(GL_RED_SCALE,sr);
      glPixelTransferf(GL_GREEN_SCALE,sg);
      glPixelTransferf(GL_BLUE_SCALE,sb);
      glPixelTransferf(GL_ALPHA_BIAS,a);
      glPixelTransferf(GL_RED_BIAS,r);
      glPixelTransferf(GL_GREEN_BIAS,g);
      glPixelTransferf(GL_BLUE_BIAS,b);
    }
    
    static inline void reset_bci(){
      setup_pixel_transfer(1,1,1,1,0,0,0,0);
    }
    
    template<class T>
    const std::vector<double> findColor(int x, int y) const {
      int nx = imageSize.width, ny = imageSize.height;
      for(int yCell=0;yCell<ny;++yCell){
        for(int xCell=0;xCell<nx;++xCell){
          const TextureElement &t = *data(xCell,yCell);
          if(Rect(t.offset,t.size).contains(x,y)){
            const T *p = reinterpret_cast<const T*>(t.data.data());
            p += imageChannels*((x-t.offset.x) + t.size.width * (y-t.offset.y));
            return std::vector<double>(p, p+imageChannels);
          }
        }
      }
      return std::vector<double>();
    }

    template<class T>
    const ImgBase &extractImage(Img<T> &dst) const {
      int nx = data.getWidth(), ny = data.getHeight();
      for(int y=0;y<ny;++y){
        for(int x=0;x<nx;++x){
          const TextureElement &t = *data(x,y);
          SmartPtr<Img<T> > roi = dst.shallowCopy(Rect(t.offset,t.size));
          interleavedToPlanar<T,T>(reinterpret_cast<const T*>(t.data.data()), roi.get());
        }
      }
      return dst;
    }

    std::vector<double> findColorGeneric(int x, int y) const{
      if(imageDepth == depth8u) return findColor<icl8u>(x,y);
      else if(imageDepth == depth16s) return findColor<icl16s>(x,y);
      return findColor<icl32f>(x,y);
    }
    
    const ImgBase &extractImageGeneric() const{
      depth d = ((imageDepth == depth8u) ? depth8u : 
                 (imageDepth == depth16s) ? depth16s : 
                 depth32f);
      ensureCompatible(&extractedImageBuffer, d, imageSize, imageChannels);
      if(extractedImageBuffer->getChannels() == 3) extractedImageBuffer->setFormat(formatRGB);
      else if(extractedImageBuffer->getChannels() == 1) extractedImageBuffer->setFormat(formatGray);
      else extractedImageBuffer->setFormat(formatMatrix);

      if(imageDepth == depth8u) return extractImage<icl8u>(*extractedImageBuffer->as8u());
      else if(imageDepth == depth16s) return extractImage<icl16s>(*extractedImageBuffer->as16s());
      return extractImage<icl32f>(*extractedImageBuffer->as32f());
    }


    void setupPixelTransfer(){
      if(!imageChannels || !imageSize.getDim()) return;
      
      icl32f fScaleRGB(0),fBiasRGB(0);
      if( (bci[0] < 0)  && (bci[1] < 0) && (bci[2] < 0)){
        // auto adaption
        std::vector<Range64f> rs = findMinMaxGeneric();
        Range64f all  = rs[0];
        for(unsigned int i=1;i<rs.size();i++){
          if(rs[i].minVal < all.minVal) all.minVal = rs[i].minVal;
          if(rs[i].maxVal > all.maxVal) all.maxVal = rs[i].maxVal;
        }
        
        icl64f l = iclMax(double(1),all.getLength());
        
        switch (imageDepth){
          case depth8u:{
            fScaleRGB  = l ? 255.0/l : 255;
            fBiasRGB = (- fScaleRGB * all.minVal)/255.0;
            break;
          }
          case depth16s:{
            static const icl64f _max = (65536/2-1);
            fScaleRGB  = l ? _max/l : _max;
            fBiasRGB = (- fScaleRGB * all.minVal)/255.0;
            break;
          }
          default: // all others are drawn as float
            fScaleRGB  = l ? 255.0/l : 255;
            fBiasRGB = (- fScaleRGB * all.minVal)/255.0;
            fScaleRGB /= 255;
        }
      }else{
        fBiasRGB = bci[0]/255.0;
        fScaleRGB  = 1;
        if(imageDepth == depth16s) fScaleRGB = 127;
        else if(imageDepth != depth8u) fScaleRGB = 1./255; 
      }
      
      float c = (float)bci[1]/255;
      if(c>0) c*=10;
      float s = fScaleRGB*(1.0+c);
      float b = fBiasRGB-c/2;
      
      setup_pixel_transfer(s,s,s,s,b,b,b,b);
      
    }

    void uploadTextureData(){
      ICLASSERT_THROW(data.getDim(),ICLException("unable to draw GLImg: no texture data available"));

      if(!isDirty()) return;
      setupPixelTransfer();
      
      if(textures.size()){
        glDeleteTextures(textures.size(),textures.data());
      }
      textures.resize(data.getDim());
      glGenTextures(textures.size(), textures.data());
      
      textureInfo = TextureInfo::getCurrentTextureInfo();
      
      textureBufferMutex.lock();
      

      static GLenum types[] = { GL_UNSIGNED_BYTE, GL_SHORT, GL_FLOAT, GL_FLOAT, GL_FLOAT };
      static GLenum chan[] = { 0, GL_LUMINANCE, GL_RGB, GL_RGB, GL_RGBA};
      
      for(int y=0;y<data.getHeight();++y){
        for(int x=0;x<data.getWidth();++x){
          TextureElement &t = *data(x,y);
          
          t.tex = textures[x+data.getWidth()*y];

          glBindTexture(GL_TEXTURE_2D, t.tex);

          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,sm == interpolateNN ? GL_NEAREST : GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,sm == interpolateNN ? GL_NEAREST : GL_LINEAR);

          setupUnpackAllignment( t.size.width );

          glTexImage2D(GL_TEXTURE_2D, 0, imageChannels == 4 ? GL_RGBA : GL_RGB, 
                       t.size.width, t.size.height, 0, 
                       chan[imageChannels], types[imageDepth], t.data.data());

        }
      }
      dirty = false;
      textureBufferMutex.unlock();
    }
  };

  
  bool GLImg::Data::useDirtyFlag = true;
  
  void GLImg::set_use_dirty_flag(bool useIt){
    Data::useDirtyFlag = useIt;
  }  
  
  bool GLImg::get_use_dirty_flag(){
    return Data::useDirtyFlag;
  }
   

  
  GLImg::GLImg(const ImgBase *src, scalemode sm, int maxCellSize):m_data(new Data){
    m_data->sm = sm;
    m_data->dirty = true;
    m_data->extractedImageBuffer = 0;
    std::fill(m_data->bci,m_data->bci+3,0);

    std::fill(m_data->gridColor,m_data->gridColor+4,1.0f);
    m_data->drawGrid = false;
    m_data->isImageNull = true;
    if(src){
      update(src,maxCellSize);
    }
  }
  GLImg::~GLImg(){
    // actually not: expose m_data as a deletable to GUI thread
    // todo: check whether we are in the GUI thread : then delete data
    // else: set up qt to free the texture when possible
    delete m_data;
  }
  
  
  void GLImg::update(const ImgBase *src, int maxCellSize){
    if(maxCellSize < 1){
      ERROR_LOG("maxCellSize must be >= 1 (using max possible size instead)");
      maxCellSize = getMaxTextureSize();
    }
    if(!src){
      m_data->isImageNull = true;
      m_data->releaseTextures();
      return;
    }
    m_data->isImageNull = false;

    SmartPtr<ImgBase> pSrc;
    if(src->getChannels() > 4){
      pSrc = const_cast<ImgBase*>(src)->shallowCopy();
      pSrc->setChannels(4);
      src = pSrc.get();
    }else if(src->getChannels() == 2){
      pSrc = const_cast<ImgBase*>(src)->shallowCopy();
      pSrc->setChannels(3); // todo use a buffer for the channel data
      src = pSrc.get();
    }

    m_data->origImageDepth = src->getDepth();
    
    switch(src->getDepth()){
      case depth8u: m_data->bufferTextureData<icl8u, icl8u>(*src->as8u(), maxCellSize); break;
      case depth16s: m_data->bufferTextureData<icl16s, icl16s>(*src->as16s(), maxCellSize); break;
      case depth32s: m_data->bufferTextureData<icl32s, icl32f>(*src->as32s(), maxCellSize); break;
      case depth32f: m_data->bufferTextureData<icl32f, icl32f>(*src->as32f(), maxCellSize); break;
      case depth64f: m_data->bufferTextureData<icl64f, icl32f>(*src->as64f(), maxCellSize); break;
      default:
        ICL_INVALID_DEPTH;
    }
  }

  bool GLImg::isNull() const{
    return m_data->isImageNull;
  }

  void GLImg::setScaleMode(scalemode sm){
    m_data->sm = sm;
  }
    
  void GLImg::draw2D(const Rect &rect, const Size &win){
    ICLASSERT_RETURN(!isNull());

    float left = winToDraw(rect.x,win.width);
    float top = winToDraw(rect.y,win.height);
    float right = winToDraw(rect.right(), win.width);
    float bottom = winToDraw(rect.bottom(), win.height);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glScalef(1,-1,1); // flip y

    const float a[3]={left,top,0}, b[3]={right,top,0}, d[3]={left,bottom,0},c[3]={right,bottom,0};
    draw3D(a,b,c,d);

    if(m_data->drawGrid){
      glLineWidth(1);
      glColor4fv(m_data->gridColor);
      
      //glLineWidth(1); dunno?
      
      float dx = (right - left)/m_data->imageSize.width;
      float dy = (bottom - top)/m_data->imageSize.height;

      float pixDX = (dx*win.width)/2.0f;
      float pixDY = (dy*win.height)/2.0f;

      // find appropriate x-Steps
      float stepx = 1;
      float stepy = 1;
      while( (stepx*pixDX) < 10) stepx *= 2;
      while( (stepy*pixDY) < 10) stepy *= 2;

      if( (stepx != 1) || (stepy != 1) ){
        float rx = stepx*dx/4.0;
        float ry = stepy*dy/4.0;
        float rr = iclMin(rx,ry);
        
        glBegin(GL_LINES);
        for(int x=0;x<=m_data->imageSize.width;x+=stepx){
          float xx = left+x*dx;
          for(int y=0;y<=m_data->imageSize.height;y+=stepy){
            float yy = top+y*dy;
            glVertex2f(xx-rr,yy);
            glVertex2f(xx+rr,yy);

            glVertex2f(xx,yy-rr);
            glVertex2f(xx,yy+rr);
          }
        }
        glEnd();
      }else{
        glBegin(GL_LINES);
        for(int x=0;x<=m_data->imageSize.width;++x){
          float xx = left+x*dx;
          glVertex2f(xx,top);
          glVertex2f(xx,bottom);
        }
        for(int y=0;y<=m_data->imageSize.height;++y){
          float yy = top+y*dy;
          glVertex2f(left,yy);
          glVertex2f(right,yy);
        }
        glEnd();
      }
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
  }

  void GLImg::draw2D(const float a[2], const float b[2], const float c[2],const float d[2], const Size &windowSize){
    const int w = windowSize.width;
    const int h = windowSize.height;
    
    const float da[3]={winToDraw(a[0],w), winToDraw(a[1],h),0};
    const float db[3]={winToDraw(b[0],w), winToDraw(b[1],h),0};
    const float dc[3]={winToDraw(c[0],w), winToDraw(c[1],h),0};
    const float dd[3]={winToDraw(d[0],w), winToDraw(d[1],h),0};
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glScalef(1,-1,1); // flip y
    
    draw3D(da,db,dc,dd);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
  }


  static float *interpolate_billinear(const float *a, const float *b, const float *c, const float *d, 
                                      float relx,float rely, float *dst){
    float &x = relx, &y=rely, x1=1-x, y1=1-y;
    for(int i=0;i<3;++i){
      dst[i] = a[i]*x1*y1 + b[i]*x*y1 + d[i]*y*x1 + c[i]*x*y;  
    }
    return dst;
  }
  static inline float vec_len(const float *f){
    return ::sqrt (f[0]*f[0] + f[1]*f[1] + f[2]*f[2] );
  }
  
  // for some reason we have to invert the normals for texture mapping ??
  static float *interpolate_billinear_and_normalize_and_invert(const float *a, const float *b, const float *c, const float *d, 
                                                             float relx,float rely, float *dst){
    interpolate_billinear(a,b,c,d,relx,rely,dst);
    float len = vec_len(dst);
    if(len > 1.E-6){
      float ilen = -1.0/len;
      dst[0] *= ilen;
      dst[1] *= ilen;
      dst[2] *= ilen;
    }
    return dst;
  }

  void GLImg::draw3D(const float a[3],const float b[3],const float c[3],const float d[3],
                     const float na[3], const float nb[3], const float nc[3], const float nd[3],
                     const Point32f &texCoordsA,
                     const Point32f &texCoordsB,
                     const Point32f &texCoordsC,
                     const Point32f &texCoordsD){
    ICLASSERT_RETURN(!isNull());
    
    if(m_data->isDirty()) m_data->uploadTextureData();
    /**
        a -- b
        |    |
        c -- d
    */

    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);

    glColor4f(1,1,1,1);
    const bool haveNormals = na && nb && nc && nd;
    
    float tmp[3];
    
    for(int y=0;y<m_data->data.getHeight();++y){
      for(int x=0;x<m_data->data.getWidth();++x){
        TextureElement &t = *m_data->data(x,y);
        glBindTexture(GL_TEXTURE_2D, t.tex);

        const float x0 = t.relUL.x, x1=t.relLR.x, y0=t.relUL.y, y1=t.relLR.y;

        glBegin(GL_QUADS);
        glTexCoord2fv(&texCoordsA.x);
        if(haveNormals){
          glNormal3fv(interpolate_billinear_and_normalize_and_invert(a,b,c,d,x0,y0,tmp));
        }
        glVertex3fv(interpolate_billinear(a,b,c,d,x0,y0,tmp));                    


        glTexCoord2fv(&texCoordsB.x);
        if(haveNormals){
          glNormal3fv(interpolate_billinear_and_normalize_and_invert(a,b,c,d,x1,y0,tmp));
        }
        glVertex3fv(interpolate_billinear(a,b,c,d,x1,y0,tmp));                    


        glTexCoord2fv(&texCoordsD.x);
        if(haveNormals){
          glNormal3fv(interpolate_billinear_and_normalize_and_invert(a,b,c,d,x1,y1,tmp));
        }
        glVertex3fv(interpolate_billinear(a,b,c,d,x1,y1,tmp));                    



        glTexCoord2fv(&texCoordsC.x);
        if(haveNormals){
          glNormal3fv(interpolate_billinear_and_normalize_and_invert(a,b,c,d,x0,y1,tmp));
        }
        glVertex3fv(interpolate_billinear(a,b,c,d,x0,y1,tmp));                    
        glEnd();
      }
    }
    
    bindWhiteTexture();
    glPopAttrib();
  }
  
  void GLImg::draw3DGeneric(int numPoints,
                            const float *xs, const float *ys, const float *zs, int xyzStride,
                            const Point32f *texCoords, const float *nxs, const float *nys,
                            const float *nzs, int nxyzStride){
    if(numPoints < 3) throw ICLException("GImg::draw3DGeneric: numPoints must be at least 3");
    ICLASSERT_RETURN(!isNull());
    
    if(m_data->isDirty()) m_data->uploadTextureData();
    if(m_data->data.getSize() != Size(1,1)) throw ICLException("GLImg::draw3DGeneric: the texture is too large for this");
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);

    glColor4f(1,1,1,1);
    const bool haveNormals = nxs && nys && nzs;

    TextureElement &t = *m_data->data(0,0);
    glBindTexture(GL_TEXTURE_2D, t.tex);
    
    glBegin(numPoints == 3 ? GL_TRIANGLES : numPoints == 4 ? GL_QUADS : GL_POLYGON);
    
    for(int i=0;i<numPoints;++i){
      glTexCoord2fv(&texCoords[i].x);
      glVertex3f(xs[i*xyzStride], ys[i*xyzStride], zs[i*xyzStride] );
      if(haveNormals){
        glNormal3f(nxs[i*nxyzStride],nys[i*nxyzStride],nzs[i*nxyzStride]);
      }
    }
    
    glEnd();
    bindWhiteTexture();
    glPopAttrib();
  }



  void GLImg::drawToGrid(int nx, int ny, const float *xs, const float *ys, const float *zs,
                         const float *nxs, const float *nys, const float *nzs,const int stride){
    if(m_data->isDirty()) m_data->uploadTextureData();
    
    if(m_data->data.getSize() != Size(1,1)){
      WARNING_LOG("GLImg::drawToGrid: the texture was split into " << m_data->data.getSize()
                  << " cells, which is not supported by this method. The first cell element is used only!");
    }
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1,1,1,1);
    
    TextureElement &t = *m_data->data(0,0);
    glBindTexture(GL_TEXTURE_2D, t.tex);

    const bool haveNormals = nxs && nys && nzs;

    if(!haveNormals) glDisable(GL_LIGHTING);
    const float nxf = nx-1, nyf = ny-1;

    
#define AT(_p,_x,_y) _p[stride*(_x+_y*nx)]
#define X(_x,_y) AT(xs,_x,_y)
#define Y(_x,_y) AT(ys,_x,_y)
#define Z(_x,_y) AT(zs,_x,_y)
#define NX(_x,_y) AT(nxs,_x,_y)
#define NY(_x,_y) AT(nys,_x,_y)
#define NZ(_x,_y) AT(nzs,_x,_y)


#define PART(_x,_y)                                             \
    glTexCoord2f((_x)/nxf, (_y)/nyf);                       \
    if(haveNormals) glNormal3f(NX(_x,_y),NY(_x,_y), NZ(_x,_y)); \
    glVertex3f(X(_x,_y), Y(_x,_y), Z(_x,_y));    

    glBegin(GL_QUADS);
    for(int y1=1;y1<ny; ++y1){    
      for(int x1=1; x1<nx; ++x1){
        const int x0 = x1-1, y0 = y1-1;
        PART(x0,y0);
        PART(x1,y0);
        PART(x1,y1);
        PART(x0,y1);
      }
    }
    glEnd();
    
#undef AT
#undef X
#undef Y
#undef Z
#undef NX
#undef NY
#undef NZ
#undef PART

    bindWhiteTexture();
    glPopAttrib();
  }
  
  /// draws the texture to an nx x ny grid whose positions and normals are defined by a function
  void GLImg::drawToGrid(int nx, int ny, grid_function gridVertices, 
                         grid_function gridNormals){
    ICLASSERT_RETURN(!isNull());
    
    bool haveNormals = gridNormals;
    std::vector<Vec3> &grid = m_data->gridBuffer;
    std::vector<Vec3> &normals = m_data->gridNormalBuffer;
    
    grid.resize(nx*ny);
    if(haveNormals){
      normals.resize(nx*ny);
    }
    for(int y=0;y<ny;++y){
      for(int x=0;x<nx;++x){
        grid[x + nx*y] = gridVertices(x,y);
      }
    }
    const Vec3 &p = grid[0];

    if(haveNormals){
      for(int y=0;y<ny;++y){
        for(int x=0;x<nx;++x){
          normals[x + nx*y] = gridNormals(x,y);
        }
      }
      const Vec3 &n = normals[0];
      drawToGrid(nx,ny,&p[0],&p[1],&p[2], &n[0], &n[1], &n[2], 3);
    }else{
      drawToGrid(nx,ny,&p[0],&p[1],&p[2], 0,0,0,3);
    }
  }


  int GLImg::getMaxTextureSize(){
    static int MAX_TEXTURE_SIZE = 0;
    if(!MAX_TEXTURE_SIZE) glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MAX_TEXTURE_SIZE);
    return MAX_TEXTURE_SIZE;
  }

  
  Size GLImg::getCells() const{
    ICLASSERT_RETURN_VAL(!isNull(), Size::null);
    return m_data->data.getSize();
  }
    
    /// binds the given texture cell using glBindTexture(...)
  void GLImg::bind(int xCell, int yCell){
    ICLASSERT_RETURN(!isNull());

    ICLASSERT_THROW(xCell >= 0 && yCell >=0 && xCell < m_data->data.getWidth() &&
                    yCell < m_data->data.getHeight(), ICLException("GLImg::bind(x,y): invalid cell index"));

    glBindTexture(GL_TEXTURE_2D, m_data->data(xCell,yCell)->tex);
  }

  int GLImg::getWidth() const{
    ICLASSERT_RETURN_VAL(!isNull(),0);
    return m_data->imageSize.width;
  }
  
  int GLImg::getHeight() const{
    ICLASSERT_RETURN_VAL(!isNull(),0);
    return m_data->imageSize.height;
  }
  
  int GLImg::getChannels() const{
    ICLASSERT_RETURN_VAL(!isNull(),0);
    return m_data->imageChannels;
  }

  void GLImg::setBCI(int b, int c, int i){
    if(m_data->bci[0] != b ||
       m_data->bci[1] != c ||
       m_data->bci[2] != i ){
      m_data->dirty = true;
    }
    m_data->bci[0] = b;
    m_data->bci[1] = c;
    m_data->bci[2] = i;
  }
  
  std::vector<Range64f> GLImg::getMinMax() const{
    ICLASSERT_RETURN_VAL(!isNull(),std::vector<Range64f>());
    return m_data->findMinMaxGeneric();
  }

  
  std::vector<icl64f> GLImg::getColor(int x, int y)const{
    ICLASSERT_RETURN_VAL(!isNull(),std::vector<double>());
    return m_data->findColorGeneric(x,y);
  }
  scalemode GLImg::getScaleMode() const{
    return m_data->sm;
  }

  const ImgBase *GLImg::extractImage() const{
    ICLASSERT_RETURN_VAL(!isNull(),0);
    return &m_data->extractImageGeneric();
  }

  const ImageStatistics &GLImg::getStats() const{
    ICLASSERT_RETURN_VAL(!isNull(),m_data->stats);
    m_data->updateStats();
    return m_data->stats;
  }
  
  void GLImg::setDrawGrid(bool enabled, float *color){
    m_data->drawGrid = enabled;
    if(color) setGridColor(color);
  }
  
  void GLImg::setGridColor(float *color){
    std::copy(color,color+4, m_data->gridColor);
  }
  
  const float *GLImg::getGridColor() const{
    return m_data->gridColor;
  }
  
  Time GLImg::getTime() const{
    ICLASSERT_RETURN_VAL(!isNull(),Time(0));
    return m_data->timeStamp;
  }

  depth GLImg::getDepth() const{
    ICLASSERT_RETURN_VAL(!isNull(),depth8u);
    return m_data->origImageDepth;
  }
  
  format GLImg::getFormat() const{
    ICLASSERT_RETURN_VAL(!isNull(),formatMatrix);
    return m_data->imageFormat;
  }

  Rect GLImg::getROI() const{
    ICLASSERT_RETURN_VAL(!isNull(),Rect::null);
    return m_data->imageROI;
  }
  
  void GLImg::lock() const{
    m_data->textureBufferMutex.lock();

  }
  void GLImg::unlock() const{
    m_data->textureBufferMutex.unlock();
  }
    
}

