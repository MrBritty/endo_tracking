
#include "iostream"
#include "stdlib.h"

// OpenCV includes.
#include "cv.h"
#include "highgui.h"

using namespace cv;
using namespace std;



void fittingline(Mat drawing)
{
	cv::Mat input = drawing;
	cv::Mat gray;
	cv::cvtColor(input,gray,CV_BGR2GRAY);

	cv::Mat mask = gray>100;
	cv::imshow("mask",mask);

	cv::Mat dt;
	cv::distanceTransform(mask,dt,CV_DIST_L1,CV_DIST_MASK_PRECISE);

	cv::imshow("dt", dt/15.0f);
	cv::imwrite("fitLineOut.png",255*dt/15.0f);


	//care: this part doesn't work for diagonal lines, a ridge detection would be better!!
	cv::Mat lines = cv::Mat::zeros(input.rows, input.cols, CV_8UC1);
	//only take the maxDist of each row
	for(unsigned int y=0; y<dt.rows; ++y)
	{
		float biggestDist = 0;
		cv::Point2i biggestDistLoc(0,0);
		for(unsigned int x=0; x<dt.cols; ++x)
		{
			cv::Point2i current(x,y);
			if(dt.at<float>(current) > biggestDist)
			{
				biggestDist = dt.at<float>(current) ;
				biggestDistLoc = current;

			}
		}
		lines.at<unsigned char>(biggestDistLoc) = 255;
	}

	//and the maxDist of each row
	for(unsigned int x=0; x<dt.cols; ++x)
	{
		float biggestDist = 0;
		cv::Point2i biggestDistLoc(0,0);
		for(unsigned int y=0; y<dt.rows; ++y)
		{
			cv::Point2i current(x,y);
			if(dt.at<float>(current) > biggestDist)
			{
				biggestDist = dt.at<float>(current) ;
				biggestDistLoc = current;

			}
		}
		lines.at<unsigned char>(biggestDistLoc) = 255;
	}

	cv::imshow("max", lines);

}


void Normalize_Color(Mat bgr_image, Mat &normalized_bgr)
{

    cv::imshow("original image", bgr_image);
    cv::Mat bgr_image_f;
    bgr_image.convertTo(bgr_image_f, CV_32FC3);

    // Extract the color planes and calculate I = (r + g + b) / 3
    std::vector<cv::Mat> planes(3);
    cv::split(bgr_image_f, planes); 

    cv::Mat intensity_f((planes[0] + planes[1] + planes[2]) / 3.0f);
    cv::Mat intensity;
    intensity_f.convertTo(intensity, CV_8UC1);
    cv::imshow("intensity", intensity);

    //void divide(InputArray src1, InputArray src2, OutputArray dst, double scale=1, int dtype=-1)
    cv::Mat b_normalized_f;
    cv::divide(planes[0], intensity_f, b_normalized_f);
    cv::Mat b_normalized;
    b_normalized_f.convertTo(b_normalized, CV_8UC1, 255.0);
    cv::imshow("b_normalized", b_normalized);

    cv::Mat g_normalized_f;
    cv::divide(planes[1], intensity_f, g_normalized_f);
    cv::Mat g_normalized;
    g_normalized_f.convertTo(g_normalized, CV_8UC1, 255.0);
    cv::imshow("g_normalized", g_normalized);

    cv::Mat r_normalized_f;
    cv::divide(planes[2], intensity_f, r_normalized_f);
    cv::Mat r_normalized;
    r_normalized_f.convertTo(r_normalized, CV_8UC1, 255.0);
    cv::imshow("r_normalized", r_normalized);

	vector<Mat>channel1;
	channel1.push_back(b_normalized);
	channel1.push_back(g_normalized);
	channel1.push_back(r_normalized);
	merge(channel1,normalized_bgr);
	//merge(b_normalized,g_normalized, r_normalized, normalized_bgr);
    //cv::waitKey();
}

void Color_Segmentation(Mat image, Mat &tempImg3)
{
	Mat hsv, mask_rings, mask_sheet, mask_tool;
	Mat mask_rings_f, mask_sheet_f, mask_tool_f;
	cvtColor(image, hsv, CV_BGR2HSV);
	image.copyTo(mask_rings);
	/*mask_rings((mask_rings >= 10))= 0; 
	b1((b1 > 0))= 255; */
	Mat channel[3];
	split(hsv, channel);
	Mat result, result1;

	
	threshold(channel[0],result,10,255,THRESH_TOZERO_INV); //b1((b1 >= T))= 0; 
	//imshow("Result", result);
	threshold(result,result1,1,255,THRESH_BINARY); //b1((b1 > 0))= 255;
	
	Mat sel = getStructuringElement(MORPH_ELLIPSE, cv::Size(4,4));
	erode(result1, result1, sel);
	//imshow("Result1", result1);
	Mat sel1 = getStructuringElement(MORPH_ELLIPSE, cv::Size(9,9));
	Mat tempImg;
	dilate(result1, tempImg, sel1);
	vector<Mat>channel1;
	channel[1] &= tempImg;
	channel[2] &= tempImg;
	channel1.push_back(tempImg);
	channel1.push_back(channel[1]);
	channel1.push_back(channel[2]);
	merge(channel1,tempImg3);

	
	//tempImg3 &= tempImg3;
	//imshow("Result2", tempImg3);
	
}




