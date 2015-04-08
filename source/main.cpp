#include "basic.h"
#include "function.h"

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cout << "Usage: " << endl;
		cout << "ACCV2009.exe Folder imfn1 imfn2" << endl;
		return -1;
	}

	string folder = argv[1];
	string imfn1  = argv[2];
	string imfn2  = argv[3];
	string outfolder = "../output";
	_mkdir(outfolder.c_str());

	Mat im1 = imread(folder + imfn1 + ".png", 1);
	Mat im2 = imread(folder + imfn2 + ".png", 1);

	int width = im1.cols;
	int height = im1.rows;

	Mat im1_H, im1_S, im1_I;
	Mat im2_H, im2_S, im2_I;
	CvtColorBGR2HSI(im1, im1_H, im1_S, im1_I);
	CvtColorBGR2HSI(im2, im2_H, im2_S, im2_I);
	cout << endl;
	
	int  sigmaS = 5;
	float sigmaR = 10;
	int minR = 800;

	vector<int> labels1(0), labels2(0);

//#define  RQ_DEBUG
#ifdef   RQ_DEBUG
	int regionum1, regionum2;
	DoMeanShift(im1, sigmaS, sigmaR, minR, labels1, regionum1);
	DoMeanShift(im2, sigmaS, sigmaR, minR, labels2, regionum2);
	cout << endl;

	saveLabels(labels1, width, height, outfolder+"/labels_" + imfn1 + ".txt");
	saveLabels(labels2, width, height, outfolder+"/labels_" + imfn2 + ".txt");
#else
	int regionum1, regionum2;
	readLabels(outfolder+"/labels_" + imfn1 + ".txt", width, height, labels1, regionum1);
	readLabels(outfolder+"/labels_" + imfn2 + ".txt", width, height, labels2, regionum2);	
#endif
	
	string siftmatchfn = folder + "matches_sift.txt";
	vector<Point2f> features1(0), features2(0);
	ReadSiftMatches(siftmatchfn, features1, features2);

	vector<vector<Point2f>> matches1(0), matches2(0);
	ComputeRegionMatches(labels2, regionum2, width, features1, features2, matches1, matches2);
	
	vector<float> DeltaH(0), DeltaS(0), DeltaI(0);
	RegionDeltaColor(im1_H, im2_H, matches1, matches2, DeltaH);
	RegionDeltaColor(im1_S, im2_S, matches1, matches2, DeltaS);
	RegionDeltaColor(im1_I, im2_I, matches1, matches2, DeltaI);
	
	Mat new_im2_H, new_im2_S, new_im2_I;
	CorrectColor(im2_H, labels2, DeltaH, new_im2_H);
	CorrectColor(im2_S, labels2, DeltaS, new_im2_S);
	CorrectColor(im2_I, labels2, DeltaI, new_im2_I);

	Mat new_im2;
	CvtColorHSI2BGR(new_im2_H, new_im2_S, new_im2_I, new_im2);

	cout << "done." << endl;

	imshow("new im2", new_im2);
	waitKey(0);
	destroyAllWindows();

	string savefn = outfolder + "/accv2009_" + imfn2 + ".png";
	cout << "save " << savefn << endl;
	imwrite(savefn, new_im2);

}