/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/CoreFunctions.cpp                          **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter, Michael Götting, Robert Haschke   **
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

#include <ICLCore/CoreFunctions.h>
#include <ICLUtils/Exception.h>
#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>

#include <vector>
#include <numeric>

using namespace std;

namespace icl{
  namespace core{
    
    ImgBase *imgNew(depth d, const ImgParams &params){
      // {{{ open
    
      switch (d){
        case depth8u:  return new Img8u(params); break;
        case depth16s: return new Img16s(params); break;
        case depth32s: return new Img32s(params); break;
        case depth32f: return new Img32f(params); break;
        case depth64f: return new Img64f(params); break;
        default: ICL_INVALID_DEPTH; break;
      }
    }
  
    // }}}
  
    int getChannelsOfFormat(format eFormat){
      // {{{ open
  
      switch (eFormat){
        case formatRGB:
        case formatHLS:
        case formatLAB:
        case formatYUV:
          return 3;
          
        case formatChroma:
          return 2;
          
        case formatGray:
        case formatMatrix:
        default:
          return 1;
      }
    }
  
    // }}}
  
  
    /// puts a string representation of format into the given stream
    std::ostream &operator<<(std::ostream &s,const format &f){
      if( ((int)f<0) || ((int)f)>=7) return s << "formatUnknown";
      static const char *fmts[7] = { 
        "formatGray",
        "formatRGB",
        "formatHLS",
        "formatYUV",
        "formatLAB",
        "formatChroma",
        "formatMatrix"
      };
      return s << fmts[f];
    }
    
    /// puts a string representation of depth into the given stream
    std::ostream &operator<<(std::ostream &s,const depth &d){
      if( ((int)d<0) || ((int)d)>=5) return s << "depthUnknown";
      static const char *depths[7] = { 
        "depth8u",
        "depth16s",
        "depth32s",
        "depth32f",
        "depth64f"
      };
      return s << depths[d];
      
    }
  
    /// puts a string representation of format into the given stream
    std::istream &operator>>(std::istream &s, format &f){
      char cs[7]={0};
      
      s >> cs[0]; 
      s.unget();
      if(cs[0] == 'f'){
        for(int i=0;i<6;++i)s>>cs[i];
        ICLASSERT(str(cs) == "format");
      }else{
        // nothing, this is just a compability mode for 
        // someone forgetting the format prefix!
      }
      std::fill(cs,cs+7,'\0');
      s >> cs[0];
      cs[0] = tolower(cs[0]);
      int rest = 2;
      std::string expect;
      switch(cs[0]){
        case 'g':
          rest = 3;
          expect="gray";
          f = formatGray;
          break;
        case 'r':
          expect="rgb";
          f = formatRGB;
          break;
        case 'h':
          expect="hls";
          f = formatHLS;
          break;
        case 'y':
          expect="yuv";
          f = formatYUV;
          break;
        case 'l':
          expect="lab";
          f = formatLAB;
          break;
        case 'c':
          rest = 5;
          f = formatChroma;
          expect="chroma";
          break;
        case 'm':
          rest = 5;
          f = formatMatrix;
          expect="matrix";
          break;
        default:
          ERROR_LOG("unable to parse format-type");
          return s;
      }
      for(int i=0;i<rest;++i){
        s >> cs[i+1];
        cs[i+1] = tolower(cs[i+1]);
      }
      if(expect != cs){
        ERROR_LOG("unabled t parse format: found: " << cs << " expected:" << expect);
      }
      return s;
    }
    
    /// puts a string representation of depth into the given stream
    std::istream &operator>>(std::istream &s, depth &d){
      char cs[6]={0};
      s >> cs[0];
      s.unget();
      if(cs[0] == 'd'){
        for(int i=0;i<5;++i) s>>cs[i];
        ICLASSERT(str(cs) == "depth");
      }else{
        // compability mode for someone forgetting the depth-prefix
      }
      std::fill(cs,cs+6,'\0');
      s >> cs[0];
      switch(cs[0]){
        case '8': 
          s >> cs[1];
          ICLASSERT(str(cs) == "8u");
          d = depth8u;
          return s;
        case '1':
          s >> cs[1] >> cs[2];
          ICLASSERT(str(cs) == "16s");
          d = depth16s;
          return s;
        case '3':
          s >> cs[1] >> cs[2];
          if(cs[2] == 's'){
            ICLASSERT(str(cs) == "32s");
            d = depth32s;
          }else{
            ICLASSERT(str(cs) == "32f");
            d = depth32f;
          }
          return s;
        case '6':    
          s >> cs[1] >> cs[2];
          ICLASSERT(str(cs) == "64f");
          d = depth64f;
          return s;
        default:
          ERROR_LOG("error parsing depth-type");
          return s;
      }    
    }
    
  
  
  
    ImgBase *ensureDepth(ImgBase **ppoImage, depth d){
      // {{{ open
      if(!ppoImage){
        return imgNew(d);
      }
      if(!*ppoImage){
        *ppoImage = imgNew(d);
      }
      else if((*ppoImage)->getDepth() != d){
        ImgBase *poNew = imgNew(d,(*ppoImage)->getParams());
        delete *ppoImage;
        *ppoImage = poNew;     
      }
      return *ppoImage;
    }
  
