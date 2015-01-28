/*
 * vision.cpp
 *
 *  Created on: Jan 26, 2015
 *      Author: vigneshv
 */

#include <iostream>
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

int iLowH = 2;
int iHighH = 157;

int iLowS = 149;
int iHighS = 255;

int iLowV = 63;
int iHighV = 255;

Mat yellowFilter(const Mat& src)
{
    Mat redOnly;
    cvtColor(src, redOnly, COLOR_BGR2HSV);
    Mat imgThresholded;
    inRange(redOnly, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);


    return imgThresholded;
}

int main()
{
	namedWindow("Control", CV_WINDOW_AUTOSIZE);

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);


	int c;
	IplImage* color_img;
    CvCapture* cv_cap = cvCaptureFromCAM(0);
    cvNamedWindow("Video",0); // create window
	for(;;)
	{
		color_img = cvQueryFrame(cv_cap); // get frame

		if(color_img != 0)
		{
//			Mat grayscale_img;
			Mat img(color_img);
//			GpuMat gpu_grayscale_img;
//			cvtColor(gpu_img, gpu_grayscale_img, CV_BGR2GRAY);
//			gpu_grayscale_img.download(grayscale_img);

			Mat yellow_filtered_img;
			Mat filtered_img(yellowFilter(img));

			imshow("Video", filtered_img); // show frame
			imshow("Unprocessed", img);
		}
		c = cvWaitKey(10); // wait 10 ms or for key stroke
		if(c == 27)
			break; // if ESC, break and quit
	}
	/* clean up */
	cvReleaseCapture( &cv_cap );
	cvDestroyWindow("Video");
	return 0;
}
