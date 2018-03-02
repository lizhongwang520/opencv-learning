#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>
/*
��Ctrl+k�ٰ�Ctrl+c���ɰ�ѡ�еĴ�����ע�͵���
���԰�Ctrl+k�ٰ�Ctrl+u����ȥ��//��
*/

using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;//�Ƿ�Ҫ���뷴��ͶӰģʽ
bool selectObject = false;//�Ƿ�Ҫѡ���ٵĳ�ʼĿ��
int trackObject = 0; //�������Ŀ����
bool showHist = true;//�Ƿ���ʾֱ��ͼ
Point origin;//��굥���ĵ�һ��λ��
Rect selection;//���������ѡ������
int vmin = 10, vmax = 256, smin = 30;
/*
����HSV
Hָhue��ɫ�ࣩ��Sָsaturation�����Ͷȣ���Vָvalue(ɫ��)��
ɫ�ࣨH������ɫ�ʵĻ������ԣ�����ƽ����˵����ɫ���ƣ����ɫ����ɫ�ȣ�
���Ͷȣ�S������ָɫ�ʵĴ��ȣ�Խ��ɫ��Խ���������𽥱�ң�ȡ0-100%����ֵ��
���ȣ�V����ȡ0-100%��
*/
// User draws box around object to track. This triggers CAMShift to start tracking
static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )//������������Ч�����´���ʵ�������ѡ�ľ�������
    {
        selection.x = MIN(x, origin.x);//������Ͻ�����
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);//���ο�
        selection.height = std::abs(y - origin.y);//���θ�

        selection &= Rect(0, 0, image.cols, image.rows);//ȷ�������ѡ������ͼ��ķ�Χ��
    }

    switch( event )
    {
    case EVENT_LBUTTONDOWN:
        origin = Point(x,y);
        selection = Rect(x,y,0,0);
        selectObject = true;
        break;
    case EVENT_LBUTTONUP:
        selectObject = false;
        if( selection.width > 0 && selection.height > 0 )
            trackObject = -1;   // Set up CAMShift properties in main() loop
        break;
    }
}

string hot_keys =
    "\n\nHot keys: \n"
    "\tESC - quit the program\n"
    "\tc - stop the tracking\n"
    "\tb - switch to/from backprojection view\n"
    "\th - show/hide object histogram\n"
    "\tp - pause video\n"
    "To initialize tracking, select the object with mouse\n";

static void help()
{
    cout << "\nThis is a demo that shows mean-shift based tracking\n"
            "You select a color objects such as your face and it tracks it.\n"
            "This reads from video camera (0 by default, or the camera number the user enters\n"
            "Usage: \n"
            "   ./camshiftdemo [camera number]\n";
    cout << hot_keys;
}

const char* keys =
{
    "{help h | | show help message}{@camera_number| 0 | camera number}"
};

