/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/Markers.h                    **
** Module : ICLMarkers                                             **
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

#include <ICLMarkers/FiducialDetector.h>

/**
    \defgroup PLUGINS FiducialDetector plugins


    The ICLMarkers package defines general interfaces for most different types of
    marker detection methods. The essential classes of this package are
    - icl::markers::FiducialDetector (for detection different types of markers in images)
    - icl::markers::Fiduical (a list of instances of this type is returned by the fiducial
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
    <TD>\image html icl1.png</TD>
    </TR>
    <TR>
    <TD>"art" marker</TD>
    <TD>"bch" marker</TD>
    <TD>"amoeba" marker</TD>
    <TD>"icl1" marker</TD>
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
    - "icl1"\n
      for ICL's former 'paper' markers. These markers consist of 4 vertical sections.
      Each of these sections contains a number of dot-regions. Therefore, the marker
      detection method is comparable to other hierarchical markers such as "amoeba",
      however, the well defined marker structure allows for the systematic identification
      of every single marker region, which provides better 3D pose-estimation results.

    \section BENCH Benchmarks

    TODO

    \subsection MT Multithreading

    We did not speed up the algorithms using multithreading techniques since usually
    the provided methods are fast enough using a single thread. "Fast enough" means, that the
    detection is faster than the amount of data, that is usually provided by common cameras.
    Perhaps, multithreading will added later as a 'utils::Configurable' property.

    \section EX Example

    \code
#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>

// static application data
HSplit hsplit;
GUI gui(hsplit);
GenericGrabber grabber;

// the global detector class
// here, using the first 100 "bch"-markers
icl::markers::FiducialDetector fid("bch","[0-100]","size=30x30");

// initialization function (called once after start)
void init(){
  // the the configurable ID
  fid.setConfigurableID("fid");

  // create the GUI
  gui << Draw().handle("draw")        // create drawing component
      << Prop("fid").maxSize(18,100)  // create the propery widged for 'fid'
      << Show();                      // show the main widget

  // initialize the grabber from given program argument
  grabber.init(pa("-input"));
}


// working loop (automatically looped in the working thread)
void run(){
  // get a handle to the "draw" component
  static DrawHandle draw = gui["draw"];

  // grab the next image
  const core::ImgBase *image = grabber.grab();

  // detect markers
  const std::vector<Fiducial> &fids = fid.detect(image);

  // visualize the image
  draw = image;

  // draw marker detection results
  draw->linewidth(2);
  for(unsigned int i=0;i<fids.size();++i){
    utils::Point32f c = fids[i].getCenter2D();
    float rot = fids[i].getRotation2D();

    draw->color(0,100,255,255);
    draw->text(fids[i].getName(),c.x,c.y,10);
    draw->color(0,255,0,255);
    draw->line(c,c+utils::Point32f( cos(rot), sin(rot))*100 );

    draw->color(255,0,0,255);
    draw->linestrip(fids[i].getCorners2D());

  }
  // update the visualization
  draw.render();
}

// main method
int main(int n, char **ppc){
  return ICLApp(n,ppc,"[m]-input|-i(2)",init,run).exec();
}
\endcode
*/
