/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLCore/Mathematics.h                          **
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
*********************************************************************/

#ifndef ICL_MATH_H
#define ICL_MATH_H

#include <ICLCore/Img.h>
#include <ICLCore/ImgIterator.h>
#include <vector>
#include <algorithm>
#include <cmath>
#ifdef HAVE_IPP
#include <ipps.h>
#endif

#include <ICLCore/Random.h>
namespace icl {
  
                              
  /* {{{ distance functions */

  /*!
    @brief Calculate the euclidian distance of two vectors v1 and v2 \ingroup MATH
    @param v1Begin first element of v1
    @param v1End   end of v1 (points the first element behind v1)
    @param v2Begin first element of v2
    @return The euclidian distance |v1-v2|
  */
  template <class ForwardIterator> 
  inline float euclidian(ForwardIterator v1Begin, ForwardIterator v1End,
                  ForwardIterator v2Begin) {
     float fSum = 0.0, fDiff;
     for (; v1Begin != v1End; ++v1Begin, ++v2Begin) {
        fDiff = (*v1Begin-*v2Begin);
        fSum += fDiff*fDiff;
     }
     return ::sqrt(fSum);
  }

  /*!
    @brief Calculate the euclidian distance of points a and b \ingroup MATH
    @param a The first point
    @param b The second point
    @return The distance of point a and b
  */
  template <class T>
  inline float euclidian(const std::vector<T> &a, const std::vector<T> &b) {
     ICLASSERT_RETURN_VAL(a.size() == b.size(), float(0));
     return euclidian (a.begin(), a.end(), b.begin());
  }
     
/* }}} */
                              
  /* {{{ statistic functions */

  /* {{{ mean  */

  /// compute mean value of a data range \ingourp MATH
  /** @param begin start iterator 
      @param end end iterator*/
  template <class ForwardIterator>
  inline double mean(ForwardIterator begin, ForwardIterator end){
    if(begin == end) return 0;
    double sum = 0;
    int num = 0;
    while(begin != end){
      sum += *begin++;
      num++;
    }
    return sum / num;
  }
  
#ifdef HAVE_IPP
  template<> inline double mean<const icl32f*>(const icl32f *begin,const icl32f *end){
    icl32f m = 0;
    // More fast: ippAlgHintFast
    ippsMean_32f(begin,end-begin,&m,ippAlgHintAccurate);
    return m;
  }
  template<> inline double mean<const icl64f*>(const icl64f *begin,const icl64f *end){
    icl64f m = 0;
    ippsMean_64f(begin,end-begin,&m);
    return m;
  }
  // Scalfactor version not used ippsMean_16s_Sfs(const Ipp16s* pSrc, int len, Ipp16s* pMean, int scaleFactor)
  // Scalfactor version not used ippsMean_32s_Sfs(const Ipp32s* pSrc, int len, Ipp32s* pMean, int scaleFactor)
#endif
  
  

  /// Computes the mean value of a ImgBase* ingroup MATH
  /** IPP-Optimized for icl32f and icl64f
      @param poImg input image
      @param iChannel channel index (-1 for all channels)
      @return mean value of image or image channel (optionally: roi)
  */
  std::vector<double> mean(const ImgBase *poImg, int iChannel=-1, bool roiOnly=false);

/* }}} */
  
  /* {{{ variance  */



  /// Compute the variance of a given data range with given mean value \ingroup MATH
  /** @param begin start iterator
      @param end end iterator
      @param mean mean value of the range
      @param empiricMean if true, sum of square distances is devidec by n-1 else by n
  */
  template <class ForwardIterator>
  inline double variance(ForwardIterator begin, ForwardIterator end, double mean, bool empiricMean=true){
    if(begin == end) return 0;
    register double sum = 0;
    register double d = 0;
    int num = 0;
    while(begin != end){
      d = *begin - mean;
      sum += d*d;
      ++begin;
      num++;
    }
    return d/(empiricMean&&num>1 ? num - 1 : num); 
  }

  /// Compute the variance of a given data range \ingroup MATH
  /** @param begin start ForwardIterator
      @param end end ForwardIterator
  */
  template <class ForwardIterator>
  inline double variance(ForwardIterator begin, ForwardIterator end){
    return variance(begin,end,mean(begin,end),true);
  }


  /// Compute the variance value of an image a with given mean \ingroup MATH
  /** @param poImg input imge
      @param mean vector with channel means
      @param empiricMean if true, sum of square distances is devidec by n-1 else by n
      @param iChannel channel index (-1 for all channels)
      @return The variance value form the vector
  */
  std::vector<double> variance(const ImgBase *poImg, const std::vector<double> &mean, bool empiricMean=true,  int iChannel=-1, bool roiOnly=false);
  
  /// Compute the variance value of an image a \ingroup MATH
  /** @param poImg input imge
      @param iChannel channel index (-1 for all channels)
      @return The variance value form the vector
      */
  std::vector<double> variance(const ImgBase *poImg, int iChannel=-1, bool roiOnly=false); 



/* }}} */

  /* {{{ standard-deviation  */

  /// Compute std-deviation of a data set with given mean (calls sqrt(variance(..))
  /** @param begin start iterator
      @param end end iterator
      @param mean given mean value
      @param empiricMean if true, sum of square distances is devidec by n-1 else by n
  */
  template <class ForwardIterator>
  inline double stdDeviation(ForwardIterator begin, ForwardIterator end, double mean, bool empiricMean=true){
    return ::sqrt(variance(begin,end,mean,empiricMean));
  }

  /// Compute std-deviation of a data set
  /** @param begin start iterator
      @param end end iterator
      */
  template <class ForwardIterator>
  inline double stdDeviation(ForwardIterator begin, ForwardIterator end){
    return ::sqrt(variance(begin,end));
  }

  /// Compute the std::deviation of an image
  /** @param poImage input image
      @param iChannel channel index (all channels if -1)
  */
  std::vector<double> stdDeviation(const ImgBase *poImage, int iChannel=-1, bool roiOnly = false);

  /// Compute the std::deviation of an image with given channel means
  /** @param poImage input image
      @param iChannel channel index (all channels if -1)
  */
  std::vector<double> stdDeviation(const ImgBase *poImage, const std::vector<double> mean, bool empiricMean=true, int iChannel=-1, bool roiOnly = false);


  /// Calculates mean and standard deviation of given data range simultanously
  /** @param begin start iterator
      @param end end iterator
      @return pair p with p.first = mean and p.second = stdDev
  */
  template<class ForwardIterator>
  inline std::pair<double,double> meanAndStdDev(ForwardIterator begin,ForwardIterator end){
    std::pair<double,double> md;
    md.first = mean(begin,end);
    md.second = stdDeviation(begin,end,md.first,true);
    return md;
  }

  /// Calculates mean and standard deviation of given image simultanously
  /** @param image input image
      @param iChannel image channel if < 0 all channels are used
      @return vector v of pairs p with p.first = mean and p.second = stdDev v[i] containing i-th channel's results
  */
  std::vector< std::pair<double,double> > meanAndStdDev(const ImgBase *image, int iChannel=-1, bool roiOnly = false);
  

/* }}} */

/* }}} */
                               
  /* {{{ histogramm functions */

  /// computes the color histogramm of given image channel                               
  std::vector<int> channelHisto(const ImgBase *image,int channel, int levels=256, bool roiOnly=false);
  
  /// computes the color histogramm of given image
  std::vector<std::vector<int> > hist(const ImgBase *image, int levels=256, bool roiOnly=false);

  /* }}} */                             
                               
} //namespace icl

#endif
