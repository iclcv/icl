

#include <ICLQt/Common.h>
#include <ICLIO/ImageCompressor.h>
#include <ICLIO/Kinect11BitCompressor.h>
#include <ICLUtils/StackTimer.h>

GUI gui;
GenericGrabber grabber;
Img32f depth_f;

void init()  {

	grabber.init(pa("-i"));

	gui << Draw().label("View Original").handle("view_original")
				 << Draw().label("View received").handle("view_received");

	gui << Show();

}

void run() {

	depth_f = *grabber.grab()->as32f();
	Img16s *img16s = depth_f.convert(icl::core::depth16s)->as16s();

	ImageCompressor compressor(ImageCompressor::CompressionSpec("1611","0"));

	ImageCompressor::CompressedData compressed_data;
	{
		BENCHMARK_THIS_SECTION(packing);
		compressed_data = compressor.compress(img16s,false);
	}
	const Img16s *result;
	{
		BENCHMARK_THIS_SECTION(unpacking);
		result = compressor.uncompress(compressed_data.bytes,compressed_data.len)->as16s();
	}

	gui["view_original"] = img16s;
	gui["view_received"] = result;

	gui["view_original"].render();
	gui["view_received"].render();

	delete img16s;

}


int main(int argc, char **argv) {
	ICLApplication app(argc,argv,"-input|-i(2)",init,run);

	return app.exec();
}
