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

GpuMat yellowFilter(const GpuMat& src)
{
    GpuMat redOnly;
    inRange(src, Scalar(0, 0, 0), Scalar(0, 0, 255), redOnly);

    return redOnly;
}

int main()
{
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
			GpuMat gpu_img(color_img);
//			GpuMat gpu_grayscale_img;
//			cvtColor(gpu_img, gpu_grayscale_img, CV_BGR2GRAY);
//			gpu_grayscale_img.download(grayscale_img);

			Mat yellow_filtered_img;
			GpuMat filtered_img_gpu(yellowFilter(gpu_img));
			filtered_img_gpu.download(yellow_filtered_img);

			imshow("Video", yellow_filtered_img); // show frame
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
