#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <highgui.h>

using namespace cv;
using namespace std;

/*
// THRESHOLDING DEFAULTS
// GaussianBlur block_size = 99
// adaptiveThreshold block_size = 99
// adaptiveThreshold constant = 6
*/

//function counts number of pixels below the absolute threshold in a masked image
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

//global argument storage
struct opts_t {
    int inpaint_flag;           // optional -i flag
	int n_elytra;				// mandatory -e parameter
    char **inputFiles;          // input files
    int numInputFiles;          // # of input files
} opts;
static const char *optString = "ie:h?";


//main function
int main( int argc, char** argv ) {
	
	///OPTION AND ERROR HANDLING
	//if no options supplied
	if(argc<2) {
		fprintf(stderr, "\nError: no options supplied\n\n");
		return(0);
	}
	
    //initialize opts
    int opt = 0;
    opts.inpaint_flag = 0;
    opts.n_elytra;
    opts.inputFiles = NULL;
    opts.numInputFiles = 0;
	
	opt = getopt( argc, argv, optString );
    while( opt != -1 ) {
		switch( opt ) {
            case 'i':
                opts.inpaint_flag = 1;
                break;
            case 'e':
                opts.n_elytra = atoi(optarg);
                break;
            case 'h':
			case '?':
				fprintf(stderr, "\nUSAGE:\n\t%s [-i] -e N file1.jpg file2.jpg ...\n\nOPTIONS:\n\t-i\timplements impainting (interpolation of occluding glare)\n\t-e N\tnumber of elytra to find in images (required)\n\n",
                    argv[0]);
				return(0);
			default:
                break;
        }
        opt = getopt( argc, argv, optString );
    }
    
    opts.inputFiles = argv + optind;
    opts.numInputFiles = argc - optind;
	
	//if no image files supplied
	if(opts.numInputFiles<1) {
		fprintf(stderr, "\nError: expected image file(s) after options\n\n");
		return(0);
	}
	
	
	///IMAGE PROCESSING
	//print results header
	std::ofstream fout("results.txt");
	fout << "filename" <<"\t"<< "elytron_id" <<"\t"<< "n_spots" <<"\t"<< "spot_px" <<"\t"<< "elytron_px" <<"\t"<< "elytron_H" <<"\t"<< "elytron_W" << endl;
	
	//file loop
	for(int i=optind; i<argc; i++){
		
		//STORAGE
		Mat img_src, img_grey, img_hsv, img_blur, img_thresh, img_keypoints, img_spots;
		vector<KeyPoint> keypoints;
		vector<vector<Point> > contours;
		
		//READ IMAGE AND CONVERT COLOURSPACE
		img_src = imread(argv[i], CV_RGB2BGR);
		cvtColor(img_src, img_grey, CV_BGR2GRAY);
		Mat img_bgr[3];
		split(img_src, img_bgr);
		
		//get filename
		string FileName = argv[i];
		FileName.erase(FileName.find(".jpg"));
		
		///FIND ALL ELYTRA
		//PROCESS IMAGE
		GaussianBlur(img_bgr[0], img_blur, Size(15,15), 0);
// 		blur(img_grey, img_blur, Size(9,9));
		adaptiveThreshold(img_blur, img_thresh, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 99, 5);
		//dilate and erode
		dilate(img_thresh, img_thresh, Mat(Size(9,9), CV_8U, Scalar(255)));
		erode(img_thresh, img_thresh, Mat(Size(9,9), CV_8U, Scalar(255)));

		//FIND ELYTRA CONTOURS
		findContours(img_thresh.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
		//sort contours by size
		vector<int> contourIndices(contours.size());
		iota(contourIndices.begin(), contourIndices.end(), 0);
		sort(contourIndices.begin(), contourIndices.end(), [&contours](int lhs, int rhs) {
			return contours[lhs].size() > contours[rhs].size();
		});
		//set number of elytra to use
		int n_elytra = opts.n_elytra;
		n_elytra = min(n_elytra, int(contours.size()));

		Mat img_output2(img_grey.size(), CV_8UC3, Scalar(0, 0, 0));
		
		///ELYTRA LOOP
		for (int j = 0; j < n_elytra; ++j) {
			
// 			//create mask with white background
			Mat img_contour(img_grey.size(), CV_8U, Scalar(0));
			drawContours(img_contour, contours, contourIndices[j], Scalar(255), CV_FILLED);
			
			///INTERPOLATE LIGHT REFLECTION
			Mat img_mask(img_grey.size(), CV_8U, Scalar(255));
			if(opts.inpaint_flag) {
				Mat mask_red(img_grey.size(), CV_8U, Scalar(255));
				img_bgr[2].copyTo(mask_red, img_contour);
				Mat img_interpol;
// 				//calculate absolute threshold from image histogram
// 				Scalar mean, stddev;
// 				meanStdDev(img_blur2, mean, stddev);
// 				float maxHue = mean[0] + stddev[0]*6;
// 				inRange(img_blur2, Scalar(maxHue), Scalar(255), img_thresh2);
				//adaptive threshold bright pixels
				adaptiveThreshold(mask_red, img_thresh, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 9, 6);
				//dilate, erode and dilate again
				dilate(img_thresh, img_thresh, Mat(Size(9,9), CV_8U, Scalar(255)));
				erode(img_thresh, img_thresh, Mat(Size(25,25), CV_8U, Scalar(255)));
				dilate(img_thresh, img_thresh, Mat(Size(25,25), CV_8U, Scalar(255)));
				//inpaint
				inpaint(mask_red, img_thresh, img_interpol, 5, 1);
				//blur
				medianBlur(img_interpol, img_interpol, 5);		
				//create inpainted mask
				img_interpol.copyTo(img_mask, img_contour);
			}
			else {
				//create raw mask
				img_bgr[2].copyTo(img_mask, img_contour);
			}
			
			///FIND SPOTS
			//threshold spot contours
			Mat mask_blur, mask_thresh;
			GaussianBlur(img_mask, mask_blur, Size(49,49), 0);
			adaptiveThreshold(mask_blur, mask_thresh, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 299, 25);
			
			//count spot pixels and elytron pixels
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

			///CREATE IMAGE MASK TO OUTPUT
			Mat img_output(img_grey.size(), CV_8U, Scalar(0));
			//draw elytron contour (in grey against white background)
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
			putText(img_output, elytron_id, extRight, FONT_HERSHEY_SIMPLEX, 8, Scalar(0,0,255), 30, 3);
			
			//process overall image mask
			bitwise_not(img_output, img_output);
			bitwise_or(img_output, img_output2, img_output2);

			//write elytron mask (for testing)
// // 			Mat img_mask2;
// // 			img_src.copyTo(img_mask2, img_contour);
// 			ostringstream text;
// 			text << FileName << "_mask" << j << ".jpg";
// 			imwrite(text.str(), img_mask);
			
			///OUTPUT DATA
			fout << FileName <<"\t"<< j <<"\t"<< keypoints.size() <<"\t"<< spot_pixels <<"\t"<< mask_pixels <<"\t"<< minRect.size.height <<"\t"<< minRect.size.width << endl;
		}
		
		//write image mask
		ostringstream text;
		text << FileName << "_mask" << ".jpg";
		bitwise_not(img_output2, img_output2);
		imwrite(text.str(), img_output2);
	}
	
	return 1;
	
}