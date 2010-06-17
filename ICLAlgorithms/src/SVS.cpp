/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/src/SVS.cpp                              **
** Module : ICLAlgorithms                                          **
** Authors: Christof Elbrechter, Andre Justus                      **
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

#include <ICLAlgorithms/SVS.h>
#include <ICLUtils/Time.h>

namespace icl {
  SVS::SVS(){
    m_pSvsI = new svsStoredImages();
    m_pSi = new svsStereoImage();
    m_pSvsP = new svsStereoProcess();
    m_oSize=Size(0,0);
  }

  void SVS::printvars(){
    printf ("corrsize - corr window size pixels:%d\n",m_pSi->dp.corrsize);
    printf ("thresh - confidence threshold:%d\n",m_pSi->dp.thresh);
    printf ("unique - uniqueness threshold:%d\n",m_pSi->dp.unique);
    printf ("ndisp - number of pixel:%d\n",m_pSi->dp.ndisp);
    printf ("dpp - disparities per pixel:%d\n",m_pSi->dp.dpp);
    printf ("offx - Horopter offset:%d\n\n",m_pSi->dp.offx);

    printf ("ix - Subimage start column:%d\n",m_pSi->ip.ix);
    printf ("iy - Subimage start row:%d\n",m_pSi->ip.iy);
    printf ("width - Subimage width:%d\n",m_pSi->ip.width);
    printf ("height - Subimage height:%d\n",m_pSi->ip.height);
    printf ("vergence - Subimage vergence between images column:%f\n",m_pSi->ip.vergence);
    printf ("MMX: %d\n",svsHasMMX); // 1 for MMX, 3 for SSE, 7 for SSE2

  }
  void SVS::setParam(svsparam p, int value){
    switch(p){
      case corrsize:
        m_pSi->dp.corrsize=value;
        break;
      case confidence:
        m_pSi->dp.thresh=value;
        break;
      case unique:
        m_pSi->dp.unique=value;
        break;
      case ndisp:
        m_pSi->dp.ndisp=value;
        break;
      case dpp:
        m_pSi->dp.dpp=value;
        break;
      case offx:
        m_pSi->dp.offx=value;
        break;

      case ix:
        m_pSi->ip.ix=value;
        break;
      case iy:
        m_pSi->ip.iy=value;
        break;
      case width:
        m_pSi->ip.width=value;
        break;
      case height:
        m_pSi->ip.height=value;
        break;
    }
  }
  int SVS::getParam(svsparam p){
    switch(p){
      case corrsize:
        return m_pSi->dp.corrsize;
      case confidence:
        return m_pSi->dp.thresh;
      case unique:
        return m_pSi->dp.unique;
      case ndisp:
        return m_pSi->dp.ndisp;
      case dpp:
        return m_pSi->dp.dpp;
      case offx:
        return m_pSi->dp.offx;

      case ix:
        return m_pSi->ip.ix;
      case iy:
        return m_pSi->ip.iy;
      case width:
        return m_pSi->ip.width;
      case height:
        return m_pSi->ip.height;

    };
    return 0;
  }

  void SVS::loadCalibration(const char *filename){
    //char fn[255];
    //sprintf(fn,"%s",filename);
    m_pSi->ReadParams(const_cast<char*>(filename));
  }

  void SVS::load(const Img8u* lim,const Img8u* rim){
    ICLASSERT_RETURN(lim && rim);
    ICLASSERT_RETURN(lim->getSize().width == rim->getSize().width);
    ICLASSERT_RETURN(lim->getSize().height == rim->getSize().height);
    setParam(width,lim->getSize().width);
    setParam(height,lim->getSize().height);
    m_pSvsI->Load(lim->getSize().width,lim->getSize().height,(const_cast<Img8u*>(lim))->getROIData(0),(const_cast<Img8u*>(rim))->getROIData(0));
    m_pSvsI->SetRect(false);
    m_pSi = m_pSvsI->GetImage(Time::now().toMilliSeconds());
    if (m_oSize.width==0 || m_oSize.width!=lim->getSize().width || m_oSize.height!=lim->getSize().height || m_eFmt!=lim->getFormat()){
      //ICL_DELETE(m_pDi);
      m_eFmt=lim->getFormat();
      m_oSize=lim->getSize();
      m_pDi = new Img16s(m_oSize,m_eFmt);
    }
  }


  void SVS::load(const Img8u* lim,const Img8u* rim,Point offset){
    ICLASSERT_RETURN(lim && rim);
    ICLASSERT_RETURN(lim->getSize().width == rim->getSize().width);
    ICLASSERT_RETURN(lim->getSize().height == rim->getSize().height);
    load(lim,rim);
    //set correct SVS - Parameters for using subimages (neccessary to stay calibrated)
		setParam(ix,offset.x);
    setParam(iy,offset.y);
  }

  void SVS::loadCut(const Img8u* lim,const Img8u* rim,Point offset,Size iDim){

    Img8u *limNew = new Img8u(iDim,lim->getFormat());
    Img8u *rimNew = new Img8u(iDim,rim->getFormat());
    deepCopyChannelROI(lim,0,Point(offset.x,offset.y),iDim,limNew,0,Point(0,0),iDim);
    deepCopyChannelROI(rim,0,Point(offset.x,offset.y),iDim,rimNew,0,Point(0,0),iDim);
    load(limNew,rimNew);
    //set correct SVS - Parameters for using subimages (neccessary to stay calibrated)
    setParam(ix,offset.x);
    setParam(iy,offset.y);
    setParam(width,iDim.width);
    setParam(height,iDim.height);
  } 


  void SVS::doStereo(){
//      m_pSi->haveDisparity=false; //only enable this for time measurement
      m_pSvsP->CalcStereo(m_pSi);
  }
  Img16s* SVS::getDisparity(){
    memcpy(m_pDi->getROIData (0),m_pSi->Disparity(),m_oSize.width*m_oSize.height*sizeof(icl16s));
    return m_pDi;
  }
  Img16s* SVS::getConfidence(){
    memcpy(m_pDi->getROIData (0),m_pSi->Confidence(),m_oSize.width*m_oSize.height*sizeof(icl16s));
    return m_pDi;
  }
} //namespace
