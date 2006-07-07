/*
  ICL.cpp

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICL.h"

//---- ICL in its own namespace ----
namespace icl {

// {{{  Konstruktor/ Destruktor: 

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth,int iHeight,int iChannels):
  ICLBase(iWidth,iHeight,iChannels,iclGetDepth<Type>()){
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(int,int,int) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_ppChannels.resize(m_iChannels);
  
  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
    }
  m_eFormat = formatMatrix;
} 

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels):
  ICLBase(iWidth,iHeight,eFormat,iclGetDepth<Type>(),iChannels){
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(int,int,int) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_ppChannels.resize(m_iChannels);
  
  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
} 


//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(int iWidth, int iHeight, iclformat eFormat, int iChannels, Type** pptData):
  ICLBase(iWidth,iHeight,eFormat,iclGetDepth<Type>(),iChannels){
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(int,int,int) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_ppChannels.resize(m_iChannels);
  
  //---- ICL Channel using shared memory ----
  for(int i=0;i<m_iChannels;i++)
    m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight,pptData[i]));
} 
 
//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(const ICL<Type>& tSrc):
  ICLBase(tSrc.getWidth(),tSrc.getHeight(),tSrc.getFormat(),tSrc.getDepth(),tSrc.getChannels())
{

  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(const ICL<Type>&) -> " << this);
  
  //---- Variable initialisation ----
  m_iChannels = tSrc.getChannels();
  m_ppChannels.resize(m_iChannels);
  
  //---- Shallow copy of source channels ----
  std::copy(tSrc.m_ppChannels.begin(), 
            tSrc.m_ppChannels.end(),
            m_ppChannels.begin());
}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::~ICL()
{
  //---- Log Message ----
  DEBUG_LOG4("Destruktor: ICL() -> " << this);
  
  //---- Delete channels ----
  deleteChannels ();
}

// }}} 

// {{{  class operator 

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>& ICL<Type>::operator=(const ICL<Type>& tSrc)
{
  //---- Log Message ----
  DEBUG_LOG4("operator=(const ICL<Type>&");
  
  //---- Free old data ----
  this->deleteChannels();
  
  //---- Assign new channels to ICL ----
  this->m_iWidth = tSrc.getWidth();
  this->m_iHeight = tSrc.getHeight();
  this->m_eFormat = tSrc.getFormat();
  this->m_eDepth = tSrc.getDepth();
  this->m_iChannels = tSrc.getChannels();  
  this->m_ppChannels.resize(this->m_iChannels);
  
  //---- Shallow copy of source channels ----
  std::copy(tSrc.m_ppChannels.begin(), 
            tSrc.m_ppChannels.end(),
            this->m_ppChannels.begin());

  //---- return ----
  return *this;
}

// }}} 

// {{{  class organisation : 

//----------------------------------------------------------------------------
template<class Type>
ICLBase*
ICL<Type>::deepCopy(ICLBase* poDst) const
{
  //---- Log Message ----
  DEBUG_LOG4("deepCopy(ICL<Type>*)"); 

  //---- Allocate memory ----
  if (poDst == NULL) 
  {
    poDst = new ICL<Type> (*this);
    poDst->detach();
    return poDst;
  } 
  else 
  {
    poDst->renew(m_iWidth,m_iHeight,m_iChannels);
    poDst->setFormat(m_eFormat);

    if(poDst->getDepth() == getDepth())
      {
        for(int c=0;c<m_iChannels;c++)
          {
            memcpy(poDst->getDataPtr(c),getDataPtr(c),m_iWidth*m_iHeight*sizeof(Type));
          }
        return poDst;
      }
    else
      {
        if(poDst->getDepth() == depth8u)
          {
            return convertTo8Bit(poDst->asIcl8u());
          }
        else
          {
            return convertTo32Bit(poDst->asIcl32f());
          }
      }
  }
}

//--------------------------------------------------------------------------
template<class Type>
ICLBase*
ICL<Type>::scaledCopy(ICLBase *poDst,iclscalemode eScaleMode) const
{
  //---- Log Message ----
  DEBUG_LOG4("smartCopy(ICL,iclscalemode)");
  
  //---- deep copy case -----
  if(!poDst || isEqual(poDst->getWidth(),poDst->getHeight(),poDst->getChannels())){
    return deepCopy(poDst); 
  }
  
  //---- type conversion case -----------
  if(getDepth() != poDst->getDepth())
    {
      if(poDst->getDepth() == depth8u)
        {
          ICL8u oTmp(poDst->getWidth(),poDst->getHeight(),poDst->getChannels());
          scaledCopy(&oTmp,eScaleMode);
          oTmp.deepCopy(poDst);
        }
      else
        {
          ICL32f oTmp(poDst->getWidth(),poDst->getHeight(),poDst->getChannels());
          scaledCopy(&oTmp,eScaleMode);
          oTmp.deepCopy(poDst);
        }
      return poDst;
    }    
  
  poDst->setNumChannels( getChannels() );
  
#ifdef WITH_IPP_OPTIMIZATION
  for(int c=0;c<m_iChannels;c++)
    {
      if(getDepth()==depth8u)
        {
          ippiResize_8u_C1R(ippData8u(c),ippSize(),ippStep(),ippRoi(),
                            poDst->ippData8u(c),poDst->ippStep(),poDst->ippRoiSize(),
                            (double)poDst->getWidth()/(double)getWidth(),
                            (double)poDst->getHeight()/(double)getHeight(),
                            (int)eScaleMode);
        }else{
          ippiResize_32f_C1R(ippData32f(c),ippSize(),ippStep(),ippRoi(),
                             poDst->ippData32f(c),poDst->ippStep(),poDst->ippRoiSize(),
                             (double)poDst->getWidth()/(double)getWidth(),
                             (double)poDst->getHeight()/(double)getHeight(),
                             (int)eScaleMode);
        }
    } 
  
#else

  //---- Variable initilazation ----
  float fXStep = ((float)getWidth()-1)/(float)(poDst->getWidth()); 
  float fYStep = ((float)getHeight()-1)/(float)(poDst->getHeight());

  //---- scale ICL ----
  for(int c=0;c<m_iChannels;c++)
    {
      //---- Take the correct scaling method ----
      switch(eScaleMode) 
        {
          case interpolateNN: 
            if(poDst->getDepth()==depth8u){
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl8u()))(x,y,c)=static_cast<iclbyte>((*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c));
                }
              }
            }else{
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl32f()))(x,y,c)=static_cast<iclfloat>((*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c));
                }
              }
            }
            break;
          
          case interpolateLIN: 
            if(poDst->getDepth()==depth8u){
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl8u()))(x,y,c)=static_cast<iclbyte>(interpolate((fXStep/2)+ x*fXStep,(fYStep/2)+y*fYStep,c));
                }
              }
            }else{
              for(int x=0;x<poDst->getWidth();x++){
                for(int y=0;y<poDst->getHeight();y++){
                  (*(poDst->asIcl32f()))(x,y,c)=static_cast<iclfloat>(interpolate((fXStep/2)+ x*fXStep,(fYStep/2)+y*fYStep,c));
                }
              }
            }
            break;
          
          case interpolateRA: 
            ERROR_LOG("not yet implemented for Region Average");
            break;
          
          default:
            ERROR_LOG("Illegal operation selected!");
            break;
        }
    }
  
#endif
  
  return poDst;
}


//----------------------------------------------------------------------------
template<class Type>
void ICL<Type>::detach(int iIndex)
{
  //---- Log Message ----
  DEBUG_LOG4("detach()");
  
  //---- Make the whole ICL independent ----
  if(iIndex == -1)
    {
      for(int i=0;i<m_iChannels;i++) 
        {
          m_ppChannels[i] = 
            ICLChannelPtr(new ICLChannel<Type>(*m_ppChannels[i]));
        }
    }
  //---- Make a specific channel independent ----
  else
    {
      m_ppChannels[iIndex] 
        = ICLChannelPtr (new ICLChannel<Type>(*m_ppChannels[iIndex]));
    } 
}

//----------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::removeChannel(int iChannel)
{
  //---- Log Message ----
  DEBUG_LOG4("removeChannel(int)"); 
  
  //---- create new channel array ----
  vector<ICLChannelPtr> vecTmp(m_iChannels-1);
  
  //---- remove selected channel ----
  m_ppChannels[iChannel] = ICLChannelPtr();

  std::copy (m_ppChannels.begin(), 
             m_ppChannels.begin()+iChannel, 
             vecTmp.begin());
  std::copy (m_ppChannels.begin()+iChannel+1, 
             m_ppChannels.end(),
             vecTmp.begin()+iChannel);
  --m_iChannels;

  //---- clear old channel array and assign new ----
  m_ppChannels.clear();
  m_ppChannels = vecTmp;
}

//----------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::append(ICL<Type> *poSrc)
{
  //---- Log Message ----
  DEBUG_LOG4("appendImage(const ICL<Type>&)"); 

  if(m_iChannels > 0)
  { 
    //---- ensure identical image size
    if(poSrc->getWidth() != getWidth() || poSrc->getHeight() != getHeight())
    {
      ERROR_LOG("error in ICL::append: image sizes are different!");
      return;
    }
    
    for(int i=0;i<poSrc->m_iChannels;i++){
      m_ppChannels.push_back(poSrc->m_ppChannels[i]); 
      m_iChannels++;
    }      
  }
  else
  {
    // cheep copy
    m_ppChannels = poSrc->m_ppChannels;
    m_iChannels = (int)(m_ppChannels.size());
    m_iWidth = poSrc->m_iWidth;
    m_iHeight = poSrc->m_iHeight;      
  }
} 

//----------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::appendChannel(int iChannel, ICL<Type> *poSrc) 
{
  //---- Log Message ----
  DEBUG_LOG4("appendChannel(int,const ICL<Type>&)");
  
  if(m_iChannels > 0)
  { 
    //---- ensure identical image size
    if(poSrc->getWidth() != getWidth() || poSrc->getHeight() != getHeight())
    {
      ERROR_LOG("error in ICL::append: image sizes are different!");
      return;
    }
    
    m_ppChannels.push_back(poSrc->m_ppChannels[iChannel]); 
    m_iChannels++;
    
    /*
    //---- append channel ----
    vector<ICLChannelPtr> vecTmp(m_iChannels+1);
    
    //---- Shallow copy of source channels ----
    std::copy (m_ppChannels.begin(), 
               m_ppChannels.end(), 
               vecTmp.begin());
    
    //---- Delete old stuff ----
    m_ppChannels.clear();
    
    //---- Manage new ICL ----
    m_ppChannels = vecTmp;
    m_ppChannels[m_iChannels] = poSrc->m_ppChannels[iChannel];
    m_iChannels++;
    */
  }
}

