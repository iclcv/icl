/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCV/demos/region-curvature/region-curvature.cpp      **
** Module : ICLCV                                                  **
** Authors: Tobias Röhlig                                          **
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
#include <ICLQt/FPSHandle.h>

#include <ICLCV/RegionDetector.h>

#include <ICLUtils/StackTimer.h>

#include "ICLCV/CurvatureExtractor.h"

//==============================================================================

using namespace icl::cv;

GUI gui;

GenericGrabber grabber;

Img32f image;

RegionDetector *detector;

//==============================================================================

void init(){

    grabber.init(pa("-i"));

    gui << Draw3D().label("3d view").handle("3d")
           << ( VBox()
                << Slider(1,20,3).handle("steps_handle").label("steps")
                << Slider(10,100,20).handle("radius_handle").label("radius")
                << FSlider(1.f,100.f,35.f).label("Threshold").handle("th_handle")
                << CheckBox("Thinned Contour",true).handle("useThinned_handle")
                << Fps(10).handle("fps").label("fps") );
    gui << Show();

    detector = new RegionDetector(200,50000,100,250);
}

//==============================================================================

int restOp(int x, int y) {
    int r = x%y;
    return r<0 ? y+r : r;
}

void run(){

    static DrawHandle3D draw = gui["3d"];

    image = *grabber.grab()->as32f();

    static Img8u newImage(image.getSize(),1);
    //newImage.setFormat(formatGray);
    Channel8u ch8u = newImage[0];
    Channel32f ch32f = image[0];

    {
        BENCHMARK_THIS_SECTION(draw1);
        for (int x = 0; x < image.getWidth(); ++x) {
            for (int y = 0; y < image.getHeight(); ++y) {
                float val = ch32f(x,y);
                if (val < 100) {
                    ch8u(x,y) = 10;
                } else if (val < 200) {
                    ch8u(x,y) = 30;
                } else if (val < 300) {
                    ch8u(x,y) = 60;
                } else if (val < 700) {
                    ch8u(x,y) = 100;
                } else if (val < 1000) {
                    ch8u(x,y) = 130;
                } else if (val < 1500) {
                    ch8u(x,y) = 170;
                } else if (val < 2000) {
                    ch8u(x,y) = 200;
                } else {
                    ch8u(x,y) = 255;
                }
                //std::cout << (int)ch8u(x,y) << ", ";
            }
        }
    }

    const std::vector<ImageRegion> &regions = detector->detect(&newImage);

    draw->color(255,0,0);
    draw->fill(0,0,0,0);
    Img8u lookup(image.getSize(),1);
    Channel8u lookCh = lookup[0];
    std::vector<uint> ids;
    {
        BENCHMARK_THIS_SECTION(create_ids);
        for (uint i = 0; i < regions.size(); ++i) {
            const ImageRegion &region = regions[i];
            const std::vector<Point> &boundary= region.getBoundary(true);
            /*for (uint k = 0; k < boundary.size(); k+=5) {
                draw->circle(boundary[k],3);
            }*/
            const std::vector<Point> &pixels = region.getPixels();
            uint id = (i+1)%255;
            for (uint k = 0; k < pixels.size(); ++k) {
                lookCh(pixels[k]) = id;
            }
            draw->polygon(boundary);
            ids.push_back(id);
        }
    }

    uint steps = gui["steps_handle"].as<uint>();//3;
    uint radius = gui["radius_handle"].as<uint>();//20;
    bool use_thinned_contour = gui["useThinned_handle"].as<bool>();

    CurvatureExtractor extractor(radius,steps,use_thinned_contour);
    std::vector<CurvatureExtractor::FloatHist> hists;

    {
        //BENCHMARK_THIS_SECTION(compute);
        extractor.extractRegionCurvature(regions,lookup,ids,hists);
    }

    const std::vector< std::vector< int > > &indices_all = extractor.getUsedIndices();

    float th = gui["th_handle"].as<float>();

    {
        BENCHMARK_THIS_SECTION(draw2);
        draw->color(0,150,150);
        draw->fill(0,0,0,0);
        for (uint i = 0; i < hists.size(); ++i) {
            CurvatureExtractor::FloatHist &hist = hists[i];
            const ImageRegion &region = regions[i];
            int max_size = 0;
            for (uint k = 1; k < indices_all[i].size()-1; ++k) {
                int index = indices_all[i][k];
                if (hist[k-1] < hist[k] && hist[k] > hist[k+1]) {
                    if (hist[k] > th) {
                        const Point p = region.getBoundary(true)[index];
                        std::stringstream sstream;
                        sstream << hist[k];
                        draw->text(sstream.str(),p,10);
                        draw->circle(p,3);
                        ++max_size;
                    }
                }
            }
        }

    }

    gui["3d"] = &newImage;
    gui["3d"].render();

    gui["fps"].render();
}

//==============================================================================

int main(int n, char **args){
    ICLApp app(n,args,"-input|-i(type=kinectd,device=0)",init,run);
    return app.exec();
}

//==============================================================================
