#include "function.h"
#include "ms.h"
#include "msImageProcessor.h"

void DoMeanShift(Mat src, int sigmaS, float sigmaR, int minR, vector<int>& labels)
{
	cout << "Here is Do Mean-shift Segmentation." << endl;
	cout << "sigmaS = " << sigmaS << endl;
	cout << "sigmaR = " << sigmaR << endl;
	cout << "minR = " << minR << endl;

	msImageProcessor proc;
	proc.DefineImage(src.data, (src.channels() == 3 ? COLOR : GRAYSCALE), src.rows, src.cols);
	proc.Segment(sigmaS,sigmaR, minR, MED_SPEEDUP);//HIGH_SPEEDUP, MED_SPEEDUP, NO_SPEEDUP; high: set speedupThreshold, otherwise the algorithm uses it uninitialized!

	int regioncnt = proc.GetRegionsCnt();

	//标签结果
	Mat labels_dst = cv::Mat(src.size(), CV_32SC1);
	proc.GetRegionsLabels(labels_dst.data);

	//meanshift分割图像
	Mat segment_dst = cv::Mat(src.size(), CV_8UC3);
	proc.GetResults(segment_dst.data);

	imshow("mean-shift", segment_dst);
	waitKey(0);
	destroyAllWindows();

	int w = src.cols;
	int h = src.rows;
	labels.resize(w*h);
	for (int y = 0; y < h; ++ y)
		for (int x = 0; x < w; ++x)
			labels[y*w+x] = labels_dst.at<int>(y,x);
	
}