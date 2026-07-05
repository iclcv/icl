// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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

    - <b>Utils</b> icl::utils::ConfigFile, icl::utils::Configurable,
      icl::utils::ProgArg, std::shared_ptr, icl::utils::Thread
    - <b>Math</b> icl::math::DynMatrix, icl::math::FixedMatrix, icl::math::Octree,
      icl::math::QuadTree, icl::math::LLM, icl::math::PolynomialRegression,
      icl::math::RansacFitter, icl::math::LevenbergMarquardtFitter,
      icl::math::SimplexOptimizer, icl::math::SOM
    - <b>Core</b> icl::core::ImgBase, icl::core::ImgBase, icl::core::Converter, icl::core::Line,
      icl::core::Line32f, icl::core::LineSampler, icl::core::cc
    - <b>Filter</b> icl::filter::UnaryOp, icl::filter::BinaryOp, icl::filter::PseudoColorOp
    - <b>Io</b> icl::io::ImageSource, icl::io::ImageSink
    - <b>Qt</b> icl::qt::GUI, icl::qt::ICLApplication, icl::qt::ICLWidget,
      icl::qt::ICLDrawWidget, icl::qt::ICLDrawWidget3D, icl::qt::PlotWidget, icl::qt::QImageConverter
    - <b>Cv</b> icl::cv::FloodFiller, icl::cv::GenericSurfDetector, icl::cv::HoughLineDetector
      icl::cv::RegionDetector, icl::cv::MeanShiftTracker, icl::cv::VectorTracker,
      icl::cv::RunLengthEncoder, icl::cv::SimpleBlobSearcher, icl::cv::TemplateTracker
    - <b>Cv3d</b> (Qt-free 3D vision) icl::cv3d::Camera, icl::cv3d::ICP,
      icl::cv3d::PlaneEquation, icl::cv3d::RigidTransformEstimator, icl::cv3d::PlanarPoseEstimator,
      icl::cv3d::PointCloudNormalEstimator, icl::cv3d::Segmentation3D,
      icl::cv3d::ObjectEdgeDetector
    - <b>Viz3d</b> (3D scene/render) icl::viz3d::Scene, icl::viz3d::PlotWidget3D,
      icl::viz3d::MeshNode, icl::viz3d::Loader
    - <b>Markers</b> icl::markers::FiducialDetector
    */
