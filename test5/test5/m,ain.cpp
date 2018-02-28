#include <opencv.hpp>
using namespace cv;

#if 0
//�������ļ��ж�ȡ������
int main()
{
	VideoCapture catpture;
	catpture.open("video.mp4");
	while (1)
	{
		Mat frame;
		catpture >> frame;//��ȡһ֡��Ȼ����ʾ
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
	catpture.open(0);//������ͷ�����ǵ����Դ�������ͷ��
	Mat edges;
	while (i !=0 )
	{
		Mat frame;
		catpture >> frame;//��ȡһ֡

		cvtColor(frame, edges, COLOR_RGB2GRAY);//ת��Ϊ�Ҷ�ֵ
		blur(edges, edges, Size(7, 7));//ģ������
		Canny(edges, edges,0, 30, 3);//CANNY�㷨����
		imwrite("canny.jpg", edges);//���洦����ͼ��
		imshow("camera", edges);//��ʾ
		i--;

		if(waitKey(30) >=0 ) break;
	}
	return 0;

}


#endif