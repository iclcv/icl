/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLMarkers/Markers.h                           **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_MARKERS_H
#define ICL_MARKERS_H

#include <ICLMarkers/FiducialDetector.h>

/** 
    \defgroup PLUGINS FiducialDetector plugins
    
    \mainpage ICLMarkers - A package for general Marker Detection
    
    The ICLMarkers package defines general interfaces for most different types of 
    marker detection methods. The essential classes of this package are
    - icl::FiducialDetector (for detection different types of markers in images)
    - icl::Fiduical (a list of instances of this type is returned by the fiducial
      detection step
    
    <b>Note:</b> We use the words "fiducial" and "marker" or "image marker" as
    synonyms.
    
    The FiducialDetector implements a plugin-based system which can be configured
    at instantiation type in order to use a certain marker detection backend.
    Dependent on the selected backend, the detection markers (of type icl::Fiducial)
    provide a different set of information -- some can only provide 2D information, 
    others do also provide 3D pose information. Also dependent on the chosen plugin
    type, markers have to be loaded in a certain way that is also generalized by
    single method FiducialDetector::loadMarkers.

    \section SPEED Speed
    Internally, most detection steps are optimized for beeing fast on large images
    (>= VGA resolution),i.e. usually the plugins try to get away from the pixel
    representation by switching to binarized image regions. Benchmarks for different
    types of input image sizes and marker types are given in the \ref BENCH section

    \section SUPPORTED_MARKER_TYPES Supported Marker Types
    
    <TABLE border=0>
    <TR>
    <TD>\image html art.png</TD>
    <TD>\image html bch.png</TD>
    <TD>\image html amoeba.png</TD>
    </TR>
    <TR>
    <TD>"art" marker</TD>
    <TD>"bch" marker</TD>
    <TD>"amoeba" marker</TD>
    </TR>
    </TABLE>
    
    - "art"\n
      for ARToolkit markers (visit the 
      <a href="http://www.hitl.washington.edu/artoolkit/">ARToolkit Webpage</a>
      for more details)
      ART Fiducials can provide the full set of information including the 3D pose
      from a single marker
    - "bch"\n
      for ARToolkit+/Studierstube-Tracker like bch-code based binary markers
      (visit the <a href="http://studierstube.icg.tugraz.at/handheld_ar/stbtracker.php">
      Studierstube Tracker Homepage</a> for more details)
      BCH Fiducials can provide the full set of information including the 3D pose
      from a single marker. Usually, the detection rate is faster and more accurate
      in comparison to artoolkit markers. 
    - "amoeba" }\n
      for The amoeba style hierarchical markers provided by the
      <a href="http://reactivision.sourceforge.net/">reactivision software</a>
      Amoeba fiducials cannot be detected in 3D. They do only provide 2D center and
      rotation and the 2D boundary. 
    - "icl1" 
          for ICL's former 'paper' markers <b>not yet supported, but comming soon :-)</a>
    
    \section BENCH Benchmarks
    
    \subsection MT Multithreading

    We did not speed up the algorithms using multithreading techniques since usually
    the provided methods are fast enough using a single thread. "Fast enough" means, the
    detection is faster than the amount of data, that is usually provided by common cameras.
    Perhaps, multithreading will added later as a 'Configurable' property.

    \section TODO Todo ...
    More Documentation including examples will be added soon!
    
*/



#endif
