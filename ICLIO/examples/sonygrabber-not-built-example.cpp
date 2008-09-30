#include <iclFileWriter.h>
#include <iclSonyFwGrabber.h>

using namespace std;
using namespace icl;

int main(int n, char **ppc) {
#ifdef WIN32
#ifdef WITH_SONYIIDC
	SonyFwGrabber *grabber = new SonyFwGrabber();
	if (!grabber->init()) {
		cout << "init failed!" << endl;
		Sleep(5000);
	}

	FileWriter writer("sony_bayer.pgm");

	const ImgBase* img = grabber->grab();
	writer.write(img);

	ImgBase *left, *right;
	left = 0;
	right = 0;
	
	grabber->grabStereo(&left, &right);
	writer.setFileName("left_0.pgm");
	writer.write(left);
	writer.setFileName("right_0.pgm");
	writer.write(right);

	grabber->grabStereo(&left, &right);
	writer.setFileName("left_1.pgm");
	writer.write(left);
	writer.setFileName("right_1.pgm");
	writer.write(right);

	delete grabber;

	return 0;
#endif
#endif

}