//----------------------------------------------------------------------------
template<class Type>
void 
ICL<Type>::swapChannels(int iIndexA, int iIndexB)
{
  //---- Log Message ----
  DEBUG_LOG4("swapChannels(int,int)"); 
  std::swap(m_ppChannels[iIndexA], m_ppChannels[iIndexB]);
}

//----------------------------------------------------------------------------
template<class Type>
void 
ICL<Type>::scale(int iNewWidth, 
                 int iNewHeight,
                 iclscalemode eScaleMode)
{  
  //---- Log Message ----
  DEBUG_LOG4("scale(int,int,iclscalemode)"); 
  
  //---- estimate destination values in respect to defaults ----
  if(iNewWidth < 0) iNewWidth = m_iWidth;
  if(iNewHeight < 0) iNewHeight = m_iHeight;
  
  if(isEqual(iNewWidth,iNewHeight,m_iChannels))
    {
      return;    
    }  
  else
    {
      ICL<Type> oTmp(iNewWidth,iNewHeight,m_eFormat,m_iChannels);
      scaledCopy(&oTmp,eScaleMode);
      (*this)=oTmp;
    }
}

//----------------------------------------------------------------------------
template<class Type>
void 
ICL<Type>::resize(int iWidth,int iHeight)
{
  
  //---- Log Message ----
  DEBUG_LOG4("resize(int,int)"); 
  
  //---- estimate destination values in respect to defaults ----
  if(iWidth < 0) iWidth = m_iWidth;
  if(iHeight < 0) iHeight = m_iHeight;
  
  if(isEqual(iWidth,iHeight,m_iChannels))
    {
      return;
    }
  
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
    }
  
  m_iWidth = iWidth;
  m_iHeight = iHeight;
}

