/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLMarkers/src/ICLMarkers/QuadDetector.cpp             **
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

#include <ICLMarkers/QuadDetector.h>

#include <ICLCV/RegionDetector.h>
#include <ICLFilter/LocalThresholdOp.h>
#include <ICLFilter/MorphologicalOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLCV/CornerDetectorCSS.h>
#include <ICLMath/StraightLine2D.h>

#include <ICLIO/FileWriter.h>
#include <float.h>
#include <ICLUtils/StackTimer.h>
using namespace icl::utils;
using namespace icl::math;
using namespace icl::core;
using namespace icl::filter;
using namespace icl::cv;
using namespace std;
typedef FixedColVector<float, 2> Vec2;
namespace icl {
  namespace markers {

    static void optimize_edges(std::vector<Point32f> &e4,
                               const std::vector<Point> &boundary) throw (int);

    class QuadDetector::Data {

      struct ClockwiseSort {
        Point32f center;
        bool operator()(Point32f a, Point32f b) {
          double aTanA = atan2(a.y - center.y, a.x - center.x);
          double aTanB = atan2(b.y - center.y, b.x - center.x);
          return (aTanA < aTanB);
        }
      } clockSort;

    public:
      //Debug information
      std::vector<std::vector<icl::utils::Point32f> > longestCorners;
      std::vector<std::vector<icl::utils::Point32f> > secLongestCorners;
      std::vector<std::vector<icl::utils::Point32f> > perpCorners;
      std::vector<std::vector<icl::utils::Point32f> > interCorners;
      std::vector<std::vector<icl::utils::Point32f> > mirrorCorners;
      std::vector<std::vector<icl::utils::Point32f> > allCorners;


      Point32f center;
      inline std::vector<Vec2> getVecsToPoints(
                                               const std::vector<Point32f> &corners) {
        std::vector<Vec2> lines;
        for (unsigned i = 0; i < corners.size(); i++) {
          unsigned vecEndIdx = (i + 1 + corners.size()) % corners.size();
          lines.push_back(getVector(corners[i], corners[vecEndIdx]));
        }
        return lines;
      }

      inline Vec2 getVector(const Point32f &start, const Point32f &end) {
        return Vec2(end.x - start.x, end.y - start.y);
      }

      inline Point32f addVectorToPoint(const Point32f &start, const Vec2 &vec) {
        return Point32f(start.x + vec[0], start.y + vec[1]);
      }

      inline Vec2 getNormal(const Vec2 &vec) {
        return Vec2(-1 * vec[1], vec[0]);
      }

      inline float dotProduct(const Vec2& v, const Vec2& v2) {
        return v[0] * v2[0] + v[1] * v2[1];
      }

      inline Vec2 pointToVec(const Point32f& point) {
        return Vec2(point.x, point.y);
      }

      inline Point32f vecToPoint(const Vec2& vec) {
        return Point32f(vec[0], vec[1]);
      }

      Point32f getIntersectionPoint(const Point32f& o1, const Vec2& vec1,
                                    const Point32f& o2, const Vec2& vec2) {
        Vec2 v1 = vec1.normalized();
        Vec2 v2 = vec2.normalized();
        if (v1[0] == 0)
          v1[0] = .00001; //to avoid division by zero
        float tmp = ((o2.x - o1.x) * v1[1]) / v1[0];
        float tmp2 = -(v2[0] * v1[1]) / v1[0] + v2[1];
        float b = (o1.y - o2.y + tmp) / tmp2;
        return addVectorToPoint(o2, v2 * b);
      }

      inline void orderClockWise(std::vector<Point32f> &points) {
        clockSort.center = Point32f(0, 0);
        for (unsigned i = 0; i < points.size(); i++) {
          clockSort.center += points[i];
        }
        clockSort.center = clockSort.center * (1. / (float) points.size());
        std::sort(points.begin(), points.end(), clockSort);
      }

