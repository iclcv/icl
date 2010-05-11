/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLOpenCV/examples/opencv-videograbber-example.cpp     **
** Module : ICLOpenCV                                              **
** Authors: Christian Groszewski 			           			   **
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
*********************************************************************/
#include <ICLQuick/Common.h>
#include <ICLIO/OpenCVVideoGrabber.h>

GUI gui("hsplit");
ImgBase *image=0;
Mutex mutex;
SmartPtr<OpenCVVideoGrabber> g=0;
char *filename=0;

bool play = false;

double framecount = 0;

///returns a timestamp
std::string getTimestamp(std::string suffix){
	Time t;
	int64_t ms = (t.now()).toMilliSeconds();
	ostringstream Str;
	Str << ms << suffix;
	return Str.str();
}

void startplaying(){
	Mutex::Locker lock(mutex);
	if(!g){
		g = new OpenCVVideoGrabber(filename);
		g->setProperty("use_video_fps","");
		g->setIgnoreDesiredParams(true);
		framecount = parse<double>(g->getValue("frame_count"));
	}else{
		g->setProperty("pos_msec","0.0");
	}
	play = true;
}

void wait(){
	Mutex::Locker lock(mutex);
	play = false;
}

void stop(){
	Mutex::Locker lock(mutex);
	play = false;
	g->setProperty("pos_msec","0.0");
}

void set_size(){
	Mutex::Locker lock(mutex);
	int f = gui["size"];
	g->setIgnoreDesiredParams(false);
	g->setDesiredSize(Size(f,int(0.75*f)));
}

void set_fps(){
	Mutex::Locker lock(mutex);
	float f = gui["fps_"];
	ostringstream Str;
	Str << f ;
	g->setProperty("use_video_fps", Str.str());
}

void printAllProperties(){
	Mutex::Locker lock(mutex);
	if(g){
		cout << "size: "<<g->getValue("size")<<endl;
		cout << "fps: "<<g->getValue("fps")<<endl;
		cout << "frame_count"<<g->getValue("frame_count")<<endl;
		cout << "fourcc: "<<g->getValue("fourcc")<<endl;
		cout << "pos_avi_ratio: "<<g->getValue("pos_avi_ratio")<<endl;
		cout << "pos_msec: "<<g->getValue("pos_msec")<<endl;
		cout << "pos_frames:"<<g->getValue("pos_frames")<<endl;
	}
}

void run(){
	Mutex::Locker lock(mutex);
	if(play && (parse<double>(g->getValue("pos_frames"))<framecount-1)){
		gui["image"] = g->grab(&image);
		gui["image"].update();

		ostringstream Str;
		Str << g->getValue("pos_frames") <<"/"<<g->getValue("frame_count") << "   " << g->getValue("pos_msec") << "   "<< g->getValue("pos_avi_ratio");
		gui["frames"]=Str.str();
		gui_FPSHandle(fps);
		fps.update();
	}
}

void init(){
	gui << (GUI("vbox")
			<<	"image[@handle=image@minsize=20x20]"
			<< "fps(10)[@handle=fps@maxsize=100x2@minsize=8x2]"
			<< "label(0/0)[@out=frames@handle=frames]"
	);

	gui << (GUI("vbox[@minsize=20x1]")
			<< "label("")[@out=fourcc@label=fourcc]"
			<< "slider(0,1000,400)[@out=size@handle=hsize@label=size]"
			<< "fslider(0,1000,400)[@out=height@handle=hheight@label=height]"
			<< "fslider(0,1000,30)[@out=fps_@handle=hfps_@label=fps]"

			<< (GUI("hbox[@minsize=6x1]")
					<< "button(play)[@out=play@handle=play_]"
					<< "button(pause)[@out=pause@handle=pause_]"
					<< "button(stop)[@out=stop@handle=stop_]"
			)
			<< "button(info)[@out=info@handle=info_@label=info]"
	);
	gui.show();

	gui["hsize"].registerCallback(new GUI::Callback(set_size));
	gui["hfps_"].registerCallback(new GUI::Callback(set_fps));
	gui["info_"].registerCallback(new GUI::Callback(printAllProperties));
	gui["play_"].registerCallback(new GUI::Callback(startplaying));
	gui["stop_"].registerCallback(new GUI::Callback(stop));
	gui["pause_"].registerCallback(new GUI::Callback(wait));

}

int main(int n, char **args){
	if(n==2){
		filename = args[1];
	} else {
		return 0;
	}
	return ICLApp(n,args,"",init,run).exec();
}
