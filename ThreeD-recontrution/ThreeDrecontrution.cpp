#include "ThreeDrecontrution.h"
#include "img2video.h"

using namespace std;
using namespace cv;
img2video i2v;
string video, output_img;

ThreeDrecontrution::ThreeDrecontrution(QWidget *parent) 
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connect(ui.bntdetect, SIGNAL(clicked()), this, SLOT(Startdetect()));
	connect(ui.KCFbnt, SIGNAL(clicked()), this, SLOT(KCF_detect()));
	connect(ui.pushButton_Edge_show, SIGNAL(clicked()), this, SLOT(show_edge_fig()));
	connect(ui.pushButton_PicedgeDetect, SIGNAL(clicked()), this, SLOT(EdgeDetect_pic()));
	connect(ui.pushButton_VideoedgeDetect, SIGNAL(clicked()), this, SLOT(EdgeDetect_video()));
	connect(ui.pushButton_txt2csv, SIGNAL(clicked()), this, SLOT(txt2csv()));
	timer = new QTimer(this);
	ui.label_image->setScaledContents(true);//可以使图片完全按QWidget缩放，而不保持原视频比列
	connect(timer, SIGNAL(timeout()), this, SLOT(ReadFrame()));
	connect(&serial, &QSerialPort::readyRead, this, &ThreeDrecontrution::serialPort_readyRead);
	ui.sendButton->setEnabled(false);

	ui.baudrateBox->setCurrentIndex(3);
}
// 视频播放预处理，以及图像处理前置函数
void img2video_initial(Mat& vd) {
	cout << "Parameters (fps):";
	cin >> i2v.para[0]; //frame
	//i2v.para[1]=vd.cols;
	//i2v.para[2]=vd.rows;
	i2v.path_save = output_img;
	i2v.iscol = 1;
}

QImage ThreeDrecontrution::MatToQImage2(const cv::Mat &mat)
{
	QImage img;
	int chana = mat.channels();
	//依据通道数不同，改变不同的装换方式
	if (3 == chana) {
		//调整通道次序
		cv::cvtColor(mat, mat, COLOR_BGR2RGB);
		img = QImage(static_cast<uchar *>(mat.data), mat.cols, mat.rows, QImage::Format_RGB888);
	}
	else if (4 == chana)
	{
		//argb
		img = QImage(static_cast<uchar *>(mat.data), mat.cols, mat.rows, QImage::Format_ARGB32);
	}
	else {
		//单通道，灰度图
		img = QImage(mat.cols, mat.rows, QImage::Format_Indexed8);
		uchar * matdata = mat.data;
		for (int row = 0; row < mat.rows; ++row)
		{
			uchar* rowdata = img.scanLine(row);
			memcpy(rowdata, matdata, mat.cols);
			matdata += mat.cols;
		}
	}
	return img;
}

// 串口操作中的机械手部分
void ThreeDrecontrution::on_pushButton_hand_x_up_clicked()
{
	ui.textEdit_controlinfo->append("Hand X UP!!!");
}

void ThreeDrecontrution::on_pushButton_hand_x_down_clicked()
{
	ui.textEdit_controlinfo->append("Hand X down!!!");
}

void ThreeDrecontrution::on_pushButton_controlclean_clicked()
{
	ui.textEdit_controlinfo->clear();
}


//串口控制部分
void ThreeDrecontrution::serialPort_readyRead()
{
	//�ӽ��ջ������ж�ȡ����
	QByteArray buffer = serial.readAll();
	//�ӽ����ж�ȡ��ǰ�յ�������
	QString recv = ui.recvTextEdit->toPlainText();
	recv += QString(buffer);
	//�����ǰ����ʾ
	ui.recvTextEdit->clear();
	//������ʾ
	ui.recvTextEdit->append(recv);
}

void ThreeDrecontrution::on_searchButton_clicked()
{
	ui.portNameBox->clear();
	//ͨ��QSerialPortInfo���ҿ��ô���
	//https://www.cnblogs.com/lomper/p/3959771.html
	//https://blog.csdn.net/weixin_40903194/article/details/83374711
	foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
	{
		ui.portNameBox->addItem(info.portName());
	}
}

void ThreeDrecontrution::on_openButton_clicked()
{
	if (ui.openButton->text() == QString("Open Port"))
	{
		//���ô�����
		serial.setPortName(ui.portNameBox->currentText());
		//���ò�����
		serial.setBaudRate(ui.baudrateBox->currentText().toInt());
		//��������λ��
		switch (ui.dataBitsBox->currentIndex())
		{
		case 8: serial.setDataBits(QSerialPort::Data8); break;
		default: break;
		}
		//������żУ��
		switch (ui.ParityBox->currentIndex())
		{
		case 0: serial.setParity(QSerialPort::NoParity); break;
		default: break;
		}
		//����ֹͣλ
		switch (ui.stopBitsBox->currentIndex())
		{
		case 1: serial.setStopBits(QSerialPort::OneStop); break;
		case 2: serial.setStopBits(QSerialPort::TwoStop); break;
		default: break;
		}
		//����������
		serial.setFlowControl(QSerialPort::NoFlowControl);

		//�򿪴���
		if (!serial.open(QIODevice::ReadWrite))
		{
			QMessageBox::about(NULL, "Oops", "Can't open!");
			return;
		}

		//�����˵��ؼ�ʧ�� 
		ui.portNameBox->setEnabled(false);
		ui.baudrateBox->setEnabled(false);
		ui.dataBitsBox->setEnabled(false);
		ui.ParityBox->setEnabled(false);
		ui.stopBitsBox->setEnabled(false);

		ui.openButton->setText(QString("Close Port"));
		//���Ͱ���ʹ��
		ui.sendButton->setEnabled(true);
	}
	else
	{
		//�رմ���
		serial.close();

		//�����˵��ؼ�ʹ��
		ui.portNameBox->setEnabled(true);
		ui.baudrateBox->setEnabled(true);
		ui.dataBitsBox->setEnabled(true);
		ui.ParityBox->setEnabled(true);
		ui.stopBitsBox->setEnabled(true);

		ui.openButton->setText(QString("Open Port"));
		//���Ͱ���ʧ��
		ui.sendButton->setEnabled(false);
	}
}

void ThreeDrecontrution::on_sendButton_clicked()
{
	//��ȡ�����ϵ����ݲ�ת����utf8��ʽ���ֽ���
	QByteArray data = ui.sendTextEdit->toPlainText().toUtf8();
	serial.write(data);
}

void ThreeDrecontrution::on_clearButton_clicked()
{
	ui.recvTextEdit->clear();
}

//以下为边缘检测部分
ofstream file_video("./1circle.txt", ios::app);
Mat on_ThreshChange_video(Mat src, int num) {
	int xx = 210, yy = 120;

	float PI = 3.1415926535897932384;
	char mk;
	Mat mid, dst, rsz;//mat类型的中间变量和目标图像  		
	dst = src.clone();//原图像深拷贝到目标图像  		
	cvtColor(dst, mid, COLOR_BGR2GRAY); //转化为灰度图像
	//Ptr<CLAHE> clahe = createCLAHE(40,Size(8,8));
	//clahe->apply(mid, mid);
	//equalizeHist(mid, mid);
	rsz = mid;
	//rsz = mid(Rect(xx, yy, 260, 200));
	GaussianBlur(rsz, rsz, Size(3, 3), 8, 1, 4);
	//Canny(rsz, rsz, 125, 350);
	//resize(rsz, rsz, Size(450, 200),0,0, INTER_NEAREST);

	//imshow("rsz", rsz);

	Mat binary_img, dst_img;
	//src_img = imread("test2.png", IMREAD_GRAYSCALE);//灰度图读入
	threshold(rsz, binary_img, 80, 255, THRESH_BINARY);

	imshow("trs", binary_img);
	//imwrite("trs.bmp", binary_img);
//可以根据实际需求选择使用的滤波进行降噪
	GaussianBlur(binary_img, binary_img, Size(3, 3), 3, 1, 2);
	//boxFilter(binary_img, binary_img,-1, Size(3, 3));
	//blur(binary_img, binary_img, Size(3, 3));
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	// 形态学操作    
	Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	// 构建形态学操作的结构元    
	morphologyEx(binary_img, binary_img, MORPH_CLOSE, kernel, Point(-1, -1));
	//闭操作    
	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	// 构建形态学操作的结构元
	morphologyEx(binary_img, binary_img, MORPH_OPEN, kernel, Point(-1, -1));
	//开操作    
	//imshow("开操作", dst_img);
	findContours(binary_img, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
	//imshow("bf", binary_img);
	//imwrite("bf.bmp", binary_img);
	Mat result_img = rsz.clone();
	cvtColor(result_img, result_img, COLOR_GRAY2BGR);
	//灰度图转为彩色图   
	int MinR = 10; int MaxR = 300;
	double radius = 1;
	for (int i = 0; i < hireachy.size(); i++)
	{
		//cout << contours[1] << endl;
		//        drawContours(src_img, contours, i, Scalar(0, 0, 255), -1, 8, Mat(), 0, Point());
		if (contours[i].size() < 5)continue;
		double area = contourArea(contours[i]);

		/*if (area < 300&& area>40) {
			RotatedRect rect = fitEllipse(contours[i]);
			double arc_length = arcLength(contours[i], true);
			double radius = arc_length / (2 * PI);
			circle(result_img, rect.center, radius, Scalar(0, 255, 255), 1, 8, 0);
			circle(result_img, rect.center, 2, Scalar(0, 255, 0), -1, 8, 0);
			continue;
		}*/

		if (area > 10) {
			RotatedRect rect = fitEllipse(contours[i]);
			double arc_length = arcLength(contours[i], true);
			radius = ((arc_length / (2 * PI)) + radius) / 1.4;
			if (radius < 5) continue;
			//circle(src, Point(rect.center.x + xx, rect.center.y + yy), radius, Scalar(0, 255, 0), 1, 8, 0);
			circle(src, Point(rect.center.x + xx, rect.center.y + yy), 2, Scalar(0, 255, 0), -1, 8, 0);

			file_video << "num:" << num << "	" << rect.center.x + xx << "	" << rect.center.y + yy << "	" << endl;

			continue;
		}
		//if (area > 2000) continue;



		//float ratio = float(rect.size.width) / float(rect.size.height);
		//drawContours(src, contours, i, Scalar(0, 255, 0), 1, 8, hireachy,214783647,Point(xx,yy));
		//RotatedRect rect = fitEllipse(contours[i]);
		//circle(src, Point(rect.center.x + xx, rect.center.y + yy), 2, Scalar(0, 255, 0), -1, 8, 0);
		//cout << "contours:" << i << endl;
	}
	//waitKey();
	return src;
}

void ThreeDrecontrution::EdgeDetect_video()
{
	//int contourthresh1, contourthresh2; 	
	videofileName = QFileDialog::getOpenFileName(
		this, "open video",
		".",
		"video (*.avi *.mp4 *.flv));;All files (*.*)");
	string filename = videofileName.toStdString();

	bool const save_output_videofile = true;//视频存储标志位 	
	string out_videofile = "./2out_ld.avi";
	//Mat mode = imread("mode.png", IMREAD_GRAYSCALE);
	int num = 0;
	while (true)
	{
		//std::cout << "thresh: ";
		//std::cin >> contourthresh1 >> contourthresh2;//对于media2，最佳阈值为70（检测气泡）；最佳阈值为85，检测microbeads
		//filename = "./1.mp4";
		//contourthresh = 120;
		//cout << filename << endl;
		if (filename.size() == 0) break;

		try {
			string const file_ext = filename.substr(filename.find_last_of(".") + 1);
			string const protocol = filename.substr(0, 7);
			if (file_ext == "avi" || file_ext == "mp4" || file_ext == "mjpg" || file_ext == "mov" || 	// video file
				protocol == "rtmp://" || protocol == "rtsp://" || protocol == "http://" || protocol == "https:/")	// video network stream
			{
				cout << "input successfully" << endl;
				Mat cap_frame, cur_frame, det_frame, write_frame, flur_frame, dst;
				Mat cur;
				atomic<bool> consumed, videowrite_ready;
				consumed = true;
				//vector<Vec2f> lines;//存储霍夫变换后的直线
				videowrite_ready = true;
				std::thread t_detect;
				std::thread t_cap;
				std::thread t_videowrite;
				mutex mtx;
				condition_variable cv_detected, cv_pre_tracked;
				chrono::steady_clock::time_point steady_start, steady_end;//计时
				VideoCapture cap(filename);
				cap >> cur_frame;//读入视频
				int const video_fps = cap.get(CAP_PROP_FPS);
				Size const frame_size = cur_frame.size();
				VideoWriter output_video;//处理结果
				if (save_output_videofile)
					output_video = VideoWriter(out_videofile, VideoWriter::fourcc('X', 'V', 'I', 'D'), video_fps, frame_size, true);

				while (!cur_frame.empty())
					//while (true)
				{
					//cap >> cur_frame;//读入视频
					// always sync
					if (t_cap.joinable()) {
						t_cap.join();
						//++fps_cap_counter;
						cur_frame = cap_frame.clone();
						//imshow("winn", cur_frame);
						//int key = cv::waitKey(3);
					}
					t_cap = std::thread([&]() { cap >> cap_frame; });//读取视频中的每帧图像
					cout << "1" << endl;
					//if (!cur_frame.empty())
					//{

					//}

					//cout << "2" << endl;
					if (!cur_frame.empty()) {
						num = num + 1;

						cur = on_ThreshChange_video(cur_frame, num);
						//imshow("src1", flur_frame);
						//将原图像转换为二值图像  
						//cv::threshold(flur_frame, flur_frame, 128, 1, cv::THRESH_BINARY);

						//二值图转化成灰度图，并绘制找到的点
						//dst = dst * 255;
						//flur_frame = flur_frame * 255;
						//显示图像  
						/*cv::namedWindow("src1", CV_WINDOW_AUTOSIZE);
						cv::namedWindow("dst1", CV_WINDOW_AUTOSIZE);
						cv::imshow("src1", flur_frame);
						cv::imshow("dst1", dst);*/
						//cur_frame = houghlinee(cur_frame);

						//char stringnum[10];
						//sprintf_s(stringnum, "%d", num);//保留四位小数
						//string timeString("frame:");
						//timeString += stringnum;
						//putText(cur_frame, timeString, Point(60, 60), CV_FONT_HERSHEY_SIMPLEX, 2, Scalar(0), 2, 8);

						steady_end = std::chrono::steady_clock::now();
						auto spenttime = std::chrono::duration<double>(steady_end - steady_start).count();
						cout << "spent time" << spenttime << endl;
						imshow("recognition", cur);
						//if (num % 10 == 0)
							//system("cls");
					}
					//imshow("window name", cur_frame);
					//imshow("window namqe", cap_frame);

					int key = cv::waitKey(3);
					//if (key == 27) { exit_flag = true; break; }

					if (output_video.isOpened() && videowrite_ready) {
						if (t_videowrite.joinable()) t_videowrite.join();
						//write_frame = dst.clone();
						//write_frame = cur_frame.clone();
						//write_frame = cur.clone();
						//imshow("write", write_frame);
						videowrite_ready = false;
						t_videowrite = std::thread([&]() {
							output_video << cur; videowrite_ready = true;
						});
					}
				}
				if (t_cap.joinable()) t_cap.join();
				if (t_detect.joinable()) t_detect.join();
				if (t_videowrite.joinable()) t_videowrite.join();
				std::cout << "Video ended \n";
				break;
			}
		}
		catch (std::exception &e) { std::cerr << "exception: " << e.what() << "\n"; getchar(); }
		catch (...) { std::cerr << "unknown exception \n"; getchar(); }
		filename.clear();
	}


	waitKey(0);

}

vector<float> *inputdata(string pth) {

	vector<float> *p = new vector<float>;
	ifstream infile(pth);
	float num;
	while (!infile.eof()) {
		infile >> num;
		p->push_back(num);
	}
	p->pop_back();
	return p;
}

void ThreeDrecontrution::txt2csv()
{
		//QStringList fileNames = QFileDialog::getOpenFileNames(this, "Open .txt Files", QDir::homePath(), "Text Files (*.txt)");

		//if (!fileNames.isEmpty()) {
		//	for (const QString &fileName : fileNames) {
		//		QFile file(fileName);
		//		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		//			QTextStream in(&file);
		//			QString fileContents = in.readAll();
		//			file.close();

		//			ui.textEdit->append("Contents of " + fileName + ":\n");
		//			ui.textEdit->append(fileContents);
		//		}
		//	}
		//}

		QString strDir = QFileDialog::getExistingDirectory(this, "Open Directory", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	//Mat mode = imread("mode.png", IMREAD_GRAYSCALE);
		string direction = strDir.toStdString() + '/';

		int ini = 1;
		string path = direction;
		string center = direction + "circle.txt";
		string pathre = direction + "all";

		ui.textEdit->append(strDir + '/');
		QDir *dir = new QDir(strDir + '/');
		QStringList filter;
		filter << "*.txt";
		dir->setNameFilters(filter); //过滤文件类型
		QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));
		int count = fileInfo->count();  //文件个数

		int endi = count-1;

		ofstream fileopen;
		int count_cal = 0;
		fileopen.open(pathre + ".csv");
		for (int i = ini; i <= endi; i++) {
			string path_revised = path + to_string(i) + ".txt";
			cout << path_revised << endl;
			//fileopen.open(pathre+to_string(i)+".csv");
			//fileopen<< i <<endl;
			string readpth = path + to_string(i) + ".txt";
			vector<float>* edge = inputdata(readpth);
			vector<float>* center_axis = inputdata(center);
			vector<float>::iterator it1;
			vector<float>::iterator it2;
			it1 = center_axis->begin() + count_cal;
			float center_axis_xy[2];
			center_axis_xy[0] = *it1;
			center_axis_xy[1] = *(it1 + 1);
			count_cal += 2;
			for (it2 = edge->begin(); it2 != edge->end(); it2 += 2) {
				float x, y;
				x = *it2;
				y = *(it2 + 1);
				fileopen << i << "," << x - center_axis_xy[0] << "," << y - center_axis_xy[1] << endl;
				//cout<<x<<" "<<y<<endl;
			}

		}
		ui.textEdit->setText("TXT TO CSV DONE!");
		fileopen.close();

}