    // }}}
    
    ImgBase *ensureCompatible(ImgBase **ppoDst, depth d, const ImgParams &params){
      // {{{ open
      if(!ppoDst){
        return imgNew(d,params);
      }
      if(!*ppoDst){
        *ppoDst = imgNew(d,params);
      }else{
        ensureDepth(ppoDst,d);
        (*ppoDst)->setParams(params);
      }
      return *ppoDst;
    }
  
    // }}}
    
    ImgBase* ensureCompatible(ImgBase **dst, depth d, const Size &size, int channels, format fmt, const Rect &roi){
      // {{{ open
  
      FUNCTION_LOG("");
      if(fmt != formatMatrix && getChannelsOfFormat(fmt) != channels){
        ensureCompatible(dst,d,size,channels,roi);
        throw InvalidImgParamException("channels and format");
        return 0;
      }else{
        ImgBase *ret = ensureCompatible(dst,d,size,channels,roi);
        (*dst)->setFormat(fmt);
        return ret;
      }
    }
  
    // }}}
  
    ImgBase *ensureCompatible(ImgBase **dst, const ImgBase *src){
      // {{{ open
      return ensureCompatible(dst,src->getDepth(),src->getParams());
    }
    // }}}
  
    unsigned int getSizeOf(depth eDepth){
      // {{{ open
      static unsigned int s_aiSizeTable[]= { sizeof(icl8u),
                                             sizeof(icl16s),
                                             sizeof(icl32s),
                                             sizeof(icl32f),                                           
                                             sizeof(icl64f) };
      return s_aiSizeTable[eDepth];
    }
  
    // }}}

  namespace{
      template<class T>
      double channel_mean(const Img<T> &image, int channel, bool roiOnly){
        if(roiOnly && !image.hasFullROI()){
          return mean(image.beginROI(channel),image.endROI(channel));
        }else{
          return mean(image.begin(channel),image.end(channel));
        }
      }
  #ifdef HAVE_IPP
      template<> double channel_mean<icl8u>(const Img8u &image, int channel, bool roiOnly){
        icl64f m=0;
        ippiMean_8u_C1R(roiOnly ? image.getROIData(channel) : image.getData(channel),image.getLineStep(),
                        roiOnly ? image.getROISize() : image.getROISize(),&m);
      return m;
      }
      template<> double channel_mean<icl16s>(const Img16s &image, int channel, bool roiOnly){
        icl64f m=0;
        ippiMean_16s_C1R(roiOnly ? image.getROIData(channel) : image.getData(channel),image.getLineStep(),
                         roiOnly ? image.getROISize() : image.getROISize(),&m);
        return m;
      }
      template<> double channel_mean<icl32f>(const Img32f &image, int channel, bool roiOnly){
        icl64f m=0;
      ippiMean_32f_C1R(roiOnly ? image.getROIData(channel) : image.getData(channel),image.getLineStep(),
                       roiOnly ? image.getROISize() : image.getROISize(),&m, ippAlgHintAccurate);
      return m;
      }
  #endif
    }
  