      std::vector<Point32f> getQCornersByMirror(Point32f &srcVecStart,
                                                Point32f &srcVecEnd, int srcVecStartIdx, int srcVecEndIdx,
                                                std::vector<Point32f> &allCorners) {
        std::vector<Point32f> qCorners;
        std::vector<Point32f> secLongest;
        Vec2 srcVec = getVector(srcVecStart, srcVecEnd);

        int postSrcVecEndIdx = (srcVecEndIdx + 1 + allCorners.size())
        % allCorners.size();
        int preSrcVecStartIdx = (srcVecStartIdx - 1 + allCorners.size())
        % allCorners.size();
        Point32f postSrcVecEnd = allCorners[postSrcVecEndIdx];
        Point32f preSrcVecStart = allCorners[preSrcVecStartIdx];
        Vec2 postSrcVec = getVector(srcVecEnd, postSrcVecEnd);
        Vec2 preSrcVec = getVector(preSrcVecStart, srcVecStart);

        Vec2 secondVec;
        Point32f cornerToMirror;
        Point32f mirrorAxisStart;
        Point32f mirrorAxisEnd;
        Point32f thirdCorner;
        mirrorAxisStart = addVectorToPoint(srcVecStart,
                                           srcVec.normalized() * 0.5 * srcVec.length());
        mirrorAxisEnd = addVectorToPoint(mirrorAxisStart, getNormal(srcVec));

        if (postSrcVec.length() > preSrcVec.length()) {
          secondVec = postSrcVec;

          cornerToMirror = postSrcVecEnd;
          thirdCorner = postSrcVecEnd;

          secLongest.push_back(srcVecEnd);
          secLongest.push_back(postSrcVecEnd);
        } else {
          secondVec = preSrcVec;
          cornerToMirror = preSrcVecStart;

          thirdCorner = preSrcVecStart;

          secLongest.push_back(preSrcVecStart);
          secLongest.push_back(srcVecStart);
        }
        secLongestCorners.push_back(secLongest);

        qCorners.push_back(thirdCorner);

        Vec2 mirrorAxis = getVector(mirrorAxisStart, mirrorAxisEnd);

        Vec2 cornerToMirrorVec = getVector(mirrorAxisStart, cornerToMirror);
        Vec2 mirrorAxisNormalized = mirrorAxis;
        mirrorAxisNormalized.normalize();
        Point32f intersection = vecToPoint(
                                           mirrorAxisNormalized
                                           * dotProduct(cornerToMirrorVec, mirrorAxisNormalized));

        cornerToMirror = cornerToMirror - mirrorAxisStart;
        Point32f mirroredCorner = intersection
        + ((cornerToMirror - intersection) * -1.) + mirrorAxisStart;
        qCorners.push_back(mirroredCorner);
        return qCorners;
      }

