// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christian Groszewski, Christof Elbrechter

#include <icl/cv/OpenCVCamCalib.h>
#include <icl/core/OpenCV.h>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/persistence.hpp>

using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;

namespace icl::cv {
    struct OpenCVCamCalib::Data{
      int bWidth;
      int bHeight;
      int bSize;
      ::cv::Size imgSize;

      ::cv::Mat intrinsicMatrix;             // 3×3 K
      ::cv::Mat distortionCoeffs;            // 5×1 distortion (k1, k2, p1, p2, k3)
      std::vector<std::vector<::cv::Point2f>> imagePoints;   // per-board image-plane corners
      std::vector<std::vector<::cv::Point3f>> objectPoints;  // per-board world-plane corners (z = 0)

      Data(unsigned int boardWidth, unsigned int boardHeight, unsigned int /*boardCount*/) :
        bWidth(static_cast<int>(boardWidth)),
        bHeight(static_cast<int>(boardHeight)),
        bSize(bWidth * bHeight),
        intrinsicMatrix(::cv::Mat::eye(3, 3, CV_64F)),
        distortionCoeffs(::cv::Mat::zeros(5, 1, CV_64F)) {}
    };

    OpenCVCamCalib::OpenCVCamCalib(unsigned int boardWidth, unsigned int boardHeight,
                                   unsigned int boardCount) :
      m_data(new OpenCVCamCalib::Data(boardWidth, boardHeight, boardCount)) {}

    OpenCVCamCalib::~OpenCVCamCalib(){
      delete m_data;
    }

    void OpenCVCamCalib::resetData(int width, int height, int /*count*/){
      m_data->bWidth = width;
      m_data->bHeight = height;
      m_data->bSize = width * height;
      m_data->imagePoints.clear();
      m_data->objectPoints.clear();
      m_data->intrinsicMatrix = ::cv::Mat::eye(3, 3, CV_64F);
      m_data->distortionCoeffs = ::cv::Mat::zeros(5, 1, CV_64F);
    }

    DynMatrix<icl64f> *OpenCVCamCalib::getIntrinsics(){
      DynMatrix<icl64f> *intr = new DynMatrix<icl64f>(3, 3);
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
          intr->at(i, j) = m_data->intrinsicMatrix.at<double>(static_cast<int>(j), static_cast<int>(i));
      return intr;
    }

    DynMatrix<icl64f> *OpenCVCamCalib::getDistortion(){
      DynMatrix<icl64f> *dist = new DynMatrix<icl64f>(1, 5);
      for (unsigned int i = 0; i < 5; ++i)
        dist->at(0, i) = m_data->distortionCoeffs.at<double>(static_cast<int>(i), 0);
      return dist;
    }

    int OpenCVCamCalib::addPoints(const ImgBase *img){
      if (!img) return static_cast<int>(m_data->imagePoints.size());

      ::cv::Mat tmp;
      img_to_mat(img, &tmp);

      ::cv::Mat gray;
      if (tmp.channels() == 3) {
        ::cv::cvtColor(tmp, gray, ::cv::COLOR_BGR2GRAY);
      } else {
        gray = tmp;
      }
      m_data->imgSize = gray.size();

      const ::cv::Size patternSize(m_data->bWidth, m_data->bHeight);
      std::vector<::cv::Point2f> corners;
      const bool found = ::cv::findChessboardCorners(
          gray, patternSize, corners,
          ::cv::CALIB_CB_ADAPTIVE_THRESH | ::cv::CALIB_CB_FILTER_QUADS);
      if (!found || static_cast<int>(corners.size()) != m_data->bSize) {
        return static_cast<int>(m_data->imagePoints.size());
      }

      ::cv::cornerSubPix(gray, corners, ::cv::Size(11, 11), ::cv::Size(-1, -1),
                       ::cv::TermCriteria(::cv::TermCriteria::EPS | ::cv::TermCriteria::COUNT, 30, 0.1));

      // build matching object-plane points (z = 0, integer grid)
      std::vector<::cv::Point3f> objPts;
      objPts.reserve(m_data->bSize);
      for (int j = 0; j < m_data->bSize; ++j) {
        objPts.emplace_back(static_cast<float>(j / m_data->bWidth),
                            static_cast<float>(j % m_data->bWidth),
                            0.0f);
      }

      m_data->imagePoints.push_back(std::move(corners));
      m_data->objectPoints.push_back(std::move(objPts));
      return static_cast<int>(m_data->imagePoints.size());
    }

    void OpenCVCamCalib::calibrateCam(){
      if (m_data->imagePoints.empty()) return;

      m_data->intrinsicMatrix.at<double>(0, 0) = 1.0;
      m_data->intrinsicMatrix.at<double>(1, 1) = 1.0;

      std::vector<::cv::Mat> rvecs, tvecs;
      ::cv::calibrateCamera(m_data->objectPoints, m_data->imagePoints, m_data->imgSize,
                          m_data->intrinsicMatrix, m_data->distortionCoeffs,
                          rvecs, tvecs, ::cv::CALIB_FIX_ASPECT_RATIO);
    }

    ImgBase *OpenCVCamCalib::undisort(const ImgBase *img){
      m_data->imgSize.width = img->getSize().width;
      m_data->imgSize.height = img->getSize().height;

      ::cv::Mat src;
      img_to_mat(img, &src);

      ::cv::Mat dst;
      ::cv::undistort(src, dst, m_data->intrinsicMatrix, m_data->distortionCoeffs);

      return mat_to_img(&dst);
    }

    void OpenCVCamCalib::loadParams(const char* filename){
      ::cv::FileStorage fs(filename, ::cv::FileStorage::READ);
      if (!fs.isOpened()) {
        throw ICLException(std::string("OpenCVCamCalib::loadParams: cannot open ") + filename);
      }
      fs["intrinsics"] >> m_data->intrinsicMatrix;
      fs["distortion"] >> m_data->distortionCoeffs;
    }

    void OpenCVCamCalib::saveParams(const char* filename){
      ::cv::FileStorage fs(filename, ::cv::FileStorage::WRITE);
      if (!fs.isOpened()) {
        throw ICLException(std::string("OpenCVCamCalib::saveParams: cannot open ") + filename);
      }
      fs << "intrinsics" << m_data->intrinsicMatrix;
      fs << "distortion" << m_data->distortionCoeffs;
    }

  } // namespace icl::cv