Mat on_ThreshChange(Mat src, int num, string fnm) {
	int xx = 210, yy = 120;
	float PI = 3.1415926535897932384; char mk;
	Mat mid, dst, rsz;//mat类型的中间变量和目标图像  		
	dst = src.clone();//原图像深拷贝到目标图像  
	ofstream file_pic(fnm + "circle.txt", ios::app);
	cvtColor(dst, mid, COLOR_BGR2GRAY); //转化为灰度图像
	//Ptr<CLAHE> clahe = createCLAHE(40,Size(8,8));
	//clahe->apply(mid, mid);
	//equalizeHist(mid, mid);
	rsz = mid;
	//rsz = mid(Rect(xx, yy, 260, 200));
	//GaussianBlur(rsz, rsz, Size(3, 3), 8, 1, 4);
	//Canny(rsz, rsz, 125, 350);
	//resize(rsz, rsz, Size(450, 200),0,0, INTER_NEAREST);

	//imshow("rsz", rsz);

	Mat binary_img, dst_img;
	//src_img = imread("test2.png", IMREAD_GRAYSCALE);//灰度图读入
	threshold(rsz, binary_img, 80, 255, THRESH_BINARY);

	imshow("trs", binary_img);
	//imwrite("trs.bmp", binary_img);
//可以根据实际需求选择使用的滤波进行降噪
	//GaussianBlur(binary_img, binary_img, Size(11, 11), 8, 1, 4);
	//boxFilter(binary_img, binary_img,-1, Size(3, 3));
	//blur(binary_img, binary_img, Size(3, 3));
	vector<vector<Point>> contours;
	vector<Vec4i> hireachy;
	// 形态学操作    
	/*Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	// 构建形态学操作的结构元
	morphologyEx(binary_img, binary_img, MORPH_CLOSE, kernel, Point(-1, -1));
	//闭操作
	kernel = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
	// 构建形态学操作的结构元
	morphologyEx(binary_img, binary_img, MORPH_OPEN, kernel, Point(-1, -1));
	//开操作
	//imshow("开操作", dst_img);*/
	findContours(binary_img, contours, hireachy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point());
	//imshow("bf", binary_img);
	//imwrite("bf.bmp", binary_img);
	Mat result_img = rsz.clone();
	cvtColor(result_img, result_img, COLOR_GRAY2BGR);
	//灰度图转为彩色图   
	int MinR = 10; int MaxR = 300;
	double radius = 1;
	for (int i = 0; i < hireachy.size(); i++)
	{
		//cout << contours[1] << endl;
		//        drawContours(src_img, contours, i, Scalar(0, 0, 255), -1, 8, Mat(), 0, Point());
		if (contours[i].size() < 5)continue;
		double area = contourArea(contours[i]);

		/*if (area < 300&& area>40) {
			RotatedRect rect = fitEllipse(contours[i]);
			double arc_length = arcLength(contours[i], true);
			double radius = arc_length / (2 * PI);
			circle(result_img, rect.center, radius, Scalar(0, 255, 255), 1, 8, 0);
			circle(result_img, rect.center, 2, Scalar(0, 255, 0), -1, 8, 0);
			continue;
		}*/
		if (area < 5000) continue;
		if (area > 5000) {
			RotatedRect rect = fitEllipse(contours[i]);
			double arc_length = arcLength(contours[i], true);
			radius = ((arc_length / (2 * PI)) + radius) / 1.4;
			if (radius < 5) continue;
			//circle(src, Point(rect.center.x + xx, rect.center.y + yy), radius, Scalar(0, 255, 0), 1, 8, 0);
			circle(src, Point(rect.center.x, rect.center.y), 2, Scalar(0, 255, 0), -1, 8, 0);

			file_pic << rect.center.x << "	" << rect.center.y << "	" << endl;
			string fn = fnm + to_string(num) + ".txt";
			ofstream file2(fn, ios::app);
			vector<Point>::iterator it1;
			for (it1 = contours[i].begin(); it1 != contours[i].end(); it1++) {
				float xx = it1->x, yy = it1->y;
				file2 << xx << "\t" << yy << endl;

			}
			file2.close();
			continue;
		}
		//if (area > 2000) continue;



		//float ratio = float(rect.size.width) / float(rect.size.height);
		//drawContours(src, contours, i, Scalar(0, 255, 0), 1, 8, hireachy,214783647,Point(xx,yy));
		//RotatedRect rect = fitEllipse(contours[i]);
		//circle(src, Point(rect.center.x + xx, rect.center.y + yy), 2, Scalar(0, 255, 0), -1, 8, 0);
		//cout << "contours:" << i << endl;
	}
	//waitKey();
	return src;
}

void ThreeDrecontrution::EdgeDetect_pic() 
{
	//int contourthresh1, contourthresh2; 	
	string filename;
	bool const save_output_videofile = true;//视频存储标志位 	
	string out_videofile = "./1out_ld.avi";
	
	QString strDir = QFileDialog::getExistingDirectory(this, "Open Directory", ".", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	//Mat mode = imread("mode.png", IMREAD_GRAYSCALE);
	string direction = strDir.toStdString() + '/';
	ui.textEdit->append(strDir+'/');
	QDir *dir = new QDir(strDir + '/');
		QStringList filter;
	filter<<"*.tif";
	dir->setNameFilters(filter); //过滤文件类型
	QList<QFileInfo> *fileInfo = new QList<QFileInfo>(dir->entryInfoList(filter));
	int count = fileInfo->count();  //文件个数
	
	ui.textEdit->setText(QString::number(count));
	int frame_number = count;
	int num = 0;
	while (true)
	{
		num++;
		filename = direction + to_string(num) + ".tif";
		Mat img = imread(filename);
		on_ThreshChange(img, num, direction);
		filename.clear();
		if (num >= frame_number)
			break;
	}
	waitKey(27);
}

void ThreeDrecontrution::show_edge_fig()
{
	figfileName = QFileDialog::getOpenFileName(
		this, "open figure",
		".",
		"figure (*.png *.jpg *.));;All files (*.*)");
	sfileName = figfileName.toStdString();
	Mat srcImg = imread(sfileName);

	if (srcImg.empty())
	{
		return;
	}
	Mat imgShow;
	cvtColor(srcImg, imgShow, COLOR_BGR2RGB); // 图像格式转换
	QImage qImg = QImage((unsigned char*)(imgShow.data), imgShow.cols,
	imgShow.rows, imgShow.cols*imgShow.channels(), QImage::Format_RGB888);
	ui.label_test->setPixmap(QPixmap::fromImage(qImg.scaled(ui.label_test->size(), Qt::KeepAspectRatio)));
}

void ThreeDrecontrution::on_pushButton_play_clicked()
{
	timer->start(1000 / fps);
}

void ThreeDrecontrution::on_pushButton_pause_clicked()
{
	timer->stop();
}

void ThreeDrecontrution::on_pushButton_open_clicked()
{
	videofileName = QFileDialog::getOpenFileName(
		this, "open video",
		".",
		"video (*.avi *.mp4 *.flv));;All files (*.*)");
	sfileName = videofileName.toStdString();
	video.open(sfileName);

	if (!video.isOpened()) {
		QMessageBox::information(this, tr("Hint"), tr("No video!"));
	}
	else {

		int videoW = int(video.get(CAP_PROP_FRAME_WIDTH));
		int videoH = int(video.get(CAP_PROP_FRAME_HEIGHT));
		fps = int(video.get(CAP_PROP_FPS));
		//ui->widget_info->setVisible(true);
		//ui->widget_show->setVisible(true);
		ui.textBrowser_info->setText(QString("Info: %1").arg(videofileName));
		ui.textBrowser_info->append(QString("Size: %2 X %3").arg(videoW).arg(videoH));
		ui.textBrowser_info->append(QString("FPS: %4").arg(fps));
		Mat frame_start;
		video >> frame_start;
		ShowImage(frame_start);
	}
}

void ThreeDrecontrution::ReadFrame()
{

	Mat frame_now;
	video >> frame_now;//读帧进frame
	if (frame_now.empty())
	{
		QMessageBox::information(this, tr("warning"), tr("the video is end!"));
		timer->stop();

	}
	else {
		ShowImage(frame_now);
	}
}

void ThreeDrecontrution::ShowImage(cv::Mat& mat)
{
	//方法二，按比例缩放
	QImage image = MatToQImage2(mat);
	QPixmap pixmap = QPixmap::fromImage(image);
	int width = ui.label_image->width();
	int height = ui.label_image->height();
	QPixmap fitpixmap = pixmap.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	//按比例缩放
	ui.label_image->setPixmap(fitpixmap);
}

void ThreeDrecontrution::KCF_detect()
{
	//Mat srcImg = imread("E:\\Myreconstruction\\edge_dectection\\edge_dectection\\onelayer.jpg");
	//if (srcImg.empty())
	//{
	//	return;
	//}
	//Mat imgShow;
	//cvtColor(srcImg, imgShow, COLOR_BGR2RGB); // 图像格式转换
	//QImage qImg = QImage((unsigned char*)(imgShow.data), imgShow.cols,
	//	imgShow.rows, imgShow.cols*imgShow.channels(), QImage::Format_RGB888);
	//ui.label_test->setPixmap(QPixmap::fromImage(qImg.scaled(ui.label_test->size(), Qt::KeepAspectRatio)));

	//Mat srcImg = imread("E:\\Myreconstruction\\edge_dectection\\edge_dectection\\onelayer.jpg");
	//if (srcImg.empty())
	//{
	//	return;
	//}
	//Mat imgShow;
	//cvtColor(srcImg, imgShow, COLOR_BGR2RGB); // 图像格式转换
	//QImage qImg = QImage((unsigned char*)(imgShow.data), imgShow.cols,
	//	imgShow.rows, imgShow.cols*imgShow.channels(), QImage::Format_RGB888);

	//QGraphicsScene *scene = new QGraphicsScene;//图像显示
	//ui.graphicsView->setScene(scene);
	//ui.graphicsView->show();
	//scene->addPixmap(QPixmap::fromImage(qImg));

	Rect roi;
	Mat frame;
	Mat frame_tmp;

	videofileName = QFileDialog::getOpenFileName(
		this, "open video",
		".",
		"video (*.avi *.mp4 *.flv));;All files (*.*)");
	sfileName = videofileName.toStdString();

	//output_path = QFileDialog::getExistingDirectory(this, "open video", "./", QFileDialog::ShowDirsOnly);
	//i2v.path_save = output_path.toStdString();
	//cout << i2v.path_save << endl;
	i2v.path_save = "E:\\Myreconstruction\\ThreeD-recontrution\\ThreeD-recontrution\\myout.mp4";
	i2v.iscol = 1;
	i2v.para[0] = 30; //一些参数

	int tracker_class;
	//"The class of tracker:(1. KCF, 2.CSRT, 3.MIL, 4.GOTURN)";
	tracker_class = 1;

	Ptr<Tracker> tracker;
	if (tracker_class == 1) {
		tracker = TrackerKCF::create();
	}
	else if (tracker_class == 2) {
		tracker = TrackerCSRT::create();
	}
	else if (tracker_class == 3) {
		tracker = TrackerMIL::create();
	}
	else if (tracker_class == 4) {
		tracker = TrackerGOTURN::create();
	}
	else {
		QMessageBox::information(this, tr("Wrong!"), tr("Set as default."));
		tracker = TrackerKCF::create();
	}

	video.open(sfileName);
	video >> frame;
	if (!video.isOpened()) {
		QMessageBox::information(this, tr("Hint"), tr("No video!"));
	}
	else {
		QMessageBox::information(this, tr("Hint"), tr("press c to leap current Image!\npress q to slect current Image!\npress ENTER key to start track RIO Object!"));
		while (1)
		{
			char key = waitKey(1);
			if (key == 'c')  // 按c键跳帧
			{
				video >> frame;
			}
			if (key == 'q')  // 按q键退出跳帧
			{
				break;
			}
			namedWindow("first", 0);
			imshow("first", frame);
		}
		cv::destroyWindow("first");
		namedWindow("tracker", 0);
		roi = selectROI("tracker", frame);
		if (roi.width == 0 || roi.height == 0)
		{
			QMessageBox::information(this, tr("Hint"), tr("No tracker!"));
		}
		else {
			tracker->init(frame, roi);
			i2v.para[1] = roi.width;
			i2v.para[2] = roi.height;
			for (;; ) {
				video >> frame;
				if (frame.rows == 0 || frame.cols == 0) {
					i2v.endcap();
					break;
				}
				tracker->update(frame, roi);
				Mat cropimg = frame(roi);
				i2v.out = cropimg;
				i2v.func();
				waitKey(10);
			}
		}
	}
}

void ThreeDrecontrution::Startdetect()
{
	//加载类别
	ifstream classNamesFile("./model/coco.names"); //ifstream默认以输入方式打开文件

	vector<string> classNamesVec;
	if (classNamesFile.is_open())
	{
		string className = "";
		while (std::getline(classNamesFile, className))
			classNamesVec.push_back(className);
	}

	//模型设置
	String cfg = "./model/yolov4.cfg";
	String weight = "./model/yolov4.weights";
	//模型读入
	dnn::Net net = readNetFromDarknet(cfg, weight);
	//预处理读取的图像，并将图像读入网络
	//Mat frame = imread("./image/test1.png");

	QString testFileName = QFileDialog::getOpenFileName(this, tr(""), "../../../../open_image", "files(*)");
	Mat frame = imread(testFileName.toStdString());

	//imshow("src", frame);
	Mat inputBlob = blobFromImage(frame, 1.0 / 255, Size(608, 608), Scalar());
	net.setInput(inputBlob);

	//获取未连接输出层
	std::vector<String> outNames = net.getUnconnectedOutLayersNames();
	std::vector<Mat> outs;
	net.forward(outs, outNames);

	//目标检测
	float* data;
	Mat scores;

	vector<Rect> boxes;
	vector<int> classIds;
	vector<float> confidences;
	int centerX, centerY, width, height, left, top;

	float confidenceThreshold = 0.2; // 置信度设置
	double confidence;

	Point classIdPoint;

	//找出所有的目标及其位置
	for (size_t i = 0; i < outs.size(); ++i)
	{
		data = (float*)outs[i].data;
		for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols)
		{
			scores = outs[i].row(j).colRange(5, outs[i].cols);
			minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
			//minMaxLoc(src, minVal, maxVal, minLoc, maxLoc, mask)在一个数组中找到全局最小值和全局最大值
			if (confidence > confidenceThreshold)
			{
				centerX = (int)(data[0] * frame.cols);
				centerY = (int)(data[1] * frame.rows);
				width = (int)(data[2] * frame.cols);
				height = (int)(data[3] * frame.rows);
				left = centerX - width / 2;
				top = centerY - height / 2;

				classIds.push_back(classIdPoint.x);
				confidences.push_back((float)confidence);
				boxes.push_back(Rect(left, top, width, height));
			}
		}
	}

	vector<int> indices;
	NMSBoxes(boxes, confidences, 0.3, 0.2, indices);

	//效果展示
	Scalar rectColor, textColor; //box 和 text 的颜色
	Rect box, textBox;
	int idx; //类别索引
	String className;
	Size labelSize;

	QString show_text;
	show_text = QString::fromLocal8Bit("当前图像的尺寸："); // QString与string的转化，解决中文乱码问题
	show_text.append(QString::number(frame.size().width)); //将int转换成QString
	show_text.append(QString::fromLocal8Bit("×"));
	show_text.append(QString::number(frame.size().height));
	show_text.append("\n");

	cout << "当前图像的尺寸：" << frame.size() << endl;

	for (size_t i = 0; i < indices.size(); ++i)
	{
		idx = indices[i];
		className = classNamesVec[classIds[idx]];

		labelSize = getTextSize(className, FONT_HERSHEY_SIMPLEX, 0.5, 1, 0);
		box = boxes[idx];
		textBox = Rect(Point(box.x - 1, box.y),
			Point(box.x + labelSize.width, box.y - labelSize.height));
		rectColor = Scalar(idx * 11 % 256, idx * 22 % 256, idx * 33 % 256);
		textColor = Scalar(255 - idx * 11 % 256, 255 - idx * 22 % 256, 255 - idx * 33 % 256);
		rectangle(frame, box, rectColor, 2, 8, 0);
		rectangle(frame, textBox, rectColor, -1, 8, 0);
		putText(frame, className.c_str(), Point(box.x, box.y - 2), FONT_HERSHEY_SIMPLEX, 0.5, textColor, 1, 8);

		// API参考：https://blog.csdn.net/KYJL888/article/details/82217192
		cout << className << ":" << "width:" << box.width << ",height:" << box.height << ",center:" << (box.tl() + box.br()) / 2 << endl;

		show_text.append(QString::fromLocal8Bit(className.c_str())); //string转化成Qstring类型
		show_text.append(": width:");
		show_text.append(QString::number(box.width));
		show_text.append(", height:");
		show_text.append(QString::number(box.height));
		show_text.append(", center:");
		int center_x = (box.tl().x + box.br().x) / 2;
		show_text.append(QString::number(center_x));
		show_text.append(",");
		int center_y = (box.tl().y + box.br().y) / 2;
		show_text.append(QString::number(center_y));
		show_text.append("\n");
	}

	ui.labelshow->setText(show_text);

	Mat show_detect_img;
	cvtColor(frame, show_detect_img, COLOR_BGR2RGB);         // 图像格式转换
	QImage disImage = QImage((const unsigned char*)(show_detect_img.data), show_detect_img.cols, show_detect_img.rows, QImage::Format_RGB888);
	ui.label_detect_display->setPixmap(QPixmap::fromImage(disImage.scaled(ui.label_detect_display->size(), Qt::KeepAspectRatio)));
}

