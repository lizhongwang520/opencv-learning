//#include "opencv2/video/tracking.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include <iostream>
//#include <ctype.h>
//using namespace cv;
//using namespace std;
//
//
////--------------------------------【help( )函数】----------------------------------------------
////		描述：输出帮助信息
////-------------------------------------------------------------------------------------------------
//static void help()
//{
//	//输出欢迎信息和OpenCV版本
//	cout << "\n\n\t\t\t非常感谢购买《OpenCV3编程入门》一书！\n"
//		<< "\n\n\t\t\t此为本书OpenCV3版的第10个配套示例程序\n"
//		<< "\n\n\t\t\t   当前使用的OpenCV版本为：" << CV_VERSION
//		<< "\n\n  ----------------------------------------------------------------------------";
//	cout << "\n\n\t该Demo演示了 Lukas-Kanade基于光流的lkdemo\n";
//	cout << "\n\t程序默认从摄像头读入视频，可以按需改为从视频文件读入图像\n";
//	cout << "\n\t操作说明: \n"
//		"\t\t通过点击在图像中添加/删除特征点\n"
//		"\t\tESC - 退出程序\n"
//		"\t\tr -自动进行追踪\n"
//		"\t\tc - 删除所有点\n"
//		"\t\tn - 开/光-夜晚模式\n" << endl;
//}
//
//Point2f point;
//bool addRemovePt = false;
//
//static void onMouse(int event, int x, int y,int,void*)
//{
//	if (event == EVENT_LBUTTONDOWN)
//	{
//		point = Point2f((float)x, (float)y);
//		addRemovePt = true;
//	}
//}
//
//int main()
//{
//	help();
//	VideoCapture cap;
//	TermCriteria termcrit(TermCriteria::MAX_ITER | TermCriteria::EPS, 20, 0.03);
//	Size subPixWinSize(10, 10), winSize(31, 31);
//	const int MAX_COUNT = 500;
//	bool needToInit = false;
//	bool nightMode = false;
//	cap.open(0);
//	if (!cap.isOpened())
//	{
//		cout << "could not initialize capturing...\n";
//		return 0;
//	}
//	namedWindow("lk demo", 1);
//	setMouseCallback("lk demo", onMouse, 0);
//
//	Mat gray, prevGray, image;
//	vector<Point2f>points[2];
//	for (;;)
//	{
//		Mat frame;
//		cap >> frame;
//		if (frame.empty())
//			break;
//		frame.copyTo(image);
//		cvtColor(image, gray, COLOR_BGR2GRAY);
//
//		if (nightMode)
//			image = Scalar::all(0);
//
//		if (needToInit)
//		{
//			goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 3, 0, 0.04);
//			//goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
//			//goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 0, 0.04);
//			cornerSubPix(gray, points[1], subPixWinSize, Size(-1, -1), termcrit);
//			addRemovePt = false;
//		}
//		else if (!points[0].empty())
//		{
//			vector<uchar> status;
//			vector <float> err;
//			if (prevGray.empty())
//				gray.copyTo(prevGray);
//			calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize, 3, termcrit, 0, 0.001);
//			size_t i, k;
//			for (i = k = 0; i < points[1].size(); i++)
//			{
//				if (addRemovePt)
//				{
//					if (norm(point - points[1][i]) <= 5)
//					{
//						addRemovePt = false;
//						continue;
//					}
//				}
//				if (!status[i])
//					continue;
//				points[i][k++] = points[1][i];
//				circle(image, points[1][i],3,Scalar(0,255,0),-1,8);
//			}
//			points[1].resize(k);
//		}
//		if (addRemovePt && points[1].size() < (size_t)MAX_COUNT)
//		{
//			vector <Point2f> tmp;
//			tmp.push_back(point);
//			cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit);
//			points[1].push_back(tmp[0]);
//			addRemovePt = false;
// 		}
//		needToInit = false;
//		imshow("lk demo", image);
//
//		char c = (char)waitKey(10);
//		if (c == 27)
//			break;
//		switch (c)
//		{
//		case 'r':
//			needToInit = true;
//			break;
//		case 'c':
//			points[0].clear();
//			points[1].clear();
//			break;
//		case 'n':
//			nightMode = !nightMode;
//			break;
//		}
//		std::swap(points[1], points[0]);
//		cv::swap(prevGray, gray);
//
//	}
//	return 0;
//
//}

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>