      std::vector<Point32f> getQCornersByIntersection(Point32f &srcVecStart,
                                                      Point32f &srcVecEnd, int srcVecStartIdx, int srcVecEndIdx,
                                                      std::vector<Point32f> &allCorners) {
        std::vector<Point32f> qCorners;
        std::vector<Point32f> secLongest;
        int postSrcVecEndIdx = (srcVecEndIdx + 1 + allCorners.size())
        % allCorners.size();
        int preSrcVecStartIdx = (srcVecStartIdx - 1
                                 + allCorners.size()) % allCorners.size();
        Point32f postSrcVecEnd = allCorners[postSrcVecEndIdx];
        Point32f preSrcVecStart = allCorners[preSrcVecStartIdx];
        Vec2 postSrcVec = getVector(srcVecEnd, postSrcVecEnd);
        Vec2 preSrcVec = getVector(preSrcVecStart, srcVecStart);

        Vec2 secondVec;
        Point32f intersectionStart;
        Vec2 intersectionVec;
        Point32f intersection2Start;
        Vec2 intersection2Vec;
        Point32f thirdCorner;

        if (postSrcVec.length() > preSrcVec.length()) {

          secondVec = postSrcVec;
          intersectionStart = preSrcVecStart;
          intersectionVec = getVector(intersectionStart, srcVecStart);
          intersection2Start = postSrcVecEnd;
          int intersection2EndIdx = (postSrcVecEndIdx + 1 + allCorners.size())
          % allCorners.size();
          intersection2Vec = getVector(intersection2Start, allCorners[intersection2EndIdx]);
          thirdCorner = postSrcVecEnd;
          secLongest.push_back(srcVecEnd);
          secLongest.push_back(postSrcVecEnd);
        } else {
          secondVec = preSrcVec;

          intersectionStart = postSrcVecEnd;
          intersectionVec = getVector(intersectionStart, srcVecEnd);
          intersection2Start = preSrcVecStart;
          int intersection2EndIdx = (preSrcVecStartIdx - 1 + allCorners.size())
          % allCorners.size();
          intersection2Vec = getVector(intersection2Start, allCorners[intersection2EndIdx]);
          thirdCorner = preSrcVecStart;
          secLongest.push_back(preSrcVecStart);
          secLongest.push_back(srcVecStart);
        }

        secLongestCorners.push_back(secLongest);

        qCorners.push_back(thirdCorner);
        if (intersection2Vec.normalized() != intersectionVec.normalized() &&
            intersection2Vec.normalized()*-1. != intersectionVec.normalized()) {
          Point32f intersection = getIntersectionPoint(intersectionStart, intersectionVec, intersection2Start, intersection2Vec);
          qCorners.push_back(intersection);
        }
        return qCorners;

      }

      std::vector<Point32f> getQCornersByNearestPerpCorners(Point32f srcVecStart, Point32f srcVecEnd, std::vector<Point32f> &allCorners) {
        std::vector<Point32f> qCorners;
        FixedMatrix<float, 2, 2> rotMat = create_rot_2D(M_PI / (float) 2);
        Vec2 srcVec = getVector(srcVecStart, srcVecEnd);
        float refLength = srcVec.length();
        Vec2 orthVec = rotMat * srcVec;
        orthVec.normalize();
        orthVec = orthVec * srcVec.length();
        Point32f orthVecDest;
        for (unsigned lineIdx = 0; lineIdx < 2; lineIdx++) {
          float minDist = FLT_MAX;
          orthVecDest = addVectorToPoint(srcVecEnd, orthVec);
          Point32f nearestCorner;
          for (unsigned i = 0; i < allCorners.size(); i++) {
            Vec2 dist(orthVecDest.x - allCorners[i].x,
                      orthVecDest.y - allCorners[i].y);
            if (dist.length() < minDist) {
              minDist = dist.length();
              nearestCorner = allCorners[i];
            }
          }
          qCorners.push_back(nearestCorner);
          srcVec = getVector(srcVecEnd, nearestCorner);
          srcVecStart = srcVecEnd;
          srcVecEnd = addVectorToPoint(srcVecStart, srcVec);
          orthVec = rotMat * srcVec;
          orthVec.normalize();
          orthVec = orthVec * refLength;
        }
        return qCorners;
      }

      float getQuadRating(std::vector<Point32f> &corners) {
        std::vector<Vec2> vecs = getVecsToPoints(corners);
        Vec2 a = vecs[0];
        Vec2 b = vecs[1];
        Vec2 c = vecs[2];
        Vec2 d = vecs[3];
        float lenQuot1 = min(a.length()/c.length(), c.length()/a.length());
        float lenQuot2 = min(b.length()/d.length(), d.length()/b.length());
        float dp1 = abs(dotProduct(a.normalized(), c.normalized()));
        dp1 = min(dp1, 0.1f);
        float dp2 = abs(dotProduct(b.normalized(), d.normalized()));
        dp2 = min(dp2, 0.1f);

        float avgLen1 = (a.length() + c.length()) / 2.0;
        float avgLen2 = (b.length() + d.length()) / 2.0;

        //this term punishes rectangles, with different edge length as:
        //       ___________
        //      |           |
        //       ___________
        float avgLenRating = min(avgLen1/avgLen2, avgLen2/avgLen1) * min(dp1/dp2, dp2/dp1);
        //the more perpendicular opposite edges are, the smaller is the length-quotient of the other 2 edges.
        float lenRating = min(dp1/lenQuot2, lenQuot2/dp1) * min(dp2/lenQuot1, lenQuot1/dp2);
        return (lenRating + avgLenRating) / 2.0;
      }

