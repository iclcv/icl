/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/ORBFeatureDetector.cpp                 **
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

#include <ICLCV/ORBFeatureDetector.h>
#include <ICLCore/OpenCV.h>

#include <ICLFilter/LocalThresholdOp.h>

#include <opencv2/features2d.hpp>

namespace ocv = ::cv;

namespace icl{
  using namespace core;
  using namespace utils;
  using namespace math;
  using namespace filter;

  namespace cv{

    struct ORBFeatureDetector::Data{
      SmartPtr<ocv::ORB> orb;
      ocv::Ptr<ocv::ORB> orbp;

      MatWrapper inputBuffer;
      ocv::Ptr<ocv::DescriptorMatcher> matcher;

      LocalThresholdOp lt;
      Img8u ltBuffer;
      const ImgBase *lastInputImage;
      Img8u grayInputBuffer;

      struct ParamSet{
        ParamSet() : scoreType(-1){}
        ParamSet(Configurable &c){
          scoreType = c.getPropertyValue("score type").as<int>();
          maxFeatures = c.getPropertyValue("max features").as<int>();
          patchSize = c.getPropertyValue("patch size").as<int>();
          WTA_K = c.getPropertyValue("WTA_K").as<int>();
          pyLevels = c.getPropertyValue("pyramid.levels").as<int>();
          pyScale = c.getPropertyValue("pyramid.scale factor").as<float>();
          pyLevel0 = c.getPropertyValue("pyramid.first level").as<int>();
        }
        int scoreType;
        int maxFeatures;
        int patchSize;
        int WTA_K;
        int pyLevels;
        float pyScale;
        int pyLevel0;
        bool operator !=(const ParamSet &other) const{
          return (scoreType != other.scoreType ||
                  maxFeatures != other.maxFeatures ||
                  patchSize != other.patchSize ||
                  WTA_K != other.WTA_K ||
                  pyLevels != other.pyLevels ||
                  pyScale != other.pyScale ||
                  pyLevel0 != other.pyLevel0);

        }
      } params;

      void updateORB(ParamSet p){
        if(p != params){
          if(p.patchSize/pow(p.pyScale,p.pyLevels) < 2){
            p.patchSize = 2*pow(p.pyScale,p.pyLevels);
            WARNING_LOG("patch size was increased to " << p.patchSize << " to ensure proper operation");
          }
          params = p;

          // todo is this the point in development where this was adapted?
#if CV_MAJOR_VERSION >= 3
          int edgeThreshold = 31;
          int fastThreshold = 20;
          orbp = ocv::ORB::create(p.maxFeatures, p.pyScale, p.pyLevels, edgeThreshold, p.pyLevel0, p.WTA_K,
                                  p.scoreType == 0 ? ocv::ORB::HARRIS_SCORE : ocv::ORB::FAST_SCORE,
                                  p.patchSize, fastThreshold);
#else
          orb = new ocv::ORB(p.maxFeatures,
                             p.pyScale,
                             p.pyLevels,
                             p.patchSize, p.pyLevel0, p.WTA_K,
                             ( p.scoreType == 0 ?
                               ocv::ORB::HARRIS_SCORE :
                               ocv::ORB::FAST_SCORE ),
                             p.patchSize);
#endif
        }
      }
    };
    struct ORBFeatureDetector::FeatureSetClass::Impl{
      std::vector<ocv::KeyPoint> keyPoints;
      MatWrapper descriptors;
    };

