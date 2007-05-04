/*
  SVS.cpp

  Written by: Andre Justus (2007)
              University of Bielefeld
              AG Neuroinformatik
              ajustus@techfak.uni-bielefeld.de
*/


#include "iclSVS.h"
//#include "/vol/vision/SVS/4.2/src/svsclass.h"

namespace icl {
  SVS::SVS(){
    m_svsI = new svsStoredImages();
    m_si = new svsStereoImage();
    m_svsP = new svsStereoProcess();
    m_size=Size(0,0);
  }

  void SVS::printvars(){
    printf ("corrsize - corr window size pixels:%d\n",m_si->dp.corrsize);
    printf ("thresh - confidence threshold:%d\n",m_si->dp.thresh);
    printf ("unique - uniqueness threshold:%d\n",m_si->dp.unique);
    printf ("ndisp - number of pixel:%d\n",m_si->dp.ndisp);
    printf ("dpp - disparities per pixel:%d\n",m_si->dp.dpp);
    printf ("offx - Horopter offset:%d\n\n",m_si->dp.offx);

    printf ("ix - Subimage start column:%d\n",m_si->ip.ix);
    printf ("iy - Subimage start row:%d\n",m_si->ip.iy);
    printf ("width - Subimage width:%d\n",m_si->ip.width);
    printf ("height - Subimage height:%d\n",m_si->ip.height);
    printf ("vergence - Subimage vergence between images column:%f\n",m_si->ip.vergence);
    printf ("MMX: %d\n",svsHasMMX); // 1 for MMX, 3 for SSE, 7 for SSE2

  }
  void SVS::setParam(svsparam p, int value){
    switch(p){
      case corrsize:
        m_si->dp.corrsize=value;
        break;
      case confidence:
        m_si->dp.thresh=value;
        break;
      case unique:
        m_si->dp.unique=value;
        break;
      case ndisp:
        m_si->dp.ndisp=value;
        break;
      case dpp:
        m_si->dp.dpp=value;
        break;
      case offx:
        m_si->dp.offx=value;
        break;

      case ix:
        m_si->ip.ix=value;
        break;
      case iy:
        m_si->ip.iy=value;
        break;
      case width:
        m_si->ip.width=value;
        break;
      case height:
        m_si->ip.height=value;
        break;
    }
  }
  int SVS::getParam(svsparam p){
    switch(p){
      case corrsize:
        return m_si->dp.corrsize;
      case confidence:
        return m_si->dp.thresh;
      case unique:
        return m_si->dp.unique;
      case ndisp:
        return m_si->dp.ndisp;
      case dpp:
        return m_si->dp.dpp;
      case offx:
        return m_si->dp.offx;

      case ix:
        return m_si->ip.ix;
      case iy:
        return m_si->ip.iy;
      case width:
        return m_si->ip.width;
      case height:
        return m_si->ip.height;

    };
    return 0;
  }

  void SVS::load_calibration(char *filename){
    m_si->ReadParams(filename);  
  }

  void SVS::Load(const Img8u* lim,const Img8u* rim){
    ICLASSERT_RETURN(lim && rim);
    ICLASSERT_RETURN(lim->getSize().width == rim->getSize().width);
    ICLASSERT_RETURN(lim->getSize().height == rim->getSize().height);
    setParam(width,lim->getSize().width);
    setParam(height,lim->getSize().height);
    m_svsI->Load(lim->getSize().width,lim->getSize().height,(const_cast<Img8u*>(lim))->getROIData(0),(const_cast<Img8u*>(rim))->getROIData(0));
    m_svsI->SetRect(false);
    m_si = m_svsI->GetImage(500);
    if (m_size.width==0 || m_size.width!=lim->getSize().width || m_size.height!=lim->getSize().height || m_fmt!=lim->getFormat()){
      m_fmt=lim->getFormat();
      m_size=lim->getSize();
      m_di = new Img16s(m_size,m_fmt);
    }
  }


  void SVS::Load(const Img8u* lim,const Img8u* rim,Point offset){
    ICLASSERT_RETURN(lim && rim);
    ICLASSERT_RETURN(lim->getSize().width == rim->getSize().width);
    ICLASSERT_RETURN(lim->getSize().height == rim->getSize().height);
    Load(lim,rim);
    //set correct SVS - Parameters for using subimages (neccessary to stay calibrated)
		setParam(ix,offset.x);
    setParam(iy,offset.y);
  }

  void SVS::Load_cut(const Img8u* lim,const Img8u* rim,Point offset,Size iDim){

    Img8u *limNew = new Img8u(iDim,lim->getFormat());
    Img8u *rimNew = new Img8u(iDim,rim->getFormat());
    deepCopyChannelROI(lim,0,Point(offset.x,offset.y),iDim,limNew,0,Point(0,0),iDim);
    deepCopyChannelROI(rim,0,Point(offset.x,offset.y),iDim,rimNew,0,Point(0,0),iDim);
    Load(limNew,rimNew);
    //set correct SVS - Parameters for using subimages (neccessary to stay calibrated)
    setParam(ix,offset.x);
    setParam(iy,offset.y);
    setParam(width,iDim.width);
    setParam(height,iDim.height);
  } 


  void SVS::do_stereo(){
//      m_si->haveDisparity=false; //only enable this for time measurement
      m_svsP->CalcStereo(m_si);
  }
  Img16s* SVS::get_disparity(){
    memcpy(m_di->getROIData (0),m_si->Disparity(),m_size.width*m_size.height*sizeof(icl16s));
    return m_di;
  }
  Img16s* SVS::get_confidence(){
    memcpy(m_di->getROIData (0),m_si->Confidence(),m_size.width*m_size.height*sizeof(icl16s));
    return m_di;
  }
} //namespace
