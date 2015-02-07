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
#include <stdio.h>
#include <utility>
#include <ctime>
#include <iostream>
#include <string>
#include "server.h"

#define DEBUG 1

pthread_mutex_t data_mutex;
using namespace std;
using namespace cv;
using namespace cv::gpu;
using std::string;

TCPServer *myServer = NULL;
directions g_direc = {0};

int iLowH = 12;
// Gray: 0
int iHighH = 80;
// Gray: 179

int iLowS = 77;
// Gray: 0
int iHighS = 192;

int iLowV = 82;
//Gray: 213

int iHighV = 202;

void htondirec(struct directions *d){
    d->diterror = htonl(d->diterror);
    d->status = htonl(d->status);
}

// Gets error

int distanceError(int horizVal, Mat img) {
	cv::Size s = img.size();
//	float sheight = s.height;
	int swidth = s.width;

	if(DEBUG) {
		cout << "Width " << swidth << "Height " << s.height << endl;
	}

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


    if(DEBUG) {
		Moments oMoments = cv::moments(imgThresholded);

		int dM01 = oMoments.m01;
		int dM10 = oMoments.m10;
		int dArea = oMoments.m00;

		int posX = 0;
		int posY = 0;
		// if the area <= 10000, I consider that the there are no object in the image and it's because of the noise, the area is not zero
		if (dArea > 10000)
		{
			//calculate the position of the ball
			posX = dM10 / dArea;
			posY = dM01 / dArea;
		}

	//    cout << "error: " << distanceError(posX, imgThresholded) << endl;

		cv::circle(imgThresholded, Point(posX, posY), 12, Scalar(255,255,255));
    }
    return imgThresholded;
}

int filterAndGetError(const Mat& src) {
	Mat imgThresholded = yellowFilter(src);
    Moments oMoments = cv::moments(imgThresholded);

    int dM01 = oMoments.m01;
    int dM10 = oMoments.m10;
    int dArea = oMoments.m00;

    int posX = -1000;
    int posY = -1000;
    if (dArea > 10000)
    {
    	//calculate the position of the ball
    	posX = dM10 / dArea;
    	posY = dM01 / dArea;
    }
	return distanceError(posX, imgThresholded);
}


void *NetFace(void*)
{
    while(1){

        if (myServer->ListenOnClient()) {
            if (myServer->Receive()) {

                pthread_mutex_lock(&data_mutex);
                directions tmp = g_direc;
                pthread_mutex_unlock(&data_mutex);

                htondirec(&tmp);
                myServer->Send(tmp);

            }
        }

        if (myServer->ListenOnHost()) {
            myServer->AcceptConnection();
        }

    }
    return NULL;
}

int main()
{
	if(DEBUG) {
		namedWindow("Control", CV_WINDOW_AUTOSIZE);

		cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
		cvCreateTrackbar("HighH", "Control", &iHighH, 179);

		cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
		cvCreateTrackbar("HighS", "Control", &iHighS, 255);

		cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
		cvCreateTrackbar("HighV", "Control", &iHighV, 255);
	}


	myServer = new TCPServer();
	myServer->InitiateSocket ( (char *) "1180" );

	g_direc.diterror = 0;
	g_direc.status = 2;

	pthread_mutex_init(&data_mutex, NULL);
		pthread_t netThread = 0;
	pthread_create(&netThread, NULL, NetFace, NULL);

	int c;
	IplImage* color_img = NULL;
    CvCapture* cv_cap = cvCreateCameraCapture(0);
    cvSetCaptureProperty( cv_cap, CV_CAP_PROP_FRAME_WIDTH, 320 );

    cvSetCaptureProperty( cv_cap, CV_CAP_PROP_FRAME_HEIGHT, 240 );

    if(DEBUG) {
    	cvNamedWindow("Video",0); // create window
    }

	for(;;)
	{
		color_img = cvQueryFrame(cv_cap); // get frame
		if(color_img != 0)
		{
			Mat img(color_img);
			resize(img, img, Size(320, 240), 0, 0, INTER_CUBIC);

			directions new_direc = {0};

			Mat yellow_filtered_img;
			if(DEBUG) {
				Mat filtered_img(yellowFilter(img));

				imshow("Video", filtered_img); // show frame
				imshow("Unprocessed", img);
			}

	    	new_direc.diterror = filterAndGetError(img);

	    	pthread_mutex_lock(&data_mutex); //Lock structure direc
	    	g_direc = new_direc;
	    	g_direc.status++;
	    	pthread_mutex_unlock(&data_mutex);
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
