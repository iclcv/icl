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
    
    The ICLFilter package provides a large variety of image filtering classes. When talking about 
    <em>image filters</em> some misapprehensions can arise. To prevent this, the following section
    will shortly introduce our understanding of image filters.

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
