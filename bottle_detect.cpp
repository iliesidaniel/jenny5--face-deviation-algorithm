#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;


double testDistance = 30;
double testWidth = 6.75;
//double testPx = 106;   //cola
double testPx = 123;   //sprite

double getDistance(Point p1, Point p2)
{
	double focalWidth = testDistance * testPx / testWidth;

	double px = abs(p1.x - p2.x);
	return testWidth * focalWidth / px;
}

double getDistance(double pxWidth)
{
	double focalWidth = testDistance * testPx / testWidth;
	return testWidth * focalWidth / pxWidth;
}


int main(int argc, char** argv)
{
	VideoCapture cap(1);

	if (!cap.isOpened()) 
	{
		cout << "Cannot open the web cam" << endl;
		return -1;
	}

	/*
	//	 cola
	int iLowH = 129;
	int iHighH = 179;

	int iLowS = 0;
	int iHighS = 255;

	int iLowV = 55;
	int iHighV = 252;
	*/

	//	sprite
	int iLowH = 74;
	int iHighH = 91;

	int iLowS = 114;
	int iHighS = 254;

	int iLowV = 83;
	int iHighV = 255;


	namedWindow("Control", CV_WINDOW_AUTOSIZE);

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);

	while (true)
	{
		Mat imgOriginal;

		bool bSuccess = cap.read(imgOriginal);

		if (!bSuccess) 
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

		Mat imgThresholded, imgTmp;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);

		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));

		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));

		imgTmp = imgThresholded;
		std::vector< std::vector<cv::Point> > contours;
		std::vector<cv::Point> points;
		cv::findContours(imgThresholded, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
		for (size_t i = 0; i<contours.size(); i++) {
			for (size_t j = 0; j < contours[i].size(); j++) {
				cv::Point p = contours[i][j];
				points.push_back(p);
			}
		}

		if (points.size() > 0){
			cv::Rect brect = cv::boundingRect(cv::Mat(points).reshape(2));

			int height = brect.br().y - brect.tl().y;
			int width = brect.br().x - brect.tl().x;


			cout << "\n\t\t" << (double)width / height << endl;

			double min, max, minCola, maxCola, minSprite, maxSprite;

			minCola = 0.4;
			maxCola = 0.6;
			minSprite = 0.285;
			maxSprite = 0.38;
			min = minSprite;
			max = maxSprite;

			cout << sqrt(pow((brect.br().y - brect.tl().y), 2) + pow((brect.br().x - brect.tl().x), 2)) << endl;

			if ((double)width / height >= min && (double)width / height <= max)
			{
				cout << brect.tl() << " " << brect.br() << "\n";
				cv::rectangle(imgOriginal, brect.tl(), brect.br(), cv::Scalar(0, 200, 0), 2, CV_AA);
			}
			else {
				cv::rectangle(imgOriginal, brect.tl(), brect.br(), cv::Scalar(200, 0, 0), 2, CV_AA);
			}

			double distance3 = getDistance(brect.tl(), brect.br());
			cout << "\t distanta :  " << distance3;
		}

		imshow("Thresholded Image", imgTmp); 
		imshow("Original", imgOriginal);

		if (waitKey(30) == 27)
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}

	return 0;
}