      std::vector<Point32f> removeObstacleCorners(std::vector<Point32f> &corners, float minRating, bool useInterHeuristic, bool usePerpHeuristic, bool useMirrorHeuristic) {

        std::vector<Point32f> quadCorners;
        if (useInterHeuristic || usePerpHeuristic || useMirrorHeuristic) {
          std::vector<Vec2> vecs = getVecsToPoints(corners);

          std::vector<Point32f> longest;

          int longestVecStartIdx = 0;
          double maxLength = 0;
          for (unsigned i = 0; i < vecs.size(); i++) {
            if (vecs[i].length() > maxLength) {
              longestVecStartIdx = i;
              maxLength = vecs[i].length();
            }
          }

          Point32f longestVecStart = corners[longestVecStartIdx];

          unsigned longestVecEndIdx = (longestVecStartIdx + 1 + corners.size())
          % corners.size();
          Point32f longestVecEnd = corners[longestVecEndIdx];

          longest.push_back(longestVecStart);
          longest.push_back(longestVecEnd);
          longestCorners.push_back(longest);

          std::vector<Point32f> cornersPerp;
          std::vector<Point32f> cornersInter;
          std::vector<Point32f> cornersMirror;
          float perpRating = -1;
          float interRating = -1;
          float mirrorRating = -1;

          if (useInterHeuristic) {
            cornersInter = getQCornersByIntersection(longestVecStart, longestVecEnd, longestVecStartIdx, longestVecEndIdx, corners);
            cornersInter.push_back(longestVecStart);
            cornersInter.push_back(longestVecEnd);
            orderClockWise(cornersInter);
            interRating = getQuadRating(cornersInter);

            interCorners.push_back(cornersInter);
          }
          if (usePerpHeuristic) {

            cornersPerp = getQCornersByNearestPerpCorners(longestVecStart, longestVecEnd, corners);
            cornersPerp.push_back(longestVecStart);
            cornersPerp.push_back(longestVecEnd);
            orderClockWise(cornersPerp);
            perpRating = getQuadRating(cornersPerp);

            perpCorners.push_back(cornersPerp);
          }

          if (useMirrorHeuristic) {

            cornersMirror = getQCornersByMirror(longestVecStart, longestVecEnd, longestVecStartIdx, longestVecEndIdx, corners);
            cornersMirror.push_back(longestVecStart);
            cornersMirror.push_back(longestVecEnd);
            orderClockWise(cornersMirror);
            mirrorRating = getQuadRating(cornersMirror);

            mirrorCorners.push_back(cornersMirror);
          }

          //                cout << "minRating " << minRating << "perpR " << perpRating << " interR " << interRating << " mirrR " << mirrorRating << endl;

          if (perpRating > interRating)
            {
              if (perpRating > mirrorRating) {
                if (perpRating > minRating) {
                  quadCorners = cornersPerp;
                }
              } else if (mirrorRating > minRating) {
                quadCorners = cornersMirror;
              }
            } else if (interRating > mirrorRating)
            {
              if (interRating > minRating) {
                quadCorners = cornersInter;
              }
            } else if (mirrorRating > minRating) {
            quadCorners = cornersMirror;
          }

        }

        return quadCorners;
      }

      std::vector<TiltedQuad> quads;

