
#ifndef ICLGLTEXTUREMAPBASEIMAGE_H
#define ICLGLTEXTUREMAPBASEIMAGE_H

#include <iclSize.h>
#include <iclRect.h>
#include <iclTypes.h>
#include <iclGLTextureMapImage.h>

namespace icl{
  
  /// wrapper of the GLTextureMapImage for processing ImgBase objects \ingroup UNCOMMON
  /** The GLTextureMapBaseImage complies an additional abstraction layer
      for displaying image using OpenGL's texture mapping abilities.
      It wraps simple GLTextureMapImages for all supported depths 
      (8u, 16s, 32s, 32f) and tackles different depths automatically.
      At each time only one of the wrapped GLTextureMapImages is valid,
      so each instance of GLTextureMapBaseImage supports only this depth
      of images to draw very quickly. If the image depth varies over time,
      the internal GLTextureMapImage must be reallocated to that depth, 
      which is a bit less performant.\n
      In addition, the GLTextureMapBase image hides the current size and
      channel count of the currently drawn image, which must explicitly be 
      given to the wrapped GLTextureMapImages constructor. So the current
      valid GLTextureMapImage must be reallocated also, if the current
      image size or channel count changes.\n
      The depth depth64f is not supported by OpenGL's glTexImage2D function,
      and so it is also not supported by the GLTextureMapImage. To ensure
      compatibility, the GLTextureMapBaseImage provides a fallback 
      implementation that internally creates an Img<icl32f> of given 
      Img<icl64f> images temporarily.
   */
  class GLTextureMapBaseImage{
    public:
    
    /// Constructor with optionally given image
    /** @param image if not NULL, the constructor calls updateTexture(image) 
                     immediately after initialization 
        @param useSingleBuffer decides whether to instantiate wrapped 
                               GLTextureMap images in single or multi buffer mode
                               <b>WARNING:</b> Settin use single buffer to true
                               may cause conflicts on some graphic cards.
    **/
    GLTextureMapBaseImage(const ImgBase* image = 0, bool useSingleBuffer = false): 
    m_po8u(0),m_po16s(0),m_po32s(0),m_po32f(0), m_poChannelBuf(0),
    m_bUseSingleBuffer(useSingleBuffer){
      m_oCurrentImageParams = ImgParams(Size::null,0);
      m_aiBCI[0]=m_aiBCI[1]=m_aiBCI[2]=0; 
      if(useSingleBuffer){
        static bool first = true;
        if(first) {
          first = false;
          WARNING_LOG("Single Buffer mode may cause serious problems with some graphic cards...");
        }
      }
      if(image) updateTextures(image);
    }
    
    /// Destructor
    ~GLTextureMapBaseImage();
    

    /// generalization of the GLTextureImageImages updateTexture(const Img<T> *) function
    void updateTextures(const ImgBase *image);

    /// this call is passed to the current valid GLTextureMapImage 
    void drawTo(const Rect &rect, const Size &windowSize);

    /// draw the image into the rectangle specified by Center and two given axis
    /** Example:
        <pre>
        Center------------->FirstAxis
          |................
          |...         ....
          |...  Image  ....
          |...         ....
          V................
        SecondAxis
        </pre>
    **/
    void drawTo3D(float *pCenter, float *pFirstAxis, float *pSecondAxis);
    
    
    /// if the GLTextureMapBase image has no Image, it cannot be drawn
    bool hasImage(){
      return m_po8u || m_po16s || m_po32s || m_po32f;
    }
    
    /// returns the size of the current image or (0,0) if there is no image
    Size getSize() const;
    
    /// returns current depth
    depth getDepth() const;
    
    /// returns current channel count
    int getChannels() const;
    
    /// returns current image format
    format getFormat() const;
    
    /// returns current image roi
    Rect getROI() const;
    
    /// retuns mininum and maximum of the current image
    std::vector<Range<icl32f> > getMinMax() const;
    
    /// returns the image color at pixel position (x,y)
    /** if (x,y) is outside the image rect, the returned vector is empty*/
    std::vector<icl32f> getColor(int x, int y) const;
    
    /// sets up brightness, contrast and intensity
    /** if b=c=i, then brightness and contrast is adpated automatically */
    void bci(int b=-1, int c=-1, int i=-1);

    /// sets if the wrapped images are created with single or multi buffer 
    void setSingleBufferMode(bool useSingleBuffer);
    
    /// creates a snapshot of the current buffered image (multi buffer mode only)
    ImgBase *deepCopy() const;
    private:
    /// creates an image with valid channel count 1 or 3
    /** - images with 0 channels are invalid
        - images with 1 or 3 channels are used directly 
        - images with 2 channels are adapted by a 3rd black channel
        - images with more then 3 channels are truncated to the first 3 channels 
    */
    const ImgBase *adaptChannels(const ImgBase *image);
    
    
    /// GLTextureMapImage for images with depth8u
    GLTextureMapImage<icl8u> *m_po8u;

    /// GLTextureMapImage for images with depth16s
    GLTextureMapImage<icl16s> *m_po16s;

    /// GLTextureMapImage for images with depth32s
    GLTextureMapImage<icl32s> *m_po32s;

    /// GLTextureMapImage for images with depth32f
    GLTextureMapImage<icl32f> *m_po32f;

    // internal buffer image for input image with 2 channels
    ImgBase *m_poChannelBuf;

    /// brightness contrast and intensity buffer
    int m_aiBCI[3];

    /// flag that decides of the wrapped GLTMIs are created with single or mult buffer
    bool m_bUseSingleBuffer;
    
    /// stores the current images params
    ImgParams m_oCurrentImageParams;
  };  
}

#endif