int main( int argc, const char** argv )
{
    VideoCapture cap;//����һ����
    Rect trackWindow;//����һ������
    int hsize = 16;
    float hranges[] = {0,180};
	/*
	const int i = 5;
    const int i2 = 10;
    const int* pInt = &i;           //��ȷ��ָ��һ��const int���󣬼�i�ĵ�ַ
   //*pInt = 10;                   //���󣬲��ܸı�����ָ�Ķ���
    pInt = &i2;                     //��ȷ�����Ըı�pIntָ�뱾���ֵ,��ʱpIntָ�����i2�ĵ�ַ
	*/
    const float* phranges = hranges;
	//����һ���������������
    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        help();
        return 0;
    }
	//<int>����ģ�庯�������Ͷ���Ϊint������camNum=0
    int camNum = parser.get<int>(0);
	//������ͷ
    cap.open(camNum);

    if( !cap.isOpened() )
    {
        help();
        cout << "***Could not initialize capturing...***\n";
        cout << "Current parameter's value: \n";
        parser.printMessage();
        return -1;
    }
    cout << hot_keys;
    namedWindow( "Histogram", 0 );
    namedWindow( "CamShift Demo", 0 );
    setMouseCallback( "CamShift Demo", onMouse, 0 );//��Ϣ��Ӧ����
	//createTrackbar���ܣ��ڶ�Ӧ���ڴ�����������vim�ǻ���ֵ�����256
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
	//���һ��0������û�е��������϶�����Ӧ����
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
	//��ʼֵΪ10��256��30
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    bool paused = false;

    for(;;) //�൱��while(1)
    {
        if( !paused )//û���ݶ�
        {
            cap >> frame;//����ͷ��ȡһ֡
            if( frame.empty() )
                break;
        }

        frame.copyTo(image);//�Ѹ�֡���Ƹ�image

        if( !paused )
        {
            cvtColor(image, hsv, COLOR_BGR2HSV);//image֡ת����HSVͼ���ʽ������HSV��

            if( trackObject )//trackObject��ʼ��Ϊ0,���߰�����̵�'c'����ҲΪ0������굥���ɿ���Ϊ-1  
            {
                int _vmin = vmin, _vmax = vmax;

                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),Scalar(180, 256, MAX(_vmin, _vmax)), mask);
				/*
				inRange�����Ĺ����Ǽ����������ÿ��Ԫ�ش�С�Ƿ���2��������ֵ֮�䣬�����ж�ͨ��,
				mask����0ͨ������Сֵ��Ҳ����h����������������hsv��3��ͨ����
				�Ƚ�h��0~180,
				    s��smin~256,
					v��min(_vmin,_vmax)~max(_vmin,_vmax)��
				���3��ͨ�����ڶ�Ӧ�ķ�Χ�ڣ���mask��Ӧ���Ǹ����ֵȫΪ1(0xff)������Ϊ0(0x00).
				*/
                int ch[] = {0, 0};
				/*��ʼ����hsv��С���һ��ͼ��ɫ���Ķ������ýǶȱ�ʾ�ģ�������֮�����120�ȣ���ɫ���180�� */
                hue.create(hsv.size(), hsv.depth());
				//��hsv��һ��ͨ��(Ҳ����ɫ��)�������Ƶ�hue�У�0��������  
                mixChannels(&hsv, 1, &hue, 1, ch, 1);

                if( trackObject < 0 )////���ѡ�������ɿ��󣬸ú����ڲ��ֽ��丳ֵ1  
                {
                    // Object has been selected by user, set up CAMShift search properties once
//�˴��Ĺ��캯��roi�õ���Mat hue�ľ���ͷ����roi������ָ��ָ��hue����������ͬ�����ݣ�selectΪ�����Ȥ������  
                    Mat roi(hue, selection), maskroi(mask, selection);
					/*
                   calcHist()������1������Ϊ����������У���2��������ʾ����ľ�����Ŀ��
				   ��3��������ʾ��������ֱ��ͼά��ͨ�����б���4��������ʾ��ѡ�����뺯��  
                    ��5��������ʾ���ֱ��ͼ����6��������ʾֱ��ͼ��ά������7������Ϊÿһάֱ��ͼ����Ĵ�С��
					��8������Ϊÿһάֱ��ͼbin�ı߽�.
					*/
					//��roi��0ͨ������ֱ��ͼ��ͨ��mask����hist�У�hsizeΪÿһάֱ��ͼ�Ĵ�С  
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
					//��hist����������鷶Χ��һ��������һ����0~255  
                    normalize(hist, hist, 0, 255, NORM_MINMAX);

                    trackWindow = selection;
					//ֻҪ���ѡ�������ɿ�����û�а�������0��'c'����trackObjectһֱ����Ϊ1��
					//��˸�if����ֻ��ִ��һ�Σ���������ѡ��������� 
                    trackObject = 1; // Don't set up again, unless user selects new ROI

                    histimg = Scalar::all(0);//�밴��'c'����һ���ģ������all(0)��ʾ���Ǳ���ȫ����0  
				//histing��һ��200*300�ľ���hsizeӦ����ÿһ��bin�Ŀ�ȣ�Ҳ����histing�����ֳܷ�����bin����  
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);//����һ�����嵥bin����  
                    for( int i = 0; i < hsize; i++ )
						//saturate_case����Ϊ��һ����ʼ����׼ȷ�任����һ����ʼ����  
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);//Vec3bΪ3��charֵ������  
                    cvtColor(buf, buf, COLOR_HSV2BGR);//��hsv��ת����bgr

                    for( int i = 0; i < hsize; i++ )
                    {
						//at����Ϊ����һ��ָ������Ԫ�صĲο�ֵ  
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
						//��һ������ͼ���ϻ�һ���򵥳�ľ��Σ�ָ�����ϽǺ����½ǣ���������ɫ����С�����͵�  
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                   Point((i+1)*binW,histimg.rows - val),
                                   Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }

                //����ֱ��ͼ�ķ���ͶӰ������hueͼ��0ͨ��ֱ��ͼhist�ķ���ͶӰ��������backproj��  
                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                backproj &= mask;

                RotatedRect trackBox = CamShift(backproj, trackWindow,
                                    TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));
                if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);//Rect����Ϊ�����ƫ�ƺʹ�С������һ��������Ϊ��������Ͻǵ����꣬�����ĸ�����Ϊ����Ŀ�͸�  
                }

                if( backprojMode )
                    cvtColor( backproj, image, COLOR_GRAY2BGR );
                ellipse( image, trackBox, Scalar(0,0,255), 3, LINE_AA );//���ٵ�ʱ������ԲΪ����Ŀ��  
            }
        }
		//����Ĵ����ǲ���pauseΪ�滹��Ϊ�ٶ�Ҫִ�е�
        else if( trackObject < 0 )
            paused = false;

        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);///bitwise_notΪ��ÿһ��bitλȡ��  
        }

        imshow( "CamShift Demo", image );
        imshow( "Histogram", histimg );

        char c = (char)waitKey(10);
        if( c == 27 )//�˳���
            break;
        switch(c)
        {
		case 'b':  //����ͶӰģ�ͽ���  
            backprojMode = !backprojMode;
            break;
        case 'c'://�������Ŀ�����
            trackObject = 0;
            histimg = Scalar::all(0);
            break;
        case 'h'://��ʾֱ��ͼ����  
            showHist = !showHist;
            if( !showHist )
                destroyWindow( "Histogram" );
            else
                namedWindow( "Histogram", 1 );
            break;
		case 'p': //��ͣ���ٽ���  
            paused = !paused;
            break;
        default:
            ;
        }
    }

    return 0;
}

