#include "opencv2/opencv.hpp"
#include <highgui.h>
// #include "opencv2/video/tracking.hpp"
// #include "opencv2/imgproc/imgproc.hpp"
// #include "opencv2/highgui/highgui.hpp"
// #include <iostream>
// #include <fstream>
// #include <stdio.h>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	//global variables
	Mat img_src, img_grey, imgBGR[3], imgR, img_lab, img_proc, img_proc_2;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	// read image
	img_src = imread(argv[1], IMREAD_ANYCOLOR );
	
	//convert colour space
	cvtColor(img_src, img_grey, CV_BGR2GRAY); //or CV_64FC3
//  cvtColor(img, img_lab, CV_BGR2Lab);
	
// 	//split by channel and select red channel
	split(img_src, imgBGR);
	imgR = imgBGR[2];
	
	///image pre-processing
// 	cv::blur(img, img, cv::Size(9,9));
// 	cv::GaussianBlur(img_grey, img_proc, cv::Size(25,25), 0 );
	cv::adaptiveThreshold(img_grey, img_proc, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 41, 15);
	dilate(img_proc, img_proc, cv::Mat(cv::Size(9,9), CV_8U, cv::Scalar(255)));	
	erode(img_proc, img_proc, cv::Mat(cv::Size(5,5), CV_8U, cv::Scalar(255)));
// 	cv::adaptiveThreshold(img, bw, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, block_size, constant);
// 	cv::threshold(bw, bw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	///find largest contour
	img_proc.copyTo(img_proc_2);
	//find all contours
	cv::findContours(img_proc_2, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	//find the single largest contour
	int contourMax = 0;
	int contourMaxID = 0;
	for(int k = 0; k < (int) contours.size(); k++){
		if ((contours[k]).size() > contourMax){
			contourMax	= (contours[k]).size();
			contourMaxID = k; 
		}
	}
	//extract main contour
	Mat mask(img_src.size(), CV_8U, cv::Scalar(0));
	drawContours(mask, contours, contourMaxID, cv::Scalar(255), CV_FILLED);
	//apply mask
	Mat img_masked;
	imgR.copyTo(img_masked, mask);
	
	///bin
/*
	// pre-processing
	blur(imgR, imgR, Size(3,3) );
	
	int erosion_size = 6;  
    Mat element = getStructuringElement(cv::MORPH_CROSS, cv::Size(2 * erosion_size + 1, 2 * erosion_size + 1), cv::Point(erosion_size, erosion_size) );
	dilate(imgR, bw, element);
	
	adaptiveThreshold(bw, bw, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY_INV, 201, 10);
	
	// apply Otsu thresholding
// 	threshold(imgR, bw, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		
// 	cv::adaptiveThreshold(imgR, imgR, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 201, 10);
	
	// Detect edges using canny
	Canny(imgR, canny_output, 140, 140*2, 3 );
	// Find contours
	findContours(canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	
*/

// 	///detect spots
// 	//blur
// 	cv::medianBlur(img_masked, img_masked, 7);
//     // Perform canny edge detection
//     cv::Canny(img_masked, img_masked, 33, 100);
	
	
	//write image mask
	cv::imwrite("mask.jpg", img_masked);
	return 1;
	
}