//以下为图像预处理内容
Mat gray_to_level(Mat gray)//灰度直方图函数
{
	QVector<int> pixel(256, 0);

	for (int i = 0; i < gray.rows; i++)
		for (int j = 0; j < gray.cols; j++) {
			pixel[gray.at<uchar>(i, j)]++;
		}

	Mat gray_level;
	gray_level.create(350, 256, CV_8UC1);

	int max_rows = 0;
	for (int i = 0; i <= 255; i++) {
		if (pixel[i] > max_rows) {
			max_rows = pixel[i];
		}
	}

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 350; j++) {
			gray_level.at<uchar>(j, i) = 255;
		}
	}

	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 350 - int(320.*float(pixel[i]) / float(max_rows)); j++) {
			gray_level.at<uchar>(j, i) = 0;
		}
	}

	return gray_level;

}

QVector<int> gray2vector(Mat gray) {
	QVector<int> pixel(256, 0);

	for (int i = 0; i < gray.rows; i++)
		for (int j = 0; j < gray.cols; j++) {
			pixel[gray.at<uchar>(i, j)]++;
		}
	return pixel;
}

Mat addSaltNoise(const Mat srcImage, int n)
{
	Mat dstImage = srcImage.clone();
	for (int k = 0; k < n; k++)
	{
		//随机取值行列
		int i = rand() % dstImage.rows;
		int j = rand() % dstImage.cols;
		//图像通道判定
		if (dstImage.channels() == 1)
		{
			dstImage.at<uchar>(i, j) = 255;		//盐噪声
		}
		else
		{
			dstImage.at<Vec3b>(i, j)[0] = 255;
			dstImage.at<Vec3b>(i, j)[1] = 255;
			dstImage.at<Vec3b>(i, j)[2] = 255;
		}
	}
	for (int k = 0; k < n; k++)
	{
		//随机取值行列
		int i = rand() % dstImage.rows;
		int j = rand() % dstImage.cols;
		//图像通道判定
		if (dstImage.channels() == 1)
		{
			dstImage.at<uchar>(i, j) = 0;		//椒噪声
		}
		else
		{
			dstImage.at<Vec3b>(i, j)[0] = 0;
			dstImage.at<Vec3b>(i, j)[1] = 0;
			dstImage.at<Vec3b>(i, j)[2] = 0;
		}
	}
	return dstImage;
}

double generateGaussianNoise(double mu, double sigma)
{
	//定义小值
	const double epsilon = std::numeric_limits<double>::lowest();
	static double z0, z1;
	static bool flag = false;
	flag = !flag;
	//flag为假构造高斯随机变量X
	if (!flag)
		return z1 * sigma + mu;
	double u1, u2;
	//构造随机变量
	do
	{
		u1 = rand() * (1.0 / RAND_MAX);
		u2 = rand() * (1.0 / RAND_MAX);
	} while (u1 <= epsilon);
	//flag为真构造高斯随机变量
	z0 = sqrt(-2.0*log(u1))*cos(2 * CV_PI*u2);
	z1 = sqrt(-2.0*log(u1))*sin(2 * CV_PI*u2);
	return z0 * sigma + mu;
}

//为图像添加高斯噪声
Mat addGaussianNoise(Mat &srcImag)
{
	Mat dstImage = srcImag.clone();
	for (int i = 0; i < dstImage.rows; i++)
	{
		for (int j = 0; j < dstImage.cols; j++)
		{
			//添加高斯噪声
			dstImage.at<Vec3b>(i, j)[0] = saturate_cast<uchar>(dstImage.at<Vec3b>(i, j)[0] + generateGaussianNoise(2, 0.8) * 32);
			dstImage.at<Vec3b>(i, j)[1] = saturate_cast<uchar>(dstImage.at<Vec3b>(i, j)[1] + generateGaussianNoise(2, 0.8) * 32);
			dstImage.at<Vec3b>(i, j)[2] = saturate_cast<uchar>(dstImage.at<Vec3b>(i, j)[2] + generateGaussianNoise(2, 0.8) * 32);
		}
	}
	return dstImage;
}

//canny双阈值处理
void DoubleThreshold(Mat &imageIput, double lowThreshold, double highThreshold)
{
	for (int i = 0; i < imageIput.rows; i++)
	{
		for (int j = 0; j < imageIput.cols; j++)
		{
			if (imageIput.at<uchar>(i, j) > highThreshold)
			{
				imageIput.at<uchar>(i, j) = 255;
			}
			if (imageIput.at<uchar>(i, j) < lowThreshold)
			{
				imageIput.at<uchar>(i, j) = 0;
			}
		}
	}
}

//canny双阈值连接
void DoubleThresholdLink(Mat &imageInput, double lowThreshold, double highThreshold)
{
	for (int i = 1; i < imageInput.rows - 1; i++)
	{
		for (int j = 1; j < imageInput.cols - 1; j++)
		{
			if (imageInput.at<uchar>(i, j) > lowThreshold&&imageInput.at<uchar>(i, j) < 255)
			{
				if (imageInput.at<uchar>(i - 1, j - 1) == 255 || imageInput.at<uchar>(i - 1, j) == 255 || imageInput.at<uchar>(i - 1, j + 1) == 255 ||
					imageInput.at<uchar>(i, j - 1) == 255 || imageInput.at<uchar>(i, j) == 255 || imageInput.at<uchar>(i, j + 1) == 255 ||
					imageInput.at<uchar>(i + 1, j - 1) == 255 || imageInput.at<uchar>(i + 1, j) == 255 || imageInput.at<uchar>(i + 1, j + 1) == 255)
				{
					imageInput.at<uchar>(i, j) = 255;
					DoubleThresholdLink(imageInput, lowThreshold, highThreshold); //递归调用
				}
				else
				{
					imageInput.at<uchar>(i, j) = 0;
				}
			}
		}
	}
}

int OSTU(QVector<int> hist) {
	float u0, u1, w0, w1; int count0, t, maxT; float devi, maxDevi = 0; //方差及最大方差 int i, sum = 0;
	int i, sum = 0;
	for (i = 0; i < 256; i++) { sum = sum + hist[i]; }

	for (t = 0; t < 255; t++) {
		u0 = 0; count0 = 0;
		for (i = 0; i <= t; i++) { //阈值为t时，c0组的均值及产生的概率;
			u0 += i * hist[i];
			count0 += hist[i];
		}
		u0 = u0 / count0;
		w0 = (float)count0 / sum;
		for (i = t + 1; i < 256; i++) { //阈值为t时，c1组的均值及产生的概率
			u1 += i * hist[i];
		}
		u1 = u1 / (sum - count0); w1 = 1 - w0;
		devi = w0 * w1 * (u1 - u0) * (u1 - u0); //两类间方差
		if (devi > maxDevi) //记录最大的方差及最佳位置
		{
			maxDevi = devi;
			maxT = t;
		}
	}
	return maxT;
}

void elbp(Mat& src, Mat &dst, int radius, int neighbors)
{

	for (int n = 0; n < neighbors; n++)
	{
		// 采样点的计算
		float x = static_cast<float>(-radius * sin(2.0*CV_PI*n / static_cast<float>(neighbors)));
		float y = static_cast<float>(radius * cos(2.0*CV_PI*n / static_cast<float>(neighbors)));
		// 上取整和下取整的值
		int fx = static_cast<int>(floor(x));
		int fy = static_cast<int>(floor(y));
		int cx = static_cast<int>(ceil(x));
		int cy = static_cast<int>(ceil(y));
		// 小数部分
		float ty = y - fy;
		float tx = x - fx;
		// 设置插值权重
		float w1 = (1 - tx) * (1 - ty);
		float w2 = tx * (1 - ty);
		float w3 = (1 - tx) * ty;
		float w4 = tx * ty;
		// 循环处理图像数据
		for (int i = radius; i < src.rows - radius; i++)
		{
			for (int j = radius; j < src.cols - radius; j++)
			{
				// 计算插值
				float t = static_cast<float>(w1*src.at<uchar>(i + fy, j + fx) + w2 * src.at<uchar>(i + fy, j + cx) + w3 * src.at<uchar>(i + cy, j + fx) + w4 * src.at<uchar>(i + cy, j + cx));
				// 进行编码
				dst.at<uchar>(i - radius, j - radius) += ((t > src.at<uchar>(i, j)) || (abs(t - src.at<uchar>(i, j)) < numeric_limits<float>::epsilon())) << n;
			}
		}
	}
}

void elbp1(Mat& src, Mat &dst)
{
	// 循环处理图像数据
	for (int i = 1; i < src.rows - 1; i++)
	{
		for (int j = 1; j < src.cols - 1; j++)
		{
			uchar tt = 0;
			int tt1 = 0;
			uchar u = src.at<uchar>(i, j);
			if (src.at<uchar>(i - 1, j - 1) > u) { tt += 1 << tt1; }
			tt1++;
			if (src.at<uchar>(i - 1, j) > u) { tt += 1 << tt1; }
			tt1++;
			if (src.at<uchar>(i - 1, j + 1) > u) { tt += 1 << tt1; }
			tt1++;
			if (src.at<uchar>(i, j + 1) > u) { tt += 1 << tt1; }
			tt1++;
			if (src.at<uchar>(i + 1, j + 1) > u) { tt += 1 << tt1; }
			tt1++;
			if (src.at<uchar>(i + 1, j) > u) { tt += 1 << tt1; }
			tt1++;
			if (src.at<uchar>(i + 1, j - 1) > u) { tt += 1 << tt1; }
			tt1++;
			if (src.at<uchar>(i - 1, j) > u) { tt += 1 << tt1; }
			tt1++;

			dst.at<uchar>(i - 1, j - 1) = tt;
		}
	}
}

void ThreeDrecontrution::on_pushButton_clicked()//选择文件
{
	QString testFileName = QFileDialog::getOpenFileName(this, tr(""), "../../../../open_image", "files(*)");
	srcImg = imread(testFileName.toStdString());
	cvtColor(srcImg, grayImg, COLOR_BGR2GRAY);

	Mat temp;
	QImage Qtemp;
	cvtColor(srcImg, temp, COLOR_BGR2RGB);//BGR convert to RGB
	Qtemp = QImage((const unsigned char*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);

	ui.label->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label->setScaledContents(true);
	ui.label->resize(Qtemp.size());
	ui.label->show();

}

void ThreeDrecontrution::on_select_files_clicked()//BGR转灰度
{
	//Mat gray;
	grayImg.create(srcImg.rows, srcImg.cols, CV_8UC1);
	QImage Qtemp;

	for (int i = 0; i < srcImg.rows; i++)
		for (int j = 0; j < srcImg.cols; j++) {
			grayImg.at<uchar>(i, j) = (int)0.11 * srcImg.at<Vec3b>(i, j)[0]
				+ 0.59 * srcImg.at<Vec3b>(i, j)[1]
				+ 0.3 * srcImg.at<Vec3b>(i, j)[2];
		}

	Qtemp = QImage((const uchar*)(grayImg.data), grayImg.cols, grayImg.rows, grayImg.cols*grayImg.channels(), QImage::Format_Indexed8);
	ui.label_1->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_1->setScaledContents(true);
	ui.label_1->resize(Qtemp.size());
	ui.label_1->show();
}

void ThreeDrecontrution::on_gray_leval_clicked()//灰度直方图
{

	//Mat gray;

	QImage Qtemp;

	Qtemp = QImage((const uchar*)(grayImg.data), grayImg.cols, grayImg.rows, grayImg.cols*grayImg.channels(), QImage::Format_Indexed8);
	ui.label_1->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_1->setScaledContents(true);
	ui.label_1->resize(Qtemp.size());
	ui.label_1->show();

	Mat gray_level;
	gray_level = gray_to_level(grayImg);

	imshow("gray_level", gray_level);
	waitKey(0);
	cv::destroyWindow("gray_level");
	waitKey(1);

}

void ThreeDrecontrution::on_gray_balance_clicked()
{
	Mat balance, gray2Img;
	gray2Img.create(srcImg.rows, srcImg.cols, CV_8UC1);
	balance.create(srcImg.rows, srcImg.cols, CV_8UC1);
	QImage Qtemp;

	//    for(int i = 0 ; i < srcImg.rows ; i++)
	//        for(int j = 0 ; j < srcImg.cols ; j++){
	//            grayImg.at<uchar>(i,j) = (int)0.11 * srcImg.at<Vec3b>(i,j)[0]
	//                                        + 0.59 * srcImg.at<Vec3b>(i,j)[1]
	//                                        + 0.3 * srcImg.at<Vec3b>(i,j)[2];
	//        }
	QVector<int> pixel(256, 0);
	QVector<float> pixel_gray(256, 0.);
	float sum = 0;

	for (int i = 0; i < grayImg.rows; i++)
		for (int j = 0; j < grayImg.cols; j++) {
			pixel[grayImg.at<uchar>(i, j)]++;
		}

	for (int i = 0; i < pixel.size(); i++) {
		sum += pixel[i];
	}

	for (int i = 0; i < 256; i++) {
		float num = 0;
		for (int j = 0; j <= i; j++) {
			num += pixel[j];
		}
		pixel_gray[i] = 255 * num / sum;
	}

	for (int i = 0; i < srcImg.rows; i++)
		for (int j = 0; j < srcImg.cols; j++) {
			balance.at<uchar>(i, j) = pixel_gray[grayImg.at<uchar>(i, j)];
		}

	gray2Img = balance;

	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_grad_sharpen_clicked()
{
	Mat grad, gray2Img;
	gray2Img.create(srcImg.rows, srcImg.cols, CV_8UC1);
	QImage Qtemp, Qtemp2;
	grad.create(gray2Img.rows, gray2Img.cols, CV_8UC1);
	for (int i = 0; i < gray2Img.rows - 1; i++)
		for (int j = 0; j < gray2Img.cols - 1; j++) {
			grad.at<uchar>(i, j) = saturate_cast<uchar>(max(fabs(grayImg.at<uchar>(i + 1, j) - grayImg.at<uchar>(i, j)), fabs(grayImg.at<uchar>(i, j + 1) - grayImg.at<uchar>(i, j))));
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - grad.at<uchar>(i, j));
		}
	//imshow("grad",grad);

	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();

	Qtemp2 = QImage((const uchar*)(grad.data), grad.cols, grad.rows, grad.cols*grad.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();

}

void ThreeDrecontrution::on_laplace_sharpen_clicked()
{
	Mat gradimg, gray2Img;
	QImage Qtemp, Qtemp2;
	gray2Img.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gradimg.create(grayImg.rows, grayImg.cols, CV_8UC1);
	for (int i = 1; i < srcImg.rows - 1; i++)
	{
		for (int j = 1; j < srcImg.cols - 1; j++)
		{
			gradimg.at<uchar>(i, j) = saturate_cast<uchar>(-4 * grayImg.at<uchar>(i, j) + grayImg.at<uchar>(i + 1, j)
				+ grayImg.at<uchar>(i, j + 1) + grayImg.at<uchar>(i - 1, j)
				+ grayImg.at<uchar>(i, j - 1));
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(5 * grayImg.at<uchar>(i, j) - grayImg.at<uchar>(i + 1, j)
				- grayImg.at<uchar>(i, j + 1) - grayImg.at<uchar>(i - 1, j)
				- grayImg.at<uchar>(i, j - 1));
		}
	}
	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();

	Qtemp2 = QImage((const uchar*)(gradimg.data), gradimg.cols, gradimg.rows, gradimg.cols*gradimg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();
}

void ThreeDrecontrution::on_roberts_edge_clicked()
{
	Mat gradimg, gray2Img;
	QImage Qtemp, Qtemp2;
	gray2Img.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gradimg.create(grayImg.rows, grayImg.cols, CV_8UC1);
	for (int i = 0; i < srcImg.rows - 1; i++)
	{
		for (int j = 0; j < srcImg.cols - 1; j++)
		{
			gradimg.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i, j) - grayImg.at<uchar>(i + 1, j + 1)) + fabs(grayImg.at<uchar>(i + 1, j) - grayImg.at<uchar>(i, j + 1)));
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - gradimg.at<uchar>(i, j));
		}
	}
	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();

	Qtemp2 = QImage((const uchar*)(gradimg.data), gradimg.cols, gradimg.rows, gradimg.cols*gradimg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();
}

