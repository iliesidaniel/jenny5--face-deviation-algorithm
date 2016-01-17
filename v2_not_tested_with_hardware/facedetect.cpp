#include "opencv2/videoio/videoio_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/core/utility.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include <unistd.h>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <cctype>


#define HORIZONTAL_FIELD_OF_VIEW 70.42
#define VERTICAL_FIELD_OF_VIEW 43.30
#define MOTOR_STEP_DEGREE 1.8
#define DELAY_UNTIL_RECALC 0
#define CAPTURE_HEIGHT 480
#define CAPTURE_WIDTH 640


using namespace std;
using namespace cv;


void detectAndDraw( Mat& img, CascadeClassifier& cascade,
					CascadeClassifier& nestedCascade,
					double scale, bool tryflip , FILE *file);
void track_point(Point center);
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
	const static Scalar colors[] =  { CV_RGB(0,0,255),
		CV_RGB(0,128,255),
		CV_RGB(0,255,255),
		CV_RGB(0,255,0),
		CV_RGB(255,128,0),
		CV_RGB(255,255,0),
		CV_RGB(255,0,0),
		CV_RGB(255,0,255)} ;
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
	for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
	{
		Mat smallImgROI;
		vector<Rect> nestedObjects;
		Point center;
		Scalar color = colors[i%8];
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
			}
		}
	}

	i = 0;
	int leftLimit, rightLimit;
	for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
	{
		if(i == biggestFaceIndex)
		{
			Mat smallImgROI;
			vector<Rect> nestedObjects;
			Point center;
			Scalar color = colors[i%8];
			int radius;

			double aspect_ratio = (double)r->width/r->height;
			if( 0.75 < aspect_ratio && aspect_ratio < 1.3 )
			{
				center.x = cvRound((r->x + r->width*0.5)*scale);
				center.y = cvRound((r->y + r->height*0.5)*scale);
				radius = cvRound((r->width + r->height)*0.25*scale);

				circle( img, center, 4, Scalar( 0, 255, 0 ), 10, 8 );
				circle( img, center, radius, color, 3, 8, 0 );

				track_point (center);
			}
			else
			{
				rectangle( img, cvPoint(cvRound(r->x*scale), cvRound(r->y*scale)),
							cvPoint(cvRound((r->x + r->width-1)*scale), cvRound((r->y + r->height-1)*scale)),
							color, 3, 8, 0);
			}

			if( nestedCascade.empty() )
				continue;
			smallImgROI = smallImg(*r);
			nestedCascade.detectMultiScale( smallImgROI, nestedObjects,
				1.1, 2, 0
				//|CASCADE_FIND_BIGGEST_OBJECT
				//|CASCADE_DO_ROUGH_SEARCH
				//|CASCADE_DO_CANNY_PRUNING
				|CASCADE_SCALE_IMAGE
				,
				Size(30, 30) );
			for( vector<Rect>::const_iterator nr = nestedObjects.begin(); nr != nestedObjects.end(); nr++ )
			{
				center.x = cvRound((r->x + nr->x + nr->width*0.5)*scale);
				center.y = cvRound((r->y + nr->y + nr->height*0.5)*scale);
				radius = cvRound((nr->width + nr->height)*0.25*scale);
				circle( img, center, radius, color, 3, 8, 0 );
			}
		}
	}

	cv::imshow( "result", img );
}

void track_point(Point center)
{
	static int counter = DELAY_UNTIL_RECALC;

	int steps = 0;
	if (counter == 0)
	{
		float h_pixel_one_percent = (float)CAPTURE_WIDTH / 100;
		float h_fov_one_percent = (float)HORIZONTAL_FIELD_OF_VIEW / 100;
		int h_offset_from_center = (center.x - CAPTURE_WIDTH / 2) / h_pixel_one_percent; 
		int h_travel_degrees = h_offset_from_center * h_fov_one_percent;
		int h_steps = -1 * h_travel_degrees / MOTOR_STEP_DEGREE;
/*
		int h_steps = h_travel_degrees / MOTOR_STEP_DEGREE;
*/

		float v_pixel_one_percent = (float)CAPTURE_HEIGHT / 100;
		float v_fov_one_percent = (float)VERTICAL_FIELD_OF_VIEW / 100;
		int v_offset_from_center = (center.y - CAPTURE_HEIGHT / 2) / v_pixel_one_percent; 
		int v_travel_degrees = v_offset_from_center * v_fov_one_percent;
		int v_steps = -1 * v_travel_degrees / MOTOR_STEP_DEGREE;
/*
		int v_steps = v_travel_degrees / MOTOR_STEP_DEGREE;
*/

		printf("\n====================================================================\n");
		printf("Horizontal position of the point : %d\n", center.x);
		printf("Offset from center : %d%%\n", h_offset_from_center);
		printf("Number of steps to center horizontally : %d\n", h_steps);
		printf("----------------------------------\n");
		printf("Vertical position of the point : %d\n", center.y);
		printf("Offset from center : %d%%\n", v_offset_from_center);
		printf("Number of steps to center vertically : %d\n", v_steps);
		printf("====================================================================\n\n");

		char comm[10];
//		sprintf(comm, "*************Insert_here_the_command_that_will_be_sent_to_Arduino*************");
//		sendCommand(comm, file);
		counter = DELAY_UNTIL_RECALC;
	}
	else
	{
		--counter;
	}
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
