/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/src/OpenSurfDetector.cpp                 **
** Module : ICLAlgorithms                                          **
** Authors: Christian Groszewski                                   **
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

#include <ICLAlgorithms/OpenSurfDetector.h>

using namespace std;

namespace icl{

  struct OpenSurfDetector::Data{
    //copy of the original object image
    IplImage *m_objectImg_org;
    //working copy of the original object image
    IplImage *m_objectImg;
    //temporary buffer
    IplImage *m_tmp_ipl_img;
    //surf for the object image
    IpVec m_ojb_ipts;
    //
    IpVec m_ipts;
    //matches
    IpPairVec m_matches;
    //default value  = false;
    bool m_upright;
    //default value  = 4;
    int m_octaves;
    //default value  = 4;
    int m_intervals;
    //default value  = 2;
    int m_init_samples;
    //default value = 0.004f;
    float m_thresh;
  
    Data(bool upright=false, int octaves=4, int intervals=4, int init_samples=2, float thresh=0.004f):
      m_objectImg_org(0),m_objectImg(0),m_tmp_ipl_img(0),m_upright(upright),m_octaves(octaves),
      m_intervals(intervals),m_init_samples(init_samples),m_thresh(thresh){}
    ~Data(){
      if(m_objectImg)
        cvReleaseImage(&m_objectImg);
      if(m_objectImg_org)
        cvReleaseImage(&m_objectImg_org);
      if(m_tmp_ipl_img)
        cvReleaseImage(&m_tmp_ipl_img);
    }
  };


  OpenSurfDetector::OpenSurfDetector(const ImgBase *obj,bool upright, int octaves,
                                     int intervals, int init_samples, float thresh):
    m_data(new Data(upright, octaves, intervals, init_samples, thresh)){
    if(obj){
      setObjectImg(obj);
    }
  }

  OpenSurfDetector::~OpenSurfDetector(){
    delete m_data;
  }

  void OpenSurfDetector::setObjectImg(const ImgBase *objectImage) throw (ICLException){
    if(!objectImage)
      throw ICLException("object image is null");
    // DON'T release all buffers (we can use them again if possible)
    /*
        if(m_data->m_objectImg){
        cvReleaseImage(&(m_data->m_objectImg));
        }
        if(m_data->m_objectImg_org){
        cvReleaseImage(&(m_data->m_objectImg_org));
        }
        if(m_data->m_tmp_ipl_img){
        cvReleaseImage(&(m_data->m_tmp_ipl_img));
        }
    */
  
    icl::img_to_ipl(objectImage,&(m_data->m_objectImg_org));
    icl::img_to_ipl(objectImage,&(m_data->m_objectImg));
  
    SHOW(m_data->m_objectImg_org);
    SHOW(m_data->m_objectImg);
    /*
        m_data->m_objectImg = cvCreateImage(cvGetSize(m_data->m_objectImg_org),
        m_data->m_objectImg_org->depth ,
        m_data->m_objectImg_org->nChannels);
        cvCopy(m_data->m_objectImg_org, m_data->m_objectImg);
        */
    computeSURF();
  }

  void OpenSurfDetector::computeSURF(){
    // Extract surf points
    if((m_data->m_ojb_ipts).size()>0){
      (m_data->m_ojb_ipts).clear();
    }
    surfDetDes(m_data->m_objectImg, m_data->m_ojb_ipts, m_data->m_upright,
               m_data->m_octaves, m_data->m_intervals,
               m_data->m_init_samples, m_data->m_thresh);
  }

  const ImgBase *OpenSurfDetector::getObjectImg() throw (ICLException){
    if(m_data->m_objectImg_org)
      return icl::ipl_to_img(m_data->m_objectImg_org);
    else
      throw ICLException("Object image is null");

  }

  void OpenSurfDetector::setRotationInvariant(bool upright){
    if(m_data->m_upright != upright){
      m_data->m_upright = upright;
      computeSURF();
    }
  }

  void OpenSurfDetector::setOctaves(int octaves){
    if(m_data->m_octaves != octaves){
      m_data->m_octaves = octaves;
      computeSURF();
    }
  }

  void OpenSurfDetector::setIntervals(int intervals){
    if(m_data->m_intervals != intervals){
      m_data->m_intervals = intervals;
      computeSURF();
    }
  }

  void OpenSurfDetector::setInitSamples(int init_samples){
    if(m_data->m_init_samples != init_samples){
      m_data->m_init_samples = init_samples;
      computeSURF();
    }
  }

  void OpenSurfDetector::setRespThresh(float thresh){
    if(m_data->m_thresh != thresh){
      m_data->m_thresh = thresh;
      computeSURF();
    }
  }