void ThreeDrecontrution::on_sobel_edge_clicked()
{
	Mat gradimg, gray2Img, f_x, f_y;
	QImage Qtemp, Qtemp2;
	gray2Img.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gradimg.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_x.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_y.create(grayImg.rows, grayImg.cols, CV_8UC1);
	for (int i = 1; i < srcImg.rows - 1; i++)
	{
		for (int j = 1; j < srcImg.cols - 1; j++)
		{
			f_x.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i + 1, j - 1) + 2 * grayImg.at<uchar>(i + 1, j) + grayImg.at<uchar>(i + 1, j + 1)
				- grayImg.at<uchar>(i - 1, j - 1) - 2 * grayImg.at<uchar>(i - 1, j) - grayImg.at<uchar>(i - 1, j + 1)));
			f_y.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i - 1, j + 1) + 2 * grayImg.at<uchar>(i, j + 1) + grayImg.at<uchar>(i + 1, j + 1)
				- grayImg.at<uchar>(i - 1, j - 1) - 2 * grayImg.at<uchar>(i, j - 1) - grayImg.at<uchar>(i + 1, j - 1)));
			gradimg.at<uchar>(i, j) = f_x.at<uchar>(i, j) + f_y.at<uchar>(i, j);
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - gradimg.at<uchar>(i, j));
		}
	}
	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();

	Qtemp2 = QImage((const uchar*)(gradimg.data), gradimg.cols, gradimg.rows, gradimg.cols*gradimg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();
}

void ThreeDrecontrution::on_prewitt_clicked()
{
	Mat gradimg, gray2Img, f_x, f_y;
	QImage Qtemp, Qtemp2;
	gray2Img.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gradimg.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_x.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_y.create(grayImg.rows, grayImg.cols, CV_8UC1);
	for (int i = 1; i < srcImg.rows - 1; i++)
	{
		for (int j = 1; j < srcImg.cols - 1; j++)
		{
			f_x.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i + 1, j - 1) + grayImg.at<uchar>(i + 1, j) + grayImg.at<uchar>(i + 1, j + 1)
				- grayImg.at<uchar>(i - 1, j - 1) - grayImg.at<uchar>(i - 1, j) - grayImg.at<uchar>(i - 1, j + 1)));
			f_y.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i - 1, j + 1) + grayImg.at<uchar>(i, j + 1) + grayImg.at<uchar>(i + 1, j + 1)
				- grayImg.at<uchar>(i - 1, j - 1) - grayImg.at<uchar>(i, j - 1) - grayImg.at<uchar>(i + 1, j - 1)));
			gradimg.at<uchar>(i, j) = max(f_x.at<uchar>(i, j), f_y.at<uchar>(i, j));
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - gradimg.at<uchar>(i, j));
		}
	}
	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();

	Qtemp2 = QImage((const uchar*)(gradimg.data), gradimg.cols, gradimg.rows, gradimg.cols*gradimg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();
}

