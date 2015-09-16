
#include <iostream>
#include <stdlib.h>
#include "ToolsTrackingApp.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;



char window_name[30] = "HSV Segemtation";
Mat src;

static void onMouse( int event, int x, int y, int f, void* ){
	Mat image=src.clone();
	Vec3b rgb=image.at<Vec3b>(y,x);
	int B=rgb.val[0];
	int G=rgb.val[1];
	int R=rgb.val[2];

	Mat HSV;
	Mat RGB=image(Rect(x,y,1,1));
	cvtColor(RGB, HSV,CV_BGR2HSV);

	Vec3b hsv=HSV.at<Vec3b>(0,0);
	int H=hsv.val[0];
	int S=hsv.val[1];
	int V=hsv.val[2];

	char name[30];
	sprintf(name,"B=%d",B);
	putText(image,name, Point(150,40) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,255,0), 2,8,false );

	sprintf(name,"G=%d",G);
	putText(image,name, Point(150,80) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,255,0), 2,8,false );

	sprintf(name,"R=%d",R);
	putText(image,name, Point(150,120) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,255,0), 2,8,false );

	sprintf(name,"H=%d",H);
	putText(image,name, Point(25,40) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,255,0), 2,8,false );

	sprintf(name,"S=%d",S);
	putText(image,name, Point(25,80) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,255,0), 2,8,false );

	sprintf(name,"V=%d",V);
	putText(image,name, Point(25,120) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,255,0), 2,8,false );

	sprintf(name,"X=%d",x);
	putText(image,name, Point(25,300) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,0,255), 2,8,false );

	sprintf(name,"Y=%d",y);
	putText(image,name, Point(25,340) , FONT_HERSHEY_SIMPLEX, .7, Scalar(0,0,255), 2,8,false );

	//imwrite("hsv.jpg",image);
	imshow( window_name, image );
}


void calculateHistogram(Mat image, Mat channel[3])
{
	vector<Mat> bgr_planes;
	split( image, bgr_planes );

	/// Establish the number of bins
	int histSize = 256;

	/// Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 } ;
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat b_hist, g_hist, r_hist;

	/// Compute the histograms:
	calcHist( &bgr_planes[0], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );
	calcHist( &bgr_planes[2], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );

	// Draw the histograms for B, G and R
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound( (double) hist_w/histSize );

	Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );

	/// Normalize the result to [ 0, histImage.rows ]
	normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
	normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );

	/// Draw for each channel
	for( int i = 1; i < histSize; i++ )
	{
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
			Scalar( 255, 0, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
			Scalar( 0, 255, 0), 2, 8, 0  );
		line( histImage, Point( bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1)) ) ,
			Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
			Scalar( 0, 0, 255), 2, 8, 0  );
	}

	/// Display
	namedWindow("calcHist Demo", CV_WINDOW_AUTOSIZE );
	imshow("calcHist Demo", histImage );
	channel[0] = b_hist;
	channel[1] = g_hist;
	channel[2] = r_hist;
}



/**
 * @function Hist_and_Backproj
 */


int main(int argc, char* argv[])
{
	string vidFile = "..\\data\\video.mpg";
	ToolsTrackingApp toolsTracking;

	toolsTracking.init(vidFile.c_str());
	toolsTracking.run();




}