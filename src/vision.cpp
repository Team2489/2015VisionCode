/*
 * vision.cpp
 *
 *  Created on: Jan 26, 2015
 *      Author: vigneshv
 */

#include <iostream>
#include "opencv2/gpu/gpu.hpp"
#include "opencv2/highgui/highgui.hpp"
# include <string>
# include <unistd.h>
# include <stdio.h>
# include <utility>
# include <libsocket/inetserverstream.hpp>
# include <libsocket/exception.hpp>

# include <libsocket/select.hpp>
# include <libsocket/socket.hpp>

using namespace std;
using namespace cv;
using namespace cv::gpu;
using std::string;
using libsocket::inet_stream_server;
using libsocket::inet_stream;
using libsocket::selectset;

int iLowH = 2;
// Gray: 0
int iHighH = 147;
// Gray: 179

int iLowS = 109;
// Gray: 0
int iHighS = 255;

int iLowV = 63;
//Gray: 213

int iHighV = 255;



// Gets error

float distanceError(float horizVal, Mat img) {
	cv::Size s = img.size();
//	float sheight = s.height;
	float swidth = s.width;

	return (horizVal-swidth/2);
}

Mat yellowFilter(const Mat& src)
{
    Mat redOnly;
    cvtColor(src, redOnly, COLOR_BGR2HSV);
    Mat imgThresholded;
    inRange(redOnly, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    dilate( imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );
    erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)) );

    blur(imgThresholded, imgThresholded, Size(3,3) );
    GpuMat imgThresholded_gpu(imgThresholded);

    cv::gpu::Canny(imgThresholded_gpu, imgThresholded_gpu, 100, 300, 3);

    imgThresholded_gpu.download(imgThresholded);


//    cout << "error: " << distanceError(posX, imgThresholded) << endl;

//    cv::circle(imgThresholded, Point(posX, posY), 12, Scalar(255,255,255));
    return imgThresholded;
}

float filterAndGetError(const Mat& src) {
	Mat imgThresholded = yellowFilter(src);
    Moments oMoments = cv::moments(imgThresholded);

    double dM01 = oMoments.m01;
    double dM10 = oMoments.m10;
    double dArea = oMoments.m00;

    int posX = 0;
    int posY = 0;
    // if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
    if (dArea > 10000)
    {
    	//calculate the position of the ball
    	posX = dM10 / dArea;
    	posY = dM01 / dArea;
    }
	return distanceError(posX, imgThresholded);
}


int main()
{
//	namedWindow("Control", CV_WINDOW_AUTOSIZE);
//
//	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
//	cvCreateTrackbar("HighH", "Control", &iHighH, 179);
//
//	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
//	cvCreateTrackbar("HighS", "Control", &iHighS, 255);
//
//	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
//	cvCreateTrackbar("HighV", "Control", &iHighV, 255);
//
//
//	int c;
//	IplImage* color_img;
//    CvCapture* cv_cap = cvCaptureFromCAM(0);
//    cvNamedWindow("Video",0); // create window
//	for(;;)
//	{
//		color_img = cvQueryFrame(cv_cap); // get frame
//		if(color_img != 0)
//		{
////			Mat grayscale_img;
//			Mat img(color_img);
////			GpuMat gpu_grayscale_img;
////			cvtColor(gpu_img, gpu_grayscale_img, CV_BGR2GRAY);
////			gpu_grayscale_img.download(grayscale_img);
//
//			Mat yellow_filtered_img;
//			Mat filtered_img(yellowFilter(img));
//
//			imshow("Video", filtered_img); // show frame
//			imshow("Unprocessed", img);
//		}
//		c = cvWaitKey(10); // wait 10 ms or for key stroke
//		if(c == 27)
//			break; // if ESC, break and quit
//	}
//	/* clean up */
//	cvReleaseCapture( &cv_cap );
//	cvDestroyWindow("Video");
//	return 0;

		string host = "::1";
	    string port = "1235";
	    string answ;

	    try {
		inet_stream_server srv(host,port,LIBSOCKET_IPv6);
		inet_stream* cl1;

		selectset<inet_stream_server> set1;
		set1.add_fd(srv,LIBSOCKET_READ);

		for ( ;; )
		{
	    	std::cout << "asdj\n";

		    /********* SELECT PART **********/
		    libsocket::selectset<inet_stream_server>::ready_socks readypair; // Create pair (libsocket::fd_struct is the return type of selectset::wait()
		    readypair = set1.wait(); // Wait for a connection and save the pair to the var
		    inet_stream_server* ready_srv = dynamic_cast<inet_stream_server*>(readypair.first.back()); // Get the last fd of the LIBSOCKET_READ vector (.first) of the pair and cast the socket* to inet_stream_server*

		    readypair.first.pop_back(); // delete the fd from the pair
		    /*******************************/

		    cl1 = ready_srv->accept();
		    answ.resize(2000);

		    while (answ.size() > 0) {
		    	*cl1 >> answ;
		    	std::cout << answ << std::endl;
			    answ.resize(2000);
		    }


		    cl1->destroy();
		}

		srv.destroy();

	    } catch (const libsocket::socket_exception& exc)
	    {
		std::cerr << exc.mesg << std::endl;
	    }
	    return 0;
}