//----------------------------------------------------------------------------
template<class Type>
void 
ICL<Type>::setNumChannels(int iNumNewChannels)
{
  //---- Log Message ----
  DEBUG_LOG4("setNumChannels(int)"); 
   
  //---- reduce number of channels ----
  if(iNumNewChannels < m_iChannels)
  {
    for (int i=m_iChannels-1;i>=iNumNewChannels;i--)
    {
      //---- Make a referenced channel independent before resize ----
      detach(i);
      
      //---- Remove channel----
      removeChannel(i);
    }
  }
  //---- Extend number of channels ----
  else if (iNumNewChannels > m_iChannels)
  {
    //---- Allocate new memory for data ----
    vector<ICLChannelPtr> vecChannelsNew(iNumNewChannels);
    
    //---- Copy pointer from old ICL ----
    std::copy(m_ppChannels.begin(),
              m_ppChannels.end(),
              vecChannelsNew.begin());
    
    //---- Manage new ICL ----
    for(int i=m_iChannels;i<iNumNewChannels;i++)
      vecChannelsNew[i] 
        = ICLChannelPtr (new ICLChannel<Type>(getWidth(),getHeight()));
    
    //---- free memory ----
    m_ppChannels.clear();

    //---- Assign data ----
    m_ppChannels = vecChannelsNew;
    m_iChannels = iNumNewChannels;
  }
}