void ThreeDrecontrution::on_laplace_edge_clicked()
{
	Mat gradimg, gray2Img;
	QImage Qtemp, Qtemp2;
	gray2Img.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gradimg.create(grayImg.rows, grayImg.cols, CV_8UC1);
	for (int i = 1; i < srcImg.rows - 1; i++)
	{
		for (int j = 1; j < srcImg.cols - 1; j++)
		{
			gradimg.at<uchar>(i, j) = saturate_cast<uchar>(-4 * grayImg.at<uchar>(i, j) + grayImg.at<uchar>(i + 1, j)
				+ grayImg.at<uchar>(i, j + 1) + grayImg.at<uchar>(i - 1, j)
				+ grayImg.at<uchar>(i, j - 1));
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(5 * grayImg.at<uchar>(i, j) - grayImg.at<uchar>(i + 1, j)
				- grayImg.at<uchar>(i, j + 1) - grayImg.at<uchar>(i - 1, j)
				- grayImg.at<uchar>(i, j - 1));
		}
	}
	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();

	Qtemp2 = QImage((const uchar*)(gradimg.data), gradimg.cols, gradimg.rows, gradimg.cols*gradimg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();
}

void ThreeDrecontrution::on_salt_noise_clicked()
{
	Mat salt;
	Mat temp;
	salt.create(srcImg.rows, srcImg.cols, CV_8UC3);
	salt = addSaltNoise(srcImg, 800);
	QImage Qtemp2;
	cvtColor(salt, temp, CV_BGR2RGB);//BGR convert to RGB

	noiseImg = temp.clone();

	Qtemp2 = QImage((const unsigned char*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();

}

void ThreeDrecontrution::on_guass_noise_clicked()
{
	Mat salt;
	Mat temp;
	salt.create(srcImg.rows, srcImg.cols, CV_8UC3);
	salt = addGaussianNoise(srcImg);
	QImage Qtemp2;
	cvtColor(salt, temp, CV_BGR2RGB);//BGR convert to RGB
	noiseImg = temp.clone();

	Qtemp2 = QImage((const unsigned char*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();

}

void ThreeDrecontrution::on_krisch_edge_clicked()
{
	Mat gradimg, gray2Img, f_1, f_2, f_3, f_4, f_5, f_6, f_7, f_8;
	QImage Qtemp, Qtemp2;
	gray2Img.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gradimg.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_1.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_2.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_3.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_4.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_5.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_6.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_7.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_8.create(grayImg.rows, grayImg.cols, CV_8UC1);
	for (int i = 1; i < srcImg.rows - 1; i++)
	{
		for (int j = 1; j < srcImg.cols - 1; j++)
		{
			f_1.at<uchar>(i, j) = saturate_cast<uchar>(fabs(-5 * grayImg.at<uchar>(i - 1, j - 1) - 5 * grayImg.at<uchar>(i - 1, j) - 5 * grayImg.at<uchar>(i - 1, j + 1)
				+ 3 * grayImg.at<uchar>(i, j - 1) + 3 * grayImg.at<uchar>(i, j + 1)
				+ 3 * grayImg.at<uchar>(i + 1, j - 1) + 3 * grayImg.at<uchar>(i + 1, j) + 3 * grayImg.at<uchar>(i + 1, j + 1)));
			f_2.at<uchar>(i, j) = saturate_cast<uchar>(fabs(3 * grayImg.at<uchar>(i - 1, j - 1) - 5 * grayImg.at<uchar>(i - 1, j) - 5 * grayImg.at<uchar>(i - 1, j + 1)
				+ 3 * grayImg.at<uchar>(i, j - 1) - 5 * grayImg.at<uchar>(i, j + 1)
				+ 3 * grayImg.at<uchar>(i + 1, j - 1) + 3 * grayImg.at<uchar>(i + 1, j) + 3 * grayImg.at<uchar>(i + 1, j + 1)));
			f_3.at<uchar>(i, j) = saturate_cast<uchar>(fabs(3 * grayImg.at<uchar>(i - 1, j - 1) + 3 * grayImg.at<uchar>(i - 1, j) - 5 * grayImg.at<uchar>(i - 1, j + 1)
				+ 3 * grayImg.at<uchar>(i, j - 1) - 5 * grayImg.at<uchar>(i, j + 1)
				+ 3 * grayImg.at<uchar>(i + 1, j - 1) + 3 * grayImg.at<uchar>(i + 1, j) - 5 * grayImg.at<uchar>(i + 1, j + 1)));
			f_4.at<uchar>(i, j) = saturate_cast<uchar>(fabs(3 * grayImg.at<uchar>(i - 1, j - 1) + 3 * grayImg.at<uchar>(i - 1, j) + 3 * grayImg.at<uchar>(i - 1, j + 1)
				+ 3 * grayImg.at<uchar>(i, j - 1) - 5 * grayImg.at<uchar>(i, j + 1)
				+ 3 * grayImg.at<uchar>(i + 1, j - 1) - 5 * grayImg.at<uchar>(i + 1, j) - 5 * grayImg.at<uchar>(i + 1, j + 1)));
			f_5.at<uchar>(i, j) = saturate_cast<uchar>(fabs(3 * grayImg.at<uchar>(i - 1, j - 1) + 3 * grayImg.at<uchar>(i - 1, j) + 3 * grayImg.at<uchar>(i - 1, j + 1)
				+ 3 * grayImg.at<uchar>(i, j - 1) + 3 * grayImg.at<uchar>(i, j + 1)
				- 5 * grayImg.at<uchar>(i + 1, j - 1) - 5 * grayImg.at<uchar>(i + 1, j) - 5 * grayImg.at<uchar>(i + 1, j + 1)));
			f_6.at<uchar>(i, j) = saturate_cast<uchar>(fabs(3 * grayImg.at<uchar>(i - 1, j - 1) + 3 * grayImg.at<uchar>(i - 1, j) + 3 * grayImg.at<uchar>(i - 1, j + 1)
				- 5 * grayImg.at<uchar>(i, j - 1) + 3 * grayImg.at<uchar>(i, j + 1)
				- 5 * grayImg.at<uchar>(i + 1, j - 1) - 5 * grayImg.at<uchar>(i + 1, j) + 3 * grayImg.at<uchar>(i + 1, j + 1)));
			f_7.at<uchar>(i, j) = saturate_cast<uchar>(fabs(-5 * grayImg.at<uchar>(i - 1, j - 1) + 3 * grayImg.at<uchar>(i - 1, j) + 3 * grayImg.at<uchar>(i - 1, j + 1)
				- 5 * grayImg.at<uchar>(i, j - 1) + 3 * grayImg.at<uchar>(i, j + 1)
				- 5 * grayImg.at<uchar>(i + 1, j - 1) + 3 * grayImg.at<uchar>(i + 1, j) + 3 * grayImg.at<uchar>(i + 1, j + 1)));
			f_8.at<uchar>(i, j) = saturate_cast<uchar>(fabs(-5 * grayImg.at<uchar>(i - 1, j - 1) - 5 * grayImg.at<uchar>(i - 1, j) + 3 * grayImg.at<uchar>(i - 1, j + 1)
				- 5 * grayImg.at<uchar>(i, j - 1) + 3 * grayImg.at<uchar>(i, j + 1)
				+ 3 * grayImg.at<uchar>(i + 1, j - 1) + 3 * grayImg.at<uchar>(i + 1, j) + 3 * grayImg.at<uchar>(i + 1, j + 1)));
			QVector<int> goal = { f_1.at<uchar>(i, j),f_2.at<uchar>(i, j),f_3.at<uchar>(i, j),f_4.at<uchar>(i, j),f_5.at<uchar>(i, j),f_6.at<uchar>(i, j),f_7.at<uchar>(i, j),f_8.at<uchar>(i, j) };
			int max_8 = 0;
			for (int i = 0; i < 8; i++) {
				if (goal[i] > max_8) {
					max_8 = goal[i];
				}
			}

			gradimg.at<uchar>(i, j) = max_8;
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - gradimg.at<uchar>(i, j));
		}
	}
	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();

	Qtemp2 = QImage((const uchar*)(gradimg.data), gradimg.cols, gradimg.rows, gradimg.cols*gradimg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();
}

void ThreeDrecontrution::on_Canny_clicked()
{
	Mat gauss, f_x, f_y, gradimg, max_control, gray2Img;
	QImage Qtemp, Qtemp2;
	gauss.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gradimg.create(grayImg.rows, grayImg.cols, CV_8UC1);
	gray2Img.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_x.create(grayImg.rows, grayImg.cols, CV_8UC1);
	f_y.create(grayImg.rows, grayImg.cols, CV_8UC1);
	QVector<double> direction((grayImg.rows - 1)*(grayImg.rows - 1), 0);
	//高斯处理
	for (int i = 0; i < grayImg.rows - 1; i++)
	{
		for (int j = 0; j < grayImg.cols - 1; j++) {
			gauss.at<uchar>(i, j) = saturate_cast<uchar>(fabs((0.751136*grayImg.at<uchar>(i - 1, j - 1) + 0.123841* grayImg.at<uchar>(i - 1, j) + 0.0751136* grayImg.at<uchar>(i - 1, j + 1)
				+ 0.123841*grayImg.at<uchar>(i, j - 1) + 0.20418*grayImg.at<uchar>(i, j) + 0.123841*grayImg.at<uchar>(i, j + 1)
				+ 0.0751136*grayImg.at<uchar>(i + 1, j - 1) + 0.123841*grayImg.at<uchar>(i + 1, j) + 0.0751136*grayImg.at<uchar>(i + 1, j + 1))));
		}
	}
	//sobel处理
	int k = 0;
	for (int i = 1; i < gauss.rows - 1; i++)
	{
		for (int j = 1; j < gauss.cols - 1; j++)
		{
			f_x.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i + 1, j - 1) + 2 * grayImg.at<uchar>(i + 1, j) + grayImg.at<uchar>(i + 1, j + 1)
				- grayImg.at<uchar>(i - 1, j - 1) - 2 * grayImg.at<uchar>(i - 1, j) - grayImg.at<uchar>(i - 1, j + 1)));
			f_y.at<uchar>(i, j) = saturate_cast<uchar>(fabs(grayImg.at<uchar>(i - 1, j + 1) + 2 * grayImg.at<uchar>(i, j + 1) + grayImg.at<uchar>(i + 1, j + 1)
				- grayImg.at<uchar>(i - 1, j - 1) - 2 * grayImg.at<uchar>(i, j - 1) - grayImg.at<uchar>(i + 1, j - 1)));
			gradimg.at<uchar>(i, j) = sqrt(pow(f_x.at<uchar>(i, j), 2) + pow(f_y.at<uchar>(i, j), 2));

			if (f_x.at<uchar>(i, j) == 0)
			{
				direction[k] = atan(f_y.at<uchar>(i, j) / 0.0000000001)*57.3;  //防止除数为0异常
			}
			else {
				direction[k] = atan(f_y.at<uchar>(i, j) / f_x.at<uchar>(i, j))*57.3;
			}
			direction[k] += 90;
			k++;
		}
	}
	//极大值抑制
//    double m,s;
//    Mat Mat_mean,Mat_vari;
//    meanStdDev(gradimg,Mat_mean,Mat_vari);
//    m = Mat_mean.at<double>(0,0);
//    s = Mat_vari.at<double>(0,0);
//    std::cout<<"m"<< m <<"    "<< "s" << s << std::endl;
//    m = m+s;
//    s = 0.4*m;
//    std::cout<<"m"<< m <<"    "<< "s" << s << std::endl;
	max_control = gradimg.clone();
	k = 0;
	for (int i = 1; i < gradimg.rows - 1; i++)
	{
		for (int j = 1; j < gradimg.cols - 1; j++) {
			int value00 = gradimg.at<uchar>((i - 1), j - 1);
			int value01 = gradimg.at<uchar>((i - 1), j);
			int value02 = gradimg.at<uchar>((i - 1), j + 1);
			int value10 = gradimg.at<uchar>((i), j - 1);
			int value11 = gradimg.at<uchar>((i), j);
			int value12 = gradimg.at<uchar>((i), j + 1);
			int value20 = gradimg.at<uchar>((i + 1), j - 1);
			int value21 = gradimg.at<uchar>((i + 1), j);
			int value22 = gradimg.at<uchar>((i + 1), j + 1);

			if (direction[k] > 0 && direction[k] <= 45)
			{
				if (value11 <= (value12 + (value02 - value12)*tan(direction[i*max_control.rows + j])) || (value11 <= (value10 + (value20 - value10)*tan(direction[i*max_control.rows + j]))))
				{
					max_control.at<uchar>(i, j) = 0;
				}
			}

			if (direction[k] > 45 && direction[k] <= 90)
			{
				if (value11 <= (value01 + (value02 - value01) / tan(direction[i*max_control.cols + j])) || value11 <= (value21 + (value20 - value21) / tan(direction[i*max_control.cols + j])))
				{
					max_control.at<uchar>(i, j) = 0;

				}
			}

			if (direction[k] > 90 && direction[k] <= 135)
			{
				if (value11 <= (value01 + (value00 - value01) / tan(180 - direction[i*max_control.cols + j])) || value11 <= (value21 + (value22 - value21) / tan(180 - direction[i*max_control.cols + j])))
				{
					max_control.at<uchar>(i, j) = 0;
				}
			}
			if (direction[k] > 135 && direction[k] <= 180)
			{
				if (value11 <= (value10 + (value00 - value10)*tan(180 - direction[i*max_control.cols + j])) || value11 <= (value12 + (value22 - value11)*tan(180 - direction[i*max_control.cols + j])))
				{
					max_control.at<uchar>(i, j) = 0;
				}
			}
			k++;
		}
	}
	DoubleThreshold(max_control, 10, 40);
	DoubleThresholdLink(max_control, 10, 40);

	for (int i = 0; i < grayImg.rows - 1; i++)
	{
		for (int j = 0; j < grayImg.cols - 1; j++) {
			gray2Img.at<uchar>(i, j) = saturate_cast<uchar>(grayImg.at<uchar>(i, j) - max_control.at<uchar>(i, j));
		}
	}

	Qtemp2 = QImage((const uchar*)(max_control.data), max_control.cols, max_control.rows, max_control.cols*max_control.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp2.size());
	ui.label_2->show();

	Qtemp = QImage((const uchar*)(gray2Img.data), gray2Img.cols, gray2Img.rows, gray2Img.cols*gray2Img.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_average_filter_clicked()
{
	Mat filterImg;
	QImage Qtemp, Qtemp2;

	filterImg = noiseImg.clone();

	for (int i = 1; i < noiseImg.rows - 1; i++)
		for (int j = 1; j < noiseImg.cols - 1; j++) {
			for (int k = 0; k < 3; k++) {
				filterImg.at<Vec3b>(i, j)[k] = saturate_cast<uchar>((noiseImg.at<Vec3b>(i - 1, j - 1)[k] + noiseImg.at<Vec3b>(i - 1, j)[k] + noiseImg.at<Vec3b>(i - 1, j + 1)[k]
					+ noiseImg.at<Vec3b>(i, j - 1)[k] + noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i, j + 1)[k]
					+ noiseImg.at<Vec3b>(i + 1, j - 1)[k] + noiseImg.at<Vec3b>(i + 1, j)[k] + noiseImg.at<Vec3b>(i + 1, j + 1)[k]) / 9);
			}
		}

	Qtemp2 = QImage((const unsigned char*)(filterImg.data), filterImg.cols, filterImg.rows, filterImg.step, QImage::Format_RGB888);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_middle_filter_clicked()
{
	Mat filterImg;
	QImage Qtemp, Qtemp2;
	QVector<double> middle(9, 0);

	filterImg = noiseImg.clone();

	for (int i = 1; i < noiseImg.rows - 1; i++)
		for (int j = 1; j < noiseImg.cols - 1; j++) {
			for (int k = 0; k < 3; k++) {
				middle[0] = noiseImg.at<Vec3b>(i - 1, j - 1)[k];
				middle[1] = noiseImg.at<Vec3b>(i - 1, j)[k];
				middle[2] = noiseImg.at<Vec3b>(i - 1, j + 1)[k];
				middle[3] = noiseImg.at<Vec3b>(i, j - 1)[k];
				middle[4] = noiseImg.at<Vec3b>(i, j)[k];
				middle[5] = noiseImg.at<Vec3b>(i, j + 1)[k];
				middle[6] = noiseImg.at<Vec3b>(i + 1, j - 1)[k];
				middle[7] = noiseImg.at<Vec3b>(i + 1, j)[k];
				middle[8] = noiseImg.at<Vec3b>(i + 1, j + 1)[k];

				std::sort(middle.begin(), middle.end());

				filterImg.at<Vec3b>(i, j)[k] = saturate_cast<uchar>(middle[5]);
			}
		}

	Qtemp2 = QImage((const unsigned char*)(filterImg.data), filterImg.cols, filterImg.rows, filterImg.step, QImage::Format_RGB888);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_window_filter_clicked()
{
	Mat filterImg;
	QImage Qtemp, Qtemp2;
	QVector<double> window(8, 0), minImg(8, 0);

	filterImg = noiseImg.clone();

	for (int i = 1; i < noiseImg.rows - 1; i++)
		for (int j = 1; j < noiseImg.cols - 1; j++) {
			for (int k = 0; k < 3; k++) {
				window[0] = (noiseImg.at<Vec3b>(i - 1, j - 1)[k] + noiseImg.at<Vec3b>(i - 1, j)[k] + noiseImg.at<Vec3b>(i, j - 1)[k] + noiseImg.at<Vec3b>(i, j)[k]) / 4;
				window[1] = (noiseImg.at<Vec3b>(i - 1, j)[k] + noiseImg.at<Vec3b>(i - 1, j + 1)[k] + noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i, j + 1)[k]) / 4;
				window[2] = (noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i, j + 1)[k] + noiseImg.at<Vec3b>(i + 1, j)[k] + noiseImg.at<Vec3b>(i + 1, j + 1)[k]) / 4;
				window[3] = (noiseImg.at<Vec3b>(i, j - 1)[k] + noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i + 1, j - 1)[k] + noiseImg.at<Vec3b>(i + 1, j)[k]) / 4;
				window[4] = (noiseImg.at<Vec3b>(i - 1, j - 1)[k] + noiseImg.at<Vec3b>(i - 1, j)[k] + noiseImg.at<Vec3b>(i - 1, j + 1)[k] + noiseImg.at<Vec3b>(i, j - 1)[k] + noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i, j + 1)[k]) / 6;
				window[5] = (noiseImg.at<Vec3b>(i + 1, j - 1)[k] + noiseImg.at<Vec3b>(i + 1, j)[k] + noiseImg.at<Vec3b>(i + 1, j + 1)[k] + noiseImg.at<Vec3b>(i, j - 1)[k] + noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i, j + 1)[k]) / 6;
				window[6] = (noiseImg.at<Vec3b>(i - 1, j)[k] + noiseImg.at<Vec3b>(i - 1, j + 1)[k] + noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i, j + 1)[k] + noiseImg.at<Vec3b>(i + 1, j)[k] + noiseImg.at<Vec3b>(i + 1, j + 1)[k]) / 6;
				window[7] = (noiseImg.at<Vec3b>(i - 1, j - 1)[k] + noiseImg.at<Vec3b>(i - 1, j)[k] + noiseImg.at<Vec3b>(i, j)[k] + noiseImg.at<Vec3b>(i, j - 1)[k] + noiseImg.at<Vec3b>(i + 1, j)[k] + noiseImg.at<Vec3b>(i + 1, j - 1)[k]) / 6;

				for (int n = 0; n < 8; n++) {
					minImg[n] = pow(window[n] - noiseImg.at<Vec3b>(i, j)[k], 2);
				}
				auto smallest = std::min_element(std::begin(minImg), std::end(minImg));
				int position = std::distance(std::begin(minImg), smallest);
				filterImg.at<Vec3b>(i, j)[k] = saturate_cast<uchar>(window[position]);
			}
		}

	Qtemp2 = QImage((const unsigned char*)(filterImg.data), filterImg.cols, filterImg.rows, filterImg.step, QImage::Format_RGB888);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_gauss_filter_clicked()
{
	Mat filterImg;
	QImage Qtemp, Qtemp2;

	filterImg = noiseImg.clone();

	for (int i = 1; i < noiseImg.rows - 1; i++)
		for (int j = 1; j < noiseImg.cols - 1; j++) {
			for (int k = 0; k < 3; k++) {
				filterImg.at<Vec3b>(i, j)[k] = saturate_cast<uchar>((noiseImg.at<Vec3b>(i - 1, j - 1)[k] + 2 * noiseImg.at<Vec3b>(i - 1, j)[k] + noiseImg.at<Vec3b>(i - 1, j + 1)[k]
					+ 2 * noiseImg.at<Vec3b>(i, j - 1)[k] + 4 * noiseImg.at<Vec3b>(i, j)[k] + 2 * noiseImg.at<Vec3b>(i, j + 1)[k]
					+ noiseImg.at<Vec3b>(i + 1, j - 1)[k] + 2 * noiseImg.at<Vec3b>(i + 1, j)[k] + noiseImg.at<Vec3b>(i + 1, j + 1)[k]) / 16);
			}
		}

	Qtemp2 = QImage((const unsigned char*)(filterImg.data), filterImg.cols, filterImg.rows, filterImg.step, QImage::Format_RGB888);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_form_filter_clicked()
{
	Mat filterImg, temp, RGB;
	QImage Qtemp, Qtemp2;

	Mat element = getStructuringElement(MORPH_RECT, Size(15, 15));
	cvtColor(srcImg, RGB, CV_BGR2RGB);
	erode(RGB, temp, element);
	dilate(temp, filterImg, element);


	Qtemp2 = QImage((const unsigned char*)(filterImg.data), filterImg.cols, filterImg.rows, filterImg.step, QImage::Format_RGB888);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();

}

void ThreeDrecontrution::on_affine_clicked()
{
	QImage Qtemp, Qtemp2;
	Point2f srcTri[3], dstTri[3];
	Mat rot_mat(2, 3, CV_32FC1);
	Mat warp_mat(2, 3, CV_32FC1);
	Mat dst, RGB;
	cvtColor(srcImg, RGB, CV_BGR2RGB);

	dst = Mat::zeros(RGB.rows, RGB.cols, RGB.type());

	srcTri[0] = Point2f(0, 0);
	srcTri[1] = Point2f(RGB.cols - 1, 0); //缩小一个像素
	srcTri[2] = Point2f(0, RGB.rows - 1);

	dstTri[0] = Point2f(RGB.cols * 0.0, RGB.rows * 0.33);
	dstTri[1] = Point2f(RGB.cols * 0.85, RGB.rows * 0.25);
	dstTri[2] = Point2f(RGB.cols* 0.15, RGB.rows* 0.7);

	warp_mat = getAffineTransform(srcTri, dstTri);

	warpAffine(RGB, dst, warp_mat, RGB.size());

	Qtemp2 = QImage((const unsigned char*)(dst.data), dst.cols, dst.rows, dst.step, QImage::Format_RGB888);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_perspective_clicked()
{
	QImage Qtemp, Qtemp2;
	Point2f srcQuad[4], dstQuad[4];
	Mat warp_matrix(3, 3, CV_32FC1);
	Mat dst, RGB;
	cvtColor(srcImg, RGB, CV_BGR2RGB);
	dst = Mat::zeros(RGB.rows, RGB.cols, RGB.type());

	srcQuad[0] = Point2f(0, 0); //src top left
	srcQuad[1] = Point2f(RGB.cols - 1, 0); //src top right
	srcQuad[2] = Point2f(0, RGB.rows - 1); //src bottom left
	srcQuad[3] = Point2f(RGB.cols - 1, RGB.rows - 1); //src bot right

	dstQuad[0] = Point2f(RGB.cols*0.05, RGB.rows*0.33); //dst top left
	dstQuad[1] = Point2f(RGB.cols*0.9, RGB.rows*0.25); //dst top right
	dstQuad[2] = Point2f(RGB.cols*0.2, RGB.rows*0.7); //dst bottom left
	dstQuad[3] = Point2f(RGB.cols*0.8, RGB.rows*0.9); //dst bot right

	warp_matrix = getPerspectiveTransform(srcQuad, dstQuad);
	warpPerspective(RGB, dst, warp_matrix, RGB.size());

	Qtemp2 = QImage((const unsigned char*)(dst.data), dst.cols, dst.rows, dst.step, QImage::Format_RGB888);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();
}

void ThreeDrecontrution::on_threshold_seg_clicked()
{
	QImage Qtemp;
	Mat targetImg;
	targetImg.create(grayImg.rows, grayImg.cols, CV_8UC1);

	for (int i = 0; i < grayImg.rows; i++) {
		for (int j = 0; j < grayImg.cols; j++) {
			if (grayImg.at<uchar>(i, j) > 100) {
				targetImg.at<uchar>(i, j) = 255;
			}
			else { targetImg.at<uchar>(i, j) = 0; }
		}
	}
	Qtemp = QImage((const uchar*)(targetImg.data), targetImg.cols, targetImg.rows, targetImg.cols*targetImg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp.size());
	ui.label_2->show();

}

void ThreeDrecontrution::on_OSTU_clicked()
{
	QVector<int> hist(256, 0);

	for (int i = 0; i < grayImg.rows; i++)
		for (int j = 0; j < grayImg.cols; j++) {
			hist[grayImg.at<uchar>(i, j)]++;
		}
	int T;
	T = OSTU(hist);
	std::cout << "OSTU:" << T << std::endl;

	QImage Qtemp;
	Mat targetImg;
	targetImg.create(grayImg.rows, grayImg.cols, CV_8UC1);

	for (int i = 0; i < grayImg.rows; i++) {
		for (int j = 0; j < grayImg.cols; j++) {
			if (grayImg.at<uchar>(i, j) > T) {
				targetImg.at<uchar>(i, j) = 255;
			}
			else { targetImg.at<uchar>(i, j) = 0; }
		}
	}
	Qtemp = QImage((const uchar*)(targetImg.data), targetImg.cols, targetImg.rows, targetImg.cols*targetImg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp.size());
	ui.label_2->show();


}

void ThreeDrecontrution::on_Kittler_clicked()
{
	QImage Qtemp;
	Mat targetImg, temp;
	temp = grayImg.clone();
	targetImg.create(grayImg.rows, grayImg.cols, CV_8UC1);

	int Grads, sumGrads = 0, sumGrayGrads = 0, KT;

	for (int i = 1; i < temp.rows - 1; i++)
	{
		uchar* previous = temp.ptr<uchar>(i - 1); // previous row
		uchar* current = temp.ptr<uchar>(i); // current row
		uchar* next = temp.ptr<uchar>(i + 1); // next row
		for (int j = 1; j < temp.cols - 1; j++)
		{   //求水平或垂直方向的最大梯度
			Grads = MAX(abs(previous[j] - next[j]), abs(current[j - 1] - current[j + 1]));
			sumGrads += Grads;
			sumGrayGrads += Grads * (current[j]); //梯度与当前点灰度的积
		}
	}
	KT = sumGrayGrads / sumGrads;
	std::cout << "OSTU:" << KT << std::endl;

	for (int i = 0; i < grayImg.rows; i++) {
		for (int j = 0; j < grayImg.cols; j++) {
			if (grayImg.at<uchar>(i, j) > KT) {
				targetImg.at<uchar>(i, j) = 255;
			}
			else { targetImg.at<uchar>(i, j) = 0; }
		}
	}
	Qtemp = QImage((const uchar*)(targetImg.data), targetImg.cols, targetImg.rows, targetImg.cols*targetImg.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp.size());
	ui.label_2->show();

}

void ThreeDrecontrution::on_frame_diff_clicked()
{
	Mat pFrame1, pFrame2, pFrame3;  //当前帧

	VideoCapture pCapture;

	int nFrmNum;

	Mat pframe;
	pCapture = VideoCapture("E:\Myreconstruction\OpenCVImage-main\open_image\\cat.avi");
	pCapture >> pframe;

	Mat pFrImg1, pFrImg2, pFrImg3;   //当前帧

	pFrImg1.create(pframe.size(), CV_8UC1);
	pFrImg2.create(pframe.size(), CV_8UC1);
	pFrImg3.create(pframe.size(), CV_8UC1);

	Mat pFrMat1, pFrMat2, pFrMat3;

	nFrmNum = 0;
	while (1)
	{
		nFrmNum++;

		pCapture >> pFrame1;
		if (pFrame1.data == NULL)
			return;
		pCapture >> pFrame2;
		pCapture >> pFrame3;

		cvtColor(pFrame1, pFrImg1, CV_BGR2GRAY);
		cvtColor(pFrame2, pFrImg2, CV_BGR2GRAY);
		cvtColor(pFrame3, pFrImg3, CV_BGR2GRAY);

		absdiff(pFrImg1, pFrImg2, pFrMat1);//当前帧跟前面帧相减
		absdiff(pFrImg2, pFrImg3, pFrMat2);//当前帧与后面帧相减
													  //二值化前景图
		threshold(pFrMat1, pFrMat1, 10, 255.0, CV_THRESH_BINARY);
		threshold(pFrMat2, pFrMat2, 10, 255.0, CV_THRESH_BINARY);

		Mat element = getStructuringElement(0, cv::Size(3, 3));
		Mat element1 = getStructuringElement(0, cv::Size(5, 5));
		//膨胀化前景图
		erode(pFrMat1, pFrMat1, element);
		erode(pFrMat2, pFrMat2, element);

		dilate(pFrMat1, pFrMat1, element1);
		dilate(pFrMat2, pFrMat2, element1);

		dilate(pFrMat1, pFrMat1, element1);
		dilate(pFrMat2, pFrMat2, element1);

		//imshow("diff1", pFrMat1);
		imshow("diff2", pFrMat2);

		vector<vector<Point>> contours;
		vector<Vec4i> hierarchy;
		//当前帧与前面帧相减后提取的轮廓线
		findContours(pFrMat2, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		double Maxarea = 0;
		int numi = 0;
		for (size_t i = 0; i < contours.size(); ++i)
		{
			double area = contourArea(contours[i], false);
			if (area > Maxarea)
			{
				Maxarea = area;
				numi = i;
			}
		}
		if (numi != 0)
			drawContours(pFrame2, contours, numi, Scalar(0, 0, 255), 2);

		Mat resultImage = Mat::zeros(pFrMat2.size(), CV_8U);

		imshow("src", pFrame2);
		//waitKey(10);
		if (waitKey(1) != -1)
			break;
	}
	pCapture.release();

	// Closes all the frames
	destroyAllWindows();
}

void ThreeDrecontrution::on_mix_guass_clicked()
{
	Mat greyimg;
	Mat foreground, foreground2;
	Ptr<BackgroundSubtractorKNN> ptrKNN = createBackgroundSubtractorKNN(100, 400, true);
	Ptr<BackgroundSubtractorMOG2> mog2 = createBackgroundSubtractorMOG2(100, 25, true);
	namedWindow("Extracted Foreground");
	VideoCapture pCapture;
	Mat pframe;
	pCapture = VideoCapture("E:\Myreconstruction\OpenCVImage-main\open_image\\pets2001.avi");

	while (1)
	{
		pCapture >> pframe;
		if (pframe.data == NULL)
			return;
		cvtColor(pframe, greyimg, CV_BGR2GRAY);
		long long t = getTickCount();
		ptrKNN->apply(pframe, foreground, 0.01);
		long long t1 = getTickCount();
		mog2->apply(greyimg, foreground2, -1);
		long long t2 = getTickCount();
		//_cprintf("t1 = %f t2 = %f\n", (t1 - t) / getTickFrequency(), (t2 - t1) / getTickFrequency());
		cout << "t1 = " << (t1 - t) / getTickFrequency() << "t2 = " << (t2 - t1) / getTickFrequency() << endl;
		imshow("Extracted Foreground", foreground);
		imshow("Extracted Foreground2", foreground2);
		imshow("video", pframe);
		if (waitKey(1) != -1)
			break;
	}
	destroyAllWindows();
}

void ThreeDrecontrution::on_circle_lbp_clicked()
{
	QImage Qtemp0, Qtemp, Qtemp1, Qtemp2;

	Mat Img = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena.jpg");

	Mat temp;
	cvtColor(Img, temp, COLOR_BGR2RGB);//BGR convert to RGB
	Qtemp = QImage((const unsigned char*)(temp.data), temp.cols, temp.rows, temp.step, QImage::Format_RGB888);

	ui.label->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label->setScaledContents(true);
	ui.label->resize(Qtemp.size());
	ui.label->show();

	Mat img = cv::imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena.jpg", 0);
	//namedWindow("image");
	//imshow("image", img);

	Qtemp = QImage((const uchar*)(img.data), img.cols, img.rows, img.cols*img.channels(), QImage::Format_Indexed8);
	ui.label_1->setPixmap(QPixmap::fromImage(Qtemp));
	Qtemp = Qtemp.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_1->setScaledContents(true);
	ui.label_1->resize(Qtemp.size());
	ui.label_1->show();

	int radius, neighbors;
	radius = 1;
	neighbors = 8;

	//创建一个LBP
	//注意为了溢出，我们行列都在原有图像上减去2个半径
	Mat dst = Mat(img.rows - 2 * radius, img.cols - 2 * radius, CV_8UC1, Scalar(0));
	elbp1(img, dst);
	//namedWindow("normal");
	//imshow("normal", dst);
	Qtemp1 = QImage((const uchar*)(dst.data), dst.cols, dst.rows, dst.cols*dst.channels(), QImage::Format_Indexed8);
	ui.label_2->setPixmap(QPixmap::fromImage(Qtemp1));
	Qtemp1 = Qtemp1.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_2->setScaledContents(true);
	ui.label_2->resize(Qtemp1.size());
	ui.label_2->show();

	Mat dst1 = Mat(img.rows - 2 * radius, img.cols - 2 * radius, CV_8UC1, Scalar(0));
	elbp(img, dst1, 1, 8);
	//namedWindow("circle");
	//imshow("circle", dst1);
	Qtemp2 = QImage((const uchar*)(dst1.data), dst1.cols, dst1.rows, dst1.cols*dst1.channels(), QImage::Format_Indexed8);
	ui.label_3->setPixmap(QPixmap::fromImage(Qtemp2));
	Qtemp2 = Qtemp2.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	ui.label_3->setScaledContents(true);
	ui.label_3->resize(Qtemp2.size());
	ui.label_3->show();

}

void ThreeDrecontrution::on_target_det_clicked()
{
	QImage Qtemp, Qtemp1;

	Mat temp0 = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena.jpg");
	Mat temp1 = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena-1.jpg");
	Mat Img0, Img1, Img2;

	cvtColor(temp0, Img0, COLOR_BGR2HSV);
	cvtColor(temp1, Img1, COLOR_BGR2HSV);
	Mat box = Img1.clone();

	int h_bins = 50;
	int s_bins = 60;
	int histSize[] = { h_bins,s_bins };
	float h_ranges[] = { 0,180 };
	float s_ranges[] = { 0,256 };
	const float* ranges[] = { h_ranges, s_ranges };
	int channels[] = { 0,1 };

	double max = 0.;
	int x_ray, y_ray;


	for (int i = 0; i < Img0.rows - Img1.rows - 1; i++) {
		for (int j = 0; j < Img0.cols - Img1.cols - 1; j++) {
			for (int x = i; x < Img1.rows + i; x++) {
				for (int y = j; y < Img1.cols + j; y++) {
					box.at<Vec3b>(x - i, y - j) = Img0.at<Vec3b>(x, y);
				}
			}
			MatND hist_src0;
			MatND hist_src1;

			calcHist(&box, 1, channels, Mat(), hist_src0, 2, histSize, ranges, true, false);
			normalize(hist_src0, hist_src0, 0, 1, NORM_MINMAX, -1, Mat());

			calcHist(&Img1, 1, channels, Mat(), hist_src1, 2, histSize, ranges, true, false);
			normalize(hist_src1, hist_src1, 0, 1, NORM_MINMAX, -1, Mat());

			double src_src = compareHist(hist_src0, hist_src1, CV_COMP_CORREL);

			cout << "src compare with src correlation value : " << src_src << endl;

			if (src_src > max) {
				max = src_src;
				x_ray = i;
				y_ray = j;
			}
		}
	}
	cout << "************************************" << endl;

	Rect rect(x_ray, y_ray, Img1.rows, Img1.cols);
	rectangle(Img0, rect, Scalar(255, 0, 0), 1, LINE_8, 0);
	imshow("check", Img0);
	imshow("Img1", Img1);

	waitKey(0);
	cv::destroyWindow("check");
	cv::destroyWindow("Img1");
	waitKey(1);
}

void ThreeDrecontrution::on_model_check_clicked()
{
	QImage Qtemp, Qtemp1;
	double minVal; double maxVal; Point minLoc; Point maxLoc;

	Mat Img0 = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena.jpg");
	Mat Img1 = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena-1.jpg");

	Mat result;

	matchTemplate(Img0, Img1, result, TM_SQDIFF);

	normalize(result, result, 1, 0, NORM_MINMAX);
	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
	rectangle(Img0, Rect(minLoc.x, minLoc.y, Img1.cols, Img1.rows), 1, 8, 0);

	imshow("src", Img0);
	imshow("template", Img1);
	imshow("0", result);
	waitKey(0);
	cv::destroyWindow("src");
	cv::destroyWindow("template");
	cv::destroyWindow("0");
	waitKey(1);

}

void ThreeDrecontrution::on_cloaking_clicked()
{
	// TODO: 在此添加控件通知处理程序代码
	// Create a VideoCapture object and open the input file
	VideoCapture cap;
	cap.open("E:\Myreconstruction\OpenCVImage-main\open_image\\input-color.mp4");

	// Check if camera opened successfully
	if (!cap.isOpened()) {
		cout << "Error opening video stream or file" << endl;
		return;
	}

	Mat background, background1;
	for (int i = 0; i < 60; i++)
	{
		cap >> background;
	}

	//flip(background,background,1);

	for (int i = 0; i < 100; i++)
	{
		cap >> background1;
	}
	while (1)
	{
		long t = getTickCount();

		Mat frame;
		// Capture frame-by-frame
		cap >> frame;


		// If the frame is empty, break immediately
		if (frame.empty())
			break;

		Mat hsv;
		//flip(frame,frame,1);
		cvtColor(frame, hsv, COLOR_BGR2HSV);

		Mat mask1, mask2;
		inRange(hsv, Scalar(0, 120, 70), Scalar(10, 255, 255), mask1);   //H为0-10的分量
		inRange(hsv, Scalar(170, 120, 70), Scalar(180, 255, 255), mask2);//H为170-180的分量


		mask1 = mask1 + mask2;

		Mat kernel = Mat::ones(3, 3, CV_32F);
		morphologyEx(mask1, mask1, MORPH_OPEN, kernel);//开
		morphologyEx(mask1, mask1, MORPH_DILATE, kernel);//膨胀

		bitwise_not(mask1, mask2);

		Mat res1, res2, final_output;
		bitwise_and(frame, frame, res1, mask2);//not red
		//imshow("res1 !!!", res1);
		bitwise_and(background, background, res2, mask1);//red

	//	long t1 = getTickCount();
		//imshow("res2 !!!",res2);
		//addWeighted(res1, 1, res2, 1, 0, final_output);
		add(res1, res2, final_output);

		imshow("Magic !!!", final_output);
		// Display the resulting frame
		//imshow( "Frame", frame );

		// Press  ESC on keyboard to exit
		char c = (char)waitKey(5);
		if (c == 27)
			break;
		// Also relese all the mat created in the code to avoid memory leakage.
		frame.release(), hsv.release(), mask1.release(), mask2.release(), res1.release(), res2.release(), final_output.release();

		long t1 = getTickCount();

		cout << "t1 =  " << (t1 - t) / getTickFrequency() * 1000 << "ms\n" << endl;

		if (waitKey(1) != -1)
			break;
	}

	// When everything done, release the video capture object
	cap.release();

	// Closes all the frames
	destroyAllWindows();
}

void ThreeDrecontrution::on_SIFT_clicked()
{
	Mat src1 = imread("E:\Myreconstruction\OpenCVImage-main\open_image\\1.1.jpg", 1);
	Mat src2 = imread("E:\Myreconstruction\OpenCVImage-main\open_image\\1.2.jpg", 1);
	imshow("src1", src1);
	imshow("src2", src2);

	if (!src1.data || !src2.data)
	{
		//_cprintf(" --(!) Error reading images \n");
		return;
	}

	//sift feature detect
	Ptr<SIFT> siftdetector = SIFT::create();
	vector<KeyPoint> kp1, kp2;

	siftdetector->detect(src1, kp1);
	siftdetector->detect(src2, kp2);
	Mat des1, des2;//descriptor
	siftdetector->compute(src1, kp1, des1);
	siftdetector->compute(src2, kp2, des2);
	Mat res1, res2;

	drawKeypoints(src1, kp1, res1);//在内存中画出特征点
	drawKeypoints(src2, kp2, res2);

	//_cprintf("size of description of Img1: %d\n",kp1.size());
	//_cprintf("size of description of Img2: %d\n",kp2.size());

	Mat transimg1, transimg2;
	transimg1 = res1.clone();
	transimg2 = res2.clone();

	char str1[20], str2[20];
	sprintf(str1, "%d", kp1.size());
	sprintf(str2, "%d", kp2.size());

	const char* str = str1;
	putText(transimg1, str1, Point(280, 230), 0, 1.0, Scalar(255, 0, 0), 2);//在图片中输出字符

	str = str2;
	putText(transimg2, str2, Point(280, 230), 0, 1.0, Scalar(255, 0, 0), 2);//在图片中输出字符

																			//imshow("Description 1",res1);
	imshow("descriptor1", transimg1);
	imshow("descriptor2", transimg2);

	BFMatcher matcher(NORM_L2, true);
	vector<DMatch> matches;
	matcher.match(des1, des2, matches);
	Mat img_match;
	drawMatches(src1, kp1, src2, kp2, matches, img_match);//,Scalar::all(-1),Scalar::all(-1),vector<char>(),drawmode);
	//_cprintf("number of matched points: %d\n",matches.size());
	imshow("matches", img_match);
	waitKey(0);
	cv::destroyWindow("matches");
	cv::destroyWindow("descriptor1");
	cv::destroyWindow("descriptor2");
	cv::destroyWindow("src1");
	cv::destroyWindow("src2");
	waitKey(1);
}

void ThreeDrecontrution::on_orb_clicked()
{
	Mat obj = imread("E:\Myreconstruction\OpenCVImage-main\open_image\\1.1.jpg");   //载入目标图像
	Mat scene = imread("E:\Myreconstruction\OpenCVImage-main\open_image\\1.2.jpg"); //载入场景图像
	if (obj.empty() || scene.empty())
	{
		cout << "Can't open the picture!\n";
		return;
	}
	vector<KeyPoint> obj_keypoints, scene_keypoints;
	Mat obj_descriptors, scene_descriptors;
	Ptr<ORB> detector = ORB::create();

	detector->detect(obj, obj_keypoints);
	detector->detect(scene, scene_keypoints);
	detector->compute(obj, obj_keypoints, obj_descriptors);
	detector->compute(scene, scene_keypoints, scene_descriptors);

	BFMatcher matcher(NORM_HAMMING, true); //汉明距离做为相似度度量
	vector<DMatch> matches;
	matcher.match(obj_descriptors, scene_descriptors, matches);
	Mat match_img;
	drawMatches(obj, obj_keypoints, scene, scene_keypoints, matches, match_img);
	imshow("match_img", match_img);

	//保存匹配对序号
	vector<int> queryIdxs(matches.size()), trainIdxs(matches.size());
	for (size_t i = 0; i < matches.size(); i++)
	{
		queryIdxs[i] = matches[i].queryIdx;
		trainIdxs[i] = matches[i].trainIdx;
	}

	Mat H12;   //变换矩阵

	vector<Point2f> points1;
	KeyPoint::convert(obj_keypoints, points1, queryIdxs);
	vector<Point2f> points2;
	KeyPoint::convert(scene_keypoints, points2, trainIdxs);
	int ransacReprojThreshold = 5;  //拒绝阈值


	H12 = findHomography(Mat(points1), Mat(points2), RANSAC, ransacReprojThreshold);
	vector<char> matchesMask(matches.size(), 0);
	Mat points1t;
	perspectiveTransform(Mat(points1), points1t, H12);
	for (size_t i1 = 0; i1 < points1.size(); i1++)  //保存‘内点’
	{
		if (norm(points2[i1] - points1t.at<Point2f>((int)i1, 0)) <= ransacReprojThreshold) //给内点做标记
		{
			matchesMask[i1] = 1;
		}
	}
	Mat match_img2;   //滤除‘外点’后
	drawMatches(obj, obj_keypoints, scene, scene_keypoints, matches, match_img2, Scalar(0, 0, 255), Scalar::all(-1), matchesMask);

	//画出目标位置
	std::vector<Point2f> obj_corners(4);
	obj_corners[0] = Point(0, 0); obj_corners[1] = Point(obj.cols, 0);
	obj_corners[2] = Point(obj.cols, obj.rows); obj_corners[3] = Point(0, obj.rows);
	std::vector<Point2f> scene_corners(4);
	perspectiveTransform(obj_corners, scene_corners, H12);
	//line( match_img2, scene_corners[0] + Point2f(static_cast<float>(obj.cols), 0),scene_corners[1] + Point2f(static_cast<float>(obj.cols), 0),Scalar(0,0,255),2);
	//line( match_img2, scene_corners[1] + Point2f(static_cast<float>(obj.cols), 0),scene_corners[2] + Point2f(static_cast<float>(obj.cols), 0),Scalar(0,0,255),2);
	//line( match_img2, scene_corners[2] + Point2f(static_cast<float>(obj.cols), 0),scene_corners[3] + Point2f(static_cast<float>(obj.cols), 0),Scalar(0,0,255),2);
	//line( match_img2, scene_corners[3] + Point2f(static_cast<float>(obj.cols), 0),scene_corners[0] + Point2f(static_cast<float>(obj.cols), 0),Scalar(0,0,255),2);
	line(match_img2, Point2f((scene_corners[0].x + static_cast<float>(obj.cols)), (scene_corners[0].y)), Point2f((scene_corners[1].x + static_cast<float>(obj.cols)), (scene_corners[1].y)), Scalar(0, 0, 255), 2);
	line(match_img2, Point2f((scene_corners[1].x + static_cast<float>(obj.cols)), (scene_corners[1].y)), Point2f((scene_corners[2].x + static_cast<float>(obj.cols)), (scene_corners[2].y)), Scalar(0, 0, 255), 2);
	line(match_img2, Point2f((scene_corners[2].x + static_cast<float>(obj.cols)), (scene_corners[2].y)), Point2f((scene_corners[3].x + static_cast<float>(obj.cols)), (scene_corners[3].y)), Scalar(0, 0, 255), 2);
	line(match_img2, Point2f((scene_corners[3].x + static_cast<float>(obj.cols)), (scene_corners[3].y)), Point2f((scene_corners[0].x + static_cast<float>(obj.cols)), (scene_corners[0].y)), Scalar(0, 0, 255), 2);

	float A_th;
	A_th = atan(abs((scene_corners[3].y - scene_corners[0].y) / (scene_corners[3].x - scene_corners[0].x)));
	A_th = 90 - 180 * A_th / 3.14;

	imshow("match_img2", match_img2);

	//line( scene, scene_corners[0],scene_corners[1],Scalar(0,0,255),2);
	//line( scene, scene_corners[1],scene_corners[2],Scalar(0,0,255),2);
	//line( scene, scene_corners[2],scene_corners[3],Scalar(0,0,255),2);
	//line( scene, scene_corners[3],scene_corners[0],Scalar(0,0,255),2);

	imshow("scense", scene);

	Mat rotimage;
	Mat rotate = getRotationMatrix2D(Point(scene.cols / 2, scene.rows / 2), A_th, 1);
	warpAffine(scene, rotimage, rotate, scene.size());
	imshow("rotimage", rotimage);


	//方法三 透视变换
	Point2f src_point[4];
	Point2f dst_point[4];
	src_point[0].x = scene_corners[0].x;
	src_point[0].y = scene_corners[0].y;
	src_point[1].x = scene_corners[1].x;
	src_point[1].y = scene_corners[1].y;
	src_point[2].x = scene_corners[2].x;
	src_point[2].y = scene_corners[2].y;
	src_point[3].x = scene_corners[3].x;
	src_point[3].y = scene_corners[3].y;


	dst_point[0].x = 0;
	dst_point[0].y = 0;
	dst_point[1].x = obj.cols;
	dst_point[1].y = 0;
	dst_point[2].x = obj.cols;
	dst_point[2].y = obj.rows;
	dst_point[3].x = 0;
	dst_point[3].y = obj.rows;

	Mat newM(3, 3, CV_32FC1);
	newM = getPerspectiveTransform(src_point, dst_point);

	Mat dst = scene.clone();

	warpPerspective(scene, dst, newM, obj.size());

	imshow("obj", obj);
	imshow("dst", dst);

	Mat resultimg = dst.clone();

	absdiff(obj, dst, resultimg);//当前帧跟前面帧相减

	imshow("result", resultimg);

	imshow("dst", dst);
	imshow("src", obj);

	waitKey(0);
	cv::destroyWindow("match_img");
	cv::destroyWindow("match_img2");
	cv::destroyWindow("obj");
	cv::destroyWindow("result");
	cv::destroyWindow("dst");
	cv::destroyWindow("src");
	cv::destroyWindow("scense");
	cv::destroyWindow("rotimage");
	waitKey(1);
}

//输入图像
Mat img_color;
//灰度值归一化
Mat bgr_color;
//HSV图像
Mat hsv_color;
//色度
int hsv_hmin = 0;
int hsv_hmin_Max = 360;
int hsv_hmax = 360;
int hsv_hmax_Max = 360;
//饱和度
int hsv_smin = 0;
int hsv_smin_Max = 255;
int hsv_smax = 255;
int hsv_smax_Max = 255;
//亮度
int hsv_vmin = 106;
int hsv_vmin_Max = 255;
int hsv_vmax = 250;
int hsv_vmax_Max = 255;
//显示原图的窗口
string windowName = "src";
//输出图像的显示窗口
string dstName = "dst";
//输出图像
Mat dst_color;
//回调函数
void callBack(int, void*)
{
	//输出图像分配内存
	dst_color = Mat::zeros(img_color.size(), CV_32FC3);
	//掩码
	Mat mask;
	inRange(hsv_color, Scalar(hsv_hmin, hsv_smin / float(hsv_smin_Max), hsv_vmin / float(hsv_vmin_Max)), Scalar(hsv_hmax, hsv_smax / float(hsv_smax_Max), hsv_vmax / float(hsv_vmax_Max)), mask);
	//只保留
	for (int r = 0; r < bgr_color.rows; r++)
	{
		for (int c = 0; c < bgr_color.cols; c++)
		{
			if (mask.at<uchar>(r, c) == 255)
			{
				dst_color.at<Vec3f>(r, c) = bgr_color.at<Vec3f>(r, c);
			}
		}
	}
	//输出图像
	imshow(dstName, dst_color);
	//imshow("mast", mask);
	//保存图像
	//dst_color.convertTo(dst_color, CV_8UC3, 255.0, 0);
	//imwrite("F://program//image//HSV_inRange.jpg", dst_color)

}

void ThreeDrecontrution::on_color_fit_clicked()
{
	img_color = imread("E:\Myreconstruction\OpenCVImage-main\open_image\\color.jpg");
	if (!img_color.data || img_color.channels() != 3)
		return;
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, img_color);
	//彩色图像的灰度值归一化
	img_color.convertTo(bgr_color, CV_32FC3, 1.0 / 255, 0);
	//颜色空间转换
	cvtColor(bgr_color, hsv_color, COLOR_BGR2HSV);
	//定义输出图像的显示窗口
	namedWindow(dstName, WINDOW_GUI_EXPANDED);
	//调节色度 H
	createTrackbar("hmin", dstName, &hsv_hmin, hsv_hmin_Max, callBack);
	createTrackbar("hmax", dstName, &hsv_hmax, hsv_hmax_Max, callBack);
	//调节饱和度 S
	createTrackbar("smin", dstName, &hsv_smin, hsv_smin_Max, callBack);
	createTrackbar("smax", dstName, &hsv_smax, hsv_smax_Max, callBack);
	//调节亮度 V
	createTrackbar("vmin", dstName, &hsv_vmin, hsv_vmin_Max, callBack);
	createTrackbar("vmax", dstName, &hsv_vmax, hsv_vmax_Max, callBack);
	callBack(0, 0);
	waitKey(0);
	cv::destroyWindow(dstName);
	cv::destroyWindow(windowName);
	waitKey(1);
}

void ThreeDrecontrution::on_svm_test_clicked()
{
	int iWidth = 512, iheight = 512;
	//    Mat matImg = Mat::zeros(iheight, iWidth, CV_8UC3);//三色通道
	Mat matImg = Mat(iheight, iWidth, CV_8UC3, Scalar(0, 255, 255));//三色通道
															  //1.获取样本
	double labels[5] = { 1.0, -1.0, -1.0, -1.0,1.0 }; //样本数据
	Mat labelsMat(5, 1, CV_32SC1, labels);     //样本标签
	float trainingData[5][2] = { { 501, 300 },{ 255, 10 },{ 501, 255 },{ 10, 501 },{ 450,500 } }; //Mat结构特征数据
	Mat trainingDataMat(5, 2, CV_32FC1, trainingData);   //Mat结构标签
														 //2.设置SVM参数
	Ptr<ml::SVM> svm = ml::SVM::create();
	svm->setType(ml::SVM::C_SVC);//可以处理非线性分割的问题
	svm->setKernel(ml::SVM::POLY);//径向基函数SVM::LINEAR
										/*svm->setGamma(0.01);
										svm->setC(10.0);*/
										//算法终止条件
	svm->setDegree(1.0);
	svm->setTermCriteria(TermCriteria(CV_TERMCRIT_ITER, 100, 1e-6));
	//3.训练支持向量
	svm->train(trainingDataMat, ml::SampleTypes::ROW_SAMPLE, labelsMat);
	//4.保存训练器
	svm->save("mnist_svm.xml");
	//5.导入训练器
	//Ptr<SVM> svm1 = StatModel::load<SVM>("mnist_dataset/mnist_svm.xml");

	//读取测试数据
	Mat sampleMat;
	Vec3b green(0, 255, 0), blue(255, 0, 0);
	for (int i = 0; i < matImg.rows; i++)
	{
		for (int j = 0; j < matImg.cols; j++)
		{
			sampleMat = (Mat_<float>(1, 2) << j, i);
			float fRespone = svm->predict(sampleMat);
			if (fRespone == 1)
			{
				matImg.at<cv::Vec3b>(i, j) = green;
			}
			else if (fRespone == -1)
			{
				matImg.at<cv::Vec3b>(i, j) = blue;
			}
		}
	}

	for (int i = 0; i < matImg.rows; i++)
	{
		for (int j = 0; j < matImg.cols; j++)
		{
			if (i > 525 - (1. / 2.)*j) {
				matImg.at<cv::Vec3b>(i, j) = green;
			}
			else { matImg.at<cv::Vec3b>(i, j) = blue; }
		}
	}
	// Show the training data
	int thickness = -1;
	int lineType = 8;
	for (int i = 0; i < trainingDataMat.rows; i++)
	{
		if (labels[i] == 1)
		{
			circle(matImg, Point(trainingData[i][0], trainingData[i][1]), 5, Scalar(0, 0, 0), thickness, lineType);
		}
		else
		{
			circle(matImg, Point(trainingData[i][0], trainingData[i][1]), 5, Scalar(255, 255, 255), thickness, lineType);
		}
	}

	//显示支持向量点
	thickness = 2;
	lineType = 8;
	Mat vec = svm->getSupportVectors();
	int nVarCount = svm->getVarCount();//支持向量的维数
	//_cprintf("vec.rows=%d vec.cols=%d\n", vec.rows, vec.cols);
	for (int i = 0; i < vec.rows; ++i)
	{
		int x = (int)vec.at<float>(i, 0);
		int y = (int)vec.at<float>(i, 1);
		//_cprintf("vec.at=%d %f,%f\n", i,vec.at<float>(i, 0), vec.at<float>(i, 1));
		//_cprintf("x=%d,y=%d\n", x, y);
		circle(matImg, Point(x, y), 6, Scalar(0, 0, 255), thickness, lineType);
	}


	imshow("circle", matImg); // show it to the user
	waitKey(0);
	cv::destroyWindow("circle");
	waitKey(1);
}

void ThreeDrecontrution::on_word_test_clicked()
{
	Ptr<ml::SVM> svm1 = ml::SVM::load("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\sample\\SVM_HOG.xml");

	if (svm1->empty())
	{
		cout << "load svm detector failed!!!\n" << endl;
		return;
	}

	Mat testimg;
	testimg = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\sample\\9\\0.png");
	cv::resize(testimg, testimg, Size(28, 28), 1);
	imshow("src", testimg);
	//waitKey(0);

	HOGDescriptor hog(Size(14, 14), Size(7, 7), Size(1, 1), Size(7, 7), 9);
	vector<float> imgdescriptor;
	hog.compute(testimg, imgdescriptor, Size(5, 5));
	Mat sampleMat;
	sampleMat.create(1, imgdescriptor.size(), CV_32FC1);

	for (int i = 0; i < imgdescriptor.size(); i++)
	{
		sampleMat.at<float>(0, i) = imgdescriptor[i];//第num个样本的特征向量中的第i个元素
	}
	int ret = svm1->predict(sampleMat);
	cout << "ret= " << ret << endl;

	waitKey(0);
	cv::destroyWindow("src");
	waitKey(1);
}

double compute_sum_of_rect(Mat src, Rect r) {
	int x = r.x;
	int y = r.y;
	int width = r.width;
	int height = r.height;
	double sum;
	//这里使用Mat::at函数需要注意第一参数为行数对应的y和高度height，第二个参数对应才是列数对应的x和宽度width
	sum = src.at<double>(y, x) + src.at<double>(y + height, x + width)
		- src.at<double>(y + height, x) - src.at<double>(y, x + width);

	return sum;
}

void ThreeDrecontrution::on_Haar_1_clicked()
{
	Mat src_img;
	src_img = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena.jpg");
	if (src_img.empty()) {
		cout << "error.could not find" << endl;
		return;
	}
	namedWindow("src_img", CV_WINDOW_AUTOSIZE);
	imshow("src_img", src_img);
	Mat gray_img;
	cvtColor(src_img, gray_img, COLOR_BGR2GRAY);
	namedWindow("gray_img", CV_WINDOW_AUTOSIZE);
	imshow("gray_img", gray_img);
	Mat sum_img = Mat::zeros(gray_img.rows + 1, gray_img.cols + 1, CV_32FC1);
	//Mat sqsum_img = Mat::zeros(gray_img.rows + 1, gray_img.cols + 1,CV_64FC1);
	integral(grayImg, sum_img, CV_64F);

	int step_x = 8, step_y = 8;
	double sum;
	Rect rect1, rect2;
	Mat dst = Mat::zeros(src_img.size(), CV_32FC1);
	for (int i = 0; i < src_img.rows; i = i + step_x) {
		for (int j = 0; j < src_img.cols; j = j + step_y) {
			rect1 = Rect(j, i, 2, 4);
			rect2 = Rect(j + 2, i, 2, 4);
			sum = compute_sum_of_rect(gray_img, rect1) - compute_sum_of_rect(gray_img, rect2);
			dst.at<float>(i, j) = sum;
		}
	}

	Mat dst_8;
	convertScaleAbs(dst, dst_8);
	imshow("dst", dst_8);

	waitKey(0);
	cv::destroyWindow("src_img");
	cv::destroyWindow("gray_img");
	cv::destroyWindow("dst");
	waitKey(1);
}

void ThreeDrecontrution::on_Haar_2_clicked()
{
	Mat src_img;
	src_img = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena.jpg");
	if (src_img.empty()) {
		cout << "error.could not find" << endl;
		return;
	}
	namedWindow("src_img", CV_WINDOW_AUTOSIZE);
	imshow("src_img", src_img);
	Mat gray_img;
	cvtColor(src_img, gray_img, COLOR_BGR2GRAY);
	namedWindow("gray_img", CV_WINDOW_AUTOSIZE);
	imshow("gray_img", gray_img);
	Mat sum_img = Mat::zeros(gray_img.rows + 1, gray_img.cols + 1, CV_32FC1);
	//Mat sqsum_img = Mat::zeros(gray_img.rows + 1, gray_img.cols + 1,CV_64FC1);
	integral(grayImg, sum_img, CV_64F);

	int step_x = 8, step_y = 8;
	double sum;
	Rect rect1, rect2;
	Mat dst = Mat::zeros(src_img.size(), CV_32FC1);
	for (int i = 0; i < src_img.rows; i = i + step_x) {
		for (int j = 0; j < src_img.cols; j = j + step_y) {
			rect1 = Rect(j, i, 2, 4);
			rect2 = Rect(j, i + 2, 2, 4);
			sum = compute_sum_of_rect(gray_img, rect1) - compute_sum_of_rect(gray_img, rect2);
			dst.at<float>(i, j) = sum;
		}
	}

	Mat dst_8;
	convertScaleAbs(dst, dst_8);
	imshow("dst", dst_8);

	waitKey(0);
	cv::destroyWindow("src_img");
	cv::destroyWindow("gray_img");
	cv::destroyWindow("dst");
	waitKey(1);
}

void ThreeDrecontrution::on_gaber_clicked()
{
	Mat src = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\lena.jpg", IMREAD_GRAYSCALE);
	namedWindow("input", CV_WINDOW_AUTOSIZE);
	imshow("input", src);
	Mat src_f;
	src.convertTo(src_f, CV_32F);
	// 参数初始化
	int kernel_size = 3;
	double sigma = 1.0, lambd = CV_PI / 8, gamma = 0.5, psi = 0;
	vector<Mat> destArray;
	double theta[4];
	Mat temp;
	// theta 法线方向
	theta[0] = 0;
	theta[1] = CV_PI / 4;
	theta[2] = CV_PI / 2;
	theta[3] = CV_PI - CV_PI / 4;
	// gabor 纹理检测器，可以更多，
	// filters = number of thetas * number of lambd
	// 这里lambad只取一个值，所有4个filter
	for (int i = 0; i < 4; i++)
	{
		Mat kernel1;
		Mat dest;
		kernel1 = getGaborKernel(cv::Size(kernel_size, kernel_size), sigma, theta[i], lambd, gamma, psi, CV_32F);
		filter2D(src_f, dest, CV_32F, kernel1);
		destArray.push_back(dest);
	}
	// 显示与保存
	Mat dst1, dst2, dst3, dst4;
	convertScaleAbs(destArray[0], dst1);
	//imwrite("F://program//image//gabor1.jpg", dst1);
	convertScaleAbs(destArray[1], dst2);
	//imwrite("F://program//image//gabor2.jpg", dst2);
	convertScaleAbs(destArray[2], dst3);
	//imwrite("F://program//image//gabor3.jpg", dst3);
	convertScaleAbs(destArray[3], dst4);
	//imwrite("F://program//image//gabor4.jpg", dst4);
	imshow("gabor1", dst1);
	imshow("gabor2", dst2);
	imshow("gabor3", dst3);
	imshow("gabor4", dst4);
	// 合并结果
	add(destArray[0], destArray[1], destArray[0]);
	add(destArray[2], destArray[3], destArray[2]);
	add(destArray[0], destArray[2], destArray[0]);
	Mat dst;
	convertScaleAbs(destArray[0], dst, 0.2, 0);
	// 二值化显示
	Mat gray, binary;
	// cvtColor(dst, gray, COLOR_BGR2GRAY);
	threshold(dst, binary, 0, 255, THRESH_BINARY_INV | THRESH_OTSU);
	imshow("result", dst);
	imshow("binary", binary);
	//imwrite("F://program//image//result_01.png", binary);
	waitKey(0);
	cv::destroyWindow("input");
	cv::destroyWindow("gabor1");
	cv::destroyWindow("gabor2");
	cv::destroyWindow("gabor3");
	cv::destroyWindow("gabor4");
	cv::destroyWindow("result");
	cv::destroyWindow("binary");
	waitKey(1);
}

void ThreeDrecontrution::on_face_haar_clicked()
{
	String label = "Face";
	CascadeClassifier faceCascade;
	faceCascade.load("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\face-haar\\haarcascade_frontalface_alt2.xml");//加载分类器
	VideoCapture capture;
	capture.open(0);// 打开摄像头
	//      capture.open("video.avi");    // 打开视频
	if (!capture.isOpened())
	{
		//_cprintf("open camera failed. \n");
		return;
	}
	Mat img, imgGray;
	vector<Rect> faces;
	while (1)
	{
		capture >> img;// 读取图像至img
		if (img.empty())continue;
		if (img.channels() == 3)
			cvtColor(img, imgGray, CV_RGB2GRAY);
		else
		{
			imgGray = img;
		}
		//double start = cv::getTickCount();
		faceCascade.detectMultiScale(imgGray, faces, 1.2, 6, 0, Size(0, 0));// 检测人脸
		//double end = cv::getTickCount();
		//_cprintf("run time: %f ms\n", (end - start));
		if (faces.size() > 0)
		{
			for (int i = 0; i < faces.size(); i++)
			{
				rectangle(img, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 255, 0), 1, 8);
				putText(img, label, Point(faces[i].x, faces[i].y - 5), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0));
			}
		}
		imshow("CamerFace", img); // 显示
		if (waitKey(1) != -1)
			break;// delay ms 等待按键退出
	}
	cv::destroyWindow("CamerFace");
}