      SmartPtr<RegionDetector> rd;
      CornerDetectorCSS css;

      SmartPtr<LocalThresholdOp> lt;
      SmartPtr<UnaryOp> pp;

      bool dynamicQuadColor;
      std::string lastPPType;
      Size lastPPSize;

      ImgBase *lastBinImage;
    };

    static const int RD_VALS[6] = { 0, 255, 0, 0, 255, 255 };
    QuadDetector::QuadDetector(QuadDetector::QuadColor c, bool dynamic,
                               float minRating) :
      data(new Data) {
      data->dynamicQuadColor = dynamic;

      addProperty("pp.filter", "menu",
                  "none,median,erosion,dilatation,opening,closing", "none", 0,
                  "Post processing filter.");
      addProperty("pp.mask size", "menu", "3x3,5x5", "3x3", 0,
                  "Mask size for post processing.");

      if (dynamic) {
        addProperty("quad value", "menu",
                    str(BlackOnly) + "," + str(WhiteOnly) + ","
                    + str(BlackAndWhite), str(c), 0,
                    "Defines whether the marker borders are black, white or mixed");
      }
      addProperty("optimize edges", "flag", "", "true", 0,
                  "Flag for optimized marker corner detection");

      addProperty("min-rating", "range:slider", "[0, 1]",
                  str(minRating));

      addProperty("intersection heuristic", "flag", "", "true", 0,
                  "Flag for reconstruct distorted quads by the intersection heuristic");

      addProperty("perpendicular heuristic", "flag", "", "true", 0,
                  "Flag for reconstruct distorted quads by the perpendicular heuristic");
      addProperty("mirror heuristic", "flag", "", "true", 0,
                  "Flag for reconstruct distorted quads by the perpendicular heuristic");

      data->rd = new RegionDetector(40, 2 << 20, RD_VALS[(int) c],
                                    RD_VALS[(int) c + 3]);
      data->rd->deactivateProperty("minimum value");
      data->rd->deactivateProperty("maximum value");
      data->rd->deactivateProperty("^CSS*");

      data->lt = new LocalThresholdOp;
      data->lt->deactivateProperty("gamma slope");
      data->lt->deactivateProperty("^UnaryOp*");

      addChildConfigurable(data->rd.get(), "quads");
      addChildConfigurable(data->lt.get(), "thresh");

      data->css.deactivateProperty("debug-mode");
      addChildConfigurable(&data->css, "css");
      addProperty("css.dynamic sigma", "flag", "", true, 0,
                  "If set to true, the border-smoothing sigma is adapted\n"
                  "relatively to the actual marker size (usually provides\n"
                  "much better results)");

      data->css.setSigma(4.2);
      data->css.setCurvatureCutoff(66);
      data->lastBinImage = 0;

      // set some default values ...
      setPropertyValue("css.angle-threshold", 180);
      setPropertyValue("css.curvature-cutoff", 66);
      setPropertyValue("css.rc-coefficient", 1);

      setPropertyValue("thresh.global threshold", -10);
      setPropertyValue("thresh.mask size", 30);
    }

    QuadDetector::~QuadDetector() {
      delete data;
    }

    icl::cv::RegionDetector* QuadDetector::getRegionDetector() {
      return data->rd.get();
    }
  
