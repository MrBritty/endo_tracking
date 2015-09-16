#include "ToolsTrackingApp.hpp"
#include <assert.h>
#define ECHAP 27
#define KEY_Q 'q'
#define MVT_MIN 16
#define MAX_TRACKING_ERR 5
#define OUTPUT_NAME "output.avi"

ToolsTrackingApp::ToolsTrackingApp() :toolbox(), trackedPoints(), capture(), frame(), fgMask(), gray(), prevGray() 
{

	// Get the moving foreground properly
	pMOG = new BackgroundSubtractorMOG2(20, 200, false);
	element[0] = getStructuringElement(MORPH_CROSS, Size(5, 5), Point(0, 0));
	element[1] = getStructuringElement(MORPH_ELLIPSE, Size(8, 8), Point(0, 0));
	element[2] = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(0, 0));
	element[3] = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(0, 0));

	// ROI params
	//roi.x = 80;
	//roi.y = 60;
	//roi.width = 1100;
	//roi.height = 680;
	//temp_mat.create(roi.height, roi.width, CV_8UC3);
	//temp_mat = Scalar::all(0);


	// Logger Open File
	logger.open("Contour_properties.txt", ios_base::out);
}

ToolsTrackingApp::~ToolsTrackingApp() 
{
	destroyAllWindows();
	capture.release();
	outputVideo.release();
	delete pMOG;
}

