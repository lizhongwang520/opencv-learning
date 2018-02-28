#include <opencv2/highgui.hpp>;
#include <opencv2/imgproc.hpp>;
using namespace cv;

int main()
{
	//show the origin picture
	Mat srcImage = imread("timg.jpg");
	imshow("origin", srcImage);
	// blur the picture,blur=Ä£ºý
	Mat dstimage;
	blur(srcImage, dstimage, Size(7, 7)); 
	imshow("process", dstimage);

	waitKey(0);
	return 0;
}