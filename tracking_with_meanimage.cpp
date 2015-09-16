
#include "iostream"
#include "stdlib.h"

// OpenCV includes.
#include "cv.h"
#include "highgui.h"

using namespace cv;
using namespace std;




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
	Mat sel1 = getStructuringElement(MORPH_ELLIPSE, cv::Size(9,9));
	dilate(result1, tempImg3, sel1);
	
}




int main(int argc, char* argv[])
{


	VideoCapture cap;
	Mat result, frame,gray_bg, frame_gray, image,silh1;
	cap.open("video_finale_Mithilesh_0Deg_input.mov");
	cap >> frame;
	frame.copyTo(image);
     // Write video
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
		if( frame.empty() )
			break;


		if (count < 30)
		{
			image_array.push_back(image);

			if( fgimg.empty() )
				fgimg.create(image.size(), image.type());


			fgimg = Scalar::all(0);
			image.copyTo(fgimg, fgmask);


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
			absdiff( Mean, image, difference ); // get difference between frames
			cvtColor( difference, difference_gray, CV_BGR2GRAY ); // convert frame to grayscale
			Mat image_rings;
			Color_Segmentation(image, image_rings);

			Mat diff_ring, diff_ring_image;
			absdiff(mean_rings, image_rings, diff_ring);
			//imshow("Diff_rings", diff_ring);
			difference_gray &= ~diff_ring;
			//imshow("Diff_rings", difference_gray);
			vector<Mat>channel1;
			channel1.push_back(difference_gray);
			channel1.push_back(difference_gray);
			channel1.push_back(difference_gray);
			merge(channel1, diff_ring_image);
			//cvtColor( tool_image, tool_image_gray, CV_BGR2GRAY );
			//difference_gray &= tool_image_gray;
			Mat canny_output;
			vector<vector<Point> > contours;
			vector<Vec4i> hierarchy;
			int thresh = 50;

			/// Detect edges using canny
			cv::Mat element[2];
			threshold( difference_gray, canny_output, 100, 255, THRESH_BINARY );
			element[0] = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(0, 0));
			element[1] = getStructuringElement(MORPH_ELLIPSE, Size(8, 8), Point(0, 0));

			erode(canny_output, canny_output, element[0]);
			erode(canny_output, canny_output, element[0]);
			dilate(canny_output, canny_output, element[1]);
			dilate(canny_output, canny_output, element[1]);
			dilate(canny_output, canny_output, element[1]);


			imshow("Diff_rings", canny_output);


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

			imshow("My Window", image);
			outputVideo.write(image);
			Scalar color( 255,255,255);
			Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
			double min_val,max_val;
			Point min_loc, max_loc;
			drawContours( drawing, contours,largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the largest contour using previously stored index.
			drawContours( drawing, contours,sec_largest_contour_index, color, CV_FILLED, 8, hierarchy ); // Draw the second largest contour using previously stored index.		
			
			frame.copyTo(image);
			/// Show in a window
			namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
			imshow( "Contours", drawing );
			outputVideo2.write(drawing);
			dst.copyTo(result);

			waitKey(10);
		}
	}
	outputVideo.release();
	outputVideo1.release();
	outputVideo2.release();
	//outputVideo3.release();

	a = 0;

	return 0;
}