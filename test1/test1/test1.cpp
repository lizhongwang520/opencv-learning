#include <opencv.hpp>
using namespace cv;
int main(void)
{

	Mat srcImage = imread("desk.png");
	imshow("����ͼƬ", srcImage);
	waitKey();
	return 0;
}