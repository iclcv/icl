/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLQt/src/GLTextureMapImage.cpp                        **
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
*********************************************************************/

#include <ICLQt/GLTextureMapImage.h>
#include <ICLCC/CCFunctions.h>
#include <limits>
#include <cstring>
using std::memset;

namespace icl{

  using namespace std;

  template<class T>
  GLTextureMapImage<T>::GLTextureMapImage(const Size &imageSize, bool useSingleBuffer,  int channels,int cellSize){
    // {{{ open
    
    ICLASSERT( channels == 3 || channels == 1 || channels == 4);
    ICLASSERT( cellSize >= 16);
    ICLASSERT( getDepth<T>() != depth64f );
    m_iChannels = channels;
    m_iCellSize = cellSize;
    m_iImageW = imageSize.width;
    m_iImageH = imageSize.height;
    m_iXCells = m_iImageW/m_iCellSize;
    m_iYCells = m_iImageH/m_iCellSize;
    
    m_bUseSingleBuffer = useSingleBuffer;
    
    m_iRestX = m_iImageW % m_iCellSize;
    m_iRestY = m_iImageH % m_iCellSize;
    if(m_iRestX) m_iXCells++;
    if(m_iRestY) m_iYCells++;
    
    m_iCellDataSize = m_iCellSize*m_iCellSize*m_iChannels;

    m_matTextureNames = SimpleMatrix<GLuint>(m_iXCells,m_iYCells);

    if(m_bUseSingleBuffer){
      m_ptCellData = new T[m_iCellDataSize];
      // This is an asyncronous gl call an may cause problems ..
      // -> set useSingleBuffer to false to avoid this
      glGenTextures(m_iXCells*m_iYCells,m_matTextureNames.data()); 
    }else{
      m_ptCellData = 0;
      m_matCellData = SimpleMatrix<T*>(m_iXCells,m_iYCells);
    }
    m_matROISizes = SimpleMatrix<Size,SimpleMatrixAllocSize>(m_iXCells,m_iYCells);

    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){
        m_matROISizes[x][y].width  = m_iRestX ? (x==m_iXCells-1 ? m_iRestX : m_iCellSize) : m_iCellSize;
        m_matROISizes[x][y].height = m_iRestY ? (y==m_iYCells-1 ? m_iRestY : m_iCellSize) : m_iCellSize;
       if(!m_bUseSingleBuffer){
          m_matCellData[x][y] = new T[m_iCellDataSize];
        }
      }
    }
    memset(m_aiBCI,0,3*sizeof(int));

    m_drawGrid = false;
    m_gridColor[0] = 1;
    m_gridColor[1] = 1;
    m_gridColor[2] = 1;
    m_gridColor[3] = 1;
  }
  
  // }}}

  template<class T>
  GLTextureMapImage<T>::~GLTextureMapImage(){
    // {{{ open
    if(m_bUseSingleBuffer){
      glDeleteTextures(m_iXCells*m_iYCells,m_matTextureNames.data());
      delete [] m_ptCellData;
    }else{
      if(m_matCellData.dim()){
        for(int y=0;y<m_iYCells;++y){
          for(int x=0;x<m_iXCells;++x){
            delete [] m_matCellData[x][y];
          }
        }
      }
    }
  }

  // }}}

  template<class T>
  void GLTextureMapImage<T>::setPackAlignment(depth d, int linewidth){
    // {{{ open

    switch (d){
      case depth8u:{
        if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        else if(linewidth%4) glPixelStorei(GL_UNPACK_ALIGNMENT,2);
        else if(linewidth%8) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        else  glPixelStorei(GL_UNPACK_ALIGNMENT,8);
        break;
      }
      case depth16s:{
        if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,2);
        else if(linewidth%4) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        else  glPixelStorei(GL_UNPACK_ALIGNMENT,8);
        break;
      }        
      case depth32s:
      case depth32f:{
        if(linewidth%2) glPixelStorei(GL_UNPACK_ALIGNMENT,4);
        else glPixelStorei(GL_UNPACK_ALIGNMENT,8);
        break;
      }
      default:
        ICL_INVALID_FORMAT;
        break;
    }
  }

  // }}}
  template<class T>
  void GLTextureMapImage<T>::bci(int b, int c, int i) {
    m_aiBCI[0] = b; m_aiBCI[1] = c; m_aiBCI[2] = i;
  }


  namespace{
    
    template<class T>
    void calculate_minmax_interleaved(T *data,
                                      Size dataROISize,
                                      int dataLineLength,
                                      int channels,
                                      std::vector<Range<T> > &ranges){
      for(int x=0;x<dataROISize.width;++x){
        for(int y=0;y<dataROISize.height;++y){
          for(int c=0;c<channels;++c){
            T &val = data[(x+dataLineLength*y)*channels+c];
            ranges[c].minVal = iclMin(ranges[c].minVal,val);
            ranges[c].maxVal = iclMax(ranges[c].maxVal,val);
          }
        }
      }
    }
  }

  template<class T>
  std::vector<Range<T> > GLTextureMapImage<T>::getMinMax() const{
    if(m_bUseSingleBuffer){
      ERROR_LOG("image range cannot be extracted in singleBufferMode");
      return std::vector<Range<T> >();
    }
    
    std::vector<Range<T> > mm(m_iChannels,Range<T>(std::numeric_limits<T>::max(),std::numeric_limits<T>::min()));
    
    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){
        calculate_minmax_interleaved(m_matCellData[x][y],m_matROISizes[x][y],m_iCellSize,m_iChannels,mm);
      }
    }
    return mm;
  }
 
  template<class T>
  vector<icl64f> GLTextureMapImage<T>::getColor(int x, int y) const{
    if(m_bUseSingleBuffer){
      ERROR_LOG("getting image colors is not supported in single buffer mode");
      return vector<icl64f>();
    }
    if(x>=0 && y>=0 && x<m_iImageW && y<m_iImageH){
      int xCell = x/m_iCellSize;
      int yCell = y/m_iCellSize;
      int xOffs = x%m_iCellSize;
      int yOffs = y%m_iCellSize;
      T *data = m_matCellData[xCell][yCell] + m_iChannels * (xOffs + yOffs*m_iCellSize);
      vector<icl64f> color;
      for(int c=0;c<m_iChannels;c++){
        color.push_back(data[c]);
      }
      return color;
    }
    return vector<icl64f>();
  }
  

  template<class T>
  void GLTextureMapImage<T>::updateTextures(const Img<T> *image){
    // {{{ open
    ICLASSERT_RETURN( image);
    ICLASSERT( m_iChannels == image->getChannels() );
    ICLASSERT( m_iImageW == image->getWidth());
    ICLASSERT( m_iImageH == image->getHeight());

    if(m_bUseSingleBuffer){ // this is not allowed, because openGL cannot be accessed thread-safe
      setPackAlignment(getDepth<T>(),image->getWidth());
      setUpPixelTransfer(getDepth<T>(),m_aiBCI[0],m_aiBCI[1],m_aiBCI[2], image);
      
      static GLenum aeGLTypes[] = { GL_UNSIGNED_BYTE, GL_SHORT, GL_INT, GL_FLOAT, GL_FLOAT };
      GLenum glType = aeGLTypes[getDepth<T>()];
      
      for(int y=0;y<m_iYCells;++y){
        for(int x=0;x<m_iXCells;++x){
          glBindTexture(GL_TEXTURE_2D, m_matTextureNames[x][y]);
          
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
          
          const Img<T> *src = image->shallowCopy(Rect(Point(x*m_iCellSize,y*m_iCellSize),m_matROISizes[x][y]));
          
          
          if(x==m_iXCells -1 || y == m_iYCells-1){
            fill(m_ptCellData,m_ptCellData+m_iCellDataSize,0);
          }
          
          planarToInterleaved(src,m_ptCellData,m_iCellSize*m_iChannels*sizeof(T));
          
          delete src;
          if(m_iChannels == 1){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iCellSize, m_iCellSize,0, GL_LUMINANCE, glType, m_ptCellData);
          }else if (m_iChannels == 3){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iCellSize, m_iCellSize,0, GL_RGB, glType, m_ptCellData);   
          }else if (m_iChannels == 4){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iCellSize, m_iCellSize,0, GL_RGBA, glType, m_ptCellData);   
          }else{
            ERROR_LOG("invalid channel count: \"" << m_iChannels << "\"");
          }
        }
      }
      resetPixelTransfer();
    }else{ // multiTextureBuffer
      for(int y=0;y<m_iYCells;++y){
        for(int x=0;x<m_iXCells;++x){
          const Img<T> *src = image->shallowCopy(Rect(Point(x*m_iCellSize,y*m_iCellSize),m_matROISizes[x][y]));
          planarToInterleaved(src,m_matCellData[x][y],m_iCellSize*m_iChannels*sizeof(T));
          delete src;
        }
      }
    }
  }

  // }}}
  
  template<class T>
  void GLTextureMapImage<T>::resetPixelTransfer(){
    glPixelTransferf(GL_ALPHA_SCALE,1);
    glPixelTransferf(GL_RED_SCALE,1);
    glPixelTransferf(GL_GREEN_SCALE,1);
    glPixelTransferf(GL_BLUE_SCALE,1);
    glPixelTransferf(GL_ALPHA_BIAS,0);
    glPixelTransferf(GL_RED_BIAS,0);
    glPixelTransferf(GL_GREEN_BIAS,0);
    glPixelTransferf(GL_BLUE_BIAS,0);
    
  }

  template<class T>
  void GLTextureMapImage<T>::setUpPixelTransfer(depth d, float brightness, float contrast, float intensity, const ImgBase *image){
    // {{{ open
        
    (void)intensity;
    
    icl32f fScaleRGB(0),fBiasRGB(0);
    if( (brightness < 0)  && (contrast < 0) && (intensity < 0)){
      // image is null, because SingleBufferMode does not work!
      
      
      icl64f dScaleRGB,dBiasRGB;
      
      // auto adaption
      std::vector<Range<T> > channelRanges = getMinMax();
      
      Range<icl64f> r;
      if(channelRanges.size()){
        r = Range<icl64f>(channelRanges[0].minVal,channelRanges[0].maxVal);
        for(int i=1;i<m_iChannels;i++){
          if(channelRanges[i].minVal < r.minVal) r.minVal = channelRanges[i].minVal;
          if(channelRanges[i].maxVal > r.maxVal) r.maxVal = channelRanges[i].maxVal;
        }
      }else{
        r = Range<icl64f>(0,255);
      }

      icl64f l = iclMax(double(1),r.getLength());

      switch (getDepth<T>()){
        case depth8u:{
          dScaleRGB  = l ? 255.0/l : 255;
          dBiasRGB = (- dScaleRGB * r.minVal)/255.0;
          break;
        }
        case depth16s:{
          static const icl64f _max = (65536/2-1);
          dScaleRGB  = l ? _max/l : _max;
          dBiasRGB = (- dScaleRGB * r.minVal)/255.0;
          break;
        }
          /* orig: not working but... why drawn as float
              case depth32s:{ // drawn as float
              dScaleRGB  = l ? 255.0/l : 255;
              dBiasRGB = (- dScaleRGB * r.minVal)/255.0;
              // if not working properly: dBiasRGB /= 255.0
              break;
              }
          */
        case depth32s:{ // drawn as float
          dScaleRGB  = l ? 255.0/l : 255;
          dBiasRGB = (- dScaleRGB * r.minVal)/255.0;
          // if not working properly: dBiasRGB /= 255.0
          break;
        }
        case depth32f:{
          dScaleRGB  = l ? 255.0/l : 255;
          dBiasRGB = (- dScaleRGB * r.minVal)/255.0;
          dScaleRGB /= 255.0;
          break;
        }
        case depth64f:{ // drawn as float
          dScaleRGB  = l ? 255.0/l : 255;
          dBiasRGB = (- dScaleRGB * r.minVal)/255.0;
          dScaleRGB /= 255.0;
          break;
        }
        default:
          ICL_INVALID_FORMAT;
          break;
      }
      fScaleRGB = dScaleRGB;
      fBiasRGB = dBiasRGB;
    }else{
      fBiasRGB = (float)brightness/255.0;
      fScaleRGB  = 1;
      switch(d){
        case depth8u:
          fScaleRGB = 1; 
          break;
        case depth16s:
          fScaleRGB = 127; 
          break; // or 255 ?
        case depth32s:
        case depth32f:
        case depth64f:
          fScaleRGB = 1.0/255; 
          break;
      }
      
      float c = (float)contrast/255;
      if(c>0) c*=10;
      fScaleRGB*=(1.0+c);
      fBiasRGB-=c/2;
    }
    
      
    glPixelTransferf(GL_ALPHA_SCALE,fScaleRGB);
    glPixelTransferf(GL_RED_SCALE,fScaleRGB);
    glPixelTransferf(GL_GREEN_SCALE,fScaleRGB);
    glPixelTransferf(GL_BLUE_SCALE,fScaleRGB);
    glPixelTransferf(GL_ALPHA_BIAS,fBiasRGB);
    glPixelTransferf(GL_RED_BIAS,fBiasRGB);
    glPixelTransferf(GL_GREEN_BIAS,fBiasRGB);
    glPixelTransferf(GL_BLUE_BIAS,fBiasRGB);
  }

  // }}}

  template<class T>
  bool GLTextureMapImage<T>::compatible(const Img<T> *image) const{
    // {{{ open

    return image->getWidth() == m_iImageW && image->getHeight() == m_iImageH && image->getChannels() == m_iChannels;
  }

  // }}}

  template<class T>
  Img<T> *GLTextureMapImage<T>::deepCopy() const{
    if(m_bUseSingleBuffer){
      return 0;
    }
    Img<T> *image = new Img<T>(Size(m_iImageW,m_iImageH), m_iChannels);
    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){
        image->setROI(Rect(Point(x*m_iCellSize,y*m_iCellSize),m_matROISizes[x][y]));
        interleavedToPlanar(m_matCellData[x][y],image,m_iCellSize*m_iChannels*sizeof(T));
      }
    }
    image->setFullROI();
    return image;
  }
  
  namespace{
    inline float winToDraw(float x, float w) { return (2/w) * x -1; }  
    inline float drawToWin(float x, float w) { return (w/2) * x + (w/2); } 
  }
  
  template<class T>
  void GLTextureMapImage<T>::drawTo(const Rect &rect, const Size &windowSize, scalemode mode){
    // {{{ open
    if(!m_bUseSingleBuffer){
      
      glGenTextures(m_iXCells*m_iYCells,m_matTextureNames.data()); 
      
      //depth d = getDepth<T>();
      //int depthIdx = (int)d;
      
      setPackAlignment(getDepth<T>(),m_iImageW);
      
      setUpPixelTransfer(getDepth<T>(),m_aiBCI[0],m_aiBCI[1],m_aiBCI[2], 0);
      
      static GLenum aeGLTypes[] = { GL_UNSIGNED_BYTE, GL_SHORT, GL_FLOAT, GL_FLOAT, GL_FLOAT };
      GLenum glType = aeGLTypes[getDepth<T>()];

      for(int y=0;y<m_iYCells;++y){
        for(int x=0;x<m_iXCells;++x){
          glBindTexture(GL_TEXTURE_2D, m_matTextureNames[x][y]);
          
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,mode == interpolateLIN ? GL_LINEAR : GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,mode == interpolateLIN ? GL_LINEAR : GL_NEAREST);
          
          if(m_iChannels == 1){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iCellSize, m_iCellSize,0, GL_LUMINANCE, glType, m_matCellData[x][y]);
          }else if (m_iChannels == 3){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iCellSize, m_iCellSize,0, GL_RGB, glType, m_matCellData[x][y]);   
          }else if (m_iChannels == 4){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iCellSize, m_iCellSize,0, GL_RGBA, glType, m_matCellData[x][y]);   
          }else{
            ERROR_LOG("invalid channel count: \"" << m_iChannels << "\"");
          }
        }
      }

      resetPixelTransfer();
    }
    

    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor4f(1,1,1,1);
    glScalef(1,-1,1); // flip y
    
  

    float winW = windowSize.width;
    float winH = windowSize.height;
    
    float rectLeft = winToDraw(rect.x,winW);
    float rectTop = winToDraw(rect.y,winH);
    float rectRight = winToDraw(rect.x+rect.width,winW);
    float rectBottom = winToDraw(rect.y+rect.height,winH);
    float lX = rectRight - rectLeft;
    float lY = rectBottom - rectTop;

    float fracXForLastPart = 1-float(m_iCellSize-m_iRestX)/m_iCellSize;    
    float fracYForLastPart = 1-float(m_iCellSize-m_iRestY)/m_iCellSize;
    
    float xOffs = rectLeft;
    float yOffs = rectTop;
   
    float dx = fracXForLastPart ? lX/(m_iXCells-1+fracXForLastPart) : lX/m_iXCells;
    float dy = fracYForLastPart ? lY/(m_iYCells-1+fracYForLastPart) : lY/m_iYCells;

    // TODO lateron prebuffer all this calculations !
     
    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){

        glBindTexture(GL_TEXTURE_2D, m_matTextureNames[x][y]);


        glBegin(GL_QUADS);
        float xPos = xOffs+x*dx;
        float yPos = yOffs+y*dy;
        
        float texCoordsXMin = 0;
        float texCoordsYMin = 0;
        float texCoordsXMax = 1;
        float texCoordsYMax = 1;
        
        float vertexCoordsXMin = xPos;
        float vertexCoordsYMin = yPos;
        float vertexCoordsXMax = xPos+dx;
        float vertexCoordsYMax = yPos+dy;


        if(fracXForLastPart != 0 && x==m_iXCells-1){
          texCoordsXMax = fracXForLastPart;
          vertexCoordsXMax = rectRight;
        }
        if(fracYForLastPart != 0 && y==m_iYCells-1){
          texCoordsYMax = fracYForLastPart;
          vertexCoordsYMax = rectBottom;
        }
        glTexCoord2f(texCoordsXMin, texCoordsYMin ); glVertex2f(vertexCoordsXMin, vertexCoordsYMin);
        glTexCoord2f(texCoordsXMin, texCoordsYMax ); glVertex2f(vertexCoordsXMin, vertexCoordsYMax);
        glTexCoord2f(texCoordsXMax, texCoordsYMax ); glVertex2f(vertexCoordsXMax, vertexCoordsYMax);
        glTexCoord2f(texCoordsXMax, texCoordsYMin ); glVertex2f(vertexCoordsXMax, vertexCoordsYMin);
        glEnd();

      }
    }    
    
    if(!m_bUseSingleBuffer){
      glDeleteTextures(m_iXCells*m_iYCells,m_matTextureNames.data());  
    }
    if(m_drawGrid){
      glColor4fv(m_gridColor);

      //glLineWidth(1); dunno?

      float dx = lX/m_iImageW;
      float dy = lY/m_iImageH;

      float pixDX = (dx*winW)/2.0f;
      float pixDY = (dy*winH)/2.0f;

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
        for(int x=0;x<=m_iImageW;x+=stepx){
          float xx = rectLeft+x*dx;
          for(int y=0;y<=m_iImageH;y+=stepy){
            float yy = rectTop+y*dy;
            glVertex2f(xx-rr,yy);
            glVertex2f(xx+rr,yy);

            glVertex2f(xx,yy-rr);
            glVertex2f(xx,yy+rr);
          }
        }
        glEnd();
      }else{
        glBegin(GL_LINES);
        for(int x=0;x<=m_iImageW;++x){
          float xx = rectLeft+x*dx;
          glVertex2f(xx,rectTop);
          glVertex2f(xx,rectBottom);
        }
        for(int y=0;y<=m_iImageH;++y){
          float yy = rectTop+y*dy;
          glVertex2f(rectLeft,yy);
          glVertex2f(rectRight,yy);
        }
        glEnd();
        
      }
    }
    
    glColor4f(1,1,1,1);    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();


  }

  // }}}
  
  template<class T>
  void GLTextureMapImage<T>::setDrawGrid(bool enabled, float *color){
    m_drawGrid = enabled;
    if(color){
      std::copy(color,color+4,m_gridColor);
    }
  }

  template<class T>
  void GLTextureMapImage<T>::setGridColor(float *color){
    std::copy(color,color+4,m_gridColor);
  }

  template<class T>
  const float *GLTextureMapImage<T>::getGridColor() const{
    return m_gridColor;
  }

  template<class T>
  void GLTextureMapImage<T>::drawTo3D(float *pCenter, float *pFirstAxis, float *pSecondAxis){
    if(!m_bUseSingleBuffer){
      
      glGenTextures(m_iXCells*m_iYCells,m_matTextureNames.data()); 
      
      setPackAlignment(getDepth<T>(),m_iImageW);
      setUpPixelTransfer(getDepth<T>(),m_aiBCI[0],m_aiBCI[1],m_aiBCI[2], 0);
      
      static GLenum aeGLTypes[] = { GL_UNSIGNED_BYTE, GL_SHORT, GL_INT, GL_FLOAT, GL_FLOAT };
      GLenum glType = aeGLTypes[getDepth<T>()];
      
      for(int y=0;y<m_iYCells;++y){
        for(int x=0;x<m_iXCells;++x){
          glBindTexture(GL_TEXTURE_2D, m_matTextureNames[x][y]);
          
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_NEAREST);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
          
          if(m_iChannels == 1){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iCellSize, m_iCellSize,0, GL_LUMINANCE, glType, m_matCellData[x][y]);
          }else if (m_iChannels == 3){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iCellSize, m_iCellSize,0, GL_RGB, glType, m_matCellData[x][y]);   
          }else if (m_iChannels == 4){
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_iCellSize, m_iCellSize,0, GL_RGBA, glType, m_matCellData[x][y]);   
          }else{
            ERROR_LOG("invalid channel count: \"" << m_iChannels << "\"");
          }
        }
      }

      resetPixelTransfer();
    }
    
    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------------
    
  
    float fracXForLastPart = 1-float(m_iCellSize-m_iRestX)/m_iCellSize;    
    float fracYForLastPart = 1-float(m_iCellSize-m_iRestY)/m_iCellSize;

    float S[3] = {pCenter[0],pCenter[1],pCenter[2]};
    
    float A[3] = {pFirstAxis[0]-S[0],pFirstAxis[1]-S[1],pFirstAxis[2]-S[2]};
    float B[3] = {pSecondAxis[0]-S[0],pSecondAxis[1]-S[1],pSecondAxis[2]-S[2]};

    float LA = ::sqrt( A[0]*A[0] + A[1]*A[1] + A[2]*A[2] );     
    float LB = ::sqrt( B[0]*B[0] + B[1]*B[1] + B[2]*B[2] );
    for(int i=0;i<3;i++){
      A[i] /= LA;
      B[i] /= LB;
    }

    float DA = fracXForLastPart ? LA/(m_iXCells-1+fracXForLastPart) : LA/m_iXCells;
    float DB = fracYForLastPart ? LB/(m_iYCells-1+fracYForLastPart) : LB/m_iYCells;
    
    float V[3];
    
    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){
        glBindTexture(GL_TEXTURE_2D, m_matTextureNames[x][y]);
        
        glBegin(GL_QUADS);

        float texCoordsXMin = 0;
        float texCoordsYMin = 0;
        float texCoordsXMax = 1;
        float texCoordsYMax = 1;
        
        float USE_DA0 = x*DA;
        float USE_DB0 = y*DB;
        float USE_DA1 = (x+1)*DA;
        float USE_DB1 = (y+1)*DB;
        
        if(fracXForLastPart != 0 && x==m_iXCells-1){
          texCoordsXMax =  fracXForLastPart;
          USE_DA1 = LA;
        }
        if(fracYForLastPart != 0 && y==m_iYCells-1){
          texCoordsYMax = fracYForLastPart;
          USE_DB1 = LB;
        }
        
        glTexCoord2f(texCoordsXMin, texCoordsYMin ); 
        for(int i=0;i<3;i++) V[i] = S[i] + USE_DA0*A[i] + USE_DB0*B[i];
        glVertex3fv(V);

        glTexCoord2f(texCoordsXMin, texCoordsYMax ); 
        for(int i=0;i<3;i++) V[i] = S[i] + USE_DA0*A[i] + USE_DB1*B[i];
        glVertex3fv(V);
        
        for(int i=0;i<3;i++) V[i] = S[i] + USE_DA1*A[i] + USE_DB1*B[i];
        glTexCoord2f(texCoordsXMax, texCoordsYMax ); 
        glVertex3fv(V);
        
        for(int i=0;i<3;i++) V[i] = S[i] + USE_DA1*A[i] + USE_DB0*B[i];
        glTexCoord2f(texCoordsXMax, texCoordsYMin ); 
        glVertex3fv(V);
        
        glEnd();
      }
    }
    if(!m_bUseSingleBuffer){
      glDeleteTextures(m_iXCells*m_iYCells,m_matTextureNames.data());  
    }
  }


  namespace{

    template<class T>
    inline void histo_entry(T v, double m, vector<int> &h, unsigned int n, double r){
      // todo check 1000 times +5 times (3Times done!)
        h[ ceil( n*(v-m)/(r+1)) ]++;
    }

    template<class T>
    void calculate_histo_interleaved(T *data,
                                     Size dataROISize,
                                     int dataLineLength,
                                     int channels, 
                                     const std::vector<Range64f> &ranges,
                                     std::vector< std::vector<int> > &histos){
      for(int x=0;x<dataROISize.width;++x){
        for(int y=0;y<dataROISize.height;++y){
          for(int c=0;c<channels;++c){
            T &val = data[(x+dataLineLength*y)*channels+c];
            histo_entry(val,ranges[c].minVal,histos[c],256,ranges[c].getLength());
          }
        }
      }
    }
  }
  
  template<class T>
  const ImageStatistics &GLTextureMapImage<T>::updateStatistics(ImageStatistics &s){
    s.isNull = false;
    s.d = getDepth<T>();
    
    std::vector<Range<T> > rangesT = getMinMax();
    s.ranges.resize(rangesT.size());
    for(unsigned int i=0;i<rangesT.size();i++){
      s.ranges[i].minVal = rangesT[i].minVal;
      s.ranges[i].maxVal = rangesT[i].maxVal;
    }

    s.histos.resize(m_iChannels);
    for(unsigned int i=0;i<s.histos.size();i++){
      s.histos[i].resize(256);
      std::fill(s.histos[i].begin(),s.histos[i].end(),0);
    }
    
    std::vector<Range64f> useRanges = s.ranges;
    if(s.params.getFormat() != formatMatrix){
      std::fill(useRanges.begin(),useRanges.end(),Range64f(0,255));
    }

    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){
        calculate_histo_interleaved(m_matCellData[x][y],
                                    m_matROISizes[x][y],
                                    m_iCellSize,
                                    m_iChannels,
                                    useRanges,
                                    s.histos);
      }
    }
    
    return s;
  }
  
  template class GLTextureMapImage<icl8u>;
  template class GLTextureMapImage<icl16s>;
  template class GLTextureMapImage<icl32s>;
  template class GLTextureMapImage<icl32f>;
}