void calRealPoint(vector<vector<Point3f>>& obj, int boardWidth, int boardHeight, int imgNumber, int squareSize)
{
	vector<Point3f> imgpoint;
	for (int rowIndex = 0; rowIndex < boardHeight; rowIndex++)
	{
		for (int colIndex = 0; colIndex < boardWidth; colIndex++)
		{
			imgpoint.push_back(Point3f(rowIndex * squareSize, colIndex * squareSize, 0));
		}
	}
	for (int imgIndex = 0; imgIndex < imgNumber; imgIndex++)
	{
		obj.push_back(imgpoint);
	}
}
Mat R, T, E, F;
Mat Rl, Rr, Pl, Pr, Q;
//映射表
Mat mapLx, mapLy, mapRx, mapRy;

Mat cameraMatrixL = (Mat_<double>(3, 3) << 530.1397548683084, 0, 338.2680507680664,
	0, 530.2291152852337, 232.4902023212199,
	0, 0, 1);
//获得的畸变参数
Mat distCoeffL = (Mat_<double>(5, 1) << -0.266294943795012, -0.0450330886310585, 0.0003024821418382528, -0.001243865371699451, 0.2973605735168139);

Mat cameraMatrixR = (Mat_<double>(3, 3) << 530.1397548683084, 0, 338.2680507680664,
	0, 530.2291152852337, 232.4902023212199,
	0, 0, 1);
