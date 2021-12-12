/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/HeartrateDetector.cpp                  **
** Module : ICLCV                                                  **
** Authors: Matthias Esau                                          **
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

#include <ICLCV/HeartrateDetector.h>
#include <ICLMath/FFTUtils.h>
#include <climits>

namespace icl{
  namespace cv{

    static const int SECONDS_IN_MINUTE = 60;
    static const int MINIMUM_HEARTRATE = 40;
    static const int MAXIMUM_HEARTRATE = 200;
    static const float DATA_SCALE_FACTOR = 200;
    static const float MIN_WINDOW_RADIUS = 5;
    static const float WINDOW_RADIUS_WIDEN = 3;
    static const float WINDOW_RADIUS_TIGHTEN = 0.5;

    struct HeartrateDetector::Data{
      template<typename T>
      class CircularBuffer {
        const std::size_t bufferSize;
        int index;
        std::size_t currentSize;
        std::vector<T> buffer;
      public:
        CircularBuffer(std::size_t size): bufferSize(size), index(-1), currentSize(0), buffer(bufferSize) {}

        void push(T t) {
          index = (index+1)%bufferSize;
          buffer[index] = t;
          currentSize = std::min(currentSize+1,bufferSize);
        }

        CircularBuffer &operator<<(T t){
          push(t);
          return *this;
        }

        T& operator[](std::size_t idx) {
          return buffer[idx];
        }

        const T& operator[](std::size_t idx) const {
          return buffer[idx];
        }

        T& back() {
          return buffer[index];
        }

        const T& back() const {
          return buffer[index];
        }

        std::size_t size() {
          return currentSize;
        }

        T* data() {
          return buffer.data();
        }

        ~CircularBuffer(){}
      };


      const int framerate, historyDepth, minIndex, maxIndex, heartrateRange;
      int imageCounter, windowLeft, windowRight;
      float windowRadius, previousHeartrate, currentHeartrate;
      CircularBuffer<std::vector<float> > frequencyHistory;
      std::vector<float> averagedFrequencies;
      CircularBuffer<float> averageDataBuffer;

      Data(int framerate, int historyDepth):framerate(framerate), historyDepth(historyDepth),
                                            minIndex(heartrateToIndex(MINIMUM_HEARTRATE)),
                                            maxIndex(heartrateToIndex(MAXIMUM_HEARTRATE)),
                                            heartrateRange(maxIndex-minIndex), imageCounter(0),
                                            windowLeft(minIndex), windowRight(maxIndex),
                                            windowRadius(heartrateRange), previousHeartrate(0), currentHeartrate(0),
                                            frequencyHistory(framerate), averagedFrequencies(heartrateRange),
                                            averageDataBuffer(historyDepth){}

      float indexToHeartrate(int i) {
        if(imageCounter != 0)
          return float(i)/historyDepth*framerate*SECONDS_IN_MINUTE;
        return 0;
      }

      int heartrateToIndex(float h) {
        return h*historyDepth/float(framerate)/float(SECONDS_IN_MINUTE);
      }

      void addImage(const core::Img8u &image) {
        float averageValue=0;
        for(core::ImgIterator<icl8u> it = image.beginROI(1); it != image.endROI(1); ++it) {
          averageValue += *it;
          *it >>= 1;
        }
        averageValue /= image.getROI().getDim()*UCHAR_MAX;
        averageDataBuffer<<averageValue;
        ++imageCounter;
      }

      void calculateHeartrate() {
        std::complex<float> *frequencies =math::fft::fft<float,float>(averageDataBuffer.size(),averageDataBuffer.data());
        frequencyHistory<<std::vector<float>(heartrateRange);
        for(int i = minIndex; i < maxIndex; ++i) {
            float magnitude = sqrt(frequencies[i].real()*frequencies[i].real()+frequencies[i].imag()*frequencies[i].imag());
            magnitude *= sqrt(i); //flatten the curve for more useful values
            frequencyHistory.back()[i-minIndex] = magnitude;
        }
        delete frequencies;

        float max = 0;
        int maxId = 0;
        for(unsigned int i = 0; i < averagedFrequencies.size(); ++i) {
          averagedFrequencies[i] = 0;
          for(unsigned int j = 0; j < frequencyHistory.size(); ++j) {
            averagedFrequencies[i] += frequencyHistory[j][i];
          }
          averagedFrequencies[i] /= frequencyHistory.size();

          int gi = i+minIndex;
          if(averagedFrequencies[i] > max && gi >= windowLeft && gi <= windowRight) {
              max = averagedFrequencies[i];
              maxId = gi;
          }
        }
        previousHeartrate = currentHeartrate;
        currentHeartrate = indexToHeartrate(maxId);
      }

      void updateWindow() {
        if((currentHeartrate-previousHeartrate)>0.01*currentHeartrate) {
          windowRadius=std::min(windowRadius+WINDOW_RADIUS_WIDEN, float(heartrateRange));
        } else {
          windowRadius=std::max(windowRadius-WINDOW_RADIUS_TIGHTEN, MIN_WINDOW_RADIUS);
        }
        int heartrateIndex = heartrateToIndex(currentHeartrate);
        windowLeft = std::max(int(heartrateIndex-windowRadius), minIndex);
        windowRight = std::min(int(heartrateIndex+windowRadius), maxIndex);
      }

      ~Data(){}
    };

    HeartrateDetector::HeartrateDetector(int framerate, int historyDepth) {
      m_data = new Data(framerate,historyDepth);
    }

    HeartrateDetector::~HeartrateDetector() {
      delete m_data;
    }

    void HeartrateDetector::addImage(const core::Img8u &image) {
      m_data->addImage(image);
      m_data->calculateHeartrate();
      if(m_data->imageCounter >= m_data->historyDepth) {
          m_data->updateWindow();
      }
    }

    float HeartrateDetector::getHeartrate() const {
      if(m_data->imageCounter >= m_data->historyDepth) {
        return m_data->currentHeartrate;
      } else {
        return 0;
      }
    }


    qt::PlotWidget::SeriesBuffer HeartrateDetector::getFrequencies() const {
      qt::PlotWidget::SeriesBuffer frequencies(m_data->heartrateRange);
      for(unsigned int i = 0; i < m_data->averagedFrequencies.size(); ++i) {
        frequencies[i] = m_data->frequencyHistory.back()[i]*DATA_SCALE_FACTOR/float(m_data->historyDepth);
      }
      return frequencies;
    }

    qt::PlotWidget::SeriesBuffer HeartrateDetector::getAveragedFrequencies() const {
      qt::PlotWidget::SeriesBuffer frequencies(m_data->heartrateRange);
      for(unsigned int i = 0; i < m_data->averagedFrequencies.size(); ++i) {
        frequencies[i] = m_data->averagedFrequencies[i]*DATA_SCALE_FACTOR/float(m_data->historyDepth);
      }
      return frequencies;
    }

    qt::PlotWidget::SeriesBuffer HeartrateDetector::getWindowBuffer() const {
      qt::PlotWidget::SeriesBuffer window(m_data->heartrateRange);
      for(unsigned int i = 0; i < window.size(); i++) {
        int gi = i+m_data->minIndex;
        window[i] = gi>=m_data->windowLeft&&gi<=m_data->windowRight?1000000:0;
      }
      return window;
    }

    int HeartrateDetector::getFramerate() const {
      return m_data->framerate;
    }

    int HeartrateDetector::getHistoryDepth() const {
      return m_data->historyDepth;
    }

  } // namespace cv
}