int ToolsTrackingApp::init(const char* filename) 
{

	//initializing the application for tool tracking
	capture.open(filename);

	if (!capture.isOpened()) 
	{
		cerr << "Unable to open video file: " << filename << endl;
		return -1;
	}

	capture >> frame;


	//namedWindow("Frame"/*, WINDOW_NORMAL*/);
	//namedWindow("seg-motion"/*, WINDOW_NORMAL*/);
	//namedWindow("seg-ring"/*, WINDOW_NORMAL*/);
	namedWindow("test-1"/*, WINDOW_NORMAL*/);

	//int ex = capture.get(CV_CAP_PROP_FOURCC);
	//Size S = Size(roi.s, height);


	int fps = capture.get(CV_CAP_PROP_FPS);

	outputVideo.open(OUTPUT_NAME, CV_FOURCC('M', 'J', 'P', 'G'), fps, frame.size(), true);

	if (!outputVideo.isOpened()) 
	{
		cerr << "Unable to open output video file: " << OUTPUT_NAME << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}


bool ToolsTrackingApp::isNotAContour(ContourFeature& cf, int hierarchy) 
{
	if (cf.getArea() > 100000)
	{
		return true;
	}
	else
	{
		return (cf.getArea() < 2000 || cf.getPerimeter() < 200 || hierarchy != -1);
	}
}

bool ToolsTrackingApp::isNotARing(ContourFeature& cf, int hierarchy)
{
	return true;
}


void ToolsTrackingApp::run() 
{
	//initialize 
	Mat test1;
	vector <Ring> rings;
	vector<vector<Point> > contours;
	vector<vector<Point> > contours_ring;
	vector<Point> poly;
	vector<vector<Point> >  poly_ring;
	vector<vector<Point> > poly1(1); // delete
	vector<Vec4i> hierarchy;
	vector<Vec4i> hierarchy_ring;
	int contoursSize = 0;
	int contours_ringSize = 0;
	BoundingBox boundingBox;
	State_Machine state_obj;
	bool forward = true;
	char key = 0;
	int nbFrames = 0;
	Scalar* color;
	Scalar blue = Scalar(200, 0, 0);
	Scalar yellow = Scalar(0, 200, 200);
	Scalar red = Scalar(0, 0, 200);
	Scalar green = Scalar(0, 200, 0);
	TermCriteria termcrit(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
	Size winSize(31, 31);
	vector<unsigned char> status;
	vector<float> err;
	int count = 0;
	ofstream outStream("rings_mask_out.log");
	int p = 0;
	int q = 0;
	
	//Main chunk of execution
	while (key != ECHAP && key != KEY_Q)
	{

		capture >> frame;
		//frame = frame(roi);
		//imshow("out1", frame);



		//  Process the contour for tool
		vector<Point> contour_tool;

		int max_y = -1;
		bool tool_present = segmentation_tool(frame, contour_tool, &max_y);
		Mat mask_tool = Mat::zeros(frame.rows, frame.cols, CV_8UC3);

		Mat mask_rings = Mat::zeros(frame.rows, frame.cols, CV_8UC1);

		// Pass tool contour to the ring analyzer;
		segmentation_ring(contour_tool, mask_rings);
		//imshow("Rings", mask_rings);

		count++;

		if (count == 1)
		{
			Mat initial_ring_mask = mask_rings.clone();


			//imshow("Initial mask", initial_ring_mask);
			//imwrite("image.jpg", initial_ring_mask);
			///**
			//Module to process the fgMaskRing
			//**/

			findContours(initial_ring_mask, contours_ring, hierarchy_ring, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
			unsigned int No_of_ring = contours_ring.size();
			//cout << "No of Rings -> " << No_of_ring << endl;


			Mat draw_rings = Mat::zeros(frame.rows, frame.cols, CV_8UC1);

			std::ostringstream os;

			for (int i = contours_ring.size() - 1; i >= 0; i--)
			{
				ContourFeature cFeat(contours_ring[i]);
				cv::Point centre = cFeat.getCentre(); centre.y += 10;
				if (centre.x < 20 && centre.y < 20)
				{
					contours_ring.erase(contours_ring.begin() + i);
				}
			}
			vector<Mat> subregions;
			vector<Point> centre;
			vector<Rect> roi;
			for (int i = 0; i <  contours_ring.size(); i++)
			{
				ContourFeature cFeat(contours_ring[i]);
				centre.push_back(cFeat.getCentre()); //centre.y += 10;

				// Get bounding box for contour
				roi.push_back(boundingRect(contours_ring[i])); // This is a OpenCV function


				//double area = cFeat.getArea();
				//double length = sqrt(area);
				drawContours(draw_rings, contours_ring, i, Scalar(255, 0, 0), CV_FILLED);
				Mat contourRegion, imageROI;
				Mat ring_mask(draw_rings.size(), CV_8UC1, Scalar::all(0));
				imageROI =  ring_mask.clone(); // 'image' is the image you used to compute the contours.
				////ring_mask = imageROI(roi).setTo(Scalar(255));
				imageROI(roi[i]).setTo(Scalar::all(255));
				ring_mask = draw_rings & imageROI;
				////imageROI= ring_mask(contourRegion);

				//// Mat maskROI = mask(roi); // Save this if you want a mask for pixels within the contour in contourRegion. 

				//// Store contourRegion. contourRegion is a rectangular image the size of the bounding rect for the contour 
				//// BUT only pixels within the contour is visible. All other pixels are set to (0,0,0).


				subregions.push_back(ring_mask);

				//RotatedRect rRect = RotatedRect(centre, Size2f(length+12+i,length-10-i), 30);
				//Point2f vertices[4];
				//rRect.points(vertices);
				////for (int i = 0; i < 4; i++)
				//	//line(draw_rings, vertices[i], vertices[(i+1)%4], Scalar(255,255,0));
				//Rect brect = rRect.boundingRect();
				//rectangle(draw_rings, brect, Scalar(0,255,0));

				//os << "Centre ->( " << centre[i].x << "," << centre[i].y << ")" << endl;
				//putText(draw_rings, os.str(), centre[i], 0, 0.4, cv::Scalar(0, 0, 255), 1, 8, false);
				//os.str("");

				//cvDestroyWindow("rectangles");

			}


			for (int j = 0; j < 6; j ++)
			{
				if( j == 2 )
				{
					Ring ring(centre[j+1], sqrt(pow(roi[j+1].width, 2.0) + pow (roi[j+1].height, 2.0))/ 2, subregions[j + 1]);
					rings.push_back(ring);
				}
				else if( j == 3 )
				{
					Ring ring(centre[j-1], sqrt(pow(roi[j-1].width, 2.0) + pow (roi[j-1].height, 2.0))/ 2, subregions[j - 1]);
					rings.push_back(ring);
				}
				else
				{
					Ring ring(centre[j], sqrt(pow(roi[j].width, 2.0) + pow (roi[j].height, 2.0))/ 2, subregions[j]);
					rings.push_back(ring);
				}
			}



		}



		vector<Mat> rings_out;
		vector<double> gray_sum;
		//Mat frameGray;
		//cvtColor(mask_rings, frameGray, CV_BGR2GRAY);
		for (int i = 0; i < 6; i ++)
		{

			Mat current_ring_mask = rings[i].getRing_mask();
			rings_out.push_back(current_ring_mask & mask_rings);
			imshow(" current ring mask", rings_out[i]);
			gray_sum.push_back(sum(sum(rings_out[i])).val[0]);
			//cout <<"Sum of mask "<< gray_sum[i] << endl;
			//outStream << 0 << "\t" << gray_sum[i] << endl;
		}

		/*
		[2.34422e+006, 0, 0, 0]
		0	[884340, 0, 0, 0] 		motion ring 2
		0	[2.40032e+006, 0, 0, 0]
		0	[1.48487e+006, 0, 0, 0] motion ring 4
		0	[1.82096e+006, 0, 0, 0]
		0	[1.14546e+006, 0, 0, 0] motion ring 6
		*/

		//gray_sum.push_back(2.3e6);
		//gray_sum.push_back(2.4e6);
		//gray_sum.push_back(2.4e6);
		//gray_sum.push_back(1.4e6);
		//gray_sum.push_back(1.8e6);
		//gray_sum.push_back(1.6e6);


		//vector<double> x;
		//x.push_back(0);
		//x.push_back(0);
		//x.push_back(2.4e6);
		//x.push_back(1.4e6);
		//x.push_back(1.8e6);
		//x.push_back(1.6e6);


		//bool is_equal = false;
		//is_equal = equal (gray_sum.begin(), gray_sum.end(), x.begin());
		vector<size_t> y(gray_sum.size());
		vector<size_t> target_allrings;
		vector<size_t> target_norings;
		ostringstream outs1;
		cv::Point textOrg(10, 130);


		if(!gray_sum.empty())
		{
			//	
			//	copy_if(gray_sum.begin(),gray_sum.end(), back_inserter(target),
			//		bind(less<int>(), 1.75e6, 1));

			iota(y.begin(), y.end(), 0);
			copy_if(y.begin(), y.end(),
				back_inserter(target_allrings),[=] (size_t i) { return (gray_sum[i] > 1.7e6); });
			copy_if(y.begin(), y.end(),
				back_inserter(target_norings),[=] (size_t i) { return (gray_sum[i] == 0); });


			//os << "Height, Width, Angle->( " << tt.size.height << "  ," << tt.size.width << "  ," << tt.angle << ")";


			if (forward == true)
			{

				switch(target_allrings.size())
				{
				case 0:
					if (target_norings.size() == 6)
					{

						if(state_obj.getStatus() == "5rings")
						{
							state_obj.setAllRings(2);
							state_obj.setStatus("Allringsmoved");
							forward = false;
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " All rings moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " All rings moved" << endl;
					}
					else if (target_norings.size() == 5)
					{
						if(state_obj.getStatus() == "fourrings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setRing(target_norings[3], 0);
							state_obj.setRing(target_norings[4], 0);
							state_obj.setStatus("5rings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " , " << target_norings[3] +1 << " and " << target_norings[4] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " , " << target_norings[3] +1 << " and " << target_norings[4] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 4)
					{
						if(state_obj.getStatus() == "three_rings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setRing(target_norings[3], 0);
							state_obj.setStatus("fourrings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " and " << target_norings[3] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " and " << target_norings[3] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 3)
					{
						if(state_obj.getStatus() == "tworings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setStatus("three_rings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and " << target_norings[2] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and " << target_norings[2] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 2)
					{
						if(state_obj.getStatus() == "onering")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setStatus("tworings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 1)
					{
						if(state_obj.getStatus() == "Stationary")
						{
							state_obj.setRing(target_norings[0], 0); 
							state_obj.setStatus("onering");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask  but ring "<<  target_norings[0]+1 <<" moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask  but ring "<<  target_norings[0]+1 <<" moved" << endl;
						
					}
					else
						outs1 << " Some motion in masks but no ring moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
					//cout << " Some motion in masks but no ring moved" << endl;
					break;
				case 1:
					if (target_norings.size() == 5)
					{
						if(state_obj.getStatus() == "fourrings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setRing(target_norings[3], 0);
							state_obj.setRing(target_norings[4], 0);
							state_obj.setStatus("5rings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " No motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " , " << target_norings[3] +1 << " and " << target_norings[4] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " No motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " , " << target_norings[3] +1 << " and " << target_norings[4] +1 << " moved" << endl;
					}
					//cout << " All but one ring moved" << endl;
					else if (target_norings.size() == 4)
					{
						if(state_obj.getStatus() == "three_rings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setRing(target_norings[3], 0);
							state_obj.setStatus("fourrings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " and " << target_norings[3] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " and " << target_norings[3] +1 << " moved" << endl;
					}

					//cout << " Some motion in mask but 4 rings moved" << endl;
					else if (target_norings.size() == 3)
					{
						if(state_obj.getStatus() == "tworings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setStatus("three_rings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 <<  " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and " << target_norings[2] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and " << target_norings[2] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 2)
					{
						if(state_obj.getStatus() == "onering")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setStatus("tworings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 1)
					{
						if(state_obj.getStatus() == "Stationary")
						{
							state_obj.setRing(target_norings[0], 0); 
							state_obj.setStatus("onering");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask  but ring "<<  target_norings[0]+1 <<" moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask  but ring "<<  target_norings[0]+1 <<" moved" << endl;
					}
					else
						outs1 << " Some motion in masks but no ring moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
					//cout << " Some motion in masks but no ring moved" << endl;
					break;

				case 2:
					if (target_norings.size() == 4)
					{
						if(state_obj.getStatus() == "three_rings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setRing(target_norings[3], 0);
							state_obj.setStatus("fourrings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " No motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " and " << target_norings[3] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " No motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<  " , " << target_norings[2] +1 << " and " << target_norings[3] +1 << " moved" << endl;
					}
					//cout << " All but two rings moved" << endl;
					else if (target_norings.size() == 3)
					{
						if(state_obj.getStatus() == "tworings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setStatus("three_rings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and " << target_norings[2] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but rings "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and " << target_norings[2] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 2)
					{
						if(state_obj.getStatus() == "onering")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setStatus("tworings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 <<  " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 1)
					{
						//state_obj.setRing(target_norings[0], 0); 
						if(state_obj.getStatus() == "Stationary")
						{
							state_obj.setRing(target_norings[0], 0); 
							state_obj.setStatus("onering");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask  but ring "<<  target_norings[0]+1 <<" moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask  but ring "<<  target_norings[0]+1 <<" moved" << endl;
						//cout << " but 1 ring moved" << endl;
					}
					else
						outs1 << "Some motion in masks but no ring moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
					//cout << "Some motion in masks but no ring moved" << endl;
					break;
				case 3:
					if (target_norings.size() == 3)
					{
						if(state_obj.getStatus() == "tworings")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setRing(target_norings[2], 0);
							state_obj.setStatus("three_rings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " No motion in mask but ring "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and ring " << target_norings[2] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " No motion in mask but ring "<< target_norings[0] +1 << " , " << target_norings[1] +1 <<" and ring " << target_norings[2] +1 << " moved" << endl;
					}

					else if (target_norings.size() == 2)
					{
						if(state_obj.getStatus() == "onering")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setStatus("tworings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 1)
					{
						if(state_obj.getStatus() == "Stationary")
						{
							state_obj.setRing(target_norings[0], 0); 
							state_obj.setStatus("onering");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but ring "<<  target_norings[0] +1 << " moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						
						//cout << " Some motion in mask but ring "<<  target_norings[0] +1 << " moved" << endl;
					}
					else
						outs1 << " Some motion in masks but no ring moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						
						//cout << " Some motion in masks but no ring moved" << endl;
					break;
				case 4:
					if (target_norings.size() == 2)
					{
						if(state_obj.getStatus() == "onering")
						{
							state_obj.setRing(target_norings[0], 0);
							state_obj.setRing(target_norings[1], 0);
							state_obj.setStatus("tworings");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " No motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved" ;
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " No motion in mask but ring "<< target_norings[0] +1 << " and ring " << target_norings[1] +1 << " moved" << endl;
					}
					else if (target_norings.size() == 1)
					{
						//state_obj.setRing(target_norings[0], 0); 
						if(state_obj.getStatus() == "Stationary")
						{
							state_obj.setRing(target_norings[0], 0); 
							state_obj.setStatus("onering");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " Some motion in mask but ring "<<  target_norings[0]+1 <<" moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in mask but ring "<<  target_norings[0]+1 <<" moved" << endl;
					}
					else
						outs1 << "Some motion in masks but no ring moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << "Some motion in masks but no ring moved" << endl;
					break;
				case 5:
					if (target_norings.size() == 1)
					{
						//state_obj.setRing(target_norings[0], 0); 
						if(state_obj.getStatus() == "Stationary")
						{
							state_obj.setRing(target_norings[0], 0); 
							state_obj.setStatus("onering");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}
						outs1 << " No motion in mask but ring "<<  target_norings[0]+1 <<" moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " No motion in mask but ring "<<  target_norings[0]+1 <<" moved" << endl;
					}
					else
						outs1 << "Some motion in masks but no ring moved";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Some motion in masks but no ring moved" << endl;
					break;
				case 6:
					if (target_norings.size() == 0)
					{
						if(state_obj.getStatus() == "start")
						{
							state_obj.setAllRings(1); 
							state_obj.setStatus("Stationary");
						}
						else
						{
							state_obj.update_state(state_obj.getState());
						}

						outs1 << "All rings stationary";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//state_obj.setStatus("Stationary");
						//cout << "All rings stationary" << endl;
					}

					else
						outs1 << " Unknown";
						putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
						//cout << " Unknown" << endl;
					break;
				}


			}
			else
			{
				switch(target_norings.size())
				{
					case 0:
						if (target_allrings.size() == 6)
						{

							if(state_obj.getStatus() == "5")
							{
								state_obj.setAllRings(1);
								state_obj.setStatus("Stationary");
								forward = true;
							}
							else
							{
								state_obj.update_state(state_obj.getState());
							}
							outs1 << " All rings moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " All rings moved back" << endl;
						}
						else if (target_allrings.size() == 5)
						{
							if(state_obj.getStatus() == "4b")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setRing(target_allrings[4], 1);
								state_obj.setStatus("5");
							}
							else if (state_obj.getStatus() == "5")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setRing(target_allrings[4], 1);
								state_obj.setStatus("5");
							}
							outs1 << " Some motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " , " << target_allrings[3] +1 << " and " << target_allrings[4] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " , " << target_allrings[3] +1 << " and " << target_allrings[4] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 4)
						{
							if(state_obj.getStatus() == "3ba")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setStatus("4b");
							}
							else if (state_obj.getStatus() == "4b")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setStatus("4b");
							}
							outs1 << " Some motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " and " << target_allrings[3] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " and " << target_allrings[3] +1 << " moved back" << endl;
						}

						else if (target_allrings.size() == 3)
						{
							if(state_obj.getStatus() == "2bac")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							else if (state_obj.getStatus() == "3ba")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							outs1 << " Some motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 2)
						{
							if(state_obj.getStatus() == "1back")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							else if (state_obj.getStatus() == "1back")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							outs1 << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 1)
						{
							//state_obj.setRing(target_norings[0], 0); 
							if(state_obj.getStatus() == "Allringsmoved")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");

							}
							else if (state_obj.getStatus() == "1back")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");
							}
							outs1 << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back" << endl;
						}
						else
						{
							if(state_obj.getStatus() == "Allringsmoved")
							{
								outs1 << " Some motion in masks but no ring moved";
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
							else
							{
								state_obj.update_state(state_obj.getState());
								outs1 << " Some motion in masks but no ring moved";
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
						}
						break;
					case 1:
						if (target_allrings.size() == 5)
						{
							if(state_obj.getStatus() == "4b")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setRing(target_allrings[4], 1);
								state_obj.setStatus("5");
							}
							else if (state_obj.getStatus() == "5")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setRing(target_allrings[4], 1);
								state_obj.setStatus("5");
							}
							outs1 << " No motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " , " << target_allrings[3] +1 << " and " << target_allrings[4] +1 << " moved back" ;
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " No motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " , " << target_allrings[3] +1 << " and " << target_allrings[4] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 4)
						{
							if(state_obj.getStatus() == "3ba")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setStatus("4b");
							}
							else if (state_obj.getStatus() == "4b")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setStatus("4b");
							}
							outs1 << " Some motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " and " << target_allrings[3] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " and " << target_allrings[3] +1 << " moved back" << endl;
						}

						else if (target_allrings.size() == 3)
						{
							if(state_obj.getStatus() == "2bac")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							else if(state_obj.getStatus() == "3ba")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							outs1 << " Some motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 2)
						{
							if(state_obj.getStatus() == "1back")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							else if (state_obj.getStatus() == "2bac")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							outs1 << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 1)
						{
							//state_obj.setRing(target_norings[0], 0); 
							if(state_obj.getStatus() == "Allringsmoved")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");

							}
							else if (state_obj.getStatus() == "1back")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");
							}
							outs1 << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back" << endl;
						}
						else
						{
							if(state_obj.getStatus() == "Allringsmoved")
							{
								outs1 << " Some motion in masks but no ring moved";
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
							else
							{
								state_obj.update_state(state_obj.getState());
								outs1 << " Some motion in masks but no ring moved";
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
						}
						break;
					case 2:
						if (target_allrings.size() == 4)
						{
							if(state_obj.getStatus() == "3ba")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setStatus("4b");
							}
							else if (state_obj.getStatus() == "4b")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setRing(target_allrings[3], 1);
								state_obj.setStatus("4b");
							}
							outs1 << " No motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " and " << target_allrings[3] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " No motion in mask but rings "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<  " , " << target_allrings[2] +1 << " and " << target_allrings[3] +1 << " moved back" << endl;
						}

						else if (target_allrings.size() == 3)
						{
							if(state_obj.getStatus() == "2bac")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							else if (state_obj.getStatus() == "3ba")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							outs1 << " Some motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 2)
						{
							if(state_obj.getStatus() == "1back")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							else if (state_obj.getStatus() == "2bac")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							outs1 << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 1)
						{
							//state_obj.setRing(target_norings[0], 0); 
							if(state_obj.getStatus() == "Allringsmoved")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");

							}
							else if(state_obj.getStatus() == "1back")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");
							}
							outs1 << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back" << endl;
						}
						else
						{
							if(state_obj.getStatus() == "Allringsmoved")
							{
								outs1 << " Some motion in masks but no ring moved" ;
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
							else
							{
								state_obj.update_state(state_obj.getState());
								
								outs1 << " Some motion in masks but no ring moved" ;
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
						}
						break;
					case 3:
						if (target_allrings.size() == 3)
						{
							if(state_obj.getStatus() == "2bac")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							else if (state_obj.getStatus() == "3ba")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setRing(target_allrings[2], 1);
								state_obj.setStatus("3ba");
							}
							
								outs1 << " No motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back" ;
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " No motion in mask but ring "<< target_allrings[0] +1 << " , " << target_allrings[1] +1 <<" and ring " << target_allrings[2] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 2)
						{
							if(state_obj.getStatus() == "1back")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							else if (state_obj.getStatus() == "2bac")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							outs1 << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 1)
						{
							//state_obj.setRing(target_norings[0], 0); 
							if(state_obj.getStatus() == "Allringsmoved")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");

							}
							else if (state_obj.getStatus() == "1back")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");
							}
							outs1 << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back" << endl;
						}
						else
						{
							if(state_obj.getStatus() == "Allringsmoved")
							{
								outs1 << " Some motion in masks but no ring moved";
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
							else
							{
								state_obj.update_state(state_obj.getState());
								outs1 << " Some motion in masks but no ring moved";
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
						}
						break;
					case 4:
						if (target_allrings.size() == 2)
						{
							if(state_obj.getStatus() == "1back")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							else if(state_obj.getStatus() == "2bac")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1);
								state_obj.setRing(target_allrings[1], 1);
								state_obj.setStatus("2bac");
							}
							outs1 << " No motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " No motion in mask but ring "<< target_allrings[0] +1 << " and ring " << target_allrings[1] +1 << " moved back" << endl;
						}
						else if (target_allrings.size() == 1)
						{
							//state_obj.setRing(target_norings[0], 0); 
							if(state_obj.getStatus() == "Allringsmoved")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");

							}
							else if (state_obj.getStatus() == "1back")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");
							}
							outs1 << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back";
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " Some motion in masks but ring "<<  target_allrings[0]+1 <<" moved back" << endl;
						}
						else
						{
							if(state_obj.getStatus() == "Allringsmoved")
							{
								outs1 << " Some motion in masks but no ring moved" ;
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
							else
							{
								state_obj.setAllRings(2); 
								state_obj.setStatus("Allringsmoved");
								outs1 << " Some motion in masks but no ring moved" ;
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
						}
						break;
					case 5:
						if (target_allrings.size() == 1)
						{
							//state_obj.setRing(target_norings[0], 0); 
							if(state_obj.getStatus() == "Allringsmoved")
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");
							}
							else if (state_obj.getStatus() == "1back")
							{
								state_obj.update_state(state_obj.getState());
							}
							else
							{
								state_obj.setAllRings(2);
								state_obj.setRing(target_allrings[0], 1); 
								state_obj.setStatus("1back");
							}
							outs1 << " No motion in masks but ring"<<  target_allrings[0]+1 <<" moved back" ;
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << " No motion in masks but ring"<<  target_allrings[0]+1 <<" moved back" << endl;
						}
						else
						{
							if(state_obj.getStatus() == "Allringsmoved")
							{
								outs1 << " Some motion in masks but no ring moved" ;
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
							else
							{
								state_obj.setAllRings(2); 
								state_obj.setStatus("Allringsmoved");
								outs1 << " Some motion in masks but no ring moved" ;
								putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
								//cout << " Some motion in masks but no ring moved" << endl;
							}
						}
						break;
					case 6:
						if (target_allrings.size() == 0)
						{
							if(state_obj.getStatus() == "Allringsmoved")
							{
								//state_obj.setAllRings(1); 
								state_obj.setStatus("Allringsmoved");
							}
							//state_obj.setStatus("Stationary");
							outs1 << "All rings moved" ;
							putText(frame, outs1.str(), textOrg, 0, 0.8, cv::Scalar(0, 0, 255), 1, 8, false);
							//cout << "All rings moved" << endl;
						}
						break;

				}

			}

		}


		imshow("Out2", frame);
		string outfilename = "E:\\Official\\IITD\\computer vision\\Assignments\\Programs\\opencv\\endo_tracking\\data\\output\\Img_" + SSTR( p+1 ) + ".png";
		imwrite(outfilename, frame);
		p++;

		if (!contour_tool.empty())
		{
			// Show the tool
			Vec4f lines;
			cv::fitLine(Mat(contour_tool), lines, 2, 0, 0.01, 0.01);
			//int lefty = ((-lines[2]) * lines[1] / lines[0]) + lines[3];
			//int righty = ((mask_tool.cols - lines[2])*lines[1] / lines[0]) + lines[3];
			RotatedRect tt = minAreaRect(Mat(contour_tool));
			Rect tt1 = tt.boundingRect();
			Point2f rect_points[4]; tt.points(rect_points);
			vector<vector<Point> >contour_tools;
			contour_tools.push_back(contour_tool);
			//rectangle(final_mask, tt1, Scalar(255, 0, 255));
			drawContours(mask_tool, contour_tools, 0, Scalar(255, 0, 0), CV_FILLED);
			vector<double> dist1(4);
			for (int j = 0; j < 4; j++)
			{
				line(mask_tool, rect_points[j], rect_points[(j + 1) % 4], Scalar(0, 255, 0), 1, 8);
				dist1[j] = sqrt(((rect_points[j].x - rect_points[j + 1].x) * (rect_points[j].x - rect_points[j + 1].x)) +
					((rect_points[j].y - rect_points[j + 1].y) * (rect_points[j].y - rect_points[j + 1].y)));
			}
			std::ostringstream os1, os2;
			//os << "Height, Width, Angle->( " << tt.size.height << "  ," << tt.size.width << "  ," << tt.angle << ")";

			os1 << "p1, p2, p3, p4 ->(" << rect_points[0] << " ," << rect_points[1] << " ," << rect_points[2] << " ," << rect_points[3] << " )";
			os2 << "d1, d2, d3, d4 ->(" << dist1[0] << " ," << dist1[1] << " ," << dist1[2] << " ," << dist1[3] << " )";
			putText(mask_tool, os1.str(), rect_points[0], 0, 0.4, cv::Scalar(0, 0, 255), 1, 8, false);
			putText(mask_tool, os2.str(), Point(rect_points[0].x, rect_points[0].y + 13), 0, 0.4, cv::Scalar(0, 0, 255), 1, 8, false);
			//cv::line(mask_tool, Point(mask_tool.cols - 1, righty), Point(0, righty), Scalar(0, 255, 0), 2);

			cv::line(mask_tool, Point(lines[2], lines[3]), Point(lines[2] + lines[0] * 100, lines[3] + lines[1] * 100), Scalar(0, 255, 0), 2);
			string outfilename2 = "E:\\Official\\IITD\\computer vision\\Assignments\\Programs\\opencv\\endo_tracking\\data\\output_1\\Img_" + SSTR( q+1 ) + ".png";
			imwrite(outfilename2, mask_tool);
			q++;
			//imshow("test1", mask_tool);
			
			contour_tools.clear();

		}
		key = waitKey(20);
		keyboardEvent(key);
	}
	logger.close();
	outStream.close();
}

void ToolsTrackingApp::keyboardEvent(char key) 
{
	switch (key) 
	{
	case 'n':
		capture.set(CV_CAP_PROP_POS_FRAMES,
			capture.get(CV_CAP_PROP_POS_FRAMES) + 100);
		break;

	case 'p':
		while (waitKey(0) != 'p');
		break;
	}
}

bool ToolsTrackingApp::segmentation_tool(const cv::Mat &frame, vector<Point> &contour_tool, int *max_y)
{
	//Get the contours for processing
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	pMOG->operator()(frame, fgMask);
	erode(fgMask, fgMask, element[0]);
	erode(fgMask, fgMask, element[0]);
	dilate(fgMask, fgMask, element[1]);
	dilate(fgMask, fgMask, element[1]);
	dilate(fgMask, fgMask, element[1]);


	Mat mask = fgMask.clone();
	frame.copyTo(temp_mat, mask);

	Mat hsv, maskring1;
	cvtColor(temp_mat, hsv, CV_BGR2HSV);

	Mat channel[3];
	split(hsv, channel);

	// Get only the tool from the moving foreground ( It also contains the rings)// I have kept the rings red here
	// It also contains the endoscope. Need to separate that as well


	threshold(channel[0], maskring1, 10, 255, THRESH_TOZERO_INV); //b1((b1 >= T))= 0; 
	threshold(maskring1, maskring1, 1, 255, THRESH_BINARY); //b1((b1 > 0))= 255;

	erode(maskring1, maskring1, element[0]);
	erode(maskring1, maskring1, element[0]);
	dilate(maskring1, maskring1, element[1]);
	dilate(maskring1, maskring1, element[1]);
	dilate(maskring1, maskring1, element[1]);


	mask -= maskring1;

	erode(mask, mask, element[0]);
	erode(mask, mask, element[0]);

	findContours(mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
	unsigned int contour_toolSize = contours.size();
	vector<vector<Point> > found_contours;
	vector<int> max_yy;
	for (unsigned int k = 0; k < contour_toolSize; k++)
	{
		ContourFeature cFeat(contours[k]);
		if (isNotAContour(cFeat, hierarchy[k][3]))
		{
			continue;
		}
		vector<int> yy(contours[k].size());
		for (int p = 0; p < contours[k].size(); p++)
		{
			yy[p] = contours[k][p].y;
		}
		double min_y = *min_element(yy.begin(), yy.end());
		if (min_y > 5)
			continue;
		else
		{
			found_contours.push_back(contours[k]);
			max_yy.push_back(*max_element(yy.begin(), yy.end()));
		}
	}

	if (found_contours.size() == 1)
	{
		contour_tool = found_contours[0];
		*max_y = max_yy[0];
		//return true;
	}
	else
	{
		if (found_contours.size() == 0)
			return false;
		else
		{
			vector<vector <int>> pt(found_contours.size());
			for (int j = 0; j < found_contours.size(); j++)
			{
				vector<Point> C = found_contours[j];
				for (int k = 0; k < C.size(); k++)
				{
					pt[j].push_back(C[k].y);
				}
			}
			pair <int, int> Pair1;
			vector <pair <int, int>>Max_y(pt.size());
			int needed_contour;
			for (int i = 0; i < pt.size(); i++)
			{
				Max_y[i].first = *max_element(pt[i].begin(), pt[i].end());
				Max_y[i].second = i;
			}
			std::sort(Max_y.begin(), Max_y.end());
			contour_tool = found_contours[Max_y[Max_y.size()-1].second];
			*max_y = Max_y[Max_y.size()-1].first;
			//return true;
		}
	}
	// Process the contour to remove endoscopy part
}

void ToolsTrackingApp::segmentation_ring(const vector<Point> &contour_tool, Mat& fgMaskRing)
{
	vector<vector<Point> > contours_ring;
	vector<Vec4i> hierarchy_ring;
	vector<ContourFeature> features_ring;
	Mat hsv;
	cvtColor(frame, hsv, CV_BGR2HSV);

	Mat channel[3];
	split(hsv, channel);


	//Mat fgMaskRing;
	threshold(channel[0], fgMaskRing, 10, 255, THRESH_TOZERO_INV); //b1((b1 >= T))= 0; 
	threshold(fgMaskRing, fgMaskRing, 1, 255, THRESH_BINARY); //b1((b1 > 0))= 255;


	erode(fgMaskRing, fgMaskRing, element[0]);
	erode(fgMaskRing, fgMaskRing, element[0]);
	dilate(fgMaskRing, fgMaskRing, element[1]);
	dilate(fgMaskRing, fgMaskRing, element[1]);
	dilate(fgMaskRing, fgMaskRing, element[1]);


	////Have to change this hardcoding for ring segmentation
	//inRange(hsv, Scalar(100, 0, 40), Scalar(120, 255, 90), fgMaskRing_R);
	//inRange(hsv, Scalar(100, 150, 20), Scalar(110, 255, 200), fgMaskRing_B);
	//inRange(hsv, Scalar(110, 0, 0), Scalar(130, 255, 255), fgMaskRing_O);


	//dilate(fgMaskRing, fgMaskRing, element[3]);
	//erode(fgMaskRing, fgMaskRing, element[2]);
	//dilate(fgMaskRing, fgMaskRing, element[3]);


	/* Have to uncomment later
	//erode(fgMaskRing_R, fgMaskRing_R, element[0]);
	//erode(fgMaskRing_R, fgMaskRing_R, element[0]);
	//dilate(fgMaskRing_R, fgMaskRing_R, element[1]);
	//dilate(fgMaskRing_R, fgMaskRing_R, element[1]);

	//erode(fgMaskRing_B, fgMaskRing_B, element[0]);
	//erode(fgMaskRing_B, fgMaskRing_B, element[0]);
	//dilate(fgMaskRing_B, fgMaskRing_B, element[1]);
	//dilate(fgMaskRing_B, fgMaskRing_B, element[1]);

	//erode(fgMaskRing_O, fgMaskRing_O, element[0]);
	//erode(fgMaskRing_O, fgMaskRing_O, element[0]);
	//dilate(fgMaskRing_O, fgMaskRing_O, element[1]);
	//dilate(fgMaskRing_O, fgMaskRing_O, element[1]);
	*/


	//Mat mask_tool = Mat::zeros(frame.rows, frame.cols, CV_8UC1);
	//vector<vector<Point> >contour_tools;
	//contour_tools.push_back(contour_tool);
	//drawContours(mask_tool, contour_tools, 0, 255, CV_FILLED);
	//Mat intersection_tool_rings = mask_tool & fgMaskRing_R;

	Mat fgMaskRing_copy = fgMaskRing.clone();

	// Here also we can process the ring


	imshow("seg-ring", fgMaskRing);
	//imshow("seg-ring-1", mask_tool);
	//imshow("seg-ring-1", fgMaskRing_R);
	//imshow("seg-ring-2", fgMaskRing_B);
	//imshow("seg-ring-3", fgMaskRing_O);
}


void ToolsTrackingApp::convertToFloat(const vector<Point>& intVect, vector<Point2f>& floatVect)
{
	for (int i = 0; i < intVect.size(); i++)
	{
		floatVect.push_back(intVect[i]);
	}
}


void ToolsTrackingApp::convertToInt(const vector<Point2f>& floatVect, vector<Point>& intVect)
{
	for (int i = 0; i < floatVect.size(); i++)
	{
		intVect.push_back(floatVect[i]);
	}
}