#ifndef ICL_HOUGH_LINE_DETECTOR_H
#define ICL_HOUGH_LINE_DETECTOR_H


#include <cmath>
#include <vector>
#include <ICLUtils/FixedVector.h>
#include <ICLUtils/Range.h>
#include <ICLUtils/Point.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/DynMatrix.h>
#include <ICLUtils/StraightLine2D.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  
  /** LineDetection class using hough-space line detection
      
      @section ALG Algorithm
      Image lines (of infinite length) can be described by a tuple (rho,r) and the line
      equation \f$r = x\cdot cos(rho) + y\cdot sin(rho)\f$. Each pixel (x,y) belongs potentially
      to a all lines (rho,r) where this equation is true. 
      Internally, the space of possible lines (sometimes referred a the hough-space) is sampled 
      (internally we use an instance of Img32s).
      Rho is mapped to the x-axis (x=0 -> rho=0, x=width-1 -> rho = 2*M_PI) and r is mapped to
      the y-axis (y=0 -> r=minR, y=height-1 -> r=maxR).
      So the size of the internal hough-space becomes 2*M_PI/dRho x (rRange.max-rRange.min)/dR.
      
      When a new border pixel (x,y) is added, the probability of each potential line that intersects
      (x,y) is increased. Technically this is performed by computing r(rho) for all possible 
      values rho (all samples) with the formula for r above which leads to values (rho,r) which's
      hough-space entries are increased by one. 
      The best line matches can be computed from the maxima in the resulting hough space.
      
      @section OPT Optimizations
      To enhance line detection performance, the HoughLineDetector provides a set of optimization
      approaches:

      @subsection S1 Local inhibition around detected lines
      As a single image line, might result in 2 or even more lines in the hough-space (because of
      spacial noise in the pixels detection procedure), the hough-space is locally inhibited around
      already detected lines. The amount of this inhibition procedure can be set by the constructor
      parameters (rInhibitionRange and rhoInhibitionRange).
      
      @subsection S2 Gaussian inhibition
      Sometimes hard borders of the inhibition sub-window lead to random 2nd and 3rd detected line
      values. To stabilize this, inhibition can be set up to be 'gaussian-blurred', which means, that
      hough-table entries that are very close to already detected lines are nearly removed completely
      and those that are also in the inhibition sub-rect but some pixels away from the detected line
      center are inhibited less.

      @subsection S2 Stabilization
      
      Resulting from the sampling procedure of both, that actual image space and the hough-space,
      entries in the hough-space might have hard borders or even gaps. To avoid this, three different
      optimization techniques are provided, which can all be combined.
      
      @subsubsection S3 Blurring the hough space
      This means, that the hough-space is simply blurred using a 3x3 linear gaussian filter. This
      optimization provides much more stable results, but it also entails a higher computational 
      expense.

      @subsubsection S4 Dilation of pixel entries
      If this optimization is activated, no only the given pixel (x,y) is added internally, but
      also it's 4 direct neighbour pixels. This also leads to a kind of blurred hough table

      @subsubsection S5 Blurred sampling of lines
      This optimization changes the line-sampling procedure. When a line is sampled for all rho-values
      using r = x cos(rho) + y sin(rho), not only the resulting pixel p in the hough table is increased,
      but also the pixel above, and the pixel below (actually, the exactly computed pixel p is increased
      by 2 and it's upper and lower neighbour's are increased by one). This again leads to a blurring effect.
      however, the other two optimzations provides better results.
      It's worth mention, that this optimization's additional computational expense is low in comparison
      to the other two optizations.
      
  */
  class HoughLineDetector : public Uncopyable{
    public:

    /// Create a new HoughLineDetectorInstance
    /** @param dRho line angle sampling distance 
        @param dR line distance to 0 sampling factor 
        @param rRange range of line distances 
        @param rInhibitionRange vertical radius for local inhibition 
        @param rhoInhibitionRange horizontal radius for local inhibition
        @param gaussianInhibition enables gaussian inhibition
        @param blurHoughSpace enables hough space blurring
        @param dilateEntries enables entry dilatation
        @param blurredSampling enables blurred sampling */
    HoughLineDetector(float dRho, float dR, const Range32f rRange, 
                      float rInhibitionRange, float rhoInhibitionRange,
                      bool gaussianInhibition=true,
                      bool blurHoughSpace=true,
                      bool dilateEntries=true,
                      bool blurredSampling=false);

    /// adds a new point
    inline void add(const Point &p){ 
      add_intern(p.x,p.y); 
    }

    /// adds new points
    inline void add(const std::vector<Point> &ps){
      for(unsigned int i=0;i<ps.size();++i) add(ps[i]);
    }
    
    /// adds a new point
    inline void add(const Point32f &p){ 
      add_intern(p.x,p.y); 
    }

    /// adds new points
    inline void add(const std::vector<Point32f> &ps){
      for(unsigned int i=0;i<ps.size();++i) add(ps[i]);
    }

    /// adds a new point
    template<class T>
    inline void add(const FixedColVector<T,2> &p){ 
      add_intern(p[0],p[1]); 
    }    
    
    /// adds new points
    template<class T>
    inline void add(const std::vector<const FixedColVector<T,2> > &ps){
      for(unsigned int i=0;i<ps.size();++i) add(ps[i]);
    }


    /// returns current hough-table image
    const Img32s &getImage() const { return m_image; }

    /// returns current gaussian inhibition map
    const Img32f &getInhibitionMap() const { return m_inhibitImage; }

    /// sets all hough table entries to 0
    void clear();
    
    /// detects up to max lines
    std::vector<StraightLine2D> getLines(int max);

    /// detects up to max lines and pushes the line significances into given destination vector
    /** significances are relative to the first line match, which's significance is always 1.0*/
    std::vector<StraightLine2D> getLines(int max, std::vector<float> &significances);
    
    private:

    /// internal utility function
    inline float r(float rho, float x, float y) const{ 
      return x*cos(rho) + y*sin(rho);
    }
    /// internal utility function
    inline int getX(float rho) const {
      return round(rho * m_mrho);
    }
    /// internal utility function
    inline int getY(float r) const {
      return round(m_mr * r + m_br); 
    }
    /// internal utility function
    inline float getRho(int x) const{
      return x/m_mrho;
    }
    /// internal utility function
    inline float getR(int y) const{
      return (y-m_br)/m_mr;
    }
    /// internal utility function
    inline void incLut(float rho, float r){
      int x = getX(rho);
      int y = getY(r);
      if(y >= 0 && y < m_h){
        m_lut(x,y)++;
      }
    }
    /// internal utility function
    inline void incLutBlurred(float rho, float r){
      int x = getX(rho);
      int y = getY(r);
      if(y >= 0 && y < m_h){
        if(y>0) m_lut(x,y-1)++;
        m_lut(x,y)+=2;
        if(y<m_w-1) m_lut(x,y+1)++;
      }
    }
    /// internal utility function
    inline int &cyclicLUT(int x, int y){
      static int _null = 0;
      if(y<0||y>=m_h) return _null;
      int dx = x<0 ? m_w : x>=m_w ? -m_w : 0;
      if(!m_image.getImageRect().contains(x+dx,y)){
        ERROR_LOG("tryed to access " << x+dx << "," << y);
        return _null;
      }
      return m_lut(x+dx,y);
    }
  
    /// internal utility function
    void apply_inhibition(const Point &p);

    /// internal utility function
    void add_intern(float x, float y);

    /// internal utility function
    void add_intern2(float x, float y);

    /// internal utility function
    void blur_hough_space_if_necessary();
    

    float m_dRho;
    float m_dR;
    Range32f m_rRange;
    int m_w;
    int m_h;

    float m_mr;
    float m_br;
    float m_mrho;

    float m_rInhib;
    float m_rhoInhib;

    bool m_gaussianInhibition;
    bool m_blurHoughSpace;
    bool m_dilateEntries;
    bool m_blurredSampling;
    
    Channel32s m_lut;
    Img32s m_image;
    Img32f m_inhibitImage;
  };
  
}

#endif
