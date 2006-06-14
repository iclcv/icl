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
ICL<Type>::ICL(int iWidth, 
               int iHeight, 
               int iChannels)
{
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(int,int,int) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_iChannels = iChannels;
  m_ppChannels.resize(m_iChannels);
  
  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(iWidth,iHeight));
  
  //---- Get type id ----
  setVarType();
} 

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(vector<int> vecWidth, 
               vector<int> vecHeight)
{
  //---- Log Message ----
  DEBUG_LOG4("Konstruktor: ICL(vector<int>,vector<int>) -> " << this);
  
  //---- Variable definiton/ initialisation ----
  m_iChannels = vecWidth.size();
  m_ppChannels.resize(m_iChannels);

  //---- ICL Channel memory allocation ----
  for(int i=0;i<m_iChannels;i++)
    m_ppChannels[i] = ICLChannelPtr(new ICLChannel<Type>(vecWidth[i],
                                                         vecHeight[i]));

  //---- Get type id ----
  setVarType();
}

//----------------------------------------------------------------------------
template<class Type>
ICL<Type>::ICL(const ICL<Type>& tSrc)
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
  
  //---- Get type id ----
  setVarType();
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
ICL<Type>* 
ICL<Type>::deepCopy(ICL<Type>* pToICL)
{
  //---- Log Message ----
  DEBUG_LOG4("deepCopy(ICL<Type>*)"); 

  //---- Allocate memory ----
  if (pToICL == NULL) 
  {
    pToICL = new ICL<Type> (*this);
  } 
  else 
  {
    //---- release old channels in destination ----
    pToICL->m_ppChannels.clear();
    
    //---- and set to new values ----
    pToICL->m_ppChannels.resize(m_iChannels);
    pToICL->m_iChannels = m_iChannels;
    
    //---- Shallow copy of source channels ----
    std::copy(this->m_ppChannels.begin(), 
              this->m_ppChannels.end(),
              pToICL->m_ppChannels.begin());
  }
  
  //---- Make ICL independent ----
  pToICL->detach();
  
  //---- return ----
  return pToICL;
}