    const std::vector<TiltedQuad> &QuadDetector::detect(const ImgBase *image) {
    
      ICLASSERT_THROW(image,
                      ICLException("QuadDetector::detect: input image was NULL"));
      if (data->dynamicQuadColor) {
        QuadColor c = getPropertyValue("quad value");
        if (c >= 0 && c <= 3) {
          data->rd->setConstraints(
                                   getPropertyValue("region detector.minimum size"),
                                   getPropertyValue("region detector.maximum size"),
                                   RD_VALS[(int) c], RD_VALS[(int) c + 3]);
        } else {
          ERROR_LOG(
                    "unable to adapt the region detectors min. and max. value property "
                    "because the 'quad value' property value is undefined");
        }
      }

      std::string kernelType = getPropertyValue("pp.filter");
      Size kernelSize = getPropertyValue("pp.mask size");

      if (data->lastPPType != kernelType || data->lastPPSize != kernelSize) {
        data->lastPPType = kernelType;
        data->lastPPSize = kernelSize;

        bool is3x3 = (kernelSize == Size(3, 3));

        //"menu","none,median,erosion,dilatation,opening,closing"

        if (kernelType == "median") {
          data->pp = new MedianOp(kernelSize);
        } else if (kernelType == "none") {
          data->pp.setNull();
        } else if (kernelType == "erosion") {
          data->pp = new MorphologicalOp(
                                         is3x3 ? MorphologicalOp::erode3x3 : MorphologicalOp::erode,
                                         kernelSize);
        } else if (kernelType == "dilatation") {
          data->pp =
          new MorphologicalOp(
                              is3x3 ? MorphologicalOp::dilate3x3 : MorphologicalOp::dilate,
                              kernelSize);
        } else if (kernelType == "opening") {
          data->pp = new MorphologicalOp(MorphologicalOp::openBorder,
                                         kernelSize);
        } else if (kernelType == "closing") {
          data->pp = new MorphologicalOp(MorphologicalOp::closeBorder,
                                         kernelSize);
        }
        if (data->pp) {
          data->pp->setClipToROI(false);
        }
      }

      if (data->pp) {
        data->pp->apply(data->lt->apply(image), &data->lastBinImage);
      } else {
        data->lt->apply(image, &data->lastBinImage);
      }

      data->quads.clear();

      const std::vector<ImageRegion> &rs = data->rd->detect(data->lastBinImage);

      const bool dynCSSSigma = getPropertyValue("css.dynamic sigma");
      const bool optEdges = getPropertyValue("optimize edges");
      const float minRating = getPropertyValue("min-rating");
      const bool useIntersectionHeuristic = getPropertyValue("intersection heuristic");
      const bool usePerpendicularHeuristic = getPropertyValue("perpendicular heuristic");
      const bool useMirrorHeuristic = getPropertyValue("mirror heuristic");
      const bool useAnyHeuristic = useIntersectionHeuristic || usePerpendicularHeuristic || useMirrorHeuristic;

      data->allCorners.clear();
      data->longestCorners.clear();
      data->secLongestCorners.clear();
      data->perpCorners.clear();
      data->interCorners.clear();
      data->mirrorCorners.clear();

      for (unsigned int i = 0; i < rs.size(); ++i) {
        const std::vector<Point> &boundary = rs[i].getBoundary();
      
        if(dynCSSSigma) {
          data->css.setSigma(iclMin(7.,boundary.size() * (3.2/60) - 0.5));
        }
        PVec corners = data->css.detectCorners(boundary);
        
        data->allCorners.push_back(corners);
        if(useAnyHeuristic && (corners.size() > 4)){
          PVec cornersCopy = data->removeObstacleCorners(corners,
                                                    minRating,
                                                    useIntersectionHeuristic,
                                                    usePerpendicularHeuristic,
                                                    useMirrorHeuristic);
          if(cornersCopy.size() == 4){
            if(optEdges){
              // implement find corner subpix!
            }
            data->quads.push_back(TiltedQuad(cornersCopy.data(), rs[i]));
          }
        }else{
          if(corners.size() == 4){
            if(optEdges) {
              try{
                optimize_edges(corners, boundary); 
                data->quads.push_back(TiltedQuad(corners.data(), rs[i]));
              }catch (int code) {
                (void) code;
              }
            }else{
              data->quads.push_back(TiltedQuad(corners.data(), rs[i]));
            }
          }
        }
      }
      return data->quads;
    }


    const QuadDetector::PVecVec &QuadDetector::getAllCorners() const{
      return data->allCorners;
    }

