#include <ICLQuick/Common.h>
#include <ICLIO/OpenCVCamGrabber.h>
#include <ICLIO/OpenCVVideoWriter.h>
#include <ICLUtils/Time.h>

GUI gui("hsplit");

ImgBase *image=0;
Mutex mutex;
SmartPtr<OpenCVCamGrabber> cg=0;
SmartPtr<OpenCVVideoWriter> vw = 0;

std::string filename = "";

bool cap = false;

void run(){
	Mutex::Locker lock(mutex);
	cg->grab(&image);
	gui["image"] = image;

	if(cap)
		vw->write(image);

	gui["image"].update();

	gui_FPSHandle(fps);
	fps.update();
}

///sets the new brightness for capturedevice
void set_brightness(){
	Mutex::Locker lock(mutex);
	float f = gui.getValue<float>("bright");
	ostringstream Str;
	Str << f ;
	cg->setProperty("brightness", Str.str());
}

///sets the new hue for capturedevice
void set_hue(){
	Mutex::Locker lock(mutex);
	float f = gui.getValue<float>("hue");
	ostringstream Str;
	Str << f ;
	cg->setProperty("hue", Str.str());
}

///sets the new contrast for capturedevice
void set_contrast(){
	Mutex::Locker lock(mutex);
	float f = gui.getValue<float>("cont");
	ostringstream Str;
	Str << f ;
	cg->setProperty("contrast", Str.str());
}

///sets the new saturation for capturedevice
void set_saturation(){
	Mutex::Locker lock(mutex);
	float f = gui.getValue<float>("sat");
	ostringstream Str;
	Str << f ;
	cg->setProperty("saturation", Str.str());
}

///sets the new size for capturedevice
void set_size(){
	Mutex::Locker lock(mutex);
	int i = gui["size"];
	ostringstream Str;
	Str << i <<"x"<<int(0.75*i) ;
	cg->setProperty("size", Str.str());
}

///returns a timestamp
std::string getTimestamp(std::string suffix){
	Time t;
	int64_t ms = (t.now()).toMilliSeconds();
	ostringstream Str;
	Str << ms << suffix;
	return Str.str();
}

/// starts the capturing
void startcap(){
	Mutex::Locker lock(mutex);
	if(!vw){
		if(filename == "")
			filename = getTimestamp(".avi");
		//CV_FOURCC('I', 'Y', 'U', 'V')
		//CV_FOURCC('P','I','M','1')    = MPEG-1 codec
		//CV_FOURCC('M','J','P','G')    = motion-jpeg codec (does not work well)
		//CV_FOURCC('M', 'P', '4', '2') = MPEG-4.2 codec
		//CV_FOURCC('D', 'I', 'V', '3') = MPEG-4.3 codec
		//CV_FOURCC('D', 'I', 'V', 'X') = MPEG-4 codec
		//CV_FOURCC('U', '2', '6', '3') = H263 codec
		//CV_FOURCC('I', '2', '6', '3') = H263I codec
		//CV_FOURCC('F', 'L', 'V', '1') = FLV1 codec
		vw = new OpenCVVideoWriter(filename ,OpenCVVideoWriter::MOTION_JPEG, 30.0, icl::Size(640,480), 1);
	}
	cap = true;
}

/// stops the capturing
void stopcap(){
	Mutex::Locker lock(mutex);
	cap = false;
	filename = "";
	vw = 0;
}

///takes a snapshot of device and writes it to a file
void takeSnapshot(){
	Mutex::Locker lock(mutex);
	if(cg){
		ImgBase *img=0;
		cg->grab(&img);
		FileWriter f(getTimestamp(".png"));
		f.write(img);
		delete img;
	}
}

///prints all properties of current device to shell
void printAllProperties(){
	Mutex::Locker lock(mutex);
	if(cg){
		cout << "brightness: "<<cg->getValue("brightness")<<endl;
		cout << "contrast: "<<cg->getValue("contrast")<<endl;
		cout << "saturation: "<<cg->getValue("saturation")<<endl;
		cout << "hue: "<<cg->getValue("hue")<<endl;
		cout << "size: "<<cg->getValue("size")<<endl;
	}
}

void init(){
	gui << (GUI("vbox")
			<<	"image[@handle=image@minsize=20x20]"
			<< "fps(10)[@handle=fps@maxsize=100x2@minsize=8x2]"//)
	);
	gui << (GUI("vbox[@minsize=20x1]")
			<< "slider(0,1000,400)[@out=size@handle=hsize@label=size]"
			<< "label(only for cameras)"
			<< "fslider(0,1,0.1)[@out=bright@handle=hbright@label=brightness]"
			<< "fslider(0,1,0.1)[@out=cont@handle=hcont@label=contrast]"
			<< "fslider(0,1,0.1)[@out=sat@handle=hsat@label=saturation]"
			<< "fslider(0,1,0.1)[@out=hue@handle=hhue@label=hue]"
			<< "button(start videocapture)[@out=startc@handle=startc_]"
			<< "button(stop videocapture)[@out=stopc@handle=stopc_]"
			<< "button(info)[@out=info@handle=info_]"
			<< "button(snapshot)[@out=snap@handle=snapshot]"
	);
	gui.show();

	gui["hbright"].registerCallback(new GUI::Callback(set_brightness));
	gui["hcont"].registerCallback(new GUI::Callback(set_contrast));
	gui["hsat"].registerCallback(new GUI::Callback(set_saturation));
	gui["hhue"].registerCallback(new GUI::Callback(set_hue));
	gui["hsize"].registerCallback(new GUI::Callback(set_size));
	gui["startc_"].registerCallback(new GUI::Callback(startcap));
	gui["stopc_"].registerCallback(new GUI::Callback(stopcap));
	gui["info_"].registerCallback(new GUI::Callback(printAllProperties));
	gui["snapshot"].registerCallback(new GUI::Callback(takeSnapshot));
}



int main(int n, char **args){
	if (n==3){
		cout << "n==3\n";
		cg = new OpenCVCamGrabber(parse<int>(args[1]));
		cg->setIgnoreDesiredParams(true);
		filename = str(args[2]);
	} else if(n==2){
		cout << "n==2\n";
		cg = new OpenCVCamGrabber();
		cg->setIgnoreDesiredParams(true);
		filename = str(args[1]);
	} else {
		cg = new OpenCVCamGrabber();
		cg->setIgnoreDesiredParams(true);
		filename = getTimestamp(".avi");
	}
	return ICLApp(n,args,"",init,run).exec();
}
