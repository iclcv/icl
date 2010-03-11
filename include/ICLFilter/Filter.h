/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_FILTER_H
#define ICL_FILTER_H

/** \defgroup UNARY Collection of Unary Operations
    \defgroup BINARY Collection of Binary Operations
    \defgroup AFFINE Collection of Affine Image Operations
    \defgroup NBH Collection of Neighborhood Operations
    \defgroup INPLACE Collection of Inplace Operations
    \defgroup OTHER Other Classes    
    
    \mainpage ICLFilter package
    
    \section OVERVIEW Overview
    
    The ICLFilter package provides a large variety of image filtering classes. Here's is an incomplete list:
    <table border=0><tr><td>
    - Generic and fixed size convolution (see icl::ConvolutionOp)
    - Median operator (see icl::MedianOp)
    - Morphological operations (see icl::MorphologicalOp)
    - Affine operation (see icl::AffineOp)
    - General Gabor-filters (see icl::GaborOp)
    - Image chamfering filters (see icl::ChamferOp)
    - Threshold operations (see icl::ThresholdOp)
    </td><td>
    - Lookup-table filters (see icl::LUTOp)
    - Arithmetical operations (see icl::UnaryArithmeticalOp)
    - Logical operations (see icl::UnaryLogicalOp)
    - Filters for combining image channels (see icl::WeightChannelsOp)
    - Wiener filer (see icl::WienerOp)
    - Local threshold operations (see icl::LocalThresholdOp)
    - Image proximity measurement (see icl::ProximityOp)
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
#endif
