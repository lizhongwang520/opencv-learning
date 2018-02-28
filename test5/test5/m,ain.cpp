#include <opencv.hpp>
using namespace cv;

#if 0
//从视屏文件中读取视屏。
int main()
{
	VideoCapture catpture;
	catpture.open("video.mp4");
	while (1)
	{
		Mat frame;
		catpture >> frame;//读取一帧，然后显示
		imshow("video", frame);
		waitKey(30);
	}
	return 0;

}
#else
int main()
{
	int i = 60;
	VideoCapture catpture;
	catpture.open(0);//打开摄像头，这是电脑自带的摄像头。
	Mat edges;
	while (i !=0 )
	{
		Mat frame;
		catpture >> frame;//读取一帧

		cvtColor(frame, edges, COLOR_RGB2GRAY);//转化为灰度值
		blur(edges, edges, Size(7, 7));//模糊处理
		Canny(edges, edges,0, 30, 3);//CANNY算法处理
		imwrite("canny.jpg", edges);//保存处理后的图像
		imshow("camera", edges);//显示
		i--;

		if(waitKey(30) >=0 ) break;
	}
	return 0;

}


#endif