//========================================================
//�����ǹ������õ���֤
//=========================================================
//#include <iostream>
//using namespace std;
//int main()
//{
//	int i = 5;
//	int& rInt = i;                      //��ȷ��int������
//	const int constInt = 10;
//	const int& rConstInt = constInt;    //��ȷ�����ü����ֵ���ǳ���
//	const int& rConstInt2 = rInt;       //��ȷ����rInt��Ķ�����и�ֵ
//	rInt = 30;                          //��ʱ��rConstInt2��rInt��i��ֵ��Ϊ30
//	//rConstInt2 = 30;                  //����rConstInt2�ǳ������ã�rConstInt2�����ܸı���ָ��Ķ���
//
//
//	int i2 = 15;
//	const int& rConstInt3 = i2;         //��ȷ���÷ǳ����Ķ���Ϊ�丳ֵ
//	const int& rConstInt4 = i + i2;     //��ȷ���ñ��ʽΪ�丳ֵ,ֵΪ45
//	cout << "i=\n" << i<< endl;
//	i = 20;                             //��ʱi=20, rInt = 20, rConstInt4 = 45,˵��rConstInt4�����i + i2����ʱ����
//	cout << "rConstInt4=\n" <<rConstInt4<< endl;
//	cout << "after calculation i=\n" << i << endl;
//	cout << "rConstInt3=\n" << rConstInt3 << endl;
//	cout << "i2=\n" << i2 << endl;
//	const int& rConstInt5 = 50;         //���⣬��һ������ֵΪ�丳ֵ
//	return 0;
//}