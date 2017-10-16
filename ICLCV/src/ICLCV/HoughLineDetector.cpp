/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/HoughLineDetector.cpp                  **
** Module : ICLCV                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLCV/HoughLineDetector.h>
#include <ICLMath/DynMatrixUtils.h>
#include <ICLFilter/ConvolutionOp.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;

namespace icl{
  namespace cv{

    struct HoughLineDetector::Data{
      float dRho;
      utils::Range32f rRange;
      int w;
      int h;

      float mr;
      float br;
      float mrho;

      float rInhib;
      float rhoInhib;

      bool gaussianInhibition;
      bool blurHoughSpace;
      bool dilateEntries;
      bool blurredSampling;

      core::Channel32s lut;
      core::Img32s image;
      core::Img32f inhibitImage;
    };

    HoughLineDetector::HoughLineDetector(float dRho, float dR, const Range32f &rRange, float rInhibitionRange, float rhoInhibitionRange,
                                         bool gaussianInhib, bool blurHoughSpace,bool dilateEntries,bool blurredSampling) :m_data(new Data){

      addProperty("delta.angle","range","[0.01,1]",dRho,0,"line angle increment");
      addProperty("delta.radius","range","[1,1000]:1",dR,0,"line radius increament");
      addProperty("range.min radius","range:spinbox","[0,10000]:1",rRange.minVal,0,"minimum line radius");
      addProperty("range.max radius","range:spinbox","[0,10000]:1",rRange.maxVal,0,"maximum line radius");
      addProperty("inhibition.radius-axis","range","[0,10000]:1",rInhibitionRange,0,"inhibition window size along radius-axis");
      addProperty("inhibition.angle-axis","range","[0,6.28]",rhoInhibitionRange,0,"inhibition window size along angle-axis");
      addProperty("inhibition.gaussian","flag","",gaussianInhib,0,"fade out inhibition using gaussian kernel");
      addProperty("adding.blur hough space","flag","",blurHoughSpace,0,"blur the whole hough space before line extraction");
      addProperty("adding.dilate entries","flag","",dilateEntries,0,"apply dilation on entries when adding");
      addProperty("adding.blurred sampling","flag","",blurredSampling,0,"sample the houghspace in a blurred fashion");

      reset();
    }

    HoughLineDetector::~HoughLineDetector(){
      delete m_data;
    }

    void HoughLineDetector::prepare_all(){
      prepare(getPropertyValue("delta.angle"),
              getPropertyValue("delta.radius"),
              Range32f(getPropertyValue("range.min radius"),
                       getPropertyValue("range.max radius")),
              getPropertyValue("inhibition.radius-axis"),
              getPropertyValue("inhibition.angle-axis"),
              getPropertyValue("adding.blur hough space"),
              getPropertyValue("adding.dilate entries"),
              getPropertyValue("adding.blur hough space"));
    }

    void HoughLineDetector::prepare(float dRho, float dR, const utils::Range32f &rRange,
                                    float rInhibitionRange, float rhoInhibitionRange,
                                    bool gaussianInhibition,
                                    bool blurHoughSpace,
                                    bool dilateEntries,
                                    bool blurredSampling){
      m_data->dRho = dRho;
      m_data->rRange = rRange;
      m_data->rInhib = rInhibitionRange;
      m_data->rhoInhib = rhoInhibitionRange;
      m_data->gaussianInhibition = gaussianInhibition;
      m_data->blurHoughSpace = blurHoughSpace;
      m_data->dilateEntries = dilateEntries;
      m_data->blurredSampling = blurredSampling;


      m_data->w = ceil(2*M_PI/dRho);
      m_data->h = (rRange.maxVal-rRange.minVal)/dR;

      m_data->image.setChannels(1);
      m_data->image.setSize(Size(m_data->w, m_data->h));
      m_data->lut = m_data->image[0];

      m_data->mr = (m_data->h-1)/(rRange.maxVal-rRange.minVal);
      m_data->br = -rRange.minVal * m_data->mr;

      m_data->mrho = (m_data->w-1)/(2*M_PI);

      if(gaussianInhibition){
        /// create inhibition image
        float dx = m_data->rhoInhib/(2*M_PI) * float(m_data->w);
        float dy = m_data->rInhib/(m_data->rRange.maxVal-m_data->rRange.minVal) * float(m_data->h);

        int w = 2*dx, h=2*dy;
        if(dx >0 && dy >0){
          m_data->inhibitImage = Img32f(Size(2*dx,2*dy),1);
          Channel32f I = m_data->inhibitImage[0];

          Point32f c(I.getWidth()/2,I.getHeight()/2);

          for(int x=0;x<I.getWidth();++x){
            for(int y=0;y<I.getHeight();++y){
              float r = (c-Point32f(x,y)).transform(2./w,2./h).norm();
              I(x,y) = 1.0 - exp(-r*r);
            }
          }
        }
      }
    }