Mat distCoeffR = (Mat_<double>(5, 1) << -0.266294943795012, -0.0450330886310585, 0.0003024821418382528, -0.001243865371699451, 0.2973605735168139);

void outputCameraParam(void)
{
	/*保存数据*/
	/*输出数据*/
	FileStorage fs("intrisics.yml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "cameraMatrixL" << cameraMatrixL << "cameraDistcoeffL" << distCoeffL << "cameraMatrixR" << cameraMatrixR << "cameraDistcoeffR" << distCoeffR;
		fs.release();
		cout << "cameraMatrixL=:" << cameraMatrixL << endl << "cameraDistcoeffL=:" << distCoeffL << endl << "cameraMatrixR=:" << cameraMatrixR << endl << "cameraDistcoeffR=:" << distCoeffR << endl;
	}
	else
	{
		cout << "Error: can not save the intrinsics!!!!" << endl;
	}

	fs.open("extrinsics.yml", FileStorage::WRITE);
	if (fs.isOpened())
	{
		fs << "R" << R << "T" << T << "Rl" << Rl << "Rr" << Rr << "Pl" << Pl << "Pr" << Pr << "Q" << Q;
		cout << "R=" << R << endl << "T=" << T << endl << "Rl=" << Rl << endl << "Rr" << Rr << endl << "Pl" << Pl << endl << "Pr" << Pr << endl << "Q" << Q << endl;
		fs.release();
	}
	else
	{
		cout << "Error: can not save the extrinsic parameters\n";
	}
}

