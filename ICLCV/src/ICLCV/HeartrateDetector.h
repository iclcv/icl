/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/src/ICLCV/HeartrateDetector.h                    **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLCore/Img.h>
#include <ICLQt/PlotWidget.h>
namespace icl{
  namespace cv{


    /// Heartrate Detector
    /** Implementation of a simple heartrate detector using video material of a face.

        The algorithm takes images cropped to the same part of the face as an input and
        estimates the heartrate using a Fast Fourier Transformation over the average green
        value of the past few frames.

        Parameters for the algorithm are:
      - <b>framrate</b>
        denotes the framerate of the source material so the algorithm
        can correctly estimate the heartrate.
      - <b>historyDepth</b>
        denotes how many frames are sampled before the algorithm starts
        estimating the heartrate. Using a larger historyDepth allows for
        a more fine grained estimation of the heartrate, but introduces a
        bigger delay before a first estimate can be given.

        usage example:
        \code
        icl::cv::HeartrateDetector detector(30,512);
        detector.addImage(image);
        float heartrate = detector.getHeartrate();
        \endcode
    **/
    class ICLCV_API HeartrateDetector : public utils::Uncopyable {

      struct Data;  //!< internal data structure
      Data *m_data; //!< internal data pointer

      public:
      /// Default constructor with given arguments.
      HeartrateDetector(int framerate = 30, int historyDepth = 512);

      /// Add an image to the detector to use in the estimation.
      void addImage(const core::Img8u &image);
      /// returns the current estimate for the heartrate. Will return 0
      /// if no enough enough samples have been provided.
      float getHeartrate() const;
      /// returns a SeriesBuffer for visualization of the frequencies.
      qt::PlotWidget::SeriesBuffer getFrequencies() const;
      /// returns a SeriesBuffer for visualization of the averaged
      /// frequencies over the last second.
      qt::PlotWidget::SeriesBuffer getAveragedFrequencies() const;
      /// returns a buffer that marks the current window of frequencies
      /// that is considered for the estimation of the heartrate.
      qt::PlotWidget::SeriesBuffer getWindowBuffer() const;
      /// returns the framerate
      int getFramerate() const;
      /// returns the history depth
      int getHistoryDepth() const;

      /// Destructor
      virtual ~HeartrateDetector();

    };

  } // namespace cv
}


