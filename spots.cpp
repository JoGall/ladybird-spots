#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <highgui.h>
#include <string>
// #include <boost/filesystem.hpp>

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
	
	//error handling
	if (argc<3) {
		std::cout << "Error!\nUsage is: " <<argv[0]<< " n_elytra file_1 file_2..." << std::endl;
		return 0;
	}
	
	//print results header
		cout << "filename" <<"\t"<< "elytron_id" <<"\t"<< "n_spots" <<"\t"<< "spot_px" <<"\t"<< "elytron_px" <<"\t"<< "elytron_H" <<"\t"<< "elytron_W" << endl;
	
	//FILELOOP
	for(int i=2; i<argc; i++){
		
		//STORAGE
		Mat img_src, img_grey, img_hsv, img_blur, img_thresh, img_keypoints, img_spots;
		vector<KeyPoint> keypoints;
		vector<vector<Point> > contours;
		
		//READ IMAGE AND CONVERT COLOURSPACE
		img_src = imread(argv[i], CV_RGB2BGR);
		cvtColor(img_src, img_grey, CV_BGR2GRAY);
		Mat img_bgr[3];
		split(img_src, img_bgr);
			
		///FIND ALL ELYTRA
		//PROCESS IMAGE
		GaussianBlur(img_bgr[0], img_blur, Size(99,99), 0);
// 		blur(img_grey, img_blur, Size(9,9));
		adaptiveThreshold(img_blur, img_thresh, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 99, 6);
		//dilate and erode
// 		dilate(img_thresh, img_thresh, Mat(Size(15,15), CV_8U, Scalar(255)));
// 		erode(img_thresh, img_thresh, Mat(Size(15,15), CV_8U, Scalar(255)));

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
		for (int j = 0; j < n_elytra; ++j) {
			
			//draw elytron contour and mask original image
			Mat img_contour(img_grey.size(), CV_8U, Scalar(0));
			Mat img_mask(img_grey.size(), CV_8U, Scalar(255));
// 			Mat img_src_mask(img_src.size(), CV_32F, Scalar(255));
			
			drawContours(img_contour, contours, contourIndices[j], Scalar(255), CV_FILLED);
			img_bgr[2].copyTo(img_mask, img_contour);
// 			img_src.copyTo(img_src_mask, img_contour);
			
			//find spot contours
			Mat mask_blur, mask_thresh;
			GaussianBlur(img_mask, mask_blur, Size(49,49), 0);
			adaptiveThreshold(mask_blur, mask_thresh, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 299, 25);
// 			threshold(img_mask, mask_thresh, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
// 			adaptiveThreshold(img_mask, mask_thresh, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 11, 2);
			
			///COUNT SPOT PIXELS AND ELYTRON PIXELS
			int mask_pixels = cv::countNonZero(img_contour);
			int spot_pixels = countDark(img_grey, img_contour, 80);
			
			///COUNT N SPOTS WITH BLOB DETECTION
			//define parameters
			SimpleBlobDetector::Params params;
			params.filterByArea = true;
			params.minArea = 2000;
			params.maxArea = 80000;
			params.minDistBetweenBlobs = 10;
			params.minRepeatability = 1;
// 			params.filterByColor = true;
// 			params.minThreshold = minThresh_slider;
// 			params.maxThreshold = maxThresh_slider;
			params.filterByCircularity = true;
			params.minCircularity = 0.2;
			params.maxCircularity = 1;
			params.filterByConvexity = true;
			params.minConvexity = 0.65;
			params.maxConvexity = 1;
			params.filterByInertia = true;
			params.minInertiaRatio = 0.1;
			params.maxInertiaRatio = 1;
			
			//detect
			SimpleBlobDetector detector(params);
			detector.detect( img_mask, keypoints);

			///CREATE OUTPUT IMAGE
			//draw elytron contour (in grey against white background)
			Mat img_output(img_grey.size(), CV_8U, Scalar(0));
			drawContours(img_output, contours, contourIndices[j], Scalar(200), CV_FILLED);
			bitwise_not(img_output, img_output);
			//add spots in black
			threshold(img_mask, img_spots, 80, 255, CV_THRESH_BINARY);
			addWeighted(img_output, 0.2, img_spots, 0.8, 0, img_output);
			//draw blob locations
			drawKeypoints( img_output, keypoints, img_output, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
			//draw minimum enclosing rectangle to elytron contour
			RotatedRect minRect = minAreaRect(Mat(contours[contourIndices[j]]));
			Point2f vertices[4];
			minRect.points(vertices);
			for (int k = 0; k < 4; k++) {
				line(img_output, vertices[k], vertices[(k+1)%4], Scalar(0,0,255),5);
			}
			//find rightmost point of elytron contour
			Point extRight = *max_element(contours[contourIndices[j]].begin(), contours[contourIndices[j]].end(), [](const Point& lhs, const Point& rhs) {
				return lhs.x < rhs.x;
			});
			//add text for elytron id
			string elytron_id = to_string(j);
			putText(img_output, elytron_id, extRight, FONT_HERSHEY_SIMPLEX, 8, Scalar(0,0,0), 40, 4);
			//make image name
			string FileName = argv[i];
			FileName.erase(FileName.find(".jpg"));
			std::ostringstream text;
			text << FileName << "_mask" << j << ".jpg";
			//write image
			imwrite(text.str(), img_output);
			
			///OUTPUT DATA
			cout << FileName <<"\t"<< j <<"\t"<< keypoints.size() <<"\t"<< spot_pixels <<"\t"<< mask_pixels <<"\t"<< minRect.size.height <<"\t"<< minRect.size.width << endl;
		
		}
	}
	return 1;
}