void ThreeDrecontrution::on_camera2_clicked()
{
	//摄像头的分辨率
	const int imageWidth = 640;
	const int imageHeight = 480;
	//横向的角点数目
	const int boardWidth = 9;
	//纵向的角点数目
	const int boardHeight = 6;
	//总的角点数目
	const int boardCorner = boardWidth * boardHeight;
	//相机标定时需要采用的图像帧数
	const int frameNumber = 14;
	//标定板黑白格子的大小 单位是mm
	const int squareSize = 10;
	//标定板的总内角点
	const Size boardSize = Size(boardWidth, boardHeight);
	Size imageSize = Size(imageWidth, imageHeight);


	//R旋转矢量 T平移矢量 E本征矩阵 F基础矩阵
	vector<Mat> rvecs; //R
	vector<Mat> tvecs; //T
					   //左边摄像机所有照片角点的坐标集合
	vector<vector<Point2f>> imagePointL;
	//右边摄像机所有照片角点的坐标集合
	vector<vector<Point2f>> imagePointR;
	//各图像的角点的实际的物理坐标集合
	vector<vector<Point3f>> objRealPoint;
	//左边摄像机某一照片角点坐标集合
	vector<Point2f> cornerL;
	//右边摄像机某一照片角点坐标集合
	vector<Point2f> cornerR;

	Mat rgbImageL, grayImageL;
	Mat rgbImageR, grayImageR;
	Mat intrinsic;
	Mat distortion_coeff;
	//校正旋转矩阵R，投影矩阵P，重投影矩阵Q
	//映射表
	Mat mapLx, mapLy, mapRx, mapRy;
	Rect validROIL, validROIR;
	//图像校正之后，会对图像进行裁剪，其中，validROI裁剪之后的区域

	Mat img;
	int goodFrameCount = 1;
	while (goodFrameCount <= frameNumber)
	{
		char filename[100];
		/*读取左边的图像*/
		sprintf(filename, "E:\\Myreconstruction\\OpenCVImage-main\\open_image\\camer_cab\\left%02d.jpg", goodFrameCount);
		rgbImageL = imread(filename, 1);
		imshow("chessboardL", rgbImageL);
		cvtColor(rgbImageL, grayImageL, CV_BGR2GRAY);
		/*读取右边的图像*/
		sprintf(filename, "E:\\Myreconstruction\\OpenCVImage-main\\open_image\\camer_cab\\right%02d.jpg", goodFrameCount);
		rgbImageR = imread(filename, 1);
		cvtColor(rgbImageR, grayImageR, CV_BGR2GRAY);

		bool isFindL, isFindR;
		isFindL = findChessboardCorners(rgbImageL, boardSize, cornerL);
		isFindR = findChessboardCorners(rgbImageR, boardSize, cornerR);
		if (isFindL == true && isFindR == true)
		{
			cornerSubPix(grayImageL, cornerL, Size(5, 5), Size(-1, 1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1));
			drawChessboardCorners(rgbImageL, boardSize, cornerL, isFindL);
			imshow("chessboardL", rgbImageL);
			imagePointL.push_back(cornerL);

			cornerSubPix(grayImageR, cornerR, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS | CV_TERMCRIT_ITER, 20, 0.1));
			drawChessboardCorners(rgbImageR, boardSize, cornerR, isFindR);
			imshow("chessboardR", rgbImageR);
			imagePointR.push_back(cornerR);

			//_cprintf("the image %d is good\n",goodFrameCount);
			goodFrameCount++;
		}
		else
		{
			//_cprintf("the image is bad please try again\n");
		}

		if (waitKey(10) == 'q')
		{
			break;
		}
	}

	//计算实际的校正点的三维坐标，根据实际标定格子的大小来设置

	calRealPoint(objRealPoint, boardWidth, boardHeight, frameNumber, squareSize);
	//_cprintf("cal real successful\n");

	//标定摄像头
	double rms = stereoCalibrate(objRealPoint, imagePointL, imagePointR,
		cameraMatrixL, distCoeffL,
		cameraMatrixR, distCoeffR,
		Size(imageWidth, imageHeight), R, T, E, F, CALIB_USE_INTRINSIC_GUESS,
		TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 100, 1e-5));

	//_cprintf("Stereo Calibration done with RMS error = %f\n",rms);

	stereoRectify(cameraMatrixL, distCoeffL, cameraMatrixR, distCoeffR, imageSize, R, T, Rl,
		Rr, Pl, Pr, Q, CALIB_ZERO_DISPARITY, -1, imageSize, &validROIL, &validROIR);

	//摄像机校正映射
	initUndistortRectifyMap(cameraMatrixL, distCoeffL, Rl, Pl, imageSize, CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraMatrixR, distCoeffR, Rr, Pr, imageSize, CV_32FC1, mapRx, mapRy);

	Mat rectifyImageL, rectifyImageR;
	cvtColor(grayImageL, rectifyImageL, CV_GRAY2BGR);
	cvtColor(grayImageR, rectifyImageR, CV_GRAY2BGR);

	imshow("RecitifyL Before", rectifyImageL);
	imshow("RecitifyR Before", rectifyImageR);

	//经过remap之后，左右相机的图像已经共面并且行对准了
	Mat rectifyImageL2, rectifyImageR2;
	remap(rectifyImageL, rectifyImageL2, mapLx, mapLy, INTER_LINEAR);
	remap(rectifyImageR, rectifyImageR2, mapRx, mapRy, INTER_LINEAR);


	imshow("rectifyImageL", rectifyImageL2);
	imshow("rectifyImageR", rectifyImageR2);

	outputCameraParam();
	//显示校正结果
	Mat canvas;
	double sf;
	int w, h;
	sf = 600. / MAX(imageSize.width, imageSize.height);
	w = cvRound(imageSize.width * sf);
	h = cvRound(imageSize.height * sf);
	canvas.create(h, w * 2, CV_8UC3);

	//左图像画到画布上
	Mat canvasPart = canvas(Rect(0, 0, w, h));
	cv::resize(rectifyImageL2, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);
	Rect vroiL(cvRound(validROIL.x*sf), cvRound(validROIL.y*sf),
		cvRound(validROIL.width*sf), cvRound(validROIL.height*sf));
	rectangle(canvasPart, vroiL, Scalar(0, 0, 255), 3, 8);

	//_cprintf("Painted ImageL\n");

	//右图像画到画布上
	canvasPart = canvas(Rect(w, 0, w, h));
	cv::resize(rectifyImageR2, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);
	Rect vroiR(cvRound(validROIR.x*sf), cvRound(validROIR.y*sf),
		cvRound(validROIR.width*sf), cvRound(validROIR.height*sf));
	rectangle(canvasPart, vroiR, Scalar(0, 255, 0), 3, 8);

	//_cprintf("Painted ImageR\n");

	//画上对应的线条
	for (int i = 0; i < canvas.rows; i += 16)
		line(canvas, Point(0, i), Point(canvas.cols, i), Scalar(0, 255, 0), 1, 8);

	imshow("rectified", canvas);
	//_cprintf("wait key\n");
	waitKey(0);

	cv::destroyAllWindows();
	waitKey(1);

}

int getDisparityImage(cv::Mat& disparity, cv::Mat& disparityImage, bool isColor)
{
	cv::Mat disp8u;
	disp8u = disparity;
	// 转换为伪彩色图像 或 灰度图像
	if (isColor)
	{
		if (disparityImage.empty() || disparityImage.type() != CV_8UC3 || disparityImage.size() != disparity.size())
		{
			disparityImage = cv::Mat::zeros(disparity.rows, disparity.cols, CV_8UC3);
		}
		for (int y = 0; y < disparity.rows; y++)
		{
			for (int x = 0; x < disparity.cols; x++)
			{
				uchar val = disp8u.at<uchar>(y, x);
				uchar r, g, b;

				if (val == 0)
					r = g = b = 0;
				else
				{
					r = 255 - val;
					g = val < 128 ? val * 2 : (uchar)((255 - val) * 2);
					b = val;
				}
				disparityImage.at<cv::Vec3b>(y, x) = cv::Vec3b(b, g, r);
			}
		}
	}
	else
	{
		disp8u.copyTo(disparityImage);
	}
	return 1;
}
const int imageWidth = 640;                             //摄像头的分辨率
const int imageHeight = 480;
Size imageSize = Size(imageWidth, imageHeight);

Mat rgbImageL, grayImageL;
Mat rgbImageR, grayImageR;
Mat rectifyImageL, rectifyImageR;

Rect validROIL;//图像校正之后，会对图像进行裁剪，这里的validROI就是指裁剪之后的区域
Rect validROIR;

Mat xyz;              //三维坐标
int blockSize = 0, uniquenessRatio = 0, numDisparities = 0;
Ptr<StereoBM> bm = StereoBM::create(16, 9);

Mat T_new = (Mat_<double>(3, 1) << -3.3269653179960471e+01, 3.7375231026230421e-01, -1.2058042444883227e-02);//T平移向量
//Mat rec = (Mat_<double>(3, 1) << -0.00306, -0.03207, 0.00206);//rec旋转向量
Mat R_new = (Mat_<double>(3, 3) << 9.9998505024526163e-01, 3.5253250461816949e-03,
	4.1798767087380161e-03, -3.4957471578341281e-03,
	9.9996894942320580e-01, -7.0625732745616225e-03,
	-4.2046447876106169e-03, 7.0478558986986593e-03,
	9.9996632377767658e-01);//R 旋转矩阵

/*****立体匹配*****/
void stereo_match(int, void*)
{
	bm->setBlockSize(2 * blockSize + 5);     //SAD窗口大小，5~21之间为宜
	bm->setROI1(validROIL);
	bm->setROI2(validROIR);
	bm->setPreFilterCap(31);
	bm->setMinDisparity(0);  //最小视差，默认值为0, 可以是负值，int型
	bm->setNumDisparities(numDisparities * 16 + 16);//视差窗口，即最大视差值与最小视差值之差,窗口大小必须是16的整数倍，int型
	bm->setTextureThreshold(10);
	bm->setUniquenessRatio(uniquenessRatio);//uniquenessRatio主要可以防止误匹配
	bm->setSpeckleWindowSize(100);
	bm->setSpeckleRange(32);
	bm->setDisp12MaxDiff(-1);
	Mat disp, disp8, disparityImage;
	bm->compute(rectifyImageL, rectifyImageR, disp);//输入图像必须为灰度图
	disp.convertTo(disp8, CV_8U, 255 / ((numDisparities * 16 + 16)*16.));//计算出的视差是CV_16S格式
	reprojectImageTo3D(disp, xyz, Q, true); //在实际求距离时，ReprojectTo3D出来的X / W, Y / W, Z / W都要乘以16(也就是W除以16)，才能得到正确的三维坐标信息。
	xyz = xyz * 16;
	getDisparityImage(disp8, disparityImage, true);
	imshow("disparity", disparityImage);
}

//立体匹配

void ThreeDrecontrution::on_camera2_2_clicked()
{
	// TODO: 在此添加控件通知处理程序代码
	//立体校正
	stereoRectify(cameraMatrixL, distCoeffL, cameraMatrixR, distCoeffR, imageSize, R_new, T_new, Rl, Rr, Pl, Pr, Q, CALIB_ZERO_DISPARITY,
		0, imageSize, &validROIL, &validROIR);
	initUndistortRectifyMap(cameraMatrixL, distCoeffL, Rl, Pr, imageSize, CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraMatrixR, distCoeffR, Rr, Pr, imageSize, CV_32FC1, mapRx, mapRy);

	rgbImageL = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\camer_cab\\left01.jpg", CV_LOAD_IMAGE_COLOR);
	cvtColor(rgbImageL, grayImageL, CV_BGR2GRAY);
	rgbImageR = imread("E:\\Myreconstruction\\OpenCVImage-main\\open_image\\camer_cab\\right01.jpg", CV_LOAD_IMAGE_COLOR);
	cvtColor(rgbImageR, grayImageR, CV_BGR2GRAY);

	imshow("ImageL Before Rectify", grayImageL);
	imshow("ImageR Before Rectify", grayImageR);

	/*
	经过remap之后，左右相机的图像已经共面并且行对准了
	*/
	remap(grayImageL, rectifyImageL, mapLx, mapLy, INTER_LINEAR);
	remap(grayImageR, rectifyImageR, mapRx, mapRy, INTER_LINEAR);

	/*
	把校正结果显示出来
	*/
	Mat rgbRectifyImageL, rgbRectifyImageR;
	cvtColor(rectifyImageL, rgbRectifyImageL, CV_GRAY2BGR);  //伪彩色图
	cvtColor(rectifyImageR, rgbRectifyImageR, CV_GRAY2BGR);
	//单独显示
	//rectangle(rgbRectifyImageL, validROIL, Scalar(0, 0, 255), 3, 8);
	//rectangle(rgbRectifyImageR, validROIR, Scalar(0, 0, 255), 3, 8);
	imshow("ImageL After Rectify", rgbRectifyImageL);
	imshow("ImageR After Rectify", rgbRectifyImageR);

	//显示在同一张图上
	Mat canvas;
	double sf;
	int w, h;
	sf = 600. / MAX(imageSize.width, imageSize.height);
	w = cvRound(imageSize.width * sf);
	h = cvRound(imageSize.height * sf);
	canvas.create(h, w * 2, CV_8UC3);   //注意通道

										//左图像画到画布上
	Mat canvasPart = canvas(Rect(w * 0, 0, w, h));                                //得到画布的一部分
	cv::resize(rgbRectifyImageL, canvasPart, canvasPart.size(), 0, 0, INTER_AREA);     //把图像缩放到跟canvasPart一样大小
	Rect vroiL(cvRound(validROIL.x*sf), cvRound(validROIL.y*sf),                //获得被截取的区域
		cvRound(validROIL.width*sf), cvRound(validROIL.height*sf));
	//rectangle(canvasPart, vroiL, Scalar(0, 0, 255), 3, 8);                      //画上一个矩形
	cout << "Painted ImageL" << endl;

	//右图像画到画布上
	canvasPart = canvas(Rect(w, 0, w, h));                                      //获得画布的另一部分
	cv::resize(rgbRectifyImageR, canvasPart, canvasPart.size(), 0, 0, INTER_LINEAR);
	Rect vroiR(cvRound(validROIR.x * sf), cvRound(validROIR.y*sf),
		cvRound(validROIR.width * sf), cvRound(validROIR.height * sf));
	//rectangle(canvasPart, vroiR, Scalar(0, 0, 255), 3, 8);
	cout << "Painted ImageR" << endl;

	//画上对应的线条
	for (int i = 0; i < canvas.rows; i += 16)
		line(canvas, Point(0, i), Point(canvas.cols, i), Scalar(0, 255, 0), 1, 8);
	imshow("rectified", canvas);

	/*
	立体匹配
	*/
	namedWindow("disparity", CV_WINDOW_AUTOSIZE);
	// 创建SAD窗口 Trackbar
	createTrackbar("BlockSize:\n", "disparity", &blockSize, 8, stereo_match);
	// 创建视差唯一性百分比窗口 Trackbar
	createTrackbar("UniquenessRatio:\n", "disparity", &uniquenessRatio, 50, stereo_match);
	// 创建视差窗口 Trackbar
	createTrackbar("NumDisparities:\n", "disparity", &numDisparities, 16, stereo_match);
	stereo_match(0, 0);

	waitKey(0);

	cv::destroyAllWindows();
	waitKey(1);

}
