#include "common.h"
#include "basic.h"

void saveLabels(vector<int> labels, int width, int height, string fn);
void readLabels(string fn, int& width, int& height, vector<int>& labels, int& regionum);

void DoMeanShift(Mat src, int sigmaS, float sigmaR, int minR, vector<int>& labels, int& regionum);

void ReadSiftMatches(string sfmfn, vector<Point2f>& features1, vector<Point2f>& features2 );

void ComputeRegionMatches(vector<int> labels2, int regionum2, int step, vector<Point2f> features1, vector<Point2f> features2, vector<vector<Point2f>>& matches1, vector<vector<Point2f>>& matches2);

void RegionDeltaColor(Mat src, Mat dst, vector<vector<Point2f>>& src_matches, vector<vector<Point2f>>& dst_matches, vector<float>& out_deltas);

void CorrectColor(Mat dst, vector<int> labels, vector<float> deltas, Mat& newdst );

void maskFromPixels(vector<Point2f> validpixels, int h, int w, Mat& out_mask);