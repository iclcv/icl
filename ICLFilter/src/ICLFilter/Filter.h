/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/Filter.h                       **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
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

/** \defgroup UNARY Collection of Unary Operations
    \defgroup BINARY Collection of Binary Operations
    \defgroup AFFINE Collection of Affine Image Operations
    \defgroup NBH Collection of Neighborhood Operations
    \defgroup INPLACE Collection of Inplace Operations
    \defgroup OTHER Other Classes
    \section OVERVIEW Overview

    The ICLFilter package provides a large variety of image filtering classes. Here's is an incomplete list:
    <table border=0><tr><td>
    - Generic and fixed size convolution (see icl::filter::ConvolutionOp)
    - Median operator (see icl::MedianOp)
    - Morphological operations (see icl::filter::MorphologicalOp)
    - Affine operation (see icl::filter::AffineOp)
    - General Gabor-filters (see icl::filter::GaborOp)
    - Image chamfering filters (see icl::filter::ChamferOp)
    - Threshold operations (see icl::filter::ThresholdOp)
    </td><td>
    - Lookup-table filters (see icl::filter::LUTOp)
    - Arithmetical operations (see icl::filter::UnaryArithmeticalOp)
    - Logical operations (see icl::filter::UnaryLogicalOp)
    - Filters for combining image channels (see icl::filter::WeightChannelsOp)
    - Wiener filer (see icl::filter::WienerOp)
    - Local threshold operations (see icl::filter::LocalThresholdOp)
    - Image proximity measurement (see icl::filter::ProximityOp)
    </td></tr></table>

    When talking about <em>image filters</em> some misapprehensions can arise. To prevent this, the following section
    will shortly introduce our understanding of image filters.


    \image html filter-array-demo.png "The icl-filter-array demo application can be used to examine filter chains"

    \section FILTERS What are Image-Filters

    In a most general view on image filters, a filter is a <em>black box</em> that has N image inputs
    and M image outputs. However this approach provides a very generic interface for image filters,
    this interface is not very feasible. Most common filters (e.g. binary image operations, linear
    filters or neighborhood operations) only need a single input and output image. Another larger group
    are filters with two input and on output images (e.g. arithmetical/logical per-pixel image operations
    or image comparison filters).\n
    To avoid a large computational overhead arising of a <em>too</em> general interface, we support
    two dedicated filtering interfaces for the above mentioned 1-1 and 2-1 input-output combinations. To
    obviate further misunderstandings, we call this filter sets <b>Unary Operations</b> and
    <b>Binary-Operations</b> - or short UnaryOp and BinaryOp. \n
    Each of this sets is represented by a equal named C++-class-interface, which is inherited by all implemented
    filters that have a corresponding input-output relation (from now on we use <em>Ops</em>(for operations).\n

    For a better overview, the ICLFilter package is grouped into some modules:

    -# \ref UNARY
    -# \ref BINARY
    -# \ref AFFINE
    -# \ref NBH
    -# \ref INPLACE
    -# \ref OTHER

    **/
