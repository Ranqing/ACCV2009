#include "function.h"

void RegionDeltaColor(Mat src, Mat dst, vector<vector<Point2f>>& src_matches, vector<vector<Point2f>>& dst_matches, vector<float>& out_deltas)
{
	cout << "Here is computation of each region delta H/S/I." << endl;

	Mat mean_src, mean_dst;
	blur(src, mean_src, cv::Size(3,3));
	blur(dst, mean_dst, cv::Size(3,3));

	int regionum = dst_matches.size();
	int global_cnt = 0;
	float global_delta = 0;
	
	for (int i = 0; i < regionum; ++ i)
	{
		int cnt = dst_matches[i].size();

		if (cnt == 0)
		{
			out_deltas.push_back(0);
			continue;
		}
		
		//计算当前区域的色彩差
		float delta = 0;
		for (int j = 0; j < cnt; ++ j)
		{
			Point2f dstpt = dst_matches[i][j];
			Point2f srcpt = src_matches[i][j];

			delta += mean_src.at<float>(srcpt.y, srcpt.x) - mean_dst.at<float>(dstpt.y, dstpt.x);	 
		}
		
		global_delta += delta;
		global_cnt   += cnt;

		delta = delta / cnt;

		out_deltas.push_back(delta);
	}
	
	global_delta /= global_cnt;

	for (int i = 0; i < regionum; ++i)
	{
		if (dst_matches[i].size() == 0)
		{
			out_deltas[i] = global_delta ;
		}
	}

}

void CorrectColor(Mat dst, vector<int> labels, vector<float> deltas, Mat& newdst )
{
	cout << "Here is color correction of ACCV2009" << endl;

	newdst = dst.clone();
	
	int height = newdst.rows;
	int width  = newdst.cols;
	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float oldc = dst.at<float>(y,x);
			int  idx = labels[y*width + x];
			float dta = deltas[idx];
			float newc = oldc + dta;

			newdst.at<float>(y,x) = newc;
		}
	}
}