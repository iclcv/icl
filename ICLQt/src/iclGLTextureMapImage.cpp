#include "iclGLTextureMapImage.h"
#include <iclCC.h>

namespace icl{

  using namespace std;

  template<class T>
  GLTextureMapImage<T>::GLTextureMapImage(const Size &imageSize,  int channels,int cellSize){
    // {{{ open
    
    ICLASSERT( channels == 3 || channels == 1);
    ICLASSERT( cellSize >= 16);
    ICLASSERT( getDepth<T>() != depth64f );
    m_iChannels = channels;
    m_iCellSize = cellSize;
    m_iImageW = imageSize.width;
    m_iImageH = imageSize.height;
    m_iXCells = m_iImageW/m_iCellSize;
    m_iYCells = m_iImageH/m_iCellSize;
    
    m_iRestX = m_iImageW % m_iCellSize;
    m_iRestY = m_iImageH % m_iCellSize;
    if(m_iRestX) m_iXCells++;
    if(m_iRestY) m_iYCells++;
    
    m_iCellDataSize = m_iCellSize*m_iCellSize*m_iChannels;
    m_ptCellData = new T[m_iCellDataSize];
    
    m_matTextureNames = SimpleMatrix<GLuint>(m_iXCells,m_iYCells);
    m_matROISizes = SimpleMatrix<Size,SimpleMatrixAllocSize>(m_iXCells,m_iYCells);
    glGenTextures(m_iXCells*m_iYCells,m_matTextureNames.data()); 
    
    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){
        m_matROISizes[x][y].width  = m_iRestX ? (x==m_iXCells-1 ? m_iRestX : m_iCellSize) : m_iCellSize;
        m_matROISizes[x][y].height = m_iRestY ? (y==m_iYCells-1 ? m_iRestY : m_iCellSize) : m_iCellSize;
      }
    }
  }
  
  // }}}

  template<class T>
  GLTextureMapImage<T>::~GLTextureMapImage(){
    // {{{ open

    glDeleteTextures(m_iXCells*m_iYCells,m_matTextureNames.data());
    delete [] m_ptCellData;
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
  void GLTextureMapImage<T>::updateTextures(const Img<T> *image){
    // {{{ open

    ICLASSERT( m_iChannels == image->getChannels() );
    ICLASSERT( m_iImageW == image->getWidth());
    ICLASSERT( m_iImageH == image->getHeight());
    
    setPackAlignment(getDepth<T>(),image->getWidth());
    setUpPixelTransfer();
    
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
        }else{
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_iCellSize, m_iCellSize,0, GL_RGB, glType, m_ptCellData);   
        }
      }
    }
  }

  // }}}
  
  template<class T>
  void GLTextureMapImage<T>::setUpPixelTransfer(){
    // {{{ open

    if(getDepth<T>() == depth32f){
      glPixelTransferf(GL_RED_SCALE,1./255.);
      glPixelTransferf(GL_GREEN_SCALE,1./255.);
      glPixelTransferf(GL_BLUE_SCALE,1./255.);
    }else if( getDepth<T>() == depth16s){
      glPixelTransferf(GL_RED_SCALE,127.5);
      glPixelTransferf(GL_GREEN_SCALE,127.5);
      glPixelTransferf(GL_BLUE_SCALE,127.5);
    }else{
      glPixelTransferf(GL_RED_SCALE,1);
      glPixelTransferf(GL_GREEN_SCALE,1);
      glPixelTransferf(GL_BLUE_SCALE,1);
    }
  }

  // }}}

  template<class T>
  bool GLTextureMapImage<T>::compatible(const Img<T> *image) const{
    // {{{ open

    return image->getWidth() == m_iImageW && image->getHeight() == m_iImageH && image->getChannels() == m_iChannels;
  }

  // }}}

  namespace{
    inline float winToDraw(float x, float w) { return (2/w) * x -1; }  
    inline float drawToWin(float x, float w) { return (w/2) * x + (w/2); } 
  }
  
  template<class T>
  void GLTextureMapImage<T>::drawTo(const Rect &rect, const Size &windowSize){
    // {{{ open

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    float winW = windowSize.width;
    float winH = windowSize.height;
    
    float rectLeft = winToDraw(rect.x,winW);
    float rectTop = winToDraw(rect.y+rect.height,winH);
    float rectRight = winToDraw(rect.x+rect.width,winW);
    float rectBottom = winToDraw(rect.y,winH);
    
    /// correcture of rest_w and rest_h pixels
    if(m_iRestX){
      float fac = float(m_iXCells*m_iCellSize)/(m_iXCells*m_iCellSize-(m_iCellSize-m_iRestX));
      rectRight = winToDraw(rect.x+(fac*rect.width),winW);
    }
    if(m_iRestY){
      float fac = float(m_iYCells*m_iCellSize)/(m_iYCells*m_iCellSize-(m_iCellSize-m_iRestY));
      rectTop = winToDraw(rect.y+(fac*rect.height),winH);
    }
    
    float xOffs = rectLeft;
    float yOffs = rectTop;
    float dx = (rectRight-rectLeft)/m_iXCells;
    float dy = (rectBottom-rectTop)/m_iYCells;
    
    for(int y=0;y<m_iYCells;++y){
      for(int x=0;x<m_iXCells;++x){
        glBindTexture(GL_TEXTURE_2D, m_matTextureNames[x][y]);
        glBegin(GL_QUADS);
        float xPos = xOffs+x*dx;
        float yPos = yOffs+y*dy;
        glTexCoord2f(0.0, 0.0); glVertex2f(xPos,yPos);
        glTexCoord2f(0.0, 1.0); glVertex2f(xPos,yPos+dy);
        glTexCoord2f(1.0, 1.0); glVertex2f(xPos+dx,yPos+dy);
        glTexCoord2f(1.0, 0.0); glVertex2f(xPos+dx,yPos);
        glEnd();
      }
    }    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  // }}}
  
  
  template class GLTextureMapImage<icl8u>;
  template class GLTextureMapImage<icl16s>;
  template class GLTextureMapImage<icl32s>;
  template class GLTextureMapImage<icl32f>;
}


