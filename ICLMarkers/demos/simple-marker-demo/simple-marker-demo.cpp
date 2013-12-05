/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLMarkers/demos/simple-marker-demo/simple-marker-demo **
 **          .cpp                                                   **
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

#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>
#include <ICLMarkers/FiducialDetectorPluginBCH.h>

HSplit gui;
GenericGrabber grabber;
FiducialDetector fid("bch", "[0-100]", ParamList("size", Size(30, 30)));

void init() {

  fid.setConfigurableID("fid");
  gui
      << (VSplit()
          << (HBox() << CamCfg("") << Draw().handle("draw").minSize(16, 12)
              << Draw().handle("regionsImg").minSize(16, 12))
          << (HBox() << Draw().handle("heuristicsImg").minSize(16, 12)
              << Draw().handle("quadImg").minSize(16, 12)))
      << Prop("fid").label("detection properties").maxSize(18, 100).minSize(14,
          1) << Show();

  grabber.init(pa("-input"));
}

void drawRegion(ICLDrawWidget *w, const vector<Point>& boundary) {
  w->linestrip(boundary, true);
}

void drawCorners(ICLDrawWidget *w,
    const std::vector<icl::utils::Point32f>& corners, bool drawIndices) {
  w->points(corners);
  if (drawIndices) {
    for (unsigned i = 0; i < corners.size(); i++) {
      stringstream ss; //create a stringstream
      ss << i; //add number to the stream
      w->text(ss.str(), corners[i].x, corners[i].y);
    }
  }

}

void drawQuad(ICLDrawWidget *w, const TiltedQuad &quad) {
  w->color(0, 0, 255);
  vector<utils::Point32f> points(quad.data(), quad.data() + 4);
  w->linestrip(points, true);
}

// working loop
void run() {

  static DrawHandle &hImgInp = gui.get<DrawHandle>("draw");
  static DrawHandle &hImgRegions = gui.get<DrawHandle>("regionsImg");

  static DrawHandle &hImgHeuristics = gui.get<DrawHandle>("heuristicsImg");
  static DrawHandle &hImgQuad = gui.get<DrawHandle>("quadImg");

  const ImgBase *image = grabber.grab();
  static Img8u imgRegions(image->getSize(), 1);
  imgRegions.clear(0, 255);

  static Img8u imgOldDet(image->getSize(), 1);
  imgOldDet.clear(0, 255);

  static Img8u imgHeuristics(image->getSize(), 1);
  imgHeuristics.clear(0, 255);
  static Img8u imgQuad(image->getSize(), 1);
  imgQuad.clear(0, 255);

  const std::vector<Fiducial> &fids = fid.detect(image);
  FiducialDetectorPlugin* plugin = fid.getPlugin();
  FiducialDetectorPluginBCH* bchplug = (FiducialDetectorPluginBCH*) plugin;
  QuadDetector& quadd = bchplug->getQuadDetector();

  const std::vector<TiltedQuad> &quadsNew = quadd.detect(image);
  icl::cv::RegionDetector* rd = quadd.getRegionDetector();
  const std::vector<ImageRegion>& regions = rd->getLastDetectedRegions();

  hImgInp = image;

  hImgRegions = &imgRegions;
  hImgHeuristics = &imgHeuristics;
  hImgQuad = &imgQuad;

  ICLDrawWidget *winpImg = *hImgInp;
  ICLDrawWidget *wRegions = *hImgRegions;
  ICLDrawWidget *wHeurstics = *hImgHeuristics;
  ICLDrawWidget *wQuad = *hImgQuad;

  wRegions->color(255, 0, 0);
  wHeurstics->color(0, 0, 0);
  wHeurstics->pointsize(3.0);
  for (unsigned int i = 0; i < regions.size(); ++i) {
    const vector<Point> &boundary = regions[i].getBoundary();
    drawRegion(wRegions, boundary);
    drawCorners(wHeurstics, quadd.getAllCorners()[i], false);
  }

  wHeurstics->color(255, 0, 0);
  wHeurstics->pointsize(8.0);
  for (unsigned int i = 0; i < quadd.getPerpCorners().size(); ++i) {
      wHeurstics->linestrip(quadd.getPerpCorners()[i], true);
  }

  wHeurstics->color(0, 0, 255);
  for (unsigned int i = 0; i < quadd.getMirrorCorners().size(); ++i) {
      wHeurstics->linestrip(quadd.getMirrorCorners()[i], true);
  }
  wHeurstics->color(0, 255, 0);
  for (unsigned int i = 0; i < quadd.getInterCorners().size(); ++i) {
      wHeurstics->linestrip(quadd.getInterCorners()[i], true);
  }

  wQuad->color(255, 0, 0);
  wQuad->pointsize(8.0);
  for (unsigned int i = 0; i < quadd.getLongestCorners().size(); ++i) {
    drawCorners(wQuad, quadd.getLongestCorners()[i], false);
  }

  wQuad->color(0, 0, 255);
  wQuad->pointsize(6.0);
  for (unsigned int i = 0; i < quadd.getSecLongestCorners().size(); ++i) {
    drawCorners(wQuad, quadd.getSecLongestCorners()[i], false);
  }

  for (unsigned int i = 0; i < quadsNew.size(); ++i) {
    drawQuad(wQuad, quadsNew[i]);
  }

  for (unsigned int i = 0; i < fids.size(); ++i) {
    Point32f c = fids[i].getCenter2D();
    float rot = fids[i].getRotation2D();

    winpImg->color(0, 100, 255, 255);
    winpImg->text(fids[i].getName(), c.x, c.y, 10);
    winpImg->color(0, 255, 0, 255);
    winpImg->line(c, c + Point32f(cos(rot), sin(rot)) * 100);

    winpImg->color(255, 0, 0, 255);
    winpImg->linestrip(fids[i].getCorners2D());

  }

  hImgInp.render();
  hImgRegions.render();
  hImgHeuristics.render();
  hImgQuad.render();
}

int main(int n, char **ppc) {
  return ICLApp(n, ppc, "[m]-input|-i(2)", init, run).exec();
}