//----------------------------------------------------------------------------
template<class Type>
void 
ICL<Type>::renew(int iNewWidth, int iNewHeight, int iNewNumChannels)
{
  //---- Log Message ----
  DEBUG_LOG4("renewICL(int,int.int)"); 

  if(iNewWidth < 0)iNewWidth = m_iWidth;
  if(iNewHeight < 0)iNewHeight = m_iHeight;
  if(iNewNumChannels < 0)iNewNumChannels = m_iChannels;
  
  //---- Execution necessary ??? ----
  if (!isEqual(iNewWidth, iNewHeight, iNewNumChannels))
  {
    //---- Make referenced channels independent before resize ----
    // ?? detach(); 
    // ?? deleteChannels ();
    
    m_ppChannels.resize(iNewNumChannels);
    m_iChannels = iNewNumChannels;    
    
    for(int i=0;i<iNewNumChannels;i++)
    {
      m_ppChannels[i] = 
        ICLChannelPtr (new ICLChannel<Type>(iNewWidth,iNewHeight));
    }
    m_iWidth = iNewWidth;
    m_iHeight = iNewHeight;
    m_iChannels = iNewNumChannels;
  }
  else
  {
    detach();    
  }
}

//----------------------------------------------------------------------------
template<class Type>
inline
void ICL<Type>::replaceChannel(int iIndexA, 
                               int iIndexB, 
                               ICL<Type>  *poSrc) 
{
  //---- Log Message ----
  DEBUG_LOG4("replaceChannel(int,int,const ICL<Type>&)");
  
  //---- replace channel ----
  m_ppChannels[iIndexA] = ICLChannelPtr(); 
  m_ppChannels[iIndexA] = poSrc->m_ppChannels[iIndexB];
}

// }}} 

// {{{  Type converter: 

//--------------------------------------------------------------------------
template<class Type>
ICL32f*
ICL<Type>::convertTo32Bit(ICL32f *poDst) const
{
  //---- Log Message ----
  DEBUG_LOG4("convertTo32Bit()");
  
  if(!poDst)
  {
    poDst = new ICL32f(getWidth(),getHeight(),getFormat(),getChannels());
  }
  else
  {
    poDst->renew(m_iWidth,m_iHeight,m_iChannels);
    poDst->setFormat(m_eFormat);
  }
  
  // not neccesary ! poDst->detach();
  
  if(m_eDepth == depth8u)
  {
    for(int c=0;c<m_iChannels;c++)
    {
#ifdef WITH_IPP_OPTIMIZATION
      ippiConvert_8u32f_C1R(ippData8u(c),ippStep(),poDst->ippData32f(c),poDst->ippStep(),poDst->ippRoiSize());
#else
      int iDim = m_iWidth * m_iHeight;
      iclbyte *pucSrc =  reinterpret_cast<iclbyte*>(getDataPtr(c));
      iclfloat *pfDst = reinterpret_cast<iclfloat*>(poDst->getDataPtr(c));
      for(int i=0;i<iDim;i++)
      {
        pfDst[i]=static_cast<iclfloat>(pucSrc[i]);
      }
#endif
    }
    return poDst;
  }
  else
  {
    return asIcl32f()->deepCopy(poDst)->asIcl32f();
  }  
}

