#include "opencv2/videoio/videoio_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/core/utility.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "point_tracker.h"

#include <unistd.h>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <cctype>


#define CAPTURE_HEIGHT 480
#define CAPTURE_WIDTH 640


using namespace std;
using namespace cv;


void detectAndDraw( Mat& img, CascadeClassifier& cascade,
					CascadeClassifier& nestedCascade,
					double scale, bool tryflip , FILE *file);
tracking_data track_point(Point center);
void staticDraw (Mat *frame);


string nestedCascadeName = "../haarcascade_eye_tree_eyeglasses.xml";
string cascadeName = "../haarcascade_frontalface_alt.xml";


int main()
{
	CascadeClassifier cascade, nestedCascade;
	Mat frame, frameCopy, image;
	CvCapture* capture = 0;
	double scale = 1;
	FILE *file;

/*
	const char *deviceName = "/dev/ttyUSB1";
	file = fopen(deviceName,"w");

	if (file == NULL)
	{
		cerr << "ERROR: Could not open " << deviceName << ".\n";
		return -1;
	}
*/

	if( !cascade.load( cascadeName ) )
	{
		cerr << "ERROR: Could not load classifier cascade" << endl;
		return -1;
	}

	capture = cvCaptureFromCAM(1);

	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, CAPTURE_WIDTH );
	cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, CAPTURE_HEIGHT );

	cvNamedWindow( "result", 1 );

	if( capture )
	{
		cout << "In capture ..." << endl;
		for(;;)
		{
			IplImage* iplImg = cvQueryFrame( capture );
			frame = cv::cvarrToMat(iplImg);

			staticDraw (&frame);

			if( frame.empty() )
				break;
			if( iplImg->origin == IPL_ORIGIN_TL )
				frame.copyTo( frameCopy );
			else
				flip( frame, frameCopy, 0 );

			detectAndDraw( frameCopy, cascade, nestedCascade, scale, true , file);

			if( waitKey( 10 ) >= 0 )
				goto _cleanup_;
		}

		waitKey(0);

_cleanup_:
		cvReleaseCapture( &capture );
	}

	cvDestroyWindow("result");

	return 0;
}

void sendCommand(const char* command, FILE *file)
{
	if (file == NULL)
	{
		printf("Error: failed to open device\n");
		exit(0);
	}

	fprintf(file,"%s", command);
	fflush(file);
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade,
					CascadeClassifier& nestedCascade,
					double scale, bool tryflip, FILE *file )
{
	int i = 0;
	double t = 0;
	vector<Rect> faces, faces2;
	Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );

	cvtColor( img, gray, COLOR_BGR2GRAY );
	resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
	equalizeHist( smallImg, smallImg );

	t = (double)cvGetTickCount();
	cascade.detectMultiScale( smallImg, faces,
		1.1, 2, 0
		//|CASCADE_FIND_BIGGEST_OBJECT
		//|CASCADE_DO_ROUGH_SEARCH
		|CASCADE_SCALE_IMAGE
		,
		Size(30, 30) );
	if( tryflip )
	{
		flip(smallImg, smallImg, 1);
		cascade.detectMultiScale( smallImg, faces2,
								 1.1, 2, 0
								 //|CASCADE_FIND_BIGGEST_OBJECT
								 //|CASCADE_DO_ROUGH_SEARCH
								 |CASCADE_SCALE_IMAGE
								 ,
								 Size(30, 30) );
		for( vector<Rect>::const_iterator r = faces2.begin(); r != faces2.end(); r++ )
		{
			faces.push_back(Rect(smallImg.cols - r->x - r->width, r->y, r->width, r->height));
		}
	}
	t = (double)cvGetTickCount() - t;



	int biggestRadius = 0;
	int biggestFaceIndex = 0;
	Point face_center;
	int face_radius;
	for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
	{
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		int radius;

		double aspect_ratio = (double)r->width/r->height;
		if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
		{
			center.x = cvRound((r->x + r->width*0.5)*scale);
			center.y = cvRound((r->y + r->height*0.5)*scale);
			radius = cvRound((r->width + r->height)*0.25*scale);

			if(radius > biggestRadius)
			{
				biggestRadius = radius;
				biggestFaceIndex = i;
				face_center = center;
				face_radius = radius;
			}
		}
	}

	circle( img, face_center, 4, Scalar( 0, 255, 0 ), 10, 8 );
	circle( img, face_center, face_radius, Scalar(255, 0, 0 ), 3, 8, 0 );

	tracking_data deviation = get_offset_angles (920, 4/3, CAPTURE_WIDTH, CAPTURE_HEIGHT, face_center);

	printf("\n====================================================================\n");
	printf("Horizontal position of the point : %d\n", face_center.x);
	printf("Number of steps to center horizontally : %f\n", deviation.grades_from_center_x);
	printf("----------------------------------\n");
	printf("Vertical position of the point : %d\n", face_center.y);
	printf("Number of steps to center vertically : %f\n", deviation.grades_from_center_y);
	printf("====================================================================\n\n");

	cv::imshow( "result", img );
}

void staticDraw (Mat *frame)
{
	Point p1, p2;

	//Vertical center line
	p1.x = CAPTURE_WIDTH / 2;	p1.y = 0;
	p2.x = CAPTURE_WIDTH / 2;	p2.y = CAPTURE_HEIGHT;
	line( *frame, p1, p2, Scalar( 0, 255, 0 ), 1, 8 );

	//Horizontal center line
	p1.y = CAPTURE_HEIGHT / 2;	p1.x = 0;
	p2.y = CAPTURE_HEIGHT / 2;	p2.x = CAPTURE_WIDTH;
	line( *frame, p1, p2, Scalar( 0, 255, 0 ), 1, 8 );

	//Center of the window
	p1.x = CAPTURE_WIDTH / 2;	p1.y = CAPTURE_HEIGHT / 2;
	circle( *frame, p1, 4, Scalar( 0, 0, 225 ), 2, 8 );
}