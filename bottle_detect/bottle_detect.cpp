#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;


ofstream fout("date.txt");

int sursa, nr = 1;

double testDistance = 30;
double testWidth = 6.75;
double testPx = 106;   //cola
//double testPx = 123;   //sprite


//	 cola
/*
int iLowH = 140;
int iHighH = 179;

int iLowS = 133;
int iHighS = 255;

int iLowV = 91;
int iHighV = 255;
  */			 
int iLowH = 120;
int iHighH = 179;

int iLowS = 100;
int iHighS = 255;

int iLowV = 100;
int iHighV = 255;

/*
//	sprite
int iLowH = 74;
int iHighH = 91;

int iLowS = 114;
int iHighS = 254;

int iLowV = 83;
int iHighV = 255;
*/



double			getDistance(Point p1, Point p2);
double			getDistance(double pxWidth);

void			HSV_controller_window();
void			run(VideoCapture *cap);

vector<string>	read_video_list();


int main(int argc, char** argv)
{
	vector<string> video_names;

	cout << "1 :   HSV test (camera).\n";
	cout << "2 :   Video to text (video files specified in \"video_list.txt\").\n";
	cout << "* :   Exit.\n";
	cin >> sursa;

	video_names = read_video_list();

	HSV_controller_window();

	if (sursa == 1)
	{
		VideoCapture cap(1);

		if (!cap.isOpened())
		{
			cout << "Cannot open the camera" << endl;
			return -1;
		}

		run(&cap);
	}
	else if (sursa == 2)
	{
		for (std::vector<string>::iterator it = video_names.begin(); it != video_names.end(); ++it)
		{
			cout << *it << endl;
			VideoCapture cap(*it);

			if (!cap.isOpened())
			{
				cout << "Cannot open the video" << endl;
				return -1;
			}

			run(&cap);
		}
	}

	fout.close();

	return 0;
}


void			run(VideoCapture *cap)
{
	while (true)
	{
		Mat imgOriginal;

		bool bSuccess = cap->read(imgOriginal);

		if (!bSuccess)
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		Mat imgHSV;

		cvtColor(imgOriginal, imgHSV, COLOR_BGR2HSV);

		Mat imgThresholded;

		inRange(imgHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV), imgThresholded);


		//morphological opening (remove small objects from the foreground)
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));

		//morphological closing (fill small holes in the foreground)
		dilate(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));
		erode(imgThresholded, imgThresholded, getStructuringElement(MORPH_ELLIPSE, Size(6, 6)));

		/*
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
		*/

		if (sursa == 2)
		{
			cv::Size size(160, 120);
			cv::resize(imgThresholded, imgThresholded, size, 0, 0, CV_INTER_AREA);

			stringstream picture_name;
			picture_name << "F:/sticla_db/poze_o/" << nr << ".png";
			imwrite(picture_name.str(), imgOriginal);

			picture_name.str(std::string());
			picture_name.clear();

			picture_name << "F:/sticla_db/poze_t/" << nr << ".png";
			imwrite(picture_name.str(), imgThresholded);

			cout << picture_name.str() << endl;

			picture_name.str(std::string());
			picture_name.clear();

			picture_name << "F:/sticla_db/date_intrare/" << nr << ".txt";

			for (int row = 0; row < imgThresholded.rows; ++row) {
				uchar* p = imgThresholded.ptr(row);
				for (int col = 0; col < imgThresholded.cols; ++col) {
					fout << ((int)*p != 0);
					++p;
				}
			}

			fout << endl;
			fout << '1' << endl;
		}

		imshow("Thresholded Image", imgThresholded);
		imshow("Original", imgOriginal);
		nr++;

		if (waitKey(30) == 27)
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
}

void			HSV_controller_window()
{
	namedWindow("Control", CV_WINDOW_AUTOSIZE);

	cvCreateTrackbar("LowH", "Control", &iLowH, 179); //Hue (0 - 179)
	cvCreateTrackbar("HighH", "Control", &iHighH, 179);

	cvCreateTrackbar("LowS", "Control", &iLowS, 255); //Saturation (0 - 255)
	cvCreateTrackbar("HighS", "Control", &iHighS, 255);

	cvCreateTrackbar("LowV", "Control", &iLowV, 255); //Value (0 - 255)
	cvCreateTrackbar("HighV", "Control", &iHighV, 255);
}

vector<string>	read_video_list()
{
	ifstream fin("video_list.txt");
	vector<string> video_list;
	bool file_is_empty = 0;

	while (!fin.eof())
	{
		string tmp;
		getline(fin, tmp);
		video_list.push_back(tmp);

		file_is_empty = 1;
	}

	return video_list;
}

double			getDistance(Point p1, Point p2)
{
	double focalWidth = testDistance * testPx / testWidth;

	double px = abs(p1.x - p2.x);
	return testWidth * focalWidth / px;
}

double			getDistance(double pxWidth)
{
	double focalWidth = testDistance * testPx / testWidth;
	return testWidth * focalWidth / pxWidth;
}
