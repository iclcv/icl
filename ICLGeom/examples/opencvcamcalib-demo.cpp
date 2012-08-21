/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLGeom/examples/opencvcamcalib-demo.cpp               **
 ** Module : ICLGeom                                                **
 ** Authors: Christian Groszewski, Andre Ueckermann                 **
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
#include <ICLGeom/OpenCVCamCalib.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLQuick/Common.h>
#include <ICLOpenCV/OpenCV.h>

#include <ICLUtils/ConfigFile.h>

using namespace icl;

GUI gui("hsplit");
SmartPtr<GenericGrabber> cg;
Mutex mutex;
OpenCVCamCalib *camc = 0;

int width = 0;
int height = 0;
int cornercount = 0;
CvSize boardSize;
CvPoint2D32f* corners = 0;
int successes = 0;
int minSuccesses = 0;

int framewidth=0;
int frameheight=0;

DynMatrix<icl64f> *intr;
DynMatrix<icl64f> *dist;

void save_params(){
  try{
	  std::string filename = saveFileDialog("XML-Files (*.xml)");
    ConfigFile f;
    f.setPrefix("config.");
    f["model"] = str("MatlabModel5Params");
	  f["size.width"] = (int)framewidth;
	  f["size.height"] = (int)frameheight;
	  f["intrin.fx"] = (double)intr->at(0,0);
	  f["intrin.fy"] = (double)intr->at(1,1);
	  f["intrin.ix"] = (double)intr->at(2,0);
	  f["intrin.iy"] = (double)intr->at(2,1);
	  f["intrin.skew"] = (double)0;
	  f["udist.k1"] = (double)dist->at(0,0);
	  f["udist.k2"] = (double)dist->at(0,1);
	  f["udist.k3"] = (double)dist->at(0,2);
	  f["udist.k4"] = (double)dist->at(0,3);
	  f["udist.k5"] = (double)dist->at(0,4);			
    f.save(filename);
  }catch(...){
    std::cout<<"save error"<<std::endl;
  }
  std::cout<<"save parameters"<<std::endl;
}

void init(){
  if(pa("-s")){
    Size size = pa("-s");
    std::cout<<"use desired size: "<<size<<std::endl;
    framewidth=size.width;
    frameheight=size.height;
  }else{
    std::cout<<"use standard size"<<std::endl;
  }
	Size s = pa("-cbs");
	width = s.width;
	height = s.height;
	minSuccesses = pa("-m");
	camc = new OpenCVCamCalib(width,height,minSuccesses);
	boardSize = cvSize(width, height);
	corners = new CvPoint2D32f[width*height];

	cg = new GenericGrabber(pa("-i"));
	if(pa("-s")){
	  cg->setDesiredSizeInternal(pa("-s"));
  }
  

        gui << (HBox()
                << Draw().handle("calib_object").minSize(20,20).label("calib")
                << Draw().handle("draw_object").minSize(20,20).label("plot"))
            << Button("save parameters").handle("saveParams")
            << Button("reset calibration").handle("reset")
            << Show();


	gui["saveParams"].disable();
	gui["saveParams"].registerCallback(save_params);
}