    QuadDetector::PVecVec &QuadDetector::getAllCorners(){
      return data->allCorners;
    }
    
    const QuadDetector::PVecVec &QuadDetector::getLongestCorners() const{
      return data->longestCorners;
    }
    
    QuadDetector::PVecVec &QuadDetector::getLongestCorners(){
      return data->longestCorners;
    }
      
    const QuadDetector::PVecVec &QuadDetector::getSecLongestCorners() const{
      return data->secLongestCorners;
    }
    
    QuadDetector::PVecVec &QuadDetector::getSecLongestCorners(){
      return data->secLongestCorners;
    }

    const QuadDetector::PVecVec &QuadDetector::getInterCorners() const{
      return data->interCorners;
    }
    
    QuadDetector::PVecVec &QuadDetector::getInterCorners(){
      return data->interCorners;
    }
    
    const QuadDetector::PVecVec &QuadDetector::getPerpCorners() const{
      return data->perpCorners;
    }
    
    QuadDetector::PVecVec &QuadDetector::getPerpCorners(){
      return data->perpCorners;
    }
    
    const QuadDetector::PVecVec &QuadDetector::getMirrorCorners() const{
      return data->mirrorCorners;
    }

    QuadDetector::PVecVec &QuadDetector::getMirrorCorners(){
      return data->mirrorCorners;
    } 

  

    const Img8u &QuadDetector::getLastBinaryImage() const {
      return *data->lastBinImage->asImg<icl8u>();
    }

    std::ostream &operator<<(std::ostream &s, const QuadDetector::QuadColor &c) {
      switch (c) {
        case QuadDetector::WhiteOnly:
          return s << "white only";
        case QuadDetector::BlackOnly:
          return s << "black only";
        case QuadDetector::BlackAndWhite:
          return s << "black and white";
        default:
          return s << "undefined";
      }
    }

    std::istream &operator>>(std::istream &s, QuadDetector::QuadColor &qc) {
      std::string a, b, c;
      s >> a >> b;
      if (a == "white" && b == "only") {
        qc = QuadDetector::WhiteOnly;
        return s;
      }
      if (a == "black" && b == "only") {
        qc = QuadDetector::BlackOnly;
        return s;
      } else
        s >> c;
      if (a == "black" && b == "and" && c == "white") {
        qc = QuadDetector::BlackAndWhite;
        return s;
      }
      qc = (QuadDetector::QuadColor) -1;
      return s;
    }

#if 0
    static StraightLine2D approx_line(const std::vector<Point32f> &ps) throw (int) {
      int nPts = ps.size();
      if(nPts < 2) throw 2;
      float avgX = 0;
      float avgY = 0;
      for(unsigned int i=0;i<ps.size();++i) {
        avgX += ps[i].x;
        avgY += ps[i].y;
      }
      avgX /= nPts;
      avgY /= nPts;
      float avgXX(0),avgXY(0),avgYY(0);

      for(unsigned int i=0;i<ps.size();++i) {
        avgXX += ps[i].x * ps[i].x;
        avgXY += ps[i].x * ps[i].y;
        avgYY += ps[i].y * ps[i].y;
      }
      avgXX /= nPts;
      avgXY /= nPts;
      avgYY /= nPts;

      double fSxx = avgXX - avgX*avgX;
      double fSyy = avgYY - avgY*avgY;
      double fSxy = avgXY - avgX*avgY;

      double fP = 0.5*(fSxx+fSyy);
      double fD = 0.5*(fSxx-fSyy);
      fD = ::sqrt(fD*fD + fSxy*fSxy);
      double fA = fP + fD;

      float angle = ::atan2(fA-fSxx,fSxy);
      return StraightLine2D(Point32f(avgX,avgY),Point32f(cos(angle),sin(angle)));
    }
#endif

