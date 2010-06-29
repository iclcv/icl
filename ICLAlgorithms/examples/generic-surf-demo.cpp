/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLAlgorithms/examples/opensurf-demo.cpp               **
** Module : ICLAlgorithms                                          **
** Authors: Christian Groszewski                                   **
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

#include <ICLQt/GUI.h>

#include <ICLQt/QtMacros.h>
#include <ICLQt/Application.h>
#include <ICLUtils/Mutex.h>
#include <ICLAlgorithms/GenericSurfDetector.h>
#include <ICLIO/OpenCVCamGrabber.h>
#include <ICLIO/GenericGrabber.h>
#include <ICLIO/FileGrabber.h>
#include <ICLUtils/ProgArg.h>

using namespace icl;
Mutex mutex;
SmartPtr<GenericGrabber> grabber=0;
//GenericSurfDetector surf(0,false,3,4,3);
//GenericSurfDetector surf(0,500,1,3,4);
SmartPtr<GenericSurfDetector> surf = new GenericSurfDetector(GenericSurfDetector::OPENSURF);

GUI gui("hsplit");

void set_octaves(){
	Mutex::Locker lock(mutex);
	int i = gui.getValue<int>("octaves");
	surf->setOctaves(i);
}

void set_intervals(){
	Mutex::Locker lock(mutex);
	int i = gui.getValue<int>("intervals");
	surf->setOctavelayer(i);
}

void set_samples(){
	Mutex::Locker lock(mutex);
	if(surf->getImpl() == GenericSurfDetector::OPENSURF){
		int i = gui.getValue<int>("samples");
		surf->setInitSamples(i);
	}
}

void set_thresh(){
	Mutex::Locker lock(mutex);
	float f = gui.getValue<float>("thresh");
	surf->setThreshold(f);
}

void set_ri(){
	Mutex::Locker lock(mutex);
	gui_CheckBox(ri_handle);
	if(surf->getImpl() == GenericSurfDetector::OPENSURF){
		surf->setRotationInvariant(ri_handle.isChecked());
	}else{
		if(ri_handle.isChecked())
			surf->setExtended(1);
		else
			surf->setExtended(0);
	}
}

void set_os(){
	Mutex::Locker lock(mutex);
	gui_CheckBox(os_handle);
	const ImgBase *im = surf->getObjectImg();
		surf = new GenericSurfDetector(GenericSurfDetector::OPENSURF);
	}else{
		surf = new GenericSurfDetector(GenericSurfDetector::OPENCV);
	}
	surf->setObjectImg(im);
}

void make_snapshot(){
	Mutex::Locker lock(mutex);
	if(grabber){
		const ImgBase *tmp = grabber->grab();
		surf->setObjectImg(tmp);
		gui_DrawHandle(draw_object);
		draw_object = surf->getObjectImg();
		draw_object->lock();
		draw_object->reset();
		draw_object->unlock();
		draw_object->update();
	}
}

void run(){
	Mutex::Locker lock(mutex);
	const ImgBase *image = grabber->grab();

	gui_DrawHandle(draw_result);
	gui_DrawHandle(draw_object);
	gui_DrawHandle(draw_image);
	draw_result = image;
	draw_image = image;
	draw_object = surf->getObjectImg();

	draw_result->lock();
	draw_object->lock();
	draw_image->lock();

	draw_image->reset();
	draw_result->reset();
	draw_object->reset();

	gui_CheckBox(sf_handle);
	if(sf_handle.isChecked()){
		const std::vector<GenericSurfDetector::GenericPoint> features = surf->extractFeatures(image);
		surf->visualizeFeatures(**draw_image,features);
	}

	gui_CheckBox(sm_handle);
	if(sm_handle.isChecked()){
		const std::vector<std::pair<GenericSurfDetector::GenericPoint,
		GenericSurfDetector::GenericPoint> > &matches = surf->match(image);
		surf->visualizeMatches(**draw_object,**draw_result,matches);
	}

	draw_result->unlock();
	draw_object->unlock();
	draw_image->unlock();
	draw_object->update();
	draw_result->update();
	draw_image->update();
	gui_FPSHandle(fps);
	fps.update();
}

void init(){
	grabber = new GenericGrabber(FROM_PROGARG("-i"));
	grabber->setIgnoreDesiredParams(true);
	if(pa("-f")){
		FileGrabber *fg = new FileGrabber(parse<std::string>(pa("-f")));
		fg->setIgnoreDesiredParams(true);
		const ImgBase *obj = fg->grab();
		surf->setObjectImg(obj);
		delete fg;
	}
	gui << (GUI("vbox")
			<<(GUI("hbox")
					<< (GUI("vbox")
							<< "draw[@handle=draw_object@minsize=16x12@label=result of surf match]"
							<< "button(snapshot)[@handle=snap_handle]")
							<< "draw[@handle=draw_result@minsize=16x12@label=result of surf match]"
							<< "draw[@handle=draw_image@minsize=16x12@label=result of surf]")

							<<(GUI("hbox")
									<< "fps(10)[@handle=fps@maxsize=100x2@minsize=8x2]"))
									<< (GUI("vbox[@minsize=8x1]")
											<< "checkbox(opensurf,checked)[@out=os@handle=os_handle]"
											<< "checkbox(show_matches,unchecked)[@out=sm@handle=sm_handle]"
											<< "checkbox(show_features,unchecked)[@out=sf@handle=sf_handle]"

											//RotationInvariant =false ;
											<< "checkbox(rotatationinvariance/extended,unchecked)[@out=ri@handle=ri_handle]"
											//octaves = 4
											<< "slider(0,20,4)[@out=octaves@handle=oct_handle@label=octaves]"
											//Intervals = 4
											<< "slider(0,20,4)[@out=intervals@handle=intervals_handle@label=intervals/octavelayer]"
											//initsamples =2
											<< "slider(0,10,2)[@out=samples@handle=sample_handle@label=init samples]"
											//ResponseTresh =0.004
											<< "fslider(0,0.04,0.001)[@out=thresh@handle=thresh_handle@label=threshold]"
									);

	gui.show();
	gui.getValue<CheckBoxHandle>("os_handle").registerCallback(new GUI::Callback(set_os));
	gui.getValue<CheckBoxHandle>("ri_handle").registerCallback(new GUI::Callback(set_ri));
	gui["snap_handle"].registerCallback(new GUI::Callback(make_snapshot));
	gui["oct_handle"].registerCallback(new GUI::Callback(set_octaves));
	gui["intervals_handle"].registerCallback(new GUI::Callback(set_intervals));
	gui["sample_handle"].registerCallback(new GUI::Callback(set_samples));
	gui["thresh_handle"].registerCallback(new GUI::Callback(set_thresh));
	gui_DrawHandle(draw_object);
	draw_object = surf->getObjectImg();
}

int main(int n, char **args){
	return ICLApp(n,args,"-input|-i(devicetype,device) -file|-f(objectfile)",init,run).exec();
}