  bool OpenSurfDetector::getRotationInvariant(){
    return m_data->m_upright;
  }

  int OpenSurfDetector::getOctaves(){
    return m_data->m_octaves;
  }

  int OpenSurfDetector::getIntervals(){
    return m_data->m_intervals;
  }

  int OpenSurfDetector::getInitSamples(){
    return m_data->m_init_samples;
  }

  float OpenSurfDetector::getRespThresh(){
    return m_data->m_thresh;
  }

  void OpenSurfDetector::setParams(bool upright, int octaves, int intervals, int init_samplex,float thresh){
    bool changed = false;
    if(m_data->m_upright != upright){
      m_data->m_upright = upright;
      changed = true;
    }
    if(m_data->m_octaves != octaves){
      m_data->m_octaves = octaves;
      changed = true;
    }
    if(m_data->m_intervals != intervals){
      m_data->m_intervals = intervals;
      changed = true;
    }
    if(m_data->m_init_samples != init_samplex){
      m_data->m_init_samples = init_samplex;
      changed = true;
    }
    if(m_data->m_thresh != thresh){
      m_data->m_thresh = thresh;
      changed = true;
    }
    if(changed)
      computeSURF();
  }

  const std::vector<Ipoint> &OpenSurfDetector::extractFeatures(const ImgBase *src){
    ICLASSERT_THROW(src,ICLException("OpenSurfDetector::extractFeatures: source image was null"));
    
    icl::img_to_ipl(src,&(m_data->m_tmp_ipl_img));
    m_data->m_ipts.clear();
    
    surfDetDes(m_data->m_tmp_ipl_img, m_data->m_ipts, m_data->m_upright,
               m_data->m_octaves, m_data->m_intervals,
               m_data->m_init_samples, m_data->m_thresh);
     
    // we do not release the image here, so we can use it again in the future
    //cvReleaseImage(&(m_data->m_tmp_ipl_img));
    return m_data->m_ipts;
  }

  const std::vector<std::pair<Ipoint, Ipoint> > &OpenSurfDetector::match(const ImgBase *image) throw (ICLException){
    if(!image)
      throw ICLException("image is null");
    if(!m_data->m_objectImg)
      throw ICLException("object image is null");
    if(m_data->m_objectImg_org){
      (m_data->m_ipts).clear();
      extractFeatures(image);
      getMatches(m_data->m_ipts,m_data->m_ojb_ipts,m_data->m_matches);
    }
    return m_data->m_matches;
  }

  const std::vector<Ipoint> &OpenSurfDetector::getObjectImgFeatures() {
    return m_data->m_ojb_ipts;
  }

#ifdef HAVE_QT
  void OpenSurfDetector::visualizeFeature(ICLDrawWidget &w,const Ipoint &p){
    float s = (2.5f * p.scale);
    int r1 = fRound(p.y);
    int c1 = fRound(p.x);

    int alpha = 200;
    w.nofill();
    w.color(0,255,0,alpha);

    if (p.orientation){ // Green line indicates orientation
      w.linewidth(2);
      int c2 = fRound(s * cos(p.orientation)) + c1;
      int r2 = fRound(s * sin(p.orientation)) + r1;
      w.line(c1, r1, c2, r2);
    }else{  // Green dot if using upright version
      w.point(c1,r1);
    }
    if (p.laplacian == 1){ // Blue circles indicate dark blobs on light backgrounds
      w.color(0,0,255,alpha);
      w.circle(c1,r1,fRound(s));
    }else if (p.laplacian == 0){ // Red circles indicate light blobs on dark backgrounds
      w.color(255,0,0,alpha);
      w.circle(c1,r1,fRound(s));
    }else if (p.laplacian == 9){ // Red circles indicate light blobs on dark backgrounds
      w.color(0,255,0,alpha);
      w.circle(c1,r1,fRound(s));
    }

    int tailSize = 0;
    //not used until now
    // Draw motion from ipoint dx and dy
    if (tailSize){
      w.color(255,255,255,alpha);
      w.line(c1, r1, int(c1+p.dx*tailSize), int(r1+p.dy*tailSize));
    }
  }

  void OpenSurfDetector::visualizeFeatures(ICLDrawWidget &w, const std::vector<Ipoint> &features){
    for(unsigned int i=0;i<features.size();++i)
      visualizeFeature(w,features.at(i));
  }

  void OpenSurfDetector::visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result, 
                                          std::vector<std::pair<Ipoint, Ipoint> > &matches){
    for(unsigned int i=0;i<matches.size();++i){
      visualizeFeature(w_object, matches[i].second);
      visualizeFeature(w_result,matches[i].first);
    }
  }
#endif
}