using namespace cv;
using namespace std;

static void help()
{
	// print a welcome message, and the OpenCV version
	cout << "\nThis is a demo of Lukas-Kanade optical flow lkdemo(),\n"
		"Using OpenCV version " << CV_VERSION << endl;
	cout << "\nIt uses camera by default, but you can provide a path to video as an argument.\n";
	cout << "\nHot keys: \n"
		"\tESC - quit the program\n"
		"\tr - auto-initialize tracking\n"
		"\tc - delete all the points\n"
		"\tn - switch the \"night\" mode on/off\n"
		"To add/remove a feature point click it\n" << endl;
}

Point2f point;
bool addRemovePt = false;

static void onMouse(int event, int x, int y, int /*flags*/, void* /*param*/)
{
	if (event == EVENT_LBUTTONDOWN)
	{
		point = Point2f((float)x, (float)y);
		addRemovePt = true;
	}
}

int main(int argc, char** argv)
{
	VideoCapture cap;
	TermCriteria termcrit(TermCriteria::COUNT | TermCriteria::EPS, 20, 0.03);
	Size subPixWinSize(10, 10), winSize(31, 31);

	const int MAX_COUNT = 500;
	bool needToInit = false;
	bool nightMode = false;

	help();
	cv::CommandLineParser parser(argc, argv, "{@input|0|}");
	string input = parser.get<string>("@input");

	if (input.size() == 1 && isdigit(input[0]))
		cap.open(input[0] - '0');
	else
		cap.open(input);

	if (!cap.isOpened())
	{
		cout << "Could not initialize capturing...\n";
		return 0;
	}

	namedWindow("LK Demo", 1);
	setMouseCallback("LK Demo", onMouse, 0);

	Mat gray, prevGray, image, frame;
	vector<Point2f> points[2];

	for (;;)
	{
		cap >> frame;
		if (frame.empty())
			break;

		frame.copyTo(image);
		cvtColor(image, gray, COLOR_BGR2GRAY);

		if (nightMode)
			image = Scalar::all(0);

		if (needToInit)
		{
			// automatic initialization
			goodFeaturesToTrack(gray, points[1], MAX_COUNT, 0.01, 10, Mat(), 3, 3, 0, 0.04);
			cornerSubPix(gray, points[1], subPixWinSize, Size(-1, -1), termcrit);
			addRemovePt = false;
		}
		else if (!points[0].empty())
		{
			vector<uchar> status;
			vector<float> err;
			if (prevGray.empty())
				gray.copyTo(prevGray);
			calcOpticalFlowPyrLK(prevGray, gray, points[0], points[1], status, err, winSize,
				3, termcrit, 0, 0.001);
			size_t i, k;
			for (i = k = 0; i < points[1].size(); i++)
			{
				if (addRemovePt)
				{
					if (norm(point - points[1][i]) <= 5)
					{
						addRemovePt = false;
						continue;
					}
				}

				if (!status[i])
					continue;

				points[1][k++] = points[1][i];
				circle(image, points[1][i], 3, Scalar(0, 255, 0), -1, 8);
			}
			points[1].resize(k);
		}

		if (addRemovePt && points[1].size() < (size_t)MAX_COUNT)
		{
			vector<Point2f> tmp;
			tmp.push_back(point);
			cornerSubPix(gray, tmp, winSize, Size(-1, -1), termcrit);
			points[1].push_back(tmp[0]);
			addRemovePt = false;
		}

		needToInit = false;
		imshow("LK Demo", image);

		char c = (char)waitKey(10);
		if (c == 27)
			break;
		switch (c)
		{
		case 'r':
			needToInit = true;
			break;
		case 'c':
			points[0].clear();
			points[1].clear();
			break;
		case 'n':
			nightMode = !nightMode;
			break;
		}

		std::swap(points[1], points[0]);
		cv::swap(prevGray, gray);
	}

	return 0;
}
