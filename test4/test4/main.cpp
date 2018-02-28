#include <highgui.hpp>;
#include <imgproc.hpp>;
using namespace cv;
/*
function:边沿检测
*/
int main()
{
	//show the origin image
	Mat srcImage = imread("timg.jpg");
	imshow("origin", srcImage);
    //define the mat type of gray,dstImage,edge
	Mat gray, dstImage, edge;
	//create a picture like as srcImage
	dstImage.create(srcImage.size(), srcImage.type());
	//tanslate the origin image to gray image
	cvtColor(srcImage, gray,COLOR_RGB2GRAY);
	//blur the gray
	blur(gray, edge, Size(3, 3));
	//canny arithmetic
	Canny(edge, edge, 3, 9, 3);

	//save the image
	//IplImage qImg;
	//qImg = IplImage(edge); // cv::Mat -> IplImage
	//cvSaveImage("out.jpg", &qImg);

	//save the mat iamge to ipg image 效果图保存下来
	imwrite("edge.jpg", edge);

	imshow("result", edge);
	waitKey(0);
	return 0;


}