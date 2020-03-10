/***********************************************************************************
	Created:	17:9:2002
	FileName: 	hdrloader.h
	Author:		Igor Kravtchenko
				https://www.flipcode.com/archives/HDR_Image_Reader.shtml

	Info:		Load HDR image and convert to a set of float32 RGB triplet.
************************************************************************************/

class HDRLoaderResult {
public:
	HDRLoaderResult();
	~HDRLoaderResult();

	int width, height;
	// each pixel takes 3 float32, each component can be of any value...
	float *cols;
};

class HDRLoader {
public:
	static bool load(const char *fileName, HDRLoaderResult &res);
};
