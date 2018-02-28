#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
/*
this project is to come true the function of corroding image.
*/
int main()
{
	//载入图片
	Mat srcImage = imread("timg.jpg");
	//显示原图
	imshow("原图", srcImage);
	//进行腐蚀操作
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	Mat dstImage;
	erode(srcImage, dstImage, element);
	//显示效果
	imshow("效果", dstImage);
	waitKey(0);
	return 0;

}