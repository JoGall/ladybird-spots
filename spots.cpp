#include "opencv2/opencv.hpp"
#include <highgui.h>
// #include <std_lib_facilities.h>
using namespace cv;
using namespace std;

//function to count number of pixels in mask below user-defined brightness threshold
int countDark(Mat input, Mat mask, int threshold){
	int nPixels = 0;
	for(int x = 0; x<input.cols; x++){
            for(int y = 0; y<input.rows; y++){
            	if((uchar) (mask.at<uchar>((int)y,(int)x)) > 0)
					if((uchar) (input.at<uchar>((int)y,(int)x)) < threshold)
					nPixels++;
            }
        }
	return nPixels;
}

///MAIN
int main( int argc, char** argv ) {
	
	//STORAGE
	Mat img_src, img_grey, img_blur, img_thresh, img_keypoints, img_spots, img_output2;
	vector<KeyPoint> keypoints;
	vector<vector<Point> > contours;
	
	//READ IMAGE AND CONVERT
	img_src = imread(argv[2], 1);
	cvtColor(img_src, img_grey, CV_BGR2GRAY);
	
	//PRINT RESULTS HEADER
	cout << "filename" <<"\t"<< "elytron_id" <<"\t"<< "n_spots" <<"\t"<< "spot_px" <<"\t"<< "elytron_px" <<"\t"<< "elytron_H" <<"\t"<< "elytron_W" << endl;
		
	///FIND ALL ELYTRA
	//PROCESS IMAGE
	blur(img_grey, img_blur, Size(9,9));
	threshold(img_blur, img_thresh, 155, 255, CV_THRESH_BINARY_INV);
// 	bilateralFilter(img_grey, img_blur, 25, 50,50);
// 	adaptiveThreshold(img_grey, img_thresh, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 201,10);

	//FIND ELYTRA CONTOURS
	findContours(img_thresh.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	//sort contours by size
	vector<int> contourIndices(contours.size());
	iota(contourIndices.begin(), contourIndices.end(), 0);
	sort(contourIndices.begin(), contourIndices.end(), [&contours](int lhs, int rhs) {
        return contours[lhs].size() > contours[rhs].size();
    });
	//set number of elytra to use
    int n_elytra = strtol(argv[1], NULL, 10);
    n_elytra = min(n_elytra, int(contours.size()));

	
	///ELYTRA LOOP
    for (int i = 0; i < n_elytra; ++i) {
		
		//DRAW CONTOUR AND MASK ORIGINAL IMAGE
		Mat img_contour(img_grey.size(), CV_8U, Scalar(0));
		drawContours(img_contour, contours, contourIndices[i], Scalar(255), CV_FILLED);
		Mat img_mask(img_grey.size(), CV_8U, Scalar(255));
		img_grey.copyTo(img_mask, img_contour);
		
		///COUNT SPOT PIXELS AND ELYTRON PIXELS
		int mask_pixels = cv::countNonZero(img_contour);
		int spot_pixels = countDark(img_grey, img_contour, 80);
		
		///COUNT N SPOTS
		//INITIALISE VALUES
		double minThresh_slider = 0, maxThresh_slider = 96, minArea_slider = 1000, maxArea_slider = 8000, minDistBetweenBlobs_slider = 10, minRepeatability_slider = 1, minCircularity_slider = 0.2, maxCircularity_slider = 1, minConvexity_slider = 0.65, maxConvexity_slider = 1, minInertiaRatio_slider = 0.1, maxInertiaRatio_slider = 1;
			
		//DEFINE PARAMETERS
		SimpleBlobDetector::Params params;
		params.filterByArea = true;
		params.minArea = minArea_slider;
		params.maxArea = maxArea_slider;
		params.minDistBetweenBlobs = minDistBetweenBlobs_slider;
		params.minRepeatability = minRepeatability_slider;
		params.filterByColor = true;
		params.minThreshold = minThresh_slider;
		params.maxThreshold = maxThresh_slider;
		params.filterByCircularity = true;
		params.minCircularity = minCircularity_slider;
		params.maxCircularity = maxCircularity_slider;
		params.filterByConvexity = true;
		params.minConvexity = minConvexity_slider;
		params.maxConvexity = maxConvexity_slider;
		params.filterByInertia = true;
		params.minInertiaRatio = minInertiaRatio_slider;
		params.maxInertiaRatio = maxInertiaRatio_slider;
		
		//DETECT
		SimpleBlobDetector detector(params);
		detector.detect( img_mask, keypoints);

		///CREATE OUTPUT IMAGE
		//draw elytron contour (in grey against white background)
		Mat img_output(img_grey.size(), CV_8U, Scalar(0));
		drawContours(img_output, contours, contourIndices[i], Scalar(200), CV_FILLED);
		bitwise_not(img_output, img_output);
		//add spots in black
		threshold(img_mask, img_spots, 80, 255, CV_THRESH_BINARY);
		addWeighted(img_output, 0.2, img_spots, 0.8, 0, img_output);
		//draw blob locations
		drawKeypoints( img_output, keypoints, img_output, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
		//draw minimum enclosing rectangle to elytron contour
		RotatedRect minRect = minAreaRect(Mat(contours[contourIndices[i]]));
		Point2f vertices[4];
		minRect.points(vertices);
		for (int j = 0; j < 4; j++) {
			line(img_output, vertices[j], vertices[(j+1)%4], Scalar(0,0,255),5);
		}
		//find rightmost point of elytron contour
		Point extRight = *max_element(contours[contourIndices[i]].begin(), contours[contourIndices[i]].end(), [](const Point& lhs, const Point& rhs) {
			return lhs.x < rhs.x;
        });
		//add text for elytron id
		string elytron_id = to_string(i);
		putText(img_output, elytron_id, extRight, FONT_HERSHEY_SIMPLEX, 8, Scalar(0,0,0), 40, 4);
		
		///OUTPUTS
		//PRINT RESULTS
		cout << argv[2] <<"\t"<< i <<"\t"<< keypoints.size() <<"\t"<< spot_pixels <<"\t"<< mask_pixels <<"\t"<< minRect.size.height <<"\t"<< minRect.size.width << endl;
		
		//WRITE IMAGE MASK
		ostringstream text;
		text << "mask_" << i << ".jpg";
		imwrite(text.str(), img_output);
	}
	
	return 1;
	
}