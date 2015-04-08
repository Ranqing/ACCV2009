#include "function.h"

void saveLabels(vector<int> labels, int width, int height, string fn)
{
	fstream fout(fn.c_str(), ios::out);
	if (fout.is_open() == false)
	{
		cout << "failed to save " << fn << endl;
		return ;
	}

	fout << width << ' ' << height << endl;
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			fout << labels[j*width+i] << ' ';
		}
		fout << endl;
	}
	fout.close();
}

void readLabels(string fn, int& width, int& height, vector<int>& labels, int& regionum)
{
	fstream fin (fn, ios::in);
	if (fin.is_open() == false)
	{
		cout << "failed to read " << fn << endl;
		return ;
	}

	regionum = 0;

	fin >> width >> height;
	labels.resize(width*height);
	for (int i = 0; i < width*height; ++i)
	{
		fin >> labels[i];
		if (labels[i] > regionum)
			regionum = labels[i];
	}
	regionum = regionum + 1;   //计数从0开始

	fin.close();
}

void ReadSiftMatches(string sfmfn, vector<Point2f>& features1, vector<Point2f>& features2 )
{
	cout << "Here read sift matches." << endl;

	fstream fin(sfmfn.c_str(), std::ios::in);
	if (fin.is_open() == false)
	{
		cout << "failed to open " << sfmfn << std::endl;
		return ;
	}

	vector<Point2f> temp1(0), temp2(0);

	int matchcnt;
	float x1, y1, x2, y2;

	fin >> matchcnt;
	for (int i = 0; i < matchcnt; ++ i )
	{
		fin >> x1 >> y1 >> x2 >> y2;

		Point2f pt1(x1, y1);
		Point2f pt2(x2, y2);

		temp1.push_back(pt1);
		temp2.push_back(pt2);
	}
	fin.close();
	
	cout << "read matches done. " << matchcnt << " matches." << endl;

	/************************************************************************/
	/* 1. 去掉Y值不等的匹配
	/* 2. 去掉重复的匹配
	/************************************************************************/

	bool times[NMAX][NMAX];
	memset(times, 0, NMAX * NMAX* sizeof(bool));

	for (int i = 0; i < matchcnt; ++i )
	{
		int sx = temp1[i].x;
		int sy = temp1[i].y;
		int dx = temp2[i].x;
		int dy = temp2[i].y;

		if (abs(sy - dy) > 5)
			continue;

		if (times[sx][sy] > 0)   //不会出现一对多的情况：其实难以保证
			continue;

		features1.push_back(Point2f(sx, sy));
		features2.push_back(Point2f(dx, dy));
		times[sx][sy] = 1;
	}

	matchcnt = features1.size();
	cout << "after delete duplicate matches. " << matchcnt << endl;	
}

void ComputeRegionMatches(vector<int> labels2, int regionum2, int step, vector<Point2f> features1, vector<Point2f> features2, vector<vector<Point2f>>& matches1, vector<vector<Point2f>>& matches2)
{
	cout << "Here compute how many matches in each region." << endl;

	matches1.resize(regionum2);
	matches2.resize(regionum2);

	for (int i = 0; i < features2.size(); ++i)
	{
		Point2f pt2 = features2[i];
		Point2f pt1 = features1[i];
		
		int idx  = labels2[pt2.y * step + pt2.x];
		matches1[idx].push_back(pt1);
		matches2[idx].push_back(pt2);
	}
}