    vector<double> mean(const ImgBase *poImg, int iChannel, bool roiOnly){
      FUNCTION_LOG("");
      vector<double> vecMean;
      ICLASSERT_RETURN_VAL(poImg,vecMean);
  
      int firstChannel = iChannel<0 ? 0 : iChannel;
      int lastChannel = iChannel<0 ? poImg->getChannels()-1 : firstChannel;
      
      switch(poImg->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                               \
        case depth##D:                                                           \
          for(int i=firstChannel;i<=lastChannel;++i){                            \
            vecMean.push_back(channel_mean(*poImg->asImg<icl##D>(),i,roiOnly));  \
          }                                                                      \
          break;
        ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
      }
      return vecMean;
    }
    
    
    
    // }}}
  
    // {{{ variance
  
   namespace{
      template<class T>
      double channel_var_with_mean(const Img<T> &image, int channel,double mean,bool empiricMean, bool roiOnly){
        if(roiOnly && !image.hasFullROI()){
          return variance(image.beginROI(channel),image.endROI(channel),mean,empiricMean);
        }else{
          return variance(image.begin(channel),image.end(channel),mean,empiricMean);
        }
      }
      // no IPP function available with given mean
    }
  
    vector<double> variance(const ImgBase *poImg, const vector<double> &mean, bool empiricMean,  int iChannel, bool roiOnly){
      FUNCTION_LOG("");
      vector<double> vecVar;
      ICLASSERT_RETURN_VAL(poImg,vecVar);
  
      int firstChannel = iChannel<0 ? 0 : iChannel;
      int lastChannel = iChannel<0 ? poImg->getChannels()-1 : firstChannel;
      
      switch(poImg->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D)                                                                           \
        case depth##D:                                                                                       \
          for(int i=firstChannel,j=0;i<=lastChannel;++i,++j){                                                \
            ICLASSERT_RETURN_VAL(j<(int)mean.size(),vecVar);                                                 \
            vecVar.push_back(channel_var_with_mean(*poImg->asImg<icl##D>(),i,mean[j],empiricMean,roiOnly));  \
          }                                                                                                  \
        break;
        ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
      }
      return vecVar;
    
    
    }
    
    /// Compute the variance value of an image a \ingroup MATH
    /** @param poImg input imge
        @param iChannel channel index (-1 for all channels)
        @return The variance value form the vector
        */
    vector<double> variance(const ImgBase *poImg, int iChannel, bool roiOnly){
      return variance(poImg,mean(poImg,iChannel,roiOnly),true,iChannel,roiOnly);
    }
    // }}}
    
    // {{{ std-deviation
  
    vector<double> stdDeviation(const ImgBase *poImage, int iChannel, bool roiOnly){
      vector<double> v = variance(poImage,iChannel,roiOnly);
      for(unsigned int i=0;i<v.size();++i){
        v[i] = ::sqrt(v[i]);
      }
      return v;
    }
  
    /// Compute the deviation of an image with given channel means
    /** @param poImage input image
        @param iChannel channel index (all channels if -1)
    */
    vector<double> stdDeviation(const ImgBase *poImage, const vector<double> mean, bool empiricMean, int iChannel, bool roiOnly){
      vector<double> v = variance(poImage,mean,empiricMean, iChannel,roiOnly);
  
      for(unsigned int i=0;i<v.size();++i){
        v[i] = ::sqrt(v[i]);
      }
      return v;
    }
    // }}}
  
    // {{{ mean-and-std-deviation
    vector< pair<double,double> > meanAndStdDev(const ImgBase *image, int iChannel, bool roiOnly){
      vector<double> channelMeans = mean(image,iChannel,roiOnly);
      vector<double> channelStdDevs = stdDeviation(image,channelMeans,true,iChannel,roiOnly);
  
      vector< pair<double,double> > md(channelMeans.size());
      for(unsigned int i=0;i<channelMeans.size();++i){
        md[i].first = channelMeans[i];
        md[i].second = channelStdDevs[i];
      }
      return md;
    }
    // }}}
  
    // {{{ histogramm functions
    
    namespace{
  
      template<class T>
      void compute_default_histo_256(const Img<T> &image, int c, vector<int> &h, bool roiOnly){
        ICLASSERT_RETURN(h.size() == 256);
        if(roiOnly && !image.hasFullROI()){
          const ImgIterator<T> it = image.beginROI(c);
          const ImgIterator<T> itEnd = image.endROI(c);
          for(;it!=itEnd;++it){
            h[clipped_cast<T,icl8u>(*it)]++;
          }
        }else{
          const T* p = image.getData(c);
          const T* pEnd = p+image.getDim();
          while(p<pEnd){
            h[clipped_cast<T,icl8u>(*p++)]++;
          }
        }
      }
      
     
      template<class T>
      inline void histo_entry(T v, double m, vector<int> &h, unsigned int n, double r){
        // todo check 1000 times
        h[ floor( n*(v-m)/(r+1)) ]++;
        //      h[ ceil( n*(v-m)/r) ]++; problem at v=255
      }
      
      template<class T>
      void compute_complex_histo(const Img<T> &image, int c, vector<int> &h, bool roiOnly){
        const Range<T> range = image.getMinMax(c);
        double r = range.getLength();
        unsigned int n = h.size();
  
        if(roiOnly && !image.hasFullROI()){
          const ImgIterator<T> it = image.beginROI(c);
          const ImgIterator<T> itEnd = image.endROI(c);
          for(;it!=itEnd;++it){
            histo_entry(*it,range.minVal,h,n,r);
          }
        }else{
          const T* p = image.begin(c);
          const T* pEnd = image.end(c);
          while(p<pEnd){
            histo_entry(*p++,range.minVal,h,n,r);
          }
        }
      }    
  
  #ifdef HAVE_IPP
      
  #define COMPUTE_DEFAULT_HISTO_256_TEMPLATE(D)                                                                                \
      template<> void compute_default_histo_256<icl##D>(const Img##D &image, int c, vector<int> &h, bool roiOnly){             \
        ICLASSERT_RETURN(h.size() == 256);                                                                                     \
        static icl32s levels[257];                                                                                             \
        static bool first = true;                                                                                              \
        if(first){                                                                                                             \
          for(int i=0;i<257;levels[i]=i,i++);                                                                                  \
          first = false;                                                                                                       \
        }                                                                                                                      \
                                                                                                                               \
        if(roiOnly && !image.hasFullROI()){                                                                                    \
          ippiHistogramEven_##D##_C1R(image.getROIData(c),image.getLineStep(),image.getROISize(),&h[0], levels, 257, 0,256);   \
        }else{                                                                                                                 \
          ippiHistogramEven_##D##_C1R(image.getData(c),image.getLineStep(),image.getSize(),&h[0], levels, 257, 0,256);         \
        }                                                                                                                      \
      }
      COMPUTE_DEFAULT_HISTO_256_TEMPLATE(8u)
      COMPUTE_DEFAULT_HISTO_256_TEMPLATE(16s)
      
  
  #define COMPUTE_COMPLEX_HISTO_TEMPLATE(D)                                                                                    \
      template<> void compute_complex_histo(const Img##D &image, int c, vector<int> &h, bool roiOnly){                         \
        Range<icl##D> range = image.getMinMax(c);                                                                              \
        double l = range.getLength();                                                                                          \
        vector<int> levels(h.size()+1);                                                                                        \
        for(unsigned int i=0;i<levels.size();i++){                                                                             \
          levels[i] = i*l/h.size() + range.minVal;                                                                             \
        }                                                                                                                      \
                                                                                                                               \
        if(roiOnly && !image.hasFullROI()){                                                                                    \
          ippiHistogramEven_##D##_C1R(image.getROIData(c),image.getLineStep(),image.getROISize(),                              \
                                       &h[0], &levels[0], levels.size(), range.minVal, range.maxVal+1 );                       \
        }else{                                                                                                                 \
          ippiHistogramEven_##D##_C1R(image.getData(c),image.getLineStep(),image.getSize(),                                    \
                                       &h[0], &levels[0], levels.size(), range.minVal, range.maxVal+1 );                       \
        }                                                                                                                      \
      }
  
  
      COMPUTE_COMPLEX_HISTO_TEMPLATE(8u) 
      COMPUTE_COMPLEX_HISTO_TEMPLATE(16s) 
      
  #endif
  
  
      template<class T>
      void compute_channel_histo(const Img<T> &image, int c, vector<int> &h, bool roiOnly){
        if(image.getFormat() != formatMatrix && h.size() == 256){
          compute_default_histo_256(image,c,h,roiOnly);
        }else{
          compute_complex_histo(image,c,h,roiOnly);
        }
      }
    }
    
    vector<int> channelHisto(const ImgBase *image,int channel, int levels, bool roiOnly){
      ICLASSERT_RETURN_VAL(image && image->getChannels()>channel, vector<int>());
      ICLASSERT_RETURN_VAL(levels > 1,vector<int>());
      
      vector<int> h(levels);
      switch(image->getDepth()){
  #define ICL_INSTANTIATE_DEPTH(D) case depth##D: compute_channel_histo(*image->asImg<icl##D>(),channel,h,roiOnly); break;
        ICL_INSTANTIATE_ALL_DEPTHS;
  #undef ICL_INSTANTIATE_DEPTH
      }
      return h;
    }
    
    
    vector<vector<int> > hist(const ImgBase *image, int levels, bool roiOnly){
      ICLASSERT_RETURN_VAL(image && image->getChannels(), vector<vector<int> >());
      vector<vector<int> > h(image->getChannels());
      for(int i=0;i<image->getChannels();i++){
        h[i] = channelHisto(image,i,levels,roiOnly);
      }
      return h;
    } 
  
    // }}}
  
  
  } // namespace core
} //namespace