//--------------------------------------------------------------------------
template<class Type>
ICL8u*
ICL<Type>::convertTo8Bit(ICL8u *poDst) const
{
  //---- Log Message ----
  DEBUG_LOG4("convertTo8Bit()");
  
  if(!poDst){
    poDst = new ICL8u(getWidth(),getHeight(),getFormat(),getChannels());
  }else{
    poDst->renew(m_iWidth,m_iHeight,m_iChannels);
    poDst->setFormat(m_eFormat);
  }
  // not neccesary ! poDst->detach();
  if(m_eDepth == depth32f)
    {
      for(int c=0;c<m_iChannels;c++)
        {
#ifdef WITH_IPP_OPTIMIZATION
          ippiConvert_32f8u_C1R(ippData32f(c),ippStep(),poDst->ippData8u(c),poDst->ippStep(),poDst->ippRoiSize(),ippRndNear);
#else
          int iDim = m_iWidth* m_iHeight;
          iclfloat *pfSrc = reinterpret_cast<iclfloat*>(getDataPtr(c));
          iclbyte *pucDst =  reinterpret_cast<iclbyte*>(poDst->getDataPtr(c));
          for(int i=0;i<iDim;i++){
            pucDst[i]=static_cast<iclbyte>(pfSrc[i]);
          }
#endif
        }
      return poDst;
    }
  else
    {
      return asIcl8u()->deepCopy(poDst)->asIcl8u();
    }  
 
}

// }}} 

// {{{  Getter Functions: 

// ---------------------------------------------------------------------
template<class Type> 
Type 
ICL<Type>::getMax(int iChannel) const
{
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMax;
      ippiMax_8u_C1R(ippData8u(iChannel),ippStep(),ippRoiSize(),&ucMax);
      return static_cast<Type>(ucMax);
    }
  else
    {
      iclfloat fMax;
      ippiMax_32f_C1R(ippData32f(iChannel),ippStep(),ippRoiSize(),&fMax);
      return static_cast<Type>(fMax);
    }
                     
#else
  return m_ppChannels[iChannel]->getMax();
#endif
}

// ---------------------------------------------------------------------  
template<class Type>
Type 
ICL<Type>::getMin(int iChannel) const
{
#ifdef WITH_IPP_OPTIMIZATION
  if(m_eDepth == depth8u)
    {
      iclbyte ucMin;
      ippiMin_8u_C1R(ippData8u(iChannel),ippStep(),ippRoiSize(),&ucMin);
      return static_cast<Type>(ucMin);
    }
  else
    {
      iclfloat fMin;
      ippiMin_32f_C1R(ippData32f(iChannel),ippStep(),ippRoiSize(),&fMin);
      return static_cast<Type>(fMin);
    }
                     
#else
  return m_ppChannels[iChannel]->getMin();
#endif
}
  

// ---------------------------------------------------------------------
template<class Type>
inline
void 
ICL<Type>::getROI(int &riX, int &riY, int &riWidth, int &riHeight) const
{
  //---- Log Message ----
  DEBUG_LOG4("getROI(int&,int&,int&,int&)");
  
  if(m_iChannels > 0)
    {
      riX = m_ppChannels[0]->getRoiXOffset();
      riY = m_ppChannels[0]->getRoiYOffset();
      riWidth = m_ppChannels[0]->getRoiWidth();
      riHeight = m_ppChannels[0]->getRoiHeight();
    }
  else
    {
      ERROR_LOG("getROI channel count is 0 \n");
    }
}

