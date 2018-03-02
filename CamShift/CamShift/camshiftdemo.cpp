#include <opencv2/core/utility.hpp>
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

#include <iostream>
#include <ctype.h>
/*
按Ctrl+k再按Ctrl+c即可把选中的代码行注释掉。
可以按Ctrl+k再按Ctrl+u即可去掉//。
*/

using namespace cv;
using namespace std;

Mat image;

bool backprojMode = false;//是否要进入反向投影模式
bool selectObject = false;//是否要选跟踪的初始目标
int trackObject = 0; //代表跟踪目标数
bool showHist = true;//是否显示直方图
Point origin;//鼠标单击的第一个位置
Rect selection;//保存鼠标所选的区域
int vmin = 10, vmax = 256, smin = 30;
/*
关于HSV
H指hue（色相）、S指saturation（饱和度）、V指value(色调)。
色相（H）：是色彩的基本属性，就是平常所说的颜色名称，如红色、黄色等；
饱和度（S）：是指色彩的纯度，越高色彩越纯，低则逐渐变灰，取0-100%的数值；
明度（V）：取0-100%。
*/
// User draws box around object to track. This triggers CAMShift to start tracking
static void onMouse( int event, int x, int y, int, void* )
{
    if( selectObject )//鼠标左键按下有效，以下代码实现鼠标所选的矩形区域。
    {
        selection.x = MIN(x, origin.x);//鼠标左上角坐标
        selection.y = MIN(y, origin.y);
        selection.width = std::abs(x - origin.x);//矩形宽
        selection.height = std::abs(y - origin.y);//矩形高

        selection &= Rect(0, 0, image.cols, image.rows);//确定鼠标所选区域在图像的范围内
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
    VideoCapture cap;//定义一个类
    Rect trackWindow;//定义一个窗口
    int hsize = 16;
    float hranges[] = {0,180};
	/*
	const int i = 5;
    const int i2 = 10;
    const int* pInt = &i;           //正确，指向一个const int对象，即i的地址
   //*pInt = 10;                   //错误，不能改变其所指缶的对象
    pInt = &i2;                     //正确，可以改变pInt指针本身的值,此时pInt指向的是i2的地址
	*/
    const float* phranges = hranges;
	//例化一个命令解释器函数
    CommandLineParser parser(argc, argv, keys);
    if (parser.has("help"))
    {
        help();
        return 0;
    }
	//<int>这是模板函数，类型定义为int。这里camNum=0
    int camNum = parser.get<int>(0);
	//打开摄像头
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
    setMouseCallback( "CamShift Demo", onMouse, 0 );//消息响应机制
	//createTrackbar功能：在对应窗口创建滑动条，vim是滑动值，最大256
    createTrackbar( "Vmin", "CamShift Demo", &vmin, 256, 0 );
	//最后一个0，代表没有调动滑动拖动的响应函数
    createTrackbar( "Vmax", "CamShift Demo", &vmax, 256, 0 );
	//初始值为10，256，30
    createTrackbar( "Smin", "CamShift Demo", &smin, 256, 0 );

    Mat frame, hsv, hue, mask, hist, histimg = Mat::zeros(200, 320, CV_8UC3), backproj;
    bool paused = false;

    for(;;) //相当于while(1)
    {
        if( !paused )//没有暂定
        {
            cap >> frame;//摄像头读取一帧
            if( frame.empty() )
                break;
        }

        frame.copyTo(image);//把该帧复制给image

        if( !paused )
        {
            cvtColor(image, hsv, COLOR_BGR2HSV);//image帧转换成HSV图像格式保存在HSV中

            if( trackObject )//trackObject初始化为0,或者按完键盘的'c'键后也为0，当鼠标单击松开后为-1  
            {
                int _vmin = vmin, _vmax = vmax;

                inRange(hsv, Scalar(0, smin, MIN(_vmin,_vmax)),Scalar(180, 256, MAX(_vmin, _vmax)), mask);
				/*
				inRange函数的功能是检查输入数组每个元素大小是否在2个给定数值之间，可以有多通道,
				mask保存0通道的最小值，也就是h分量；这里利用了hsv的3个通道，
				比较h：0~180,
				    s：smin~256,
					v：min(_vmin,_vmax)~max(_vmin,_vmax)。
				如果3个通道都在对应的范围内，则mask对应的那个点的值全为1(0xff)，否则为0(0x00).
				*/
                int ch[] = {0, 0};
				/*初始化与hsv大小深度一样图像，色调的度量是用角度表示的，红绿蓝之间相差120度，反色相差180度 */
                hue.create(hsv.size(), hsv.depth());
				//将hsv第一个通道(也就是色调)的数复制到hue中，0索引数组  
                mixChannels(&hsv, 1, &hue, 1, ch, 1);

                if( trackObject < 0 )////鼠标选择区域松开后，该函数内部又将其赋值1  
                {
                    // Object has been selected by user, set up CAMShift search properties once
//此处的构造函数roi用的是Mat hue的矩阵头，且roi的数据指针指向hue，即共用相同的数据，select为其感兴趣的区域  
                    Mat roi(hue, selection), maskroi(mask, selection);
					/*
                   calcHist()函数第1个参数为输入矩阵序列，第2个参数表示输入的矩阵数目，
				   第3个参数表示将被计算直方图维数通道的列表，第4个参数表示可选的掩码函数  
                    第5个参数表示输出直方图，第6个参数表示直方图的维数，第7个参数为每一维直方图数组的大小，
					第8个参数为每一维直方图bin的边界.
					*/
					//将roi的0通道计算直方图并通过mask放入hist中，hsize为每一维直方图的大小  
                    calcHist(&roi, 1, 0, maskroi, hist, 1, &hsize, &phranges);
					//将hist矩阵进行数组范围归一化，都归一化到0~255  
                    normalize(hist, hist, 0, 255, NORM_MINMAX);

                    trackWindow = selection;
					//只要鼠标选完区域松开后，且没有按键盘清0键'c'，则trackObject一直保持为1，
					//因此该if函数只能执行一次，除非重新选择跟踪区域 
                    trackObject = 1; // Don't set up again, unless user selects new ROI

                    histimg = Scalar::all(0);//与按下'c'键是一样的，这里的all(0)表示的是标量全部清0  
				//histing是一个200*300的矩阵，hsize应该是每一个bin的宽度，也就是histing矩阵能分出几个bin出来  
                    int binW = histimg.cols / hsize;
                    Mat buf(1, hsize, CV_8UC3);//定义一个缓冲单bin矩阵  
                    for( int i = 0; i < hsize; i++ )
						//saturate_case函数为从一个初始类型准确变换到另一个初始类型  
                        buf.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i*180./hsize), 255, 255);//Vec3b为3个char值的向量  
                    cvtColor(buf, buf, COLOR_HSV2BGR);//将hsv又转换成bgr

                    for( int i = 0; i < hsize; i++ )
                    {
						//at函数为返回一个指定数组元素的参考值  
                        int val = saturate_cast<int>(hist.at<float>(i)*histimg.rows/255);
						//在一幅输入图像上画一个简单抽的矩形，指定左上角和右下角，并定义颜色，大小，线型等  
                        rectangle( histimg, Point(i*binW,histimg.rows),
                                   Point((i+1)*binW,histimg.rows - val),
                                   Scalar(buf.at<Vec3b>(i)), -1, 8 );
                    }
                }

                //计算直方图的反向投影，计算hue图像0通道直方图hist的反向投影，并让入backproj中  
                calcBackProject(&hue, 1, 0, hist, backproj, &phranges);
                backproj &= mask;

                RotatedRect trackBox = CamShift(backproj, trackWindow,
                                    TermCriteria( TermCriteria::EPS | TermCriteria::COUNT, 10, 1 ));
                if( trackWindow.area() <= 1 )
                {
                    int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5)/6;
                    trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
                                       trackWindow.x + r, trackWindow.y + r) &
                                  Rect(0, 0, cols, rows);//Rect函数为矩阵的偏移和大小，即第一二个参数为矩阵的左上角点坐标，第三四个参数为矩阵的宽和高  
                }

                if( backprojMode )
                    cvtColor( backproj, image, COLOR_GRAY2BGR );
                ellipse( image, trackBox, Scalar(0,0,255), 3, LINE_AA );//跟踪的时候以椭圆为代表目标  
            }
        }
		//后面的代码是不管pause为真还是为假都要执行的
        else if( trackObject < 0 )
            paused = false;

        if( selectObject && selection.width > 0 && selection.height > 0 )
        {
            Mat roi(image, selection);
            bitwise_not(roi, roi);///bitwise_not为将每一个bit位取反  
        }

        imshow( "CamShift Demo", image );
        imshow( "Histogram", histimg );

        char c = (char)waitKey(10);
        if( c == 27 )//退出键
            break;
        switch(c)
        {
		case 'b':  //反向投影模型交替  
            backprojMode = !backprojMode;
            break;
        case 'c'://清零跟踪目标对象
            trackObject = 0;
            histimg = Scalar::all(0);
            break;
        case 'h'://显示直方图交替  
            showHist = !showHist;
            if( !showHist )
                destroyWindow( "Histogram" );
            else
                namedWindow( "Histogram", 1 );
            break;
		case 'p': //暂停跟踪交替  
            paused = !paused;
            break;
        default:
            ;
        }
    }

    return 0;
}

