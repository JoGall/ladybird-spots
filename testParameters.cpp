#include "opencv2/opencv.hpp"
#include <highgui.h>
using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	// global variables
	Mat img_src, img_keypoints;
	vector<KeyPoint> keypoints;
			
	//initialise values
	int minThresh_slider = 0, maxThresh_slider = 96, blobColor_slider = 0, minArea_slider = 1000, maxArea_slider = 8000, minDistBetweenBlobs_slider = 10, minRepeatability_slider = 1, i_minCircularity_slider = 200, i_maxCircularity_slider = 1000, i_minConvexity_slider = 650, i_maxConvexity_slider = 1000, i_minInertiaRatio_slider = 100, i_maxInertiaRatio_slider = 1000;
	
	// read image
	img_src = imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE );

	// create image window
	namedWindow("sliders", WINDOW_NORMAL);
	namedWindow("output", WINDOW_NORMAL);
	
	// create trackbars
	createTrackbar( "Min threshold", "sliders", &minThresh_slider, 255 );
	createTrackbar( "Max threshold", "sliders", &maxThresh_slider, 255 );
 	createTrackbar( "Colour", "sliders", &blobColor_slider, 255 );
	
	createTrackbar( "Min area", "sliders", &minArea_slider, 10000 );
	createTrackbar( "Max area", "sliders", &maxArea_slider, 10000 );
	
	createTrackbar( "Min distance", "sliders", &minDistBetweenBlobs_slider,  1000);
	createTrackbar( "Min repeatability", "sliders", &minRepeatability_slider,  10);
	
	createTrackbar( "Min circularity", "sliders", &i_minCircularity_slider, 1000 );
	createTrackbar( "Max circularity", "sliders", &i_maxCircularity_slider, 1000 );
	createTrackbar( "Min convexity", "sliders", &i_minConvexity_slider, 1000 );
	createTrackbar( "Max convexity", "sliders", &i_maxConvexity_slider, 1000 );
	createTrackbar( "Min inertia ratio", "sliders", &i_minInertiaRatio_slider, 1000 );
	createTrackbar( "Max inertia ratio", "sliders", &i_maxInertiaRatio_slider, 1000 );
	
	///blob detector
	while (waitKey(1) != 27)
	{
		//convert integers to doubles
		double minCircularity_slider = i_minCircularity_slider / 1000.0;
		double maxCircularity_slider = i_maxCircularity_slider / 1000.0;
		double minConvexity_slider = i_minConvexity_slider / 1000.0;
		double maxConvexity_slider = i_maxConvexity_slider / 1000.0;
		double minInertiaRatio_slider = i_minInertiaRatio_slider / 1000.0;
		double maxInertiaRatio_slider = i_maxInertiaRatio_slider / 1000.0;
		
		//define parameters
		SimpleBlobDetector::Params params;
		
		params.filterByArea = true;
		params.minArea = minArea_slider;
		params.maxArea = maxArea_slider;
		
		params.minDistBetweenBlobs = minDistBetweenBlobs_slider;
		params.minRepeatability = minRepeatability_slider;
				
		params.filterByColor = true;
		params.minThreshold = minThresh_slider;
		params.maxThreshold = maxThresh_slider;
	 	params.blobColor = blobColor_slider;
		
		params.filterByCircularity = true;
		params.minCircularity = minCircularity_slider;
		params.maxCircularity = maxCircularity_slider;
		
		params.filterByConvexity = true;
		params.minConvexity = minConvexity_slider;
		params.maxConvexity = maxConvexity_slider;
		
		params.filterByInertia = true;
		params.minInertiaRatio = minInertiaRatio_slider;
		params.maxInertiaRatio = maxInertiaRatio_slider;
		
		// detect blobs
		SimpleBlobDetector detector(params);
		detector.detect( img_src, keypoints);

		// draw detected blobs as red circles
		drawKeypoints( img_src, keypoints, img_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
		
		imshow( "output", img_keypoints );
	}
	
	///output
	//results headers
	cout << "filename" <<"\t"<< "elytron_id" <<"\t"<< "n_spots" <<"\t"<< "spot_area" <<"\t"<< "elytron_area" << endl;
	
	//results
	cout << argv[1] <<"\t"<< "TODO" <<"\t"<< keypoints.size() <<"\t"<< "TODO" <<"\t"<< "TODO" << endl;
	
	//write image mask
	cv::imwrite("mask.jpg", img_keypoints);
	return 1;

}