// ---------------------------------------------------------------------
template<class Type>
inline
void 
ICL<Type>::getROIOffset(int &riX, int &riY) const
{
  //---- Log Message ----
  DEBUG_LOG4("getROIOffset(int&,int&)");

  if(m_iChannels > 0)
    {
      riX = m_ppChannels[0]->getRoiXOffset();
      riY = m_ppChannels[0]->getRoiYOffset();
    }
  else
    {
      ERROR_LOG("getROIOffset channel count is 0 \n");
    }
}
// ---------------------------------------------------------------------
template<class Type>
inline
void 
ICL<Type>::getROISize(int &riWidth, int &riHeight) const
{

  //---- Log Message ----
  DEBUG_LOG4("getROISize(int&,int&)");
  
  if(m_iChannels > 0)
    {
      riWidth = m_ppChannels[0]->getRoiWidth();
      riHeight = m_ppChannels[0]->getRoiHeight();
    }
  else
    {
      ERROR_LOG("getROISize channel count is 0 \n");
    }
}

// ---------------------------------------------------------------------
template<class Type>
void ICL<Type>::clear(int iChannel, Type tValue) 
{
  //---- Log Message ----
  DEBUG_LOG4("clear(int, type)");
  
  if(iChannel == -1) 
    {
      for(int i=0;i<m_iChannels;i++) 
        m_ppChannels[i]->clear(tValue);
    }
  else 
    {
      m_ppChannels[iChannel]->clear(tValue);
    }
}



// }}}
  
// {{{  Setter Function:
  
template<class Type> 
void 
ICL<Type>::
setROI(int iX, int iY,int iWidth,int iHeight)
{
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i]->setImageRoi(iWidth,iHeight);
      m_ppChannels[i]->setImageRoiOffset(iX,iY);
    }
}
  
template<class Type> 
void 
ICL<Type>::
setROIOffset(int iX, int iY)
{
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i]->setImageRoiOffset(iX,iY);
    }
}
  
template<class Type> 
void 
ICL<Type>::
setROISize(int iWidth, int iHeight)
{
  for(int i=0;i<m_iChannels;i++)
    {
      m_ppChannels[i]->setImageRoi(iWidth,iHeight);
    }
}
  

// }}}

// {{{  Auxillary and basic image manipulation functions

template<class Type>
Type
ICL<Type>::interpolate(float fX, float fY, int iChannel) const
{
  //---- Variable iitilization ----
  float fY1,fY2,fY3,fY4,fT,fU;
  float fReturn;
  
  fY1 = fY2 = fY3 = fY4 = fT = fU = 0;
  
  //---- interpolate  ----
  if( (int) ceil(fX) >= this->getWidth(iChannel) ||  
      (int) ceil(fY) >= this->getHeight(iChannel) )
  {
    ERROR_LOG("Interpolation position out of range");
  }
  else
  {
    fY1=(float)(*this)((int)floor(fX), (int)floor(fY), iChannel);
    fY4=(float)(*this)((int)floor(fX), (int)ceil(fY), iChannel);
    fY3=(float)(*this)((int)ceil(fX), (int)ceil(fY), iChannel);
    fY2=(float)(*this)((int)ceil(fX), (int)floor(fY), iChannel);
    fT=fX-floor(fX);
    fU=fY-floor(fY);
  }
  
  fReturn=(1-fT)*(1-fU)*fY1+ fT*(1-fU)*fY2 + fT*fU*fY3 + (1-fT)*fU*fY4;
  
  //---- return ----
  if(m_eDepth == depth8u)
    return (Type) rint(fReturn);
  else
    return (Type) fReturn;
}

//--------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::scaleRange(float tMin, float tMax, int iChannel)
{
  DEBUG_LOG4("scaleRange(float,float,iChannel");
  
  if (iChannel == -1)
  {
    for(int i=0;i<getChannels();i++)
    {
      DEBUG_LOG4("Scale channel :" << i);
      
      m_ppChannels[i]->scaleRange(tMin,
                                  tMax,
                                  m_ppChannels[i]->getMin(),
                                  m_ppChannels[i]->getMax() );
    }
  }
  else
  {
    m_ppChannels[iChannel]->scaleRange(tMin,
                                       tMax,
                                       m_ppChannels[iChannel]->getMin(),
                                       m_ppChannels[iChannel]->getMax() );
  }
}

// }}}

template class ICL<iclbyte>;
template class ICL<iclfloat>;

} //namespace icl