double compute_error(const ImgBase *img){
	double error = 0.0;
	IplImage *iplimage = img_to_ipl(img);
	IplImage *ipl_gray = img_to_ipl(img);
	if(iplimage->nChannels == 3){
		ipl_gray = cvCreateImage(cvGetSize(iplimage), 8, 1);
		cvCvtColor( iplimage, ipl_gray, CV_BGR2GRAY );
	}else{
		ipl_gray = iplimage;
	}

	int found = cvFindChessboardCorners(iplimage, boardSize, corners,
			&cornercount, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS );

	cvFindCornerSubPix( ipl_gray, corners, cornercount, cvSize(11, 11),
			cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1));
	double m = 0.0;
	double c = 0.0;
	double m1 = 0.0;
	double c1 = 0.0;
	double k1 = 0.0;
	double k2 = 0.0;
	double dist = 0.0;
	if(found && (cornercount == (boardSize.width*boardSize.height))){
		for(int j=0;j<boardSize.height;++j){
			m = (corners[(j+1)*boardSize.width-1].y-corners[j*boardSize.width].y)/
					(corners[(j+1)*boardSize.width-1].x-corners[j*boardSize.width].x);
			c = corners[j*boardSize.width].y - m*corners[j*boardSize.width].x;
			m1 = -1/m;
			for(int i=1;i<boardSize.width-1;++i){
				c1 = corners[j*boardSize.width+i].y - m1*corners[j*boardSize.width+i].x;
				k1 = (c - c1)/(m1-m);
				k2 = m1*k1+c1;
				dist = std::sqrt((corners[j*boardSize.width+i].y-k2)*(corners[j*boardSize.width+i].y-k2) +
						(corners[j*boardSize.width+i].x-k1)*(corners[j*boardSize.width+i].x-k1));
				error += dist;
			}
		}
		error = error/(boardSize.height*boardSize.width);
	}
	if(iplimage->nChannels == 3)
		cvReleaseImage(&ipl_gray);
	cvReleaseImage(&iplimage);
	return error;
}

void run(){
  ButtonHandle reset = gui["reset"];

	if(reset.wasTriggered()){
	  gui["saveParams"].disable();
	  successes=0;
	  camc->resetData(width,height,minSuccesses);
  }
	Mutex::Locker lock(mutex);
	static FPSLimiter fps(30);
	const ImgBase *img = cg->grab();
	const ImgBase *img2 = 0;
	framewidth=img->getWidth();
	frameheight=img->getHeight();
	if(successes < minSuccesses){
		Thread::msleep(400);

		IplImage *image = img_to_ipl(img);
		IplImage *gray_image = 0;
		if(image->nChannels == 3){
			gray_image = cvCreateImage(cvGetSize(image), 8, 1);
			cvCvtColor( image, gray_image, CV_BGR2GRAY );
		}else{
			gray_image = image;
		}
		int found = cvFindChessboardCorners(image, boardSize, corners,
				&cornercount, CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS );

		cvFindCornerSubPix( gray_image, corners, cornercount, cvSize( 11, 11 ),
				cvSize( -1, -1 ), cvTermCriteria( CV_TERMCRIT_EPS+CV_TERMCRIT_ITER, 30, 0.1 ));
		cvDrawChessboardCorners(image, boardSize, corners, cornercount, found);

		if(found && (successes < minSuccesses)){
			camc->addPoints(img);
			successes++;
		}

		img2 = ipl_to_img(image);
		if(image->nChannels == 3)
			cvReleaseImage(&gray_image);
		cvReleaseImage(&image);
		gui["draw_object"] = img2;
		gui["draw_object"].render();
	}
  
	if(successes == minSuccesses){
		camc->calibrateCam();
		successes++;
		intr = camc->getIntrinsics();
		SHOW(*intr);
		//delete intr;
		dist = camc->getDistortion();
		SHOW(*dist);
		//delete dist;
		gui["saveParams"].enable();
	} else if(successes > minSuccesses){
		const ImgBase *img2 = camc->undisort(img);

		gui["draw_object"] = img2;
		gui["draw_object"].render();
#ifdef OPENCVCAMCALIBDEBUG
		std::coust << "error on new image: " << compute_error(img2) << std::endl;
		std::coust << "error on old image: " << compute_error(img) << std::endl;
#endif
		delete img2;
	}
	gui["calib_object"] = img;
	gui["calib_object"].render();
	fps.wait();
}

void finalize(){
	delete corners;
	delete camc;
}

int main(int n, char **args){
	paex
	("-i","defines input device and parameters")
	("-s","defines image size to use")
	("-cbs","defines number of checkerboard fields (XxY) to use")
	("-m","minimal count of pics of success");
	ICLApp app(n,args,"[m]-input|-i(device,device-params) -size|-s(Size) -checkerboardsize|-cbs(Size=7x9) -minImg|-m(int)",init,run);
	app.addFinalization(finalize);
	return app.exec();
	//return ICLApp(n,args,"-size|-s(Size=256x256) -minImg|-m",init,run).exec();
}