    void HoughLineDetector::add(const Img8u &binaryImage){
      ICLASSERT_THROW(binaryImage.getChannels() == 1, ICLException("HoughLineDetector::add: can only work with 1 channel images"));
      const Channel8u c = binaryImage[0];
      for(int y=0;y<c.getHeight();++y){
        for(int x=0;x<c.getWidth();++x){
          if(c(x,y)) add_intern(x,y);
        }
      }
    }

    void HoughLineDetector::add_intern(float x, float y){
      if(m_data->dilateEntries){
        add_intern2(x,y);
        add_intern2(x-1,y);
        add_intern2(x+2,y);
        add_intern2(x,y-1);
        add_intern2(x,y+1);
      }else{
        add_intern2(x,y);
      }
    }

    void HoughLineDetector::add_intern2(float x, float y){
      if(m_data->blurredSampling){
        for(float rho=0;rho<2*M_PI;rho+=m_data->dRho){
          incLutBlurred(rho,r(rho,x,y));
        }
      }else{
        for(float rho=0;rho<2*M_PI;rho+=m_data->dRho){
          incLut(rho,r(rho,x,y));
        }
      }
    }

    void HoughLineDetector::reset(){
      prepare_all();
      std::fill(m_data->lut.begin(),m_data->lut.end(),0);
    }

    void HoughLineDetector::apply_inhibition(const Point &p){
      float dx = m_data->rhoInhib/(2*M_PI) * float(m_data->w);
      float dy = m_data->rInhib/(m_data->rRange.maxVal-m_data->rRange.minVal) * float(m_data->h);
      const Rect r(p.x-dx,p.y-dy,2*dx,2*dy);

      if(m_data->gaussianInhibition){
        Channel32f c0 = m_data->inhibitImage[0];
        for(int x=r.x;x<r.right();++x){
          for(int y=r.y;y<r.bottom();++y){
            if(!m_data->inhibitImage.getImageRect().contains(x-r.x,y-r.y)){
            }else{
              cyclicLUT(x,y) *= c0(x-r.x,y-r.y);
            }
          }
        }
      }else{
        for(int x=r.x;x<r.right();++x){
          for(int y=r.y;y<r.bottom();++y){
            cyclicLUT(x,y)=0;
          }
        }
      }
    }

    void HoughLineDetector::blur_hough_space_if_necessary(){
      if(m_data->blurHoughSpace){
        ImgBase *image = 0;
        ConvolutionOp co( (ConvolutionKernel(ConvolutionKernel::gauss3x3)) );
        co.setClipToROI(false);
        co.apply(&m_data->image,&image);
        m_data->image = *image->asImg<icl32s>();
        ICL_DELETE(image);
        m_data->image.setFullROI();
        m_data->lut = m_data->image[0];
      }
    }

    std::vector<StraightLine2D> HoughLineDetector::getLines(int max, bool resetAfterwards) {
      blur_hough_space_if_necessary();

      std::vector<StraightLine2D> ls;
      ls.reserve(max);

      for(int i=0;i<max;++i){
        Point p(-1,-1);
        int m = m_data->image.getMax(0,&p);
        if(m == 0) return ls;
        ls.push_back(StraightLine2D(getRho(p.x),getR(p.y)));
        apply_inhibition(p);

      }

      if(resetAfterwards){
        reset();
      }
      return ls;
    }


    std::vector<StraightLine2D> HoughLineDetector::getLines(int max, std::vector<float> &significances, bool resetAfterwards){
      blur_hough_space_if_necessary();

      std::vector<StraightLine2D> ls;
      ls.reserve(max);
      significances.clear();
      significances.reserve(max);

      int firstMax = -1;
      for(int i=0;i<max;++i){
        Point p(-1,-1);
        int m = m_data->image.getMax(0,&p);
        if(!i) firstMax = m;
        significances.push_back(float(m)/firstMax);
        if(m == 0) return ls;
        ls.push_back(StraightLine2D(getRho(p.x),getR(p.y)));

        apply_inhibition(p);

      }
      if(resetAfterwards){
        reset();
      }

      return ls;
    }



    /// adds a new point
    void HoughLineDetector::add(const utils::Point &p){
      add_intern(p.x,p.y);
    }

    /// adds new points
    void HoughLineDetector::add(const std::vector<utils::Point> &ps){
      for(unsigned int i=0;i<ps.size();++i) add(ps[i]);
    }

    /// adds a new point
    void HoughLineDetector::add(const utils::Point32f &p){
      add_intern(p.x,p.y);
    }

    /// adds new points
    void HoughLineDetector::add(const std::vector<utils::Point32f> &ps){
      for(unsigned int i=0;i<ps.size();++i) add(ps[i]);
    }

    /// adds a new point

    /// adds all non zero pixels of the given binary image
	//void HoughLineDetector::add(const core::Img8u &binaryImage){
	//	const Channel8u c = binaryImage[0];
//		for (int y = 0; y < c.getHeight(); ++y){
//			for (int x = 0; x < c.getWidth(); ++x){
//				if (c(x, y)) add(Point(x, y));
//			}
//		}
//	}

    /// returns current hough-table image
    const core::Img32s &HoughLineDetector::getImage() const { return m_data->image; }

    /// returns current gaussian inhibition map
    const core::Img32f &HoughLineDetector::getInhibitionMap() const { return m_data->inhibitImage; }

    /// internal utility function
    float HoughLineDetector::r(float rho, float x, float y) const{
      return x*cos(rho) + y*sin(rho);
    }
    /// internal utility function
    int HoughLineDetector::getX(float rho) const {
      return round(rho * m_data->mrho);
    }
    /// internal utility function
    int HoughLineDetector::getY(float r) const {
      return round(m_data->mr * r + m_data->br);
    }
    /// internal utility function
    float HoughLineDetector::getRho(int x) const{
      return x/m_data->mrho;
    }
    /// internal utility function
    float HoughLineDetector::getR(int y) const{
      return (y-m_data->br)/m_data->mr;
    }
    /// internal utility function
    void HoughLineDetector::incLut(float rho, float r){
      int x = getX(rho);
      int y = getY(r);
      if(y >= 0 && y < m_data->h){
        m_data->lut(x,y)++;
      }
    }
    /// internal utility function
    void HoughLineDetector::incLutBlurred(float rho, float r){
      int x = getX(rho);
      int y = getY(r);
      if(y >= 0 && y < m_data->h){
        if(y>0) m_data->lut(x,y-1)++;
        m_data->lut(x,y)+=2;
        if(y<m_data->w-1) m_data->lut(x,y+1)++;
      }
    }
    /// internal utility function
    int &HoughLineDetector::cyclicLUT(int x, int y){
      static int _null = 0;
      if(y<0||y>=m_data->h) return _null;
      int dx = x<0 ? m_data->w : x>=m_data->w ? -m_data->w : 0;
      if(!m_data->image.getImageRect().contains(x+dx,y)){
        ERROR_LOG("tryed to access " << x+dx << "," << y);
        return _null;
      }
      return m_data->lut(x+dx,y);
    }


  } // namespace cv
}
