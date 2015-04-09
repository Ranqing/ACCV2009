#include "function.h"

void maskFromPixels(vector<Point2f> validpixels, int h, int w, Mat& out_mask)
{
	out_mask = Mat::zeros(h, w, CV_8UC1);
	for (int i = 0; i < validpixels.size(); ++i)
	{
		int x = validpixels[i].x;
		int y = validpixels[i].y;

		if (x>=w || x<0 || y>=h || y<0)
		{
			continue;
		}

		out_mask.at<uchar>(y,x) = 255;
	}

	//test
	/*imshow("mask from pixels", out_mask);
	waitKey(0);
	destroyWindow("mask from pixels");*/
}

void ComputeRegionMatches(vector<int> labels2, int regionum2, int step, vector<Point2f> features1, vector<Point2f> features2, vector<vector<Point2f>>& matches1, vector<vector<Point2f>>& matches2)
{
	cout << "Here compute how many matches in each region." << endl;

	matches1.resize(regionum2);
	matches2.resize(regionum2);

	// matched region
	for (int i = 0; i < features2.size(); ++i)
	{
		Point2f pt2 = features2[i];
		Point2f pt1 = features1[i];

		int idx  = labels2[pt2.y * step + pt2.x];
		matches1[idx].push_back(pt1);
		matches2[idx].push_back(pt2);
	}

	// unmatched region:

	// 构建特征点的KDTree
	Mat tmpmat(features2.size(), 2, CV_32FC1);
	for (int i = 0; i < features2.size(); ++i)
	{
		float * data = tmpmat.ptr<float>(i);
		data[0] = features2[i].x;
		data[1] = features2[i].y;
	}

	cv::flann::Index flann_Idx(tmpmat, cv::flann::KDTreeIndexParams(4),cvflann::FLANN_DIST_EUCLIDEAN);
	flann_Idx.save("index.fln");	
	cout << "flann based KDTree have been done." << endl;	
	
	//人为添加五个点
	
	vector<vector<Point2i>> pixelTable2(regionum2);
	for (int i = 0; i < labels2.size(); ++i)
	{
		Point2i pxl = Point2i(i%step, i/step);
		int idx = labels2[i];
		pixelTable2[idx].push_back(pxl);
	}
	//cout << "pixel table done." << endl ;

	for (int i = 0; i < regionum2; ++i)
	{
		if (matches2[i].size() != 0)
			continue;

		cout << i <<  endl;
	
		int xmax = 0, ymax = 0;
		int xmin = NMAX, ymin = NMAX;

		for (int j = 0; j < pixelTable2[i].size(); ++j)
		{
			Point2i pt = pixelTable2[i][j];

			if (pt.x > xmax)				xmax = pt.x;
			if (pt.x < xmin)				xmin = pt.x;
			if (pt.y > ymax)				ymax = pt.y;
			if (pt.y < ymin)				ymin = pt.y;
		}

		int xmid = (xmax + xmin)/2;
		int ymid = (ymax + ymin)/2;

		Point2i dstM0 = Point2i(xmid, ymid);									//M0 = xmid, ymid;
		Point2i dstM1 = Point2i(xmid, (int)(ymin*2/3.0 + ymax/3.0) );			//M1 = xmid, 2/3 * ymin + 1/3 * ymax
		Point2i dstM2 = Point2i(xmid, (int)(ymin/3.0 + ymax*2/3.0) );			//M2 = xmid, 1/3 * ymin + 2/3 * ymax
		Point2i dstM3 = Point2i((int)( xmin*2/3.0 + xmax/3.0), ymid);			//M3 = 2/3 * xmin + 1/3 * xmax, ymid
		Point2i dstM4 = Point2i((int)( xmin/3.0 + xmax*2.0/3.0), ymid);			//M4 = 1/3 * xmin + 2/3 * xmax, ymid
		
		//找到最近的sift feature
		Mat pt(1,2,CV_32FC1);
		pt.ptr<float>(0)[0] = dstM0.x;
		pt.ptr<float>(0)[1] = dstM0.y;

		Mat idx, dist;
		flann_Idx.knnSearch(pt, idx, dist, 1, cv::flann::SearchParams());
		
		//nearest index and sift feature
		int nIdx = idx.at<int>(0,0);
		Point2i nPt2 = features2[nIdx];
		Point2i nPt1 = features1[nIdx];

		int deltaY = nPt1.y - nPt2.y;
		int deltaX = nPt1.x - nPt2.x;

		Point2i srcM0 = Point2i(dstM0.x + deltaX, dstM0.y + deltaY );
		Point2i srcM1 = Point2i(dstM1.x + deltaX, dstM1.y + deltaY);
		Point2i srcM2 = Point2i(dstM2.x + deltaX, dstM2.y + deltaY);
		Point2i srcM3 = Point2i(dstM3.x + deltaX, dstM3.y + deltaY);
		Point2i srcM4 = Point2i(dstM4.x + deltaX, dstM4.y + deltaY);

		matches2[i].push_back(dstM0);		matches1[i].push_back(srcM0);   
		matches2[i].push_back(dstM1);		matches1[i].push_back(srcM1);
		matches2[i].push_back(dstM2);		matches1[i].push_back(srcM2);
		matches2[i].push_back(dstM3);		matches1[i].push_back(srcM3);
		matches2[i].push_back(dstM4);		matches1[i].push_back(srcM4);
	}
}

void RegionDeltaColor(Mat src, Mat dst, vector<vector<Point2f>>& src_matches, vector<vector<Point2f>>& dst_matches, vector<float>& out_deltas)
{
	cout << "Here is computation of each region delta H/S/I." << endl;

	Mat mean_src, mean_dst;
	blur(src, mean_src, cv::Size(3,3));
	blur(dst, mean_dst, cv::Size(3,3));

	int regionum = dst_matches.size();
	int global_cnt = 0;
	float global_delta = 0;

	int width = src.cols;
	int height = src.rows;
	
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

			if (dstpt.x < 0 || dstpt.x >= width || dstpt.y < 0 || dstpt.y >= height ||
				srcpt.x < 0 || srcpt.x >= width || srcpt.y < 0 || srcpt.y >= height)
				continue;

			delta += mean_src.at<float>(srcpt.y, srcpt.x) - mean_dst.at<float>(dstpt.y, dstpt.x);	 
		}
		
		global_delta += delta;
		global_cnt   += cnt;

		if (cnt != 0)
			delta = delta / cnt;

		out_deltas.push_back(delta);
	}
	
	/*global_delta = global_delta / global_cnt;
	for (int i = 0; i < regionum; ++i)
	{
		if (dst_matches[i].size() == 0)
		{
			out_deltas[i] = global_delta ;
		}
	}*/

	

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