    static StraightLine2D fit_line(float x, float y, float xx, float xy, float yy)
      throw (int) {
      float sxx = xx - x * x;
      float syy = yy - y * y;
      float sxy = xy - x * y;
      float p = 0.5 * (sxx + syy);
      float d = 0.5 * (sxx - syy);
      float sdd = ::sqrt(d * d + sxy * sxy);
      float a = p + sdd;
      float angle = ::atan2(a - sxx, sxy);
      return StraightLine2D(Point32f(x, y), Point32f(cos(angle), sin(angle)));
    }

    static float line_sprod(const StraightLine2D &a, const StraightLine2D &b) {
      return fabs(a.v[0] * b.v[0] + a.v[1] * b.v[1]);
    }

    void optimize_edges(std::vector<Point32f> &e4,
                        const std::vector<Point> &boundary) throw (int) {
      int num = boundary.size();
      int i0 = (int) (std::find(boundary.begin(), boundary.end(), Point(e4[0]))
                      - boundary.begin());
      ICLASSERT_THROW(i0 < num,
                      ICLException("edge point was not found in the boundary"));
      std::vector<Point> b2(num);
      std::copy(boundary.begin() + i0, boundary.end(), b2.begin());
      std::copy(boundary.begin(), boundary.begin() + i0, b2.begin() + (num - i0));

      int i = 0;
      Point next = e4[1];
      Point32f means[4];
      int ns[4] = { 0 };
      float xx[4] = { 0 }, xy[4] = { 0 }, yy[4] = { 0 };

      for (; i < num && b2[i] != next; ++i) {
        int x = b2[i].x, y = b2[i].y;
        means[0].x += x;
        means[0].y += y;
        xx[0] += x * x;
        xy[0] += x * y;
        yy[0] += y * y;
        ++ns[0];
      }
      next = e4[2];
      for (; i < num && b2[i] != next; ++i) {
        int x = b2[i].x, y = b2[i].y;
        means[1].x += x;
        means[1].y += y;
        xx[1] += x * x;
        xy[1] += x * y;
        yy[1] += y * y;
        ++ns[1];
      }
      next = e4[3];
      for (; i < num && b2[i] != next; ++i) {
        int x = b2[i].x, y = b2[i].y;
        means[2].x += x;
        means[2].y += y;
        xx[2] += x * x;
        xy[2] += x * y;
        yy[2] += y * y;
        ++ns[2];
      }
      for (; i < num; ++i) {
        int x = b2[i].x, y = b2[i].y;
        means[3].x += x;
        means[3].y += y;
        xx[3] += x * x;
        xy[3] += x * y;
        yy[3] += y * y;
        ++ns[3];
      }
      for (int i = 0; i < 4; ++i) {
        float fac = 1.0 / ns[i];
        means[i].x *= fac;
        means[i].y *= fac;
        xx[i] *= fac;
        xy[i] *= fac;
        yy[i] *= fac;
      }
      StraightLine2D a = fit_line(means[0].x, means[0].y, xx[0], xy[0], yy[0]);
      StraightLine2D b = fit_line(means[1].x, means[1].y, xx[1], xy[1], yy[1]);
      StraightLine2D c = fit_line(means[2].x, means[2].y, xx[2], xy[2], yy[2]);
      StraightLine2D d = fit_line(means[3].x, means[3].y, xx[3], xy[3], yy[3]);

      if (line_sprod(a, c) < 0.90 || line_sprod(d, b) < 0.90)
        throw 1;

      try {
        StraightLine2D::Pos intersections[] = { a.intersect(d), a.intersect(b),
                                                c.intersect(b), c.intersect(d) };

        for (int i = 0; i < 4; ++i) {
          e4[i].x = intersections[i][0];
          e4[i].y = intersections[i][1];
        }
      } catch (...) {
        throw 3;
      }
    }

    REGISTER_CONFIGURABLE_DEFAULT(QuadDetector)
    ;
  } // namespace markers
}
