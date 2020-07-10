#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <hls_opencv.h>
#include "main.h"


void savefile(const std::string path, cv::InputArray inArr)
{
	double min;
	double max;
	cv::minMaxIdx(inArr, &min, &max);
	cv::Mat adjMap;
	cv::convertScaleAbs(inArr, adjMap, 255 / max);
	cv::imwrite(path,adjMap);
}


char threshold = 40;

// Image File path
char outImage[IMG_ROWS][IMG_COLS];


int main()
{
	// Read input image
	printf("Load image %s\n",INPUT_IMAGE);
	cv::Mat image1;

	image1 = cv::imread(INPUT_IMAGE);
	cv::cvtColor(image1, image1, CV_BGR2GRAY);//grayscale conversion
	printf("Image Rows:%d Cols:%d\n",image1.rows, image1.cols);


	// Define streams for input and output
	hls::stream<uint_8_side_channel> inputData;
	hls::stream<int_8_side_channel> segmentedData;

	// OpenCV mat that point to a array (cv::Size(Width, Height))
	cv::Mat OutputImg(cv::Size(image1.cols, image1.rows), CV_8UC1, outImage, cv::Mat::AUTO_STEP);

	// Populate the input stream with the image bytes
	for (int i=0; i < image1.rows; i++)
	{
		for (int j=0; i < image1.cols; j++)
		{
			uint_8_side_channel valIn;
			valIn.data = image1.at<unsigned char>(i,j);
			valIn.keep = 1; valIn.strb = 1; valIn.user = 1; valIn.id = 0; valIn.dest = 0;
			inputData << valIn;
		}
	}


	printf("Calling image segmentation\n");
	imageSegmentation(inputData, segmentedData, threshold); //Image segmentation
	printf("image segmentation applied\n");

	// Take data from the output stream to our array outImage )
	for (int i=0; i < image1.rows; i++)
	{
		for (int j=0; j < image1.cols; j++)
		{
			int_8_side_channel valOut;
			segmentedData.read(valOut);
			outImage[i][j] = valOut.data;
		}
	}

	// Save image out file or display

		printf("Saving image\n");
		savefile(std::string(OUTPUT_IMAGE) ,OutputImg);

	return 0;
}