int main(int argc, char* argv[])
{


	VideoCapture cap;
	Mat result, frame,gray_bg, frame_gray, image,silh1;
	cap.open("video_finale_Mithilesh_0Deg_input.mov");
	cap >> frame;
	frame.copyTo(image);
	// Video Write 
	
	VideoWriter outputVideo,  outputVideo1, outputVideo2, outputVideo3;       
	int ex = static_cast<int>(cap.get(CV_CAP_PROP_FOURCC)); 
	Size S = Size((int) cap.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int) cap.get(CV_CAP_PROP_FRAME_HEIGHT));

	int count = 0;
	int a;
	//Create a new window.
	cvNamedWindow("My Window", CV_WINDOW_AUTOSIZE);

	Mat fgimg, fgmask;
	Mat Mean   = Mat::zeros(frame.rows, frame.cols,CV_32FC3); 

	Mat bgimg, mean_rings;

	vector<Mat> image_array;


	for(;;)
	{
		cap >> frame;
		frame.copyTo(image);
		if (frame.empty())
			break;


		if (count < 30)
		{
			image_array.push_back(image);

			if( fgimg.empty() )
				fgimg.create(image.size(), image.type());

			//bg_model(image, fgmask, -1 /*update_bg_model ? -1 : 0*/);

			//fgimg = Scalar::all(0);
			//image.copyTo(fgimg, fgmask);

			//bg_model.getBackgroundImage(bgimg);

		}
		count++;
		if (count == 31)
		{
			int size_l = image_array.size();
			for (int i = 0; i < size_l; i++)
			{
				accumulate(image_array[i], Mean);
			}
			Mean = Mean / size_l;
			//Mat Mean_image = ;
			Mean.convertTo(Mean,CV_8U);
			//Normalize_Color(Mean, Mean);
			//imwrite("D:\\Videos\\mean_image.jpg", Mean);
			//imshow("mean",Mean);
			//if(!bgimg.empty())
				//imshow("mean background image", bgimg );
			//imwrite("D:\\Videos\\GMM_bgd_image.jpg", bgimg);
			//hist_image(Mean);
			Color_Segmentation(Mean, mean_rings);
		}

		if (count > 31)
		{
			Mat difference, difference_gray, tool_image, tool_image_gray;
			Normalize_Color(Mean, Mean);
			Normalize_Color(image, image);
			absdiff( Mean, image, difference ); // get difference between frames
			//string address = "D:\\Videos\\Tools\\img_" + stringcount + ".jpg";
			//cv::imwrite(address.toUtf8().constData(),FINAL_IM_VEC[i]);
			ostringstream convert;
			convert << "D:/Videos/Tool/img" << count << ".jpg";
			//cvSaveImage(convert.str().c_str(), difference);
			string filename = convert.str();
			//cvSaveImage(filename.c_str(), img2);
			//imwrite(filename.c_str(), difference);
			//imshow("Difference",difference);
			//hist_image(difference);
			//waitKey(10);


			cvtColor( difference, difference_gray, CV_BGR2GRAY ); // convert frame to grayscale
			
			
			//cvtColor( frame, frame_gray, CV_BGR2GRAY ); // convert frame to grayscale

			//absdiff( gray_bg, frame_gray, difference_gray1 ); // get difference between frames


			//blur( difference_gray1, difference_gray1, Size(3,3) );

			// Decision making on which difference to take
			//Scalar avg_diff = mean( difference_gray );
			//Scalar avg_diff1 = mean( difference_gray1 );
			Mat image_rings;
			Color_Segmentation(image, image_rings);

			Mat diff_ring, diff_ring_image;
			//cvtColor(mean_rings, mean_rings, CV_HSV2BGR);
			//cvtColor(image_rings, image_rings, CV_HSV2BGR);

			absdiff(mean_rings, image_rings, diff_ring);
			
			imshow("Diff_rings", diff_ring);
			//Mat difference_hsv;
			//cvtColor(diff_ring, difference_hsv, CV_HSV2BGR);
			
			difference &= ~diff_ring;
			//cvtColor(difference_hsv,difference,CV_HSV2BGR);
			imshow("Diff_", difference);
			
			vector<Mat>channel1;
			channel1.push_back(difference_gray);
			channel1.push_back(difference_gray);
			channel1.push_back(difference_gray);
			merge(channel1, diff_ring_image);
			
			//outputVideo1.write(difference); //Needed
			
			
			//cvtColor( tool_image, tool_image_gray, CV_BGR2GRAY );
			//difference_gray &= tool_image_gray;
			Mat canny_output;
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			int thresh = 50;
			
			//cvFillHoles(difference_gray);
			
			/// Detect edges using canny

			Canny( difference, canny_output, thresh, thresh*4, 3 );


			normalize(canny_output, canny_output, 0, 1, cv::NORM_MINMAX);
			Mat kernel = (Mat_<uchar>(3,3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
			Mat dst;
			dilate(canny_output, dst, kernel);
			dilate(dst, dst, kernel);

			normalize(dst, dst, 0, 255, cv::NORM_MINMAX);


			/// Find contours
			findContours( dst, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );


			// approximate contours
			std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
			for( int i = 0; i < contours.size(); i++ ) {
				approxPolyDP(cv::Mat(contours[i]), contours_poly[i], 5, true );
			}


			//Find the largest and second largest contour

			int largest_area=0;
			int second_largest_area = 0;
			int largest_contour_index = 0;
			int sec_largest_contour_index = 0;
			Rect bounding_rect_1, bounding_rect_2;
			for( int i = 0; i< contours_poly.size(); i++ ) // iterate through each contour. 
			{
				double a = contourArea( contours_poly[i],false);	 
				double b = arcLength(contours_poly[i],false);
				if(a > largest_area)
				{
					largest_area = a;
					largest_contour_index = i;//Store the index of largest contour
					bounding_rect_1 = boundingRect(contours_poly[i]); // Find the bounding rectangle for biggest contour
				}
				else if(a > second_largest_area)
				{
					second_largest_area = a;
					sec_largest_contour_index = i;
					bounding_rect_2 = boundingRect(contours_poly[i]);
				}
			}

			// Look for the rectangle with lower y value of the bounding box
			if (bounding_rect_1.y < bounding_rect_2.y)
			{

				Point* startpt = new Point();
				startpt->x = bounding_rect_1.x;
				startpt->y = bounding_rect_1.y + bounding_rect_1.height;
				Point* endpt = new Point();
				endpt->x = bounding_rect_1.x + bounding_rect_1.width;
				endpt->y = bounding_rect_1.y + bounding_rect_1.height;
				line(image, *startpt, *endpt, Scalar(0,0,255),5,8,0);
			}
			else
			{
				Point* startpt = new Point();
				startpt->x = bounding_rect_2.x;
				startpt->y = bounding_rect_2.y + bounding_rect_2.height;
				Point* endpt = new Point();
				endpt->x = bounding_rect_2.x + bounding_rect_2.width;
				endpt->y = bounding_rect_2.y + bounding_rect_2.height;
				line(image, *startpt, *endpt, Scalar(0,0,255),5,8,0);
			}
			//rectangle(image, bounding_rect,  Scalar(0,255,255), 1, CV_AA ); 

			
			imshow("My Window", image); //Needed
			
			//outputVideo.write(image); //Needed
			
			Scalar color( 255,255,255);
			Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
			double min_val,max_val;
			Point min_loc, max_loc;
			drawContours( drawing, contours,largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.
			drawContours( drawing, contours,sec_largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the second largest contour using previously stored index.		
			

			///////Fitting line
			//if(contours.size())
			//{
			//	vector<Point> aa= contours[largest_contour_index];
			//	Point tt;
			//	for (int i = 0; i < aa.size(); i++)
			//	{
			//		tt = aa[i];
			//	}
			//	vector<double> line;
			//	if(aa.size() > 20)
			//	fitLine(aa, line , CV_DIST_L2, 0, 0.01,0.01);
			//}
			//double minlinex, maxliney;
			//Point minpoint, maxpoint;
			//minMaxLoc(line, minlinex, maxliney, minpoint,  maxpoint, noArray());
			
			//fittingline(drawing);
			
			frame.copyTo(image);
			/// Show in a window
			namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
			
			imshow( "Contours", drawing );
			
			//outputVideo2.write(drawing); //Needed
			
			dst.copyTo(result);
			waitKey(10);
		}
	}

	a = 0;

	return 0;
}