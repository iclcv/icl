/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/QuadDetector.cpp             **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/QuadDetector.h>

#include <ICLCV/RegionDetector.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLCV/CornerDetectorCSS.h>
#include <ICLMath/StraightLine2D.h>

#include <ICLIO/FileWriter.h>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;
using namespace icl::cv;

namespace icl{
  namespace markers{
  
    static void optimize_edges(std::vector<Point32f> &e4, const std::vector<Point> &boundary) throw (int);
  
    class QuadDetector::Data{
      public:
      std::vector<TiltedQuad> quads;
  
      SmartPtr<RegionDetector> rd;
      CornerDetectorCSS css;
      
      SmartPtr<LocalThresholdOp> lt;
      SmartPtr<UnaryOp> pp;
      
      bool dynamicQuadColor;
      std::string lastPPType;
      Size lastPPSize;
      
      ImgBase *lastBinImage;
    };
  
    static const int RD_VALS[6]={0,255,0,0,255,255};  
    
    QuadDetector::QuadDetector(QuadDetector::QuadColor c, bool dynamic):data(new Data){
      data->dynamicQuadColor = dynamic;
      
      addProperty("pp.filter","menu","none,median,erosion,dilatation,opening,closing","none",0,"Post processing filter.");
      addProperty("pp.mask size","menu","3x3,5x5","3x3", 0,"Mask size for post processing.");
  
      if(dynamic){
        addProperty("quad value","menu",str(BlackOnly) + "," + str(WhiteOnly) + "," + str(BlackAndWhite),str(c), 0,
                    "Defines whether the marker borders are black, white or mixed");
      }
      addProperty("optimize edges","flag","","true", 0, "Flag for optimized marker corner detection");
      
      data->rd = new RegionDetector(0,2<<20,RD_VALS[(int)c],RD_VALS[(int)c + 3]);
      data->rd->deactivateProperty("minimum value");
      data->rd->deactivateProperty("maximum value");
      data->rd->deactivateProperty("^CSS*");
      
      data->lt = new LocalThresholdOp;
      data->lt->deactivateProperty("gamma slope");
      data->lt->deactivateProperty("^UnaryOp*");
      
      addChildConfigurable(data->rd.get(),"quads");
      addChildConfigurable(data->lt.get(),"thresh");
  
      data->css.deactivateProperty("debug-mode");
      addChildConfigurable(&data->css,"css");
      addProperty("css.dynamic sigma","flag","",true,0,
                  "If set to true, the border-smoothing sigma is adapted\n"
                  "relatively to the actual marker size (usually provides\n"
                  "much better results)");
  
      data->css.setSigma(4.2);
      data->css.setCurvatureCutoff(66);
      data->lastBinImage = 0;
  
      // set some default values ...
      setPropertyValue("css.angle-threshold",180);
      setPropertyValue("css.curvature-cutoff",66);
      setPropertyValue("css.rc-coefficient",1);
      
      setPropertyValue("thresh.global threshold",-10);
      setPropertyValue("thresh.mask size",30);
    }
    
