#pragma once

#include <QtWidgets/QWidget>

#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息
#include <QTimer>
#include <QFileDialog>
#include <QPushButton>
#include <QImage>
#include <QString>
#include <QMessageBox>
#include <QDebug>

#include "ui_ThreeDrecontrution.h"

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/highgui.hpp>
#include <opencv2/optflow/motempl.hpp>
#include <opencv2/xfeatures2d.hpp>
#include "opencv2/imgcodecs/legacy/constants_c.h"
#include <opencv2/calib3d/calib3d.hpp>
#include "opencv2/calib3d.hpp"
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <fstream>
#include <iostream>

#include <vector>
#include <limits>
#include <qmath.h>
#include <Windows.h>
#include <opencv2/videoio/videoio.hpp>

using namespace std;
using namespace cv;
using namespace cv::dnn;


class ThreeDrecontrution : public QMainWindow
{
	Q_OBJECT

public:
	ThreeDrecontrution(QWidget *parent = Q_NULLPTR);

	QImage  MatToQImage2(const cv::Mat& mat);

	void ShowImage(cv::Mat& mat);

private:
	Ui::ThreeDrecontrutionClass ui;

	VideoCapture video;
	QTimer *timer;

	int fps;
	QString videofileName;
	QString figfileName;
	QString output_path;
	std::string sfileName;

	QSerialPort serial;

	Mat srcImg, grayImg, noiseImg;
	//槽函数
private slots:
	//以下为视觉相关，包括轮廓提取，视频播放，追踪算法以及文件处理等
	void Startdetect();

	void KCF_detect();

	void on_pushButton_open_clicked();

	void on_pushButton_play_clicked();

	void ReadFrame();

	void on_pushButton_pause_clicked();

	void show_edge_fig();

	void EdgeDetect_pic();

	void EdgeDetect_video();

	void txt2csv();

	// 以下为串口调试相关与机器人控制相关函数
	void on_pushButton_controlclean_clicked();

	void on_pushButton_hand_x_up_clicked();

	void on_pushButton_hand_x_down_clicked();

	void serialPort_readyRead();

	void on_searchButton_clicked();

	void on_openButton_clicked();

	void on_sendButton_clicked();

	void on_clearButton_clicked();

	// 以下为图片预处理相关函数
	void on_select_files_clicked();

	void on_pushButton_clicked();

	void on_gray_leval_clicked();

	void on_gray_balance_clicked();

	void on_grad_sharpen_clicked();

	void on_laplace_sharpen_clicked();

	void on_roberts_edge_clicked();

	void on_sobel_edge_clicked();

	void on_prewitt_clicked();

	void on_laplace_edge_clicked();

	void on_salt_noise_clicked();

	void on_guass_noise_clicked();

	void on_krisch_edge_clicked();

	void on_Canny_clicked();

	void on_average_filter_clicked();

	void on_middle_filter_clicked();

	void on_window_filter_clicked();

	void on_gauss_filter_clicked();

	void on_form_filter_clicked();

	void on_affine_clicked();

	void on_perspective_clicked();

	void on_threshold_seg_clicked();

	void on_OSTU_clicked();

	void on_Kittler_clicked();

	void on_frame_diff_clicked();

	void on_mix_guass_clicked();

	void on_circle_lbp_clicked();

	void on_target_det_clicked();

	void on_model_check_clicked();

	void on_cloaking_clicked();

	void on_SIFT_clicked();

	void on_orb_clicked();

	void on_color_fit_clicked();

	void on_svm_test_clicked();

	void on_word_test_clicked();

	void on_Haar_1_clicked();

	void on_Haar_2_clicked();

	void on_gaber_clicked();

	void on_face_haar_clicked();

	void on_camera2_clicked();

	void on_camera2_2_clicked();
};
