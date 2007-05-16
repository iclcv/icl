#ifdef WIN32
#ifdef WITH_SONYIIDC

#include <iclFileWriter.h>
#include <iclSonyFwGrabber.h>

using namespace std;
using namespace icl;

int main() {

	SonyFwGrabber *grabber = new SonyFwGrabber();
	if (!grabber->init()) {
		cout << "init failed!" << endl;
		Sleep(5000);
	}

	FileWriter writer("sony_bayer.pgm");

	const ImgBase* img = grabber->grab();
	writer.write(img);

	delete img;
	delete grabber;
	return 0;
}

#endif
#endif