    QuadDetector::~QuadDetector(){
      delete data;
    }
  
  
    const std::vector<TiltedQuad> &QuadDetector::detect(const ImgBase *image){
      ICLASSERT_THROW(image,ICLException("QuadDetector::detect: input image was NULL"));
      if(data->dynamicQuadColor){
        QuadColor c = getPropertyValue("quad value");
        if(c >= 0 && c <= 3){
          data->rd->setConstraints(getPropertyValue("region detector.minimum size"),
                                   getPropertyValue("region detector.maximum size"),
                                   RD_VALS[(int)c],RD_VALS[(int)c + 3]);
        }else{
          ERROR_LOG("unable to adapt the region detectors min. and max. value property "
                    "because the 'quad value' property value is undefined");
        }
      }
  
      std::string kernelType = getPropertyValue("pp.filter");    
      Size kernelSize = getPropertyValue("pp.mask size");
  
      if(data->lastPPType != kernelType || data->lastPPSize != kernelSize){
        data->lastPPType = kernelType;
        data->lastPPSize = kernelSize;
  
        bool is3x3 = (kernelSize == Size(3,3)); 
        
        //"menu","none,median,erosion,dilatation,opening,closing"
  
        if(kernelType == "median"){
          data->pp = new MedianOp(kernelSize);
        }else if(kernelType == "none"){
          data->pp.setNull();
        }else if(kernelType == "erosion"){
          data->pp = new MorphologicalOp(is3x3 ? MorphologicalOp::erode3x3 : MorphologicalOp::erode,kernelSize);
        }else if(kernelType == "dilatation"){
          data->pp = new MorphologicalOp(is3x3 ? MorphologicalOp::dilate3x3 : MorphologicalOp::dilate,kernelSize);
        }else if(kernelType == "opening"){
          data->pp = new MorphologicalOp(MorphologicalOp::openBorder,kernelSize);
        }else if(kernelType == "closing"){
          data->pp = new MorphologicalOp(MorphologicalOp::closeBorder,kernelSize);
        }
        if(data->pp){
          data->pp->setClipToROI(false);
        }
      }
      
      if(data->pp){
        data->pp->apply(data->lt->apply(image),&data->lastBinImage);
      }else{
        data->lt->apply(image,&data->lastBinImage);
      }
      
      data->quads.clear();    
      
      const std::vector<ImageRegion> &rs = data->rd->detect(data->lastBinImage);

      const bool dynCSSSigma = getPropertyValue("css.dynamic sigma");
      const bool optEdges = getPropertyValue("optimize edges");
      for(unsigned int i=0;i<rs.size();++i){
        const std::vector<Point> &boundary = rs[i].getBoundary();
        
        if(dynCSSSigma){
          data->css.setSigma(iclMin(7.,boundary.size() * (3.2/60) - 0.5));
        }
        const std::vector<Point32f> &corners = data->css.detectCorners(boundary);
        
        if( corners.size() == 4 ){ 

          if(optEdges){
            std::vector<Point32f> cornersCopy = corners;
            try{
              optimize_edges(cornersCopy,boundary); // todo optimize !!
              data->quads.push_back(TiltedQuad(cornersCopy.data(),rs[i]));
            }catch(int code){ (void)code; }
          }else{
            data->quads.push_back(TiltedQuad(corners.data(),rs[i]));
          }
        }
      }
      return data->quads;
    }
  
    const Img8u &QuadDetector::getLastBinaryImage() const{
      return *data->lastBinImage->asImg<icl8u>();
    }
  
    std::ostream &operator<<(std::ostream &s, const QuadDetector::QuadColor &c){
      switch(c){
        case QuadDetector::WhiteOnly: return s << "white only";
        case QuadDetector::BlackOnly: return s << "black only";
        case QuadDetector::BlackAndWhite: return s << "black and white";
        default:
          return s << "undefined";
      }
    }
  
    std::istream &operator>>(std::istream &s, QuadDetector::QuadColor &qc){
      std::string a,b,c;
      s >> a >> b;
      if(a == "white" && b == "only") { qc = QuadDetector::WhiteOnly; return s; }
      if(a == "black" && b == "only") { qc = QuadDetector::BlackOnly; return s; }
      else s >> c;
      if(a == "black" && b == "and" && c == "white") { qc = QuadDetector::BlackAndWhite; return s; }
      qc = (QuadDetector::QuadColor)-1;
      return s;
    }
  
  
  #if 0
    static StraightLine2D approx_line(const std::vector<Point32f> &ps) throw (int){
      int nPts = ps.size();
      if(nPts < 2) throw 2;
      float avgX = 0;
      float avgY = 0;
      for(unsigned int i=0;i<ps.size();++i){
        avgX += ps[i].x;
        avgY += ps[i].y;
      }
      avgX /= nPts;
      avgY /= nPts;
      float avgXX(0),avgXY(0),avgYY(0);
      
      for(unsigned int i=0;i<ps.size();++i){
        avgXX += ps[i].x * ps[i].x;
        avgXY += ps[i].x * ps[i].y;
        avgYY += ps[i].y * ps[i].y;
      }
      avgXX /= nPts;
      avgXY /= nPts;
      avgYY /= nPts;
      
      double fSxx = avgXX - avgX*avgX;
      double fSyy = avgYY - avgY*avgY;
      double fSxy = avgXY - avgX*avgY;
      
      double fP = 0.5*(fSxx+fSyy);
      double fD = 0.5*(fSxx-fSyy);
      fD = ::sqrt(fD*fD + fSxy*fSxy);
      double fA  = fP + fD;
      
      float angle = ::atan2(fA-fSxx,fSxy);  
      return StraightLine2D(Point32f(avgX,avgY),Point32f(cos(angle),sin(angle)));
    }
  #endif
  
