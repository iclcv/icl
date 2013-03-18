/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/DoxygenMainPage.h                  **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
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

/** \cond file only contains the doxygen mainpage for the ICL library \endcond
    
    \mainpage ICL Modules

    ICL is split over a set of modules, each providing a special set
    datatypes, classes and functions for the development of computer vision
    applications. All modules come up with an extra namespace that is
    embedded to the global ::icl namespace

    - The ::icl::utils module provides general support functions, classes and types\n
    - The ::icl::math module provides matrix classes and several machine learning tools\n
    - The ::icl::core module provides basic image templates ImgBase and Img and further basic types and functions
    - The ::icl::filter module provides a huge set of unary- and binay operators for low level image processing
    - The ::icl::io module provides a versatile image in- and output-framework
    - The ::icl::qt module provides a powerful GUI-create framework as well as high speed image visualization tools
    - The ::icl::cv module provides a huge set of medium-level tools for computer-vision
    - The ::icl::geom module provides a functions and classes for 3D vision and visualization
    - The ::icl::markers module provides a full featuered highspeed fiducial marker tracking library

    \section SC Shortcuts
    
    - <b>Utils</b> icl::utils::ConfigFile, icl::utils::Configurable, icl::utils::Function, 
      icl::utils::ProgArg, icl::utils::SmartPtr, icl::utils::Thread 
    - <b>Math</b> icl::math::DynMatrix, icl::math::FixedMatrix, icl::math::Octree, 
      icl::math::QuadTree, icl::math::LLM, icl::math::PolynomialRegression, 
      icl::math::RansacFitter, icl::math::LevenbergMarquardtFitter, 
      icl::math::SimplexOptimizer, icl::math::SOM
    - <b>Core</b> icl::core::ImgBase, icl::core::ImgBase, icl::core::Converter, icl::core::Line,
      icl::core::Line32f, icl::core::LineSampler, icl::core::PseudoColorConverter, icl::core::cc
    - <b>Filter</b> icl::filter::UnaryOp, icl::filter::BinaryOp
    - <b>Io</b> icl::io::GenericGrabber, icl::io::GenericImageOutput
    - <b>Qt</b> icl::qt::GUI, icl::qt::ICLApplication, icl::qt::ICLWidget, 
      icl::qt::ICLDrawWidget, icl::qt::ICLDrawWidget3D, icl::qt::PlotWidget, icl::qt::QImageConverter
    - <b>Cv</b> icl::cv::FloodFiller, icl::cv::GenericSurfDetector, icl::cv::HoughLineDetector
      icl::cv::RegionDetector, icl::cv::MeanShiftTracker, icl::cv::VectorTracker, 
      icl::cv::RunLengthEncoder, icl::cv::SimpleBlobSearcher, icl::cv::TemplateTracker
    - <b>Geom</b> icl::geom::Camera, icl::geom::Scene, icl::geom::SceneObject, icl::geom::ICP,
      icl::geom::PlaneEquation, icl::geom::PlotWidget3D, icl::geom::PointCloudCreator,
      icl::geom::PointNormalEstimation, icl::geom::PoseEstimator, icl::geom::Posit,
      icl::geom::Segmentation3D
    - <b>Markers</b> icl::markers::FiducialDetector
    */