    ORBFeatureDetector::ORBFeatureDetector() : m_data(new Data){

      m_data->matcher = ocv::DescriptorMatcher::create("BruteForce");

      addProperty("contrast adjustment.on","flag","",false);
      addProperty("contrast adjustment.slope","range","[0.05,20]",1);
      addProperty("contrast adjustment.mask size","range","[3,100]:1",10);
      addProperty("contrast adjustment.threshold","range", "[-50,50]", 0);

      addProperty("score type","menu","fast,harris","harris",0, "Score type o use: harris is slightly slower but more accurate");
      addProperty("max features","range:spinbox","[1,100000]:1","500",0, "Maximum number of features to detect");
      addProperty("patch size","range","[1,1001]:1","31",0,
                  "Minimum patch size that is used compute BRIF discriptors on. Since\n"
                  "the logical patch size is larger in smaller pyramid layers, the\n"
                  "maximum feature size is distinguished by 'patch size', the pyramid\n"
                  "scale factor and the number of pyramid levels. Note that features\n"
                  "will only be detected at positions, where the full patch fits into\n"
                  "the image.");
      addProperty("WTA_K","menu","2,3,4","2",0,"Number of random points used for computing elements of the ORB descriptors");
      addProperty("pyramid.levels","range","[1,100]:1","8",0, "Number of pyramid levels to use for key-point detection");
      addProperty("pyramid.scale factor","range","[1,4]","1.4",0,"Scale down factor between consecutive pyramid layers");
      addProperty("pyramid.first level","menu","0","0",0,"First pyramid level to actually use (non-0 values are not supported yet");

      addProperty("bench.enable","flag","",false,0,"Enable/Disable time benchmarks");
      addProperty("bench.preprocessing time","info","","??? ms",0,"Last time for preprocessing");
      addProperty("bench.ORB extraction time","info","","??? ms",0,"Time for the last time ORB features were detecdted");
      addProperty("bench.detection time","info","","??? ms",0,"Time for the whole last detection cycle");
      addProperty("bench.matching time","info","","??? ms",0,"Last feature matching step");

      m_data->updateORB(Data::ParamSet(*this));

      m_data->ltBuffer = Img8u(Size(1,1),1);
      m_data->grayInputBuffer = Img8u(Size(1,1),formatGray);
    }
    ORBFeatureDetector::~ORBFeatureDetector(){
      delete m_data;
    }

    ORBFeatureDetector::FeatureSetClass::FeatureSetClass(){
      impl = new Impl;
    }

    ORBFeatureDetector::FeatureSetClass::~FeatureSetClass(){
      delete impl;
    }

    const ImgBase * ORBFeatureDetector::getIntermediateImage(const std::string &id){
      if(id == "input") return m_data->lastInputImage;
      if(id == "gray") return &m_data->grayInputBuffer;
      if(id == "contrast enhanced") return &m_data->ltBuffer;
      else return 0;
    }

    utils::VisualizationDescription ORBFeatureDetector::FeatureSetClass::vis() const{
      VisualizationDescription d;
      d.color(255,0,0,255);
      for(size_t i=0;i<impl->keyPoints.size();++i){
        const ocv::KeyPoint &k = impl->keyPoints[i];
        float s = k.size / 2;

        float cx = k.pt.x;
        float cy = k.pt.y;

        d.color(0,255,0,255);

        if(k.angle != -1){
          float angle = k.angle*M_PI/180.0;
          int cx2 = cx + cos(angle) * s;
          int cy2 = cy + sin(angle) * s;
          d.linewidth(2);
          d.line(cx,cy,cx2,cy2);
        }

        d.linewidth(1);
        d.color(0,100,255,255);

        d.fill(255,0,0,0);
        d.circle(cx,cy,s);
      }
      return d;
    }

    static std::string bench_time_string(const Time &t){
      int ms10 = (int)(t.toMilliSecondsDouble()*10);
      return str(float(ms10)/10.0f) + " ms";
    }

