#include <opencv.hpp>
using namespace cv;
int main(void)
{

	Mat srcImage = imread("desk.png");
	imshow("ÔØÈëÍ¼Æ¬", srcImage);
	waitKey();
	return 0;
}