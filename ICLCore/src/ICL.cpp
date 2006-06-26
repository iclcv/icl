/*
  ICL.cpp

  Written by: Michael Götting (2004)
              University of Bielefeld
              AG Neuroinformatik
              mgoettin@techfak.uni-bielefeld.de
*/

#include "ICL.h"

//---- ICL in its own namespace ----
namespace ICL {

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
    m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
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
  
template<class Type>
Type& ICL<Type>::operator()(int iX, int iY, int iChannel) const
{
  return (*m_ppChannels[iChannel])(iX,iY);
}  

// }}} 

// {{{  class organisation : 

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>* 
ICL<Type>::deepCopy(ICL<Type>* poDst) const
{
  //---- Log Message ----
  DEBUG_LOG4("deepCopy(ICL<Type>*)"); 

  //---- Allocate memory ----
  if (poDst == NULL) 
  {
    poDst = new ICL<Type> (*this);
  } 
  else 
  {
    //---- release old channels in destination ----
    poDst->m_ppChannels.clear();
    
    //---- and set to new values ----
    poDst->m_ppChannels.resize(m_iChannels);
    poDst->m_iChannels = m_iChannels;
    poDst->m_iWidth = m_iWidth;
    poDst->m_iHeight = m_iHeight;
    poDst->m_eFormat = m_eFormat;
    
    //---- Shallow copy of source channels ----
    std::copy(this->m_ppChannels.begin(), 
              this->m_ppChannels.end(),
              poDst->m_ppChannels.begin());
  }
  
  //---- Make ICL independent ----
  poDst->detach();
  
  //---- return ----
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
ICL<Type>::appendICL(ICL<Type> *poSrc)
{
  //---- Log Message ----
  DEBUG_LOG4("appendImage(const ICL<Type>&)"); 
  
  //---- Variable initialisation ----
  int iNumExternChannels = poSrc->m_iChannels;
  int iNumInternChannels = m_iChannels;
  vector<ICLChannelPtr> vecTmp(iNumInternChannels+iNumExternChannels);
  
  //---- copy channels from internal ICL ----
  std::copy (m_ppChannels.begin(), m_ppChannels.end(), vecTmp.begin());
  
  //---- clear  old stuff ----
  m_ppChannels.clear();
  
  //---- Manage new ICL ----
  m_ppChannels = vecTmp;
  
  std::copy(poSrc->m_ppChannels.begin(),
            poSrc->m_ppChannels.end(),
            m_ppChannels.begin()+iNumInternChannels);
  
  m_iChannels += iNumExternChannels;
} 

//----------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::appendChannel(int iChannel, ICL<Type> *poSrc) 
{
  //---- Log Message ----
  DEBUG_LOG4("appendChannel(int,const ICL<Type>&)");
  
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
  DEBUG_LOG4("resize(int,int,iclscalemode)"); 
  
  //---- estimate destination values in respect to defaults ----
  if(iNewWidth < 0)iNewWidth = m_iWidth;
  if(iNewHeight < 0)iNewHeight = m_iHeight;
  
  if(isEqual(iNewWidth,iNewHeight,m_iChannels))
    {
      return;    
    }  
  else
    {
      ICL<Type> oTmp(iNewWidth,iNewHeight,m_eFormat,m_iChannels);
      smartCopy(&oTmp,eScaleMode);
      (*this)=oTmp;
    }
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
ICL<Type>::renewICL(int iNewWidth, int iNewHeight, int iNumNewChannels)
{
  //---- Log Message ----
  DEBUG_LOG4("renewICL(int,int.int)"); 

  //---- Execution necessary ??? ----
  if (!isEqual(iNewWidth, iNewHeight, iNumNewChannels))
  {
    //---- Make referenced channels independent before resize ----
    detach(); 
    deleteChannels ();
    
    m_ppChannels.resize(iNumNewChannels);
    m_iChannels = iNumNewChannels;    
    
    for(int i=0;i<iNumNewChannels;i++)
    {
      m_ppChannels[i] = 
        ICLChannelPtr (new ICLChannel<Type>(iNewWidth,iNewHeight));
    }
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
  
  if(!poDst)poDst = new ICL32f(getWidth(),getHeight(),getFormat(),getChannels());
  poDst->detach();
  
  //---- convert ----
  for(int c=0;c<m_iChannels;c++)
    {
    for(int x=0;x<m_iWidth;x++)
      {
      for(int y=0;y<m_iHeight;y++)
        {
          (*poDst)(x,y,c) = static_cast<iclfloat>((*this)(x,y,c));
        }
      }
    }
  
  return poDst;
}

//--------------------------------------------------------------------------
template<class Type>
ICL8u*
ICL<Type>::convertTo8Bit(ICL8u *poDst) const
{
  //---- Log Message ----
  DEBUG_LOG4("convertTo8Bit()");
  
  if(!poDst)poDst = new ICL8u(getWidth(),getHeight(),getFormat(),getChannels());
  poDst->detach();
  
  //---- convert ----
  for(int c=0;c<m_iChannels;c++)
    {
    for(int x=0;x<m_iWidth;x++)
      {
      for(int y=0;y<m_iHeight;y++)
        {
          (*poDst)(x,y,c) = static_cast<iclbyte>((*this)(x,y,c));
        }
      }
    }
  
  return poDst;
}

// }}} 

// {{{  Get Functions: 

template<class Type>
inline
void 
ICL<Type>::getROI(int &riX, int &riY, int &riWidth, int &riHeight) const
{
  //---- Log Message ----
  DEBUG_LOG4("getROI(int&,int&,int&,int&)");
  
  for(int c=0;c<m_iChannels;c++)
    {
      riX = m_ppChannels[c]->getRoiXOffset();
      riY = m_ppChannels[c]->getRoiYOffset();
      riWidth = m_ppChannels[c]->getRoiWidth();
      riHeight = m_ppChannels[c]->getRoiHeight();
    }
}

template<class Type>
inline
void 
ICL<Type>::getROIOffset(int &riX, int &riY) const
{
  //---- Log Message ----
  DEBUG_LOG4("getROIOffset(int&,int&)");

  for(int c=0;c<m_iChannels;c++)
    {
      riX = m_ppChannels[c]->getRoiXOffset();
      riY = m_ppChannels[c]->getRoiYOffset();
    }
}

template<class Type>
inline
void 
ICL<Type>::getROISize(int &riWidth, int &riHeight) const
{

  //---- Log Message ----
  DEBUG_LOG4("getROISize(int&,int&)");
  
  for(int c=0;c<m_iChannels;c++)
    {
      riWidth = m_ppChannels[c]->getRoiWidth();
      riHeight = m_ppChannels[c]->getRoiHeight();
    }
}



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

//--------------------------------------------------------------------------
template<class Type>
ICL<Type>*
ICL<Type>::smartCopy(ICL<Type> *poDst,iclscalemode eScaleMode) const
{
  //---- Log Message ----
  DEBUG_LOG4("smartCopy(ICL,iclscalemode)");
  
  //---- deep copy case -----
  if(!poDst || isEqual(poDst->getWidth(),poDst->getHeight(),poDst->getChannels())){
    return deepCopy(poDst); 
  }
  
  //---- Variable initilazation ----
  float fXStep = ((float)getWidth()-1)/(float)(poDst->getWidth()); 
  float fYStep = ((float)getHeight()-1)/(float)(poDst->getHeight());
  
  int iNChannels = m_iChannels < poDst->getChannels() ? m_iChannels : poDst->getChannels();
  
  //---- scale ICL ----
  for(int c=0;c<iNChannels;c++)
    {
      //---- Take the correct scaling method ----
      switch(eScaleMode) 
        {
          case interpolateNN: 
            for(int x=0;x<poDst->getWidth();x++)
              for(int y=0;y<poDst->getHeight();y++)
                (*poDst)(x,y,c)=(*this)((int)rint(x*fXStep),(int)rint(y*fYStep),c);
            break;
          case interpolateBL: 
            for(int x=0;x<poDst->getWidth();x++)
              for(int y=0;y<poDst->getHeight();y++)
                (*poDst)(x,y,c)=interpolate((fXStep/2)+ x*fXStep,(fYStep/2)+y*fYStep,c);
            break;
          case interpolateAV: 
            ERROR_LOG("not yet implemented for Region Average");
            break;
            /*
            if (fFactor>=1.0) 
            {
            DEBU_LOG1("Illegal scaleFactor for average Scaling!");
            exit(0);
            } 
            else 
            {
            int   iStep=(int) floor(1.0/fFactor);
            float sum,sq_factor=1.0/float(iStep*iStep);
            img->resizeChannel((int)floor(float(getWidth(c))/float(iStep)),
            (int)floor(float(getHeight(c))/float(iStep)), 
            c);
            for(int x=0;x<img->getWidth(c);x++)
            for(int y=0;y<img->getHeight(c);y++)
            {
            sum=0;
            for(int a=0;a<iStep;a++)
            for(int b=0;b<iStep;b++)
            sum+=(float)getPixel(x*iStep+a, y*iStep+b,c);
            img->setPixel(x,y,c,(Type) (sum*sq_factor));
            }
            
            }     
            break;
            */
          default:
            ERROR_LOG("Illegal operation selected!");
            break;
        }
    }
  return poDst;
}


// }}}
  
// {{{  Auxillary functions

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

//--------------------------------------------------------------------------
template<class Type>
int
ICL<Type>::isEqual(int iNewWidth, int iNewHeight, int iNewNumChannels) const
{
  return m_iWidth == iNewWidth && m_iHeight == iNewHeight && m_iChannels == iNewNumChannels;
    /*
    if (iNumNewChannels != m_iChannels)
    {
    cout << "isEqual != Channel (" <<iNumNewChannels<<"/"<<m_iChannels<<")"<< endl;
    return ICL_FALSE;
    }
    
    for (int i=0;i<m_iChannels;i++)
    {
    if ( (iNewWidth != getWidth(i)) ||
    (iNewHeight != getHeight(i)) )
    {
    cout << "isEqual != Channel(" << i << ")" << endl;   
    cout << "iNewWidth: " << iNewWidth << "/ " << getWidth(i) << endl;
    cout << "iNewHeight: " << iNewHeight << "/ " << getHeight(i) << endl;
    return ICL_FALSE;
    }
    }
  
    return ICL_TRUE;
    */
  
}

// }}}


template class ICL<iclbyte>;
template class ICL<iclfloat>;

} //namespace ICL