    ORBFeatureDetector::FeatureSet ORBFeatureDetector::detect(const core::Img8u &image){
      bool bench = getPropertyValue("bench.enable");

      m_data->updateORB(Data::ParamSet(*this));

      Time t = Time::now();

      m_data->lastInputImage = &image;

      if(image.getFormat() != formatGray){
        cc(&image, &m_data->grayInputBuffer);
      }else{
        m_data->grayInputBuffer = image;
      }

      if(getPropertyValue("contrast adjustment.on")){
        float slope = getPropertyValue("contrast adjustment.slope");
        int maskSize = getPropertyValue("contrast adjustment.mask size");
        float threshold = getPropertyValue("contrast adjustment.threshold");
        m_data->lt.setMaskSize(maskSize);
        m_data->lt.setGammaSlope(slope);
        m_data->lt.setGlobalThreshold(threshold);
        const ImgBase *result = m_data->lt.apply(&m_data->grayInputBuffer);
        result->convert(&m_data->ltBuffer);
        m_data->inputBuffer = m_data->ltBuffer;
      }else{
        m_data->inputBuffer = m_data->grayInputBuffer;
      }

      if(bench){
        setPropertyValue("bench.preprocessing time",bench_time_string(t.age()));
      }

      Time tOrb = Time::now();

      FeatureSetClass *ret = new FeatureSetClass;
#if CV_MAJOR_VERSION >= 3
      m_data->orbp->detectAndCompute(m_data->inputBuffer.mat,
                                     ocv::noArray(),
                                     ret->impl->keyPoints,
                                     ret->impl->descriptors.mat);
#else
      m_data->orb->operator()(m_data->inputBuffer.mat,
                              ocv::noArray(),
                              ret->impl->keyPoints,
                              ret->impl->descriptors.mat);
#endif

      if(bench){
        setPropertyValue("bench.ORB extraction time",bench_time_string(tOrb.age()));
        setPropertyValue("bench.detection time",bench_time_string(t.age()));
      }

      return SmartPtr<FeatureSetClass>(ret);
    }

    std::vector<ORBFeatureDetector::Match>
    ORBFeatureDetector::match(const ORBFeatureDetector::FeatureSet &a,
                              const ORBFeatureDetector::FeatureSet &b){

      bool bench = getPropertyValue("bench.enable");
      Time t = Time::now();

      std::vector<ocv::DMatch> matches;
      m_data->matcher->match(a->impl->descriptors.mat, b->impl->descriptors.mat,
                             matches);

      const std::vector<ocv::KeyPoint> &k1 = a->impl->keyPoints;
      const std::vector<ocv::KeyPoint> &k2 = b->impl->keyPoints;

      std::vector<Match> ret(matches.size());
      for(size_t i=0;i<ret.size();++i){
        const ocv::DMatch &m = matches[i];
        int i1 = matches[i].queryIdx;
        int i2 = matches[i].trainIdx;
        if((unsigned)i1 >= k1.size()){
          ERROR_LOG("invalid key point index k1");
          continue;
        }
        if((unsigned)i2 >= k2.size()){
          ERROR_LOG("invalid key point index k2");
          continue;
        }
        ret[i].a.x = k1[i1].pt.x;
        ret[i].a.y = k1[i1].pt.y;
        ret[i].distance = m.distance;
      }

      if(bench){
        setPropertyValue("bench.matching time",bench_time_string(t.age()));
      }

      return ret;

    }

    REGISTER_CONFIGURABLE_DEFAULT(ORBFeatureDetector);

  }
}


#if 0
void crossCheckMatching( Ptr<DescriptorMatcher>& descriptorMatcher,
                         const Mat& descriptors1, const Mat& descriptors2,
                         vector<DMatch>& filteredMatches12, int knn=1 )
{
    filteredMatches12.clear();
    vector<vector<DMatch> > matches12, matches21;
    descriptorMatcher->knnMatch( descriptors1, descriptors2, matches12, knn );
    descriptorMatcher->knnMatch( descriptors2, descriptors1, matches21, knn );
    for( size_t m = 0; m < matches12.size(); m++ )
    {
        bool findCrossCheck = false;
        for( size_t fk = 0; fk < matches12[m].size(); fk++ )
        {
            DMatch forward = matches12[m][fk];

            for( size_t bk = 0; bk < matches21[forward.trainIdx].size(); bk++ )
            {
                DMatch backward = matches21[forward.trainIdx][bk];
                if( backward.trainIdx == forward.queryIdx )
                {
                    filteredMatches12.push_back(forward);
                    findCrossCheck = true;
                    break;
                }
            }
            if( findCrossCheck ) break;
        }
    }
}
#endif
