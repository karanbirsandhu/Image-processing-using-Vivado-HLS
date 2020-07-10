#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <hls_opencv.h>
#include "core.h"

void saveImage(const std::string path, cv::InputArray inArr)
{
	double min;
	double max;
	cv::minMaxIdx(inArr, &min, &max);
	cv::Mat adjMap;
	cv::convertScaleAbs(inArr, adjMap, 255 / max);
	cv::imwrite(path,adjMap);
}


// Sobel
char kernel[3*3] = {
		-1, -2, -1,
		 0, 0, 0,
		1, 2, 1,
};

// Image File path
char outImage[IMG_HEIGHT][IMG_WIDTH];


int main()
{
	// Read input image
	printf("Load image %s\n",INPUT_IMAGE);
	cv::Mat imageSrc;
	imageSrc = cv::imread(INPUT_IMAGE);
	// Convert to grayscale
	cv::cvtColor(imageSrc, imageSrc, CV_BGR2GRAY);
	printf("Image Rows:%d Cols:%d\n",imageSrc.rows, imageSrc.cols);

	// Define streams for input and output
	hls::stream<uint_8_channel> inputStream;
	hls::stream<int_8_channel> outputStream;


	cv::Mat imgCvOut(cv::Size(imageSrc.cols, imageSrc.rows), CV_8UC1, outImage, cv::Mat::AUTO_STEP);

	// Populate the input stream with the image data
	for (int idxRows=0; idxRows < imageSrc.rows; idxRows++)
	{
		for (int idxCols=0; idxCols < imageSrc.cols; idxCols++)
		{
			uint_8_channel valIn;
			valIn.data = imageSrc.at<unsigned char>(idxRows,idxCols);
			valIn.keep = 1; valIn.strb = 1; valIn.user = 1; valIn.id = 0; valIn.dest = 0;
			inputStream << valIn;
		}
	}

	//Applying sobel filter
	printf("Calling Sobel filter\n");
	doImgProc(inputStream, outputStream, kernel);
	printf("Sobel filter applied\n");

	// Take data from the output stream to our array outImage
	for (int idxRows=0; idxRows < imageSrc.rows; idxRows++)
	{
		for (int idxCols=0; idxCols < imageSrc.cols; idxCols++)
		{
			int_8_channel valOut;
			outputStream.read(valOut);
			outImage[idxRows][idxCols] = valOut.data;
		}
	}

	// Save image out file or display

		printf("Saving image\n");
		saveImage(std::string(OUTPUT_IMAGE) ,imgCvOut);

	return 0;
}


