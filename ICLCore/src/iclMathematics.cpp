#include <iclMathematics.h>
#include <vector>
#include <fstream>
#include <sstream>


using namespace std;

namespace icl{

  // {{{ uniform Random 

  // this anonymous namespace holds utiliy functions, that are used only here
  namespace { 
    template<class T>
    void uniform_image_random(Img<T> *image, const Range<double> &range, bool roiOnly){
      for(int c=0;c<image->getChannels();++c){
        ImgIterator<T> it = roiOnly ? image->getROIIterator(c) : image->getIterator(c);
        while(it.inRegion()){
          *it++ = Cast<double,T>::cast( random(range.minVal, range.maxVal) );
        }
      }
    } 
    template<class T>
    void gaussian_image_random(Img<T> *image,double mean, double var, const Range<double> &range, bool roiOnly){
      for(int c=0;c<image->getChannels();++c){
        ImgIterator<T> it = roiOnly ? image->getROIIterator(c) : image->getIterator(c);
        while(it.inRegion()){
          *it++ = Cast<double,T>::cast( gaussRandom(mean,var,range) );
        }
      }
    }
  }
    
  void random(ImgBase *poImage, const Range<double> &range, bool roiOnly){
    ICLASSERT_RETURN( poImage );
    switch(poImage->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: uniform_image_random(poImage->asImg<icl##D>(),range,roiOnly); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
  
  // }}}

  // {{{ gaussian random

  void gaussRandom(ImgBase *poImage, double mean, double var, const Range<double> &minAndMax, bool roiOnly){
    ICLASSERT_RETURN( poImage );
    switch(poImage->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) case depth##D: gaussian_image_random(poImage->asImg<icl##D>(),mean,var,minAndMax,roiOnly); break;
      ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH
    }
  }
  
  //--------------------------------------------------------------------------
  double gaussRandom(double mu, double sigma){
    static bool haveNextGaussian = false;
    static double nextGaussian = 0;
    if(haveNextGaussian){
      haveNextGaussian = false;
      return nextGaussian*sigma + mu;
    } else{
      double v1(0), v2(0), s(0);
      do{
        v1 = 2 * random(1.0)-1;
        v2 = 2 * random(1.0)-1;
        s = v1*v1 + v2*v2;
      }while(s>=1 || s == 0);
      double fac = sqrt(-2.0*log(s)/s);
      nextGaussian = v2 * fac;
      haveNextGaussian = true;
      return v1 * fac * sigma + mu;
    }
  }   

  // }}}

  // {{{ mean

  namespace{
    template<class T>
    double channel_mean(const Img<T> &image, int channel, bool roiOnly){
      double sum = 0;
      if(roiOnly && !image.hasFullROI()){
        ConstImgIterator<T> it = image.getROIIterator(channel);
        while(it.inRegion()){
          sum += *it;
        }
        return sum/image.getROISize().getDim();
      }else{
        return mean(image.getData(channel),image.getData(channel)+image.getDim());
      }
    }
#ifdef WITH_IPP_OPTIMIZATION
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
      register double sum = 0;
      register double d = 0;
      if(roiOnly && !image.hasFullROI()){
        ConstImgIterator<T> it = image.getROIIterator(channel);
        while(it.inRegion()){
          d = *it - mean;
          sum += d*d;
          ++it;
        }
        return sum/(empiricMean ? iclMax(double(image.getROISize().getDim()-1),double(1)) : image.getROISize().getDim());
      }else{
        return variance(image.getData(channel),image.getData(channel)+image.getDim(),mean,empiricMean);
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
        ConstImgIterator<T> it = image.getROIIterator(c);
        for(;it.inRegion();++it){
          h[Cast<T,icl8u>::cast(*it)]++;
        }
      }else{
        const T* p = image.getData(c);
        const T* pEnd = p+image.getDim();
        while(p<pEnd){
          h[Cast<T,icl8u>::cast(*p++)]++;
        }
      }
    }
    
   
    template<class T>
    inline void histo_entry(T v, double m, vector<int> &h, unsigned int n, double r){
      // todo check 1000 times
      h[ ceil( n*(v-m)/(r+1)) ]++;
      //      h[ ceil( n*(v-m)/r) ]++; problem at v=255
    }
    
    template<class T>
    void compute_complex_histo(const Img<T> &image, int c, vector<int> &h, bool roiOnly){
      const Range<T> range = image.getMinMax(c);
      double r = range.getLength();
      unsigned int n = h.size();

      if(roiOnly && !image.hasFullROI()){
        ConstImgIterator<T> it = image.getROIIterator(c);
        for(;it.inRegion();++it){
          histo_entry(*it,range.minVal,h,n,r);
        }
      }else{
        const T* p = image.getData(c);
        const T* pEnd = p+image.getDim();
        while(p<pEnd){
          histo_entry(*p,range.minVal,h,n,r);
        }
      }
    }    

#ifdef WITH_IPP_OPTIMIZATION
    
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
  



} //namespace icl
  
