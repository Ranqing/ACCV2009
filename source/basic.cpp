#include "basic.h"

float idot(const cv::Vec3f a, const cv::Vec3f b)
{
	float innerp = a.val[0] * b.val[0] + a.val[1] * b.val[1] + a.val[2] * b.val[2];	
	return innerp;
}

float idot(const cv::Vec4f a, const cv::Vec4f b)
{
	float innerp = a.val[0] * b.val[0] + a.val[1] * b.val[1] + a.val[2] * b.val[2] + a.val[3] * b.val[3];
	return innerp;
}

void mul(const cv::Mat mtx, const cv::Vec4f a, cv::Vec4f& ret)
{
	if (mtx.cols != 4)
	{
		std::cerr << "can not multiply the mtx and the vector. mtx rows != 4" << std::endl;
		exit(1);
	}

	int rows = mtx.rows;
	int cols = mtx.cols;

	for (int i = 0; i < rows; ++ i)
	{
		cv::Vec4f tmpvec; 
		for(int j = 0; j < cols; ++ j)
			tmpvec.val[j] = mtx.at<float>(i, j);			

		ret.val[i] = idot(tmpvec, a);
	}
}

Scalar randomColor(RNG& rng)
{
	int icolor = (unsigned) rng;
	return Scalar(icolor&255, (icolor>>8)&255, (icolor>>16)&255);
}

//BGR->HSI
void CvtColorBGR2HSI(Mat RGBim, Mat& HSI_Him, Mat& HSI_Sim, Mat& HSI_Iim)
{
	cout << "here is convert BGR to HSI." << endl; 

	int width = RGBim.cols;
	int height = RGBim.rows;

	vector<Mat> mv(3);
	split(RGBim, mv);

	Mat Bim = mv[0].clone();
	Mat Gim = mv[1].clone();
	Mat Rim = mv[2].clone();

	HSI_Him.create(height, width, CV_32FC1);
	HSI_Sim.create(height, width, CV_32FC1);
	HSI_Iim.create(height, width, CV_32FC1);

	//计算H,S,I分量
	for(int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float blue = Bim.at<uchar>(y,x);
			float green = Gim.at<uchar>(y,x);
			float red = Rim.at<uchar>(y,x);

			float hsi_h, hsi_s, hsi_i;

			//h: [0~360]
			float theta ;
			float fenzi  = (red-green + red-blue)/2;  //numerator
			float fenmu2 = (red-green)*(red-green) + (red-blue)*(green-blue);
			float fenmu  = sqrt( (red-green)*(red-green) + (red-blue)*(green-blue) ); //dominator

			if (fenmu != 0)
			{
				theta = acos(fenzi / fenmu) * 180 / _PI;

				if (blue <= green)
				{
					hsi_h = theta;
				}
				else
				{
					hsi_h = 360 - theta;
				}
			}
			else
				hsi_h = 0;

			//s: [0~1]
			hsi_s = 1 - 3 * min(min(red,green),blue)/(red+green+blue);

			//i: [0~1]
			hsi_i = ((red+green+blue)/3)/255.0f;

			HSI_Him.at<float>(y,x) = hsi_h;
			HSI_Sim.at<float>(y,x) = hsi_s;
			HSI_Iim.at<float>(y,x) = hsi_i;			
		}
	}

	//对H,S,I进行显示
	/*Mat showHSIim(height, width, CV_8UC3);  
	for (int y = 0; y < height; ++y)
	{
	for (int x = 0; x < width; ++x)
	{
	int ihsi_h  = (int)(HSI_Him.at<float>(y,x) * 255 / 360);
	int ihsi_s  = (int)(HSI_Sim.at<float>(y,x) * 255);
	int ihsi_i  = (int)(HSI_Iim.at<float>(y,x) * 255);

	showHSIim.at<Vec3b>(y,x)[0] = ihsi_h;
	showHSIim.at<Vec3b>(y,x)[1] = ihsi_s;
	showHSIim.at<Vec3b>(y,x)[2] = ihsi_i;
	}
	}
	imshow("showhsi", showHSIim);
	waitKey(0);
	destroyAllWindows();*/
}

//HSI->BGR
void CvtColorHSI2BGR(Mat HSI_Him, Mat HSI_Sim, Mat HSI_Iim, Mat& RGBim )
{
	cout << "here is convert HSI to BGR." << endl;

	int width  = HSI_Him.cols;
	int height = HSI_Him.rows;

	Mat Bim (height, width, CV_8UC1);
	Mat Gim (height, width, CV_8UC1);
	Mat Rim (height, width, CV_8UC1);
	
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			double hsi_h = HSI_Him.at<float>(y,x);
			double hsi_s = HSI_Sim.at<float>(y,x);
			double hsi_i = HSI_Iim.at<float>(y,x);

			//转换公式参考网上
			//double blue = 0, green = 0, red = 0;
			double blue , green, red;
			if ( hsi_h < 120 && hsi_h >= 0)
			{
				hsi_h = hsi_h * _PI / 180;    //弧度表示
				
				blue = hsi_i * ( 1 - hsi_s );
				red  = hsi_i * ( 1 + (hsi_s * cos(hsi_h)) / cos(_PI/3 - hsi_h) );
				green = 3 * hsi_i - (blue + red);
			}
			else if ( hsi_h < 240 && hsi_h >= 120)
			{
				hsi_h = (hsi_h - 120) * _PI / 180;

				red   = hsi_i * (1- hsi_s);
				green = hsi_i * (1 + (hsi_s * cos(hsi_h)) / cos(_PI/3 - hsi_h) );
				blue  = 3 * hsi_i - (red + blue); 
			}
			else 
			{
				hsi_h = (hsi_h - 240) * _PI / 180;

				green = hsi_i * (1-hsi_s);
				blue  = hsi_i * (1 + (hsi_s * cos(hsi_h)) / cos(_PI/3 - hsi_h) );
				red   = 3 * hsi_i - (green + blue);
			}

			int iblue  = blue * 255;
			int igreen = green * 255;
			int ired   = red * 255;

			Bim.at<uchar>(y,x) = iblue;
			Gim.at<uchar>(y,x) = igreen;
			Rim.at<uchar>(y,x) = ired;
		}
	}

	vector<Mat> mv(3);
	Bim.copyTo(mv[0]);
	Gim.copyTo(mv[1]);
	Rim.copyTo(mv[2]);
	merge(mv, RGBim);

	/*imshow("rgb", RGBim);
	waitKey(0);
	destroyAllWindows();*/
}