//----------------------------------------------------------------------------
template<class Type>
void ICL<Type>::detach(int iIndex)
{
  //---- Log Message ----
  DEBUG_LOG4("detach()");
  
  if (m_iChannels > 0)
  {
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
ICL<Type>::appendICL(const ICL<Type>& pExternal)
{
  //---- Log Message ----
  DEBUG_LOG4("appendImage(const ICL<Type>&)"); 
  
  //---- Variable initialisation ----
  int iNumExternChannels = pExternal.m_iChannels;
  int iNumInternChannels = m_iChannels;
  vector<ICLChannelPtr> vecTmp(iNumInternChannels+iNumExternChannels);
  
  //---- copy channels from internal ICL ----
  std::copy (m_ppChannels.begin(), m_ppChannels.end(), vecTmp.begin());
  
  //---- clear  old stuff ----
  m_ppChannels.clear();
  
  //---- Manage new ICL ----
  m_ppChannels = vecTmp;
  
  std::copy(pExternal.m_ppChannels.begin(),
            pExternal.m_ppChannels.end(),
            m_ppChannels.begin()+iNumInternChannels);
  
  m_iChannels += iNumExternChannels;
} 

//----------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::appendChannel(int iChannel, const ICL<Type>& srcICL) 
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
  m_ppChannels[m_iChannels] = srcICL.m_ppChannels[iChannel];
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
ICL<Type>::resizeChannel(int iNewWidth, int iNewHeight, int iChannel)
{
  //---- Log Message ----
  DEBUG_LOG4("resizeChannel(int,int.int)"); 
  
  //---- Make a referenced channel independent before resize ----
  detach(iChannel);
  
  //---- Resize ICL channel ----
  m_ppChannels[iChannel] 
    = ICLChannelPtr (new ICLChannel<Type>(iNewWidth,iNewHeight));
}


//----------------------------------------------------------------------------
template<class Type>
void 
ICL<Type>::resizeICL(int iNewWidth, int iNewHeight, int iNumNewChannels)
{
  //---- Log Message ----
  DEBUG_LOG4("resizeICL(int,int.int)"); 

  //----- Initialize variables ----
  int iNumChannels = m_iChannels;
  
  //---- reduce number of channels ----
  if(iNumNewChannels < iNumChannels)
  {
    for (int i=iNumChannels-1;i>=iNumNewChannels;i--)
    {
      //---- Make a referenced channel independent before resize ----
      detach(i);
      
      //---- Remove channel----
      removeChannel(i);
    }
  }
  //---- Extend number of channels ----
  else if (iNumNewChannels > iNumChannels)
  {
    //---- Allocate new memory for data ----
    vector<ICLChannelPtr> vecChannelsNew(iNumNewChannels);
    
    //---- Copy pointer from old ICL ----
    std::copy(m_ppChannels.begin(),
              m_ppChannels.end(),
              vecChannelsNew.begin());
    
    //---- Manage new ICL ----
    for(int i=iNumChannels;i<iNumNewChannels;i++)
      vecChannelsNew[i] 
        = ICLChannelPtr (new ICLChannel<Type>(iNewWidth,iNewHeight));
    
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
  if (isEqual(iNewWidth, iNewHeight, iNumNewChannels) == ICL_FALSE)
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
void 
ICL<Type>::renewICL(vector<int> vecNewWidth, 
                    vector<int> vecNewHeight)
{
  //---- Log Message ----
  DEBUG_LOG4("renewICL(vector<int>, vector<int>)"); 

  //---- Make referenced channels independent before resize ----
  detach(); 
  deleteChannels ();
  
  m_ppChannels.resize(vecNewWidth.size());
  m_iChannels = vecNewWidth.size();    
  
  for(int i=0;i<m_iChannels;i++)
  {
    m_ppChannels[i] = 
      ICLChannelPtr (new ICLChannel<Type>(vecNewWidth[i],
                                          vecNewHeight[i]));
  }
}

//----------------------------------------------------------------------------
template<class Type>
inline
void ICL<Type>::replaceChannel(int iIndexA, 
                               int iIndexB, 
                               const ICL<Type>& srcICL) 
{
  //---- Log Message ----
  DEBUG_LOG4("replaceChannel(int,int,const ICL<Type>&)");
  
  //---- replace channel ----
  m_ppChannels[iIndexA] = ICLChannelPtr(); 
  m_ppChannels[iIndexA] = srcICL.m_ppChannels[iIndexB];
}

// }}} 

// {{{  Type converter: 

//--------------------------------------------------------------------------
template<class Type>
ICL<DEPTH32>
ICL<Type>::convertTo32Bit() const
{
  //---- Log Message ----
  DEBUG_LOG4("convertTo32Bit()");
  
  ICL<DEPTH32> tImg(getWidth(0),getHeight(0),m_iChannels);
  
  //---- convert ----
  for(int i=0;i<m_iChannels;i++)
  {
    tImg.resizeChannel(getWidth(i),getHeight(i),i);
    
    for(int x=0;x<getWidth(i);x++)
      for(int y=0;y<getHeight(i);y++)
        tImg.setPixel(x,y,i,(DEPTH32) m_ppChannels[i]->getPixel(x,y));    
  }
  
  return tImg;
}

//--------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::convertTo32Bit(ICL<DEPTH32> &tImg) const
{
  //---- Log Message ----
  DEBUG_LOG4("convertTo32Bit(ICL<DEPTH32> &)");
  
  //---- resize image if necessary ----
  tImg.resizeICL(getWidth(0),getHeight(0),m_iChannels);
  
  //---- convert ----
  for(int i=0;i<m_iChannels;i++)
  {
    tImg.resizeChannel(getWidth(i),getHeight(i),i);
    
    for(int x=0;x<getWidth(i);x++)
      for(int y=0;y<getHeight(i);y++)
        tImg.setPixel(x,y,i,(DEPTH32) m_ppChannels[i]->getPixel(x,y));
  }
}

//--------------------------------------------------------------------------
template<class Type>
ICL<DEPTH8>
ICL<Type>::convertTo8Bit() const
{
  //---- Log Message ----
  DEBUG_LOG4("convertTo8Bit()");
  
  ICL<DEPTH8> tImg(getWidth(0),getHeight(0),m_iChannels);
  
  for(int i=0;i<m_iChannels;i++)
  {
    for(int x=0;x<getWidth(i);x++)
      for(int y=0;y<getHeight(i);y++)
        tImg.setPixel(x,y,i,(DEPTH8) m_ppChannels[i]->getPixel(x,y));    
  }

  return tImg;
}

//--------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::convertTo8Bit(ICL<DEPTH8> &tImg) const
{
  //---- Log Message ----
  DEBUG_LOG4("convertTo8Bit(ICL<DEPTH8> &)");
  
  //---- resize image if necessary ----
  tImg.resizeICL(getWidth(0),getHeight(0),m_iChannels);
  
  //---- convert ----
  for(int i=0;i<m_iChannels;i++)
  {  
    for(int x=0;x<getWidth(i);x++)
      for(int y=0;y<getHeight(i);y++)
        tImg.setPixel(x,y,i,(DEPTH8) m_ppChannels[i]->getPixel(x,y));
  }
}

// }}} 

// {{{  Set functions: 
//----------------------------------------------------------------------------
template<class Type>
inline
void 
ICL<Type>::setPixel(int iX, int iY, int iChannel,Type value)
{
  m_ppChannels[iChannel]->setPixel(iX,iY,value);
}

//----------------------------------------------------------------------------
template<class Type>
inline
void 
ICL<Type>::setPixel(int iX, int iY, Type* pValue)
{
  for(int i=0;i<m_iChannels;i++)
    m_ppChannels[i]->setPixel(iX,iY,pValue[i]);
}

//----------------------------------------------------------------------------
template<class Type>
void ICL<Type>::setVarType()
{
  //---- Detect var type ----
  m_sVarType = typeid(Type).name();
}

// }}} 

// {{{  Get Functions: 

//----------------------------------------------------------------------------
template<class Type>
vector<int>
ICL<Type>::getHeight() const
{
  //---- Variable initilisation ----
  vector<int> vecHeight;
  vecHeight.clear();
  
  //---- Main ----
  for (int i=0;i<m_iChannels;i++)
  {
    vecHeight.push_back(getHeight(i));
  }

  //---- return----
  return vecHeight;
}

//----------------------------------------------------------------------------
template<class Type>
vector<int>
ICL<Type>::getWidth() const
{
  //---- Variable initilisation ----
  vector<int> vecWidth;
  vecWidth.clear();
  
  //---- Main ----
  for (int i=0;i<m_iChannels;i++)
  {
    vecWidth.push_back(getWidth(i));
  }
  
  //---- return----
  return vecWidth;
}

//----------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::getChannelData(vector<Type> &vecTarget, int iChannel) const
{
  //---- Get channel data ----
  std::copy(m_ppChannels[iChannel]->getDataPtrBegin(),
            m_ppChannels[iChannel]->getDataPtrEnd(),
            vecTarget.begin());
}
  
//----------------------------------------------------------------------------
template<class Type>
inline
void 
ICL<Type>::getPixel(int iX, int iY, Type* ptTarget) const
{
  //---- Get the pixel value ----
  for(int i=0;i<m_iChannels;i++)
    ptTarget[i] = m_ppChannels[i]->getPixel(iX,iY);
}

//----------------------------------------------------------------------------
template<class Type>
inline
void
ICL<Type>::getChannelRoi(int* piWidth, int* piHeight, int iChannel)
{
  *piWidth = m_ppChannels[iChannel]->getRoiWidth();
  *piHeight = m_ppChannels[iChannel]->getRoiHeight();
}

//----------------------------------------------------------------------------
template<class Type>
inline
void
ICL<Type>::getChannelRoiOffset(int* piXOffset, int* piYOffset, int iChannel)
{
  *piXOffset = m_ppChannels[iChannel]->getRoiXOffset();
  *piYOffset = m_ppChannels[iChannel]->getRoiYOffset();
}

//----------------------------------------------------------------------------
template<class Type>
int ICL<Type>::getFilteredPixel(int iX, 
                                int iY, 
                                bool* pbMask,
                                Type* ptTarget) const 
{
  //---- Variable initialization ----
  int iCounter =0;
  
  //---- Get pixel values ----
  for(int i=0;i<m_iChannels;i++)
    if(pbMask[i]) 
    {
      ptTarget[iCounter]=getPixel(iX,iY,i);
      iCounter++;
    }
  
  return iCounter; 
}

//--------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::getRegion(int iCenterMode,
                     int iX, 
                     int iY, 
                     int iRegionWidth, 
                     int iRegionHeight,
                     ICL<Type> &oTarget)
{
  //---- Log Message ----
  DEBUG_LOG3("void getRegion(int,int,int,int,int,ICL<Type>)");
  
  //---- Allocate memory ----
  Type tmpPix;
  int iWidth = 0;
  int iHeight = 0;

  //---- Initilaize new ICL ----
  oTarget.renewICL(iRegionWidth,iRegionHeight,m_iChannels);
  
  switch(iCenterMode)
  {
    case 0:
      //---- iX, iY specify the lower left corner of the image region ----
      //CHECK_COORDINATE(this,(iX+iRegionWidth-1),(iY+iRegionHeight-1));   
      break;
      
    case 1:
      iWidth = getWidth(0);
      iHeight = getHeight(0);

      //---- calc new iX, iY position ----
      iX = (int)floor(0.5*iWidth) - (int)floor(0.5*iRegionWidth);
      iY = (int)floor(0.5*iHeight) - (int)floor(0.5*iRegionHeight);
      
      break;
  }
  
  //---- Log Message: parameter summary ----
  DEBUG_LOG2("getRegion: Parameter summary:");
  DEBUG_LOG2("-----------------------------");
  DEBUG_LOG2("iCenterMode  : " << iCenterMode);
  DEBUG_LOG2("iX (Offset)  : " << iX);
  DEBUG_LOG2("iY (Offset)  : " << iY);
  DEBUG_LOG2("iWidth (src) : " << iWidth);
  DEBUG_LOG2("iHeight (src): " << iHeight);
  DEBUG_LOG2("iRegionWidth : " << iRegionWidth);
  DEBUG_LOG2("iRegionHeight: " << iRegionHeight);
  
  //---- get image region ----
  for (int x=0;x<iRegionWidth;x++)
    for (int y=0;y<iRegionHeight;y++)
      for(int i=0;i<m_iChannels;i++)
      {
        tmpPix = getPixel(x+iX,y+iY,i);
        oTarget.setPixel(x,y,i,tmpPix);
      }
}

//----------------------------------------------------------------------------
template<class Type> 
inline
int ICL<Type>::getVarType() const
{
  //---- Initialize Variables ----
  int iType;
  iType = 0;
  
  //---- Var type to int ----
  if (m_sVarType == "h")
    iType = 8;
  else if (m_sVarType == "f")
    iType = 32;
  
  //---- return value ----
  return iType;
}

// }}} 

// {{{  basic image manipulation: 

//--------------------------------------------------------------------------
template<class Type>
void ICL<Type>::clear(int iChannel) 
{
  //---- Log Message ----
  DEBUG_LOG4("clear()");
  
  if(iChannel == -1) 
  {
    for(int i=0;i<m_iChannels;i++) 
      m_ppChannels[i]->clear();
  }
  else 
    m_ppChannels[iChannel]->clear();
}

//--------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::scale(float fFactor, int method, ICL<Type> *img) const
{
  //---- Log Message ----
  DEBUG_LOG4("scale(float, int)");
  
  //---- Variable initilazation ----
  float fXStep,fYStep;

  fXStep = 0;
  fYStep = 0;
  
  img->renewICL(1,1,getChannels());
  
  //---- scale ICL ----
  for(int c=0;c<img->getChannels();c++)
  {
    //---- Average ----  
    if (method!=2) 
    {
      img->resizeChannel((int)floor(fFactor * (float)getWidth(c)),
                        (int)floor(fFactor * (float)getHeight(c)), 
                        c);
      
      fXStep = ((float)getWidth(c)-1)/(float)(img->getWidth(c)); 
      fYStep = ((float)getHeight(c)-1)/(float)(img->getHeight(c));
    }
    
    //---- Take the correct scaling method ----
    switch(method) 
    {
      case 0: //---- Simple ----
        for(int x=0;x<img->getWidth(c);x++)
          for(int y=0;y<img->getHeight(c);y++)
            img->setPixel(x,y,c,getPixel((int)rint(x*fXStep), 
                                        (int)rint(y*fYStep),c));
        break;
      case 1: //---- BiLinInter ----
        for(int x=0;x<img->getWidth(c);x++)
          for(int y=0;y<img->getHeight(c);y++)
            img->setPixel(x,y,c,interpolate((fXStep/2) + x*fXStep, 
                                           (fYStep/2)+y*fYStep,c));
        break;
      case 2: //---- Average ----
        if (fFactor>=1.0) 
        {
          DEBUG_LOG1("Illegal scaleFactor for average Scaling!");
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
        
      default:
        ERROR_LOG("Illegal operation selected!");
        exit(0);
        break;
    }
  }
}

//--------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::scaleRange(vector<Type> scaleFactor, 
                      int iChannel)
{  
  //---- scale ----
  if (iChannel == -1)
  {
    for(int i=0;i<getChannels();i++)
    {
      m_ppChannels[i]->scaleRange(scaleFactor[i]);    
    }
  }
  else
  { 
    m_ppChannels[iChannel]->scaleRange(scaleFactor[iChannel]);
  }
}

//--------------------------------------------------------------------------
template<class Type>
ICL<Type>
ICL<Type>::mask(ICL<Type> &mask)
{
  int iWidth = getWidth(0);
  int iHeight = getHeight(0);
  
  ICL<Type> maskedImage(iWidth, iHeight ,m_iChannels);
  
  for (int z=0;z<m_iChannels;z++)
    for (int y=0;y<iHeight;y++)
      for (int x=0;x<iWidth;x++)
      {
        if (mask.getPixel(x,y,0) != 0)
          maskedImage.setPixel(x,y,z,this->getPixel(x,y,z));
        else
          maskedImage.setPixel(x,y,z,0);
      }
  
  return maskedImage;
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
    fY1=(float)getPixel((int)floor(fX), (int)floor(fY), iChannel);
    fY4=(float)getPixel((int)floor(fX), (int)ceil(fY), iChannel);
    fY3=(float)getPixel((int)ceil(fX), (int)ceil(fY), iChannel);
    fY2=(float)getPixel((int)ceil(fX), (int)floor(fY), iChannel);
    fT=fX-floor(fX);
    fU=fY-floor(fY);
  }
  
  fReturn=(1-fT)*(1-fU)*fY1+ fT*(1-fU)*fY2 + fT*fU*fY3 + (1-fT)*fU*fY4;
  
  //---- return ----
  if(getVarType() == 8)
    return (Type) rint(fReturn);
  else
    return (Type) fReturn;
}

//--------------------------------------------------------------------------
template<class Type>
void
ICL<Type>::scaleRange(float tMin, float tMax, int iChannel)
{
  DEBUG_LOG4("Scale image");
  
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
ICL<Type>::isEqual(int iNewWidth, int iNewHeight, int iNumNewChannels)
{
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
  
}

// }}}


template class ICL<DEPTH8>;
template class ICL<DEPTH32>;

} //namespace ICL