//========================================================
//下面是关于引用的验证
//=========================================================
//#include <iostream>
//using namespace std;
//int main()
//{
//	int i = 5;
//	int& rInt = i;                      //正确，int的引用
//	const int constInt = 10;
//	const int& rConstInt = constInt;    //正确，引用及邦定的值都是常量
//	const int& rConstInt2 = rInt;       //正确，用rInt邦定的对象进行赋值
//	rInt = 30;                          //这时，rConstInt2、rInt、i的值都为30
//	//rConstInt2 = 30;                  //错误，rConstInt2是常量引用，rConstInt2本身不能改变所指向的对象
//
//
//	int i2 = 15;
//	const int& rConstInt3 = i2;         //正确，用非常量的对象为其赋值
//	const int& rConstInt4 = i + i2;     //正确，用表达式为其赋值,值为45
//	cout << "i=\n" << i<< endl;
//	i = 20;                             //此时i=20, rInt = 20, rConstInt4 = 45,说明rConstInt4邦定的是i + i2的临时变量
//	cout << "rConstInt4=\n" <<rConstInt4<< endl;
//	cout << "after calculation i=\n" << i << endl;
//	cout << "rConstInt3=\n" << rConstInt3 << endl;
//	cout << "i2=\n" << i2 << endl;
//	const int& rConstInt5 = 50;         //正解，用一个常量值为其赋值
//	return 0;
//}