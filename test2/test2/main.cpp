#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
/*
this project is to come true the function of corroding image.
*/
int main()
{
	//����ͼƬ
	Mat srcImage = imread("timg.jpg");
	//��ʾԭͼ
	imshow("ԭͼ", srcImage);
	//���и�ʴ����
	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	Mat dstImage;
	erode(srcImage, dstImage, element);
	//��ʾЧ��
	imshow("Ч��", dstImage);
	waitKey(0);
	return 0;

}