    static StraightLine2D fit_line(float x, float y, float xx, float xy, float yy) throw (int){
      float sxx = xx - x*x;
      float syy = yy - y*y;
      float sxy = xy - x*y;
      float p = 0.5*(sxx+syy);
      float d = 0.5*(sxx-syy);
      float sdd = ::sqrt(d*d + sxy*sxy);
      float a  = p+sdd;
      float angle = ::atan2(a-sxx,sxy);  
      return StraightLine2D(Point32f(x,y),Point32f(cos(angle),sin(angle)));
    }
  
    
    static float line_sprod(const StraightLine2D &a, const StraightLine2D &b){
      return fabs(a.v[0] * b.v[0] + a.v[1] * b.v[1]);
    }
    
    void optimize_edges(std::vector<Point32f> &e4, const std::vector<Point> &boundary) throw (int){
      int num = boundary.size();
      int i0 = (int)(std::find(boundary.begin(),boundary.end(),Point(e4[0]))-boundary.begin());
      ICLASSERT_THROW(i0 < num,ICLException("edge point was not found in the boundary"));
      std::vector<Point> b2(num);
      std::copy(boundary.begin()+i0,boundary.end(),b2.begin());
      std::copy(boundary.begin(),boundary.begin()+i0,b2.begin()+(num-i0));
      
      int i = 0;
      Point next = e4[1];
      Point32f means[4];
      int ns[4]={0};
      float xx[4]={0}, xy[4]={0}, yy[4]={0};
      
      for(;i<num && b2[i] != next;++i){
        int x = b2[i].x, y = b2[i].y;
        means[0].x += x;
        means[0].y += y;
        xx[0] += x*x;
        xy[0] += x*y;
        yy[0] += y*y;
        ++ns[0];
      }
      next = e4[2];
      for(;i<num && b2[i] != next;++i){
        int x = b2[i].x, y = b2[i].y;
        means[1].x += x;
        means[1].y += y;
        xx[1] += x*x;
        xy[1] += x*y;
        yy[1] += y*y;
        ++ns[1];
      }
      next = e4[3];
      for(;i<num && b2[i] != next;++i){
        int x = b2[i].x, y = b2[i].y;
        means[2].x += x;
        means[2].y += y;
        xx[2] += x*x;
        xy[2] += x*y;
        yy[2] += y*y;
        ++ns[2];
      }
      for(; i < num; ++i){
        int x = b2[i].x, y = b2[i].y;
        means[3].x += x;
        means[3].y += y;
        xx[3] += x*x;
        xy[3] += x*y;
        yy[3] += y*y;
        ++ns[3];
      }
      for(int i=0;i<4; ++i){
        float fac = 1.0/ns[i];
        means[i].x *= fac;
        means[i].y *= fac;
        xx[i] *= fac;
        xy[i] *= fac;
        yy[i] *= fac;
      }
      StraightLine2D a = fit_line(means[0].x,means[0].y,xx[0],xy[0],yy[0]);
      StraightLine2D b = fit_line(means[1].x,means[1].y,xx[1],xy[1],yy[1]);
      StraightLine2D c = fit_line(means[2].x,means[2].y,xx[2],xy[2],yy[2]);
      StraightLine2D d = fit_line(means[3].x,means[3].y,xx[3],xy[3],yy[3]);
  
      if(line_sprod(a,c) < 0.90 || 
         line_sprod(d,b) < 0.90 ) throw 1;
      
      try{  
        StraightLine2D::Pos intersections[] = {
          a.intersect(d), a.intersect(b),
          c.intersect(b), c.intersect(d)
        };
        
        for(int i=0;i<4;++i){
          e4[i].x = intersections[i][0];
          e4[i].y = intersections[i][1];
        }
      }catch(...){ throw 3;}
    }
  
    REGISTER_CONFIGURABLE_DEFAULT(QuadDetector);
  } // namespace markers
}
