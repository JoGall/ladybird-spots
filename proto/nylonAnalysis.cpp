/*********************************************************** 
* 
* 04/02/15 by Joe Gallagher (joedgallagher@gmail.com)
* 
* This script analyses images of nylon monofilaments to calculate 
* the area of encapsulating material (number of 'black' pixels), 
* overall level of melanisation (mean brightness) and length of nylon.
* 
* This assay requires a user-defined brightness threshold to subjectively 
* define areas of encapsulation. For an example of calibration, see the R 
* script, "calibrateThreshold.R".
* 
* During the calibration phase, users can input a range of brightness 
* thresholds (between lower_threshold and upper_threshold values) to be 
* iterated over. During final usage (only one threshold), 
* lower_threshold should equal the desired threshold and upper_threshold 
* should equal lower_threshold+1.
* For example, For a threshold of 99, command line usage would be:
* './nylon 99 100 *.jpg'
* 
* Calculated values are outputted to "results.txt" and an image of 
* the mask outputted to "mask_FILENAME.jpg"
*
* For general usage examples, see "README.txt".
* 
***********************************************************/

#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <stdio.h>

/*compilation example using 	GNU C++ compiler*/
//g++ -ggdb `pkg-config --cflags opencv` -o ./nylon ./nylonAnalysis.cpp `pkg-config --libs opencv`


//function to calculate encapsulation area (number of pixels in mask below user-defined brightness threshold)
int calcEncapsulation(cv::Mat input, cv::Mat mask, int threshold){
	int nPixels = 0;
	for(int x = 0; x<input.cols; x++){
            for(int y = 0; y<input.rows; y++){
            	if((uchar) (mask.at<uchar>((int)y,(int)x)) > 0)
					if((uchar) (input.at<uchar>((int)y,(int)x)) > threshold)
					nPixels++;
            }
        }
	return nPixels;
}

//main function
int main( int argc, char** argv ){
	
	//error handling
	if (argc<4) {
		std::cout << "Error!\nUsage is: " <<argv[0]<< " lower_threshold upper_threshold file_1 file_2..." << std::endl;
		return 0;
	} else {
		int no1 = 0, no2 = 0, ret = 0;
	
		ret = sscanf(argv[1], "%d", &no1);
        if (ret != 1) return 0;
        ret = sscanf(argv[2], "%d", &no2);
        if (ret != 1) return 0;          

        if ((no1<0) || (no1>256)) {
			std::cout << "Error!\nlower_threshold & upper_threshold must be an integer between 0-255" << std::endl;
			return 0;
		}
        if ((no2<0) || (no2>256)) {
			std::cout << "Error!\nlower_threshold & upper_threshold must be an integer between 0-255" << std::endl;
			return 0;
		}
	}
	
	//parse thresholding options
	long threshLower, threshUpper;
	threshLower = strtol(argv[1], NULL, 10);
	threshUpper = strtol(argv[2], NULL, 10);
	
	//output results headers
	std::ofstream fout("results.txt");
	fout << "Image" <<"\t"<< "meanB" <<"\t"<< "meanG" <<"\t"<< "meanR" <<"\t"<< "meanHue" <<"\t"<< "meanSat" <<"\t"<< "meanValue" <<"\t"<< "encapArea" <<"\t"<< "totalSurfArea" <<"\t"<< "encapThreshold" <<"\t"<< "nylonLength" << std::endl;

	//fileloop
	for(int i=3; i<argc; i++){
		//create storage
		cv::Mat input, imgGreyFirst, imgGrey, imgHue, output;
		
		//read and convert image
		input=cv::imread(argv[i]);
		cvtColor(input, imgGrey, CV_BGR2GRAY);
		imgGrey.copyTo(imgGreyFirst);
		cvtColor(input, imgHue, CV_RGB2HSV);
		
		//image pre-processing
	   	cv::blur(imgGrey , imgGrey, cv::Size(9,9));
		cv::adaptiveThreshold(imgGrey, imgGrey, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, 201, 10);
		dilate(imgGrey, imgGrey, cv::Mat(cv::Size(25,25), CV_8U, cv::Scalar(255)));
		erode(imgGrey, imgGrey, cv::Mat(cv::Size(25,25), CV_8U, cv::Scalar(255)));

		//find contours of binary image mask
		std::vector<std::vector<cv::Point> > contours;
		cv::findContours(imgGrey, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

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
		cv::Mat mask(imgGreyFirst.size(), CV_8U, cv::Scalar(0));
		cv::drawContours(mask, contours, contourMaxID, cv::Scalar(255), CV_FILLED);

		//find length of largest contour
		std::vector<cv::Point> mainContour = contours[contourMaxID];
		cv::Point pointA, pointB;
		int lengthMax = 0;
		int distance = 0;
		for(int length1 = 0; length1 < mainContour.size(); length1++){
			for(int length2=0; length2 < mainContour.size(); length2++){
				if(((mainContour[length1].x-mainContour[length2].x)*(mainContour[length1].x-mainContour[length2].x)) + ((mainContour[length1].y-mainContour[length2].y)*(mainContour[length1].y-mainContour[length2].y)) > lengthMax){
					lengthMax = (((mainContour[length1].x-mainContour[length2].x)*(mainContour[length1].x-mainContour[length2].x)) + ((mainContour[length1].y-mainContour[length2].y)*(mainContour[length1].y-mainContour[length2].y)));
					pointA.x = mainContour[length1].x;
					pointA.y = mainContour[length1].y;
					pointB.x = mainContour[length2].x;
					pointB.y = mainContour[length2].y;
				}
			}
		}	
		int nylonLength = std::sqrt(lengthMax);

		//calculate overall melanisation levels (mean BGR and hue values)
		cv::Scalar meansBGR = cv::mean(input, mask);
		cv::Scalar meansHue = cv::mean(imgHue, mask);
		int totalSurfaceArea = cv::countNonZero(mask);

		//loop for testing threshold values
		for(int thresh = threshLower; thresh < threshUpper; thresh++){
			
			//calculate encapsulation area (n pixels below threshold brightness value)
			int encapsulationArea = totalSurfaceArea - (calcEncapsulation(imgGreyFirst, mask, thresh));
			
			//output results to text file
			fout << argv[i] <<"\t"<< (int)meansBGR[0] <<"\t"<< (int)meansBGR[1] <<"\t"<< (int)meansBGR[2] <<"\t"<< (int)meansHue[0] <<"\t"<< (int)meansHue[1] <<"\t"<< (int)meansHue[2] <<"\t"<< encapsulationArea <<"\t"<< totalSurfaceArea <<"\t"<< thresh <<"\t"<< nylonLength << std::endl;
			
			//output image mask if only one encapsulation threshold defined, otherwise just return text file
			if(threshUpper - threshLower < 2) {
				//draw main contour in grey, threshold original image, and draw encapsulation area in black
				cv::Mat crop(imgGreyFirst.size(), CV_8U, cv::Scalar(0));
				cv::Mat outputImg(imgGreyFirst.size(), CV_8U, cv::Scalar(255));
				imgGreyFirst.copyTo(crop, mask);
				cv::threshold(crop, crop, thresh, 127, cv::THRESH_BINARY);
				crop.copyTo(outputImg, mask);
				
				//convert image to colour and draw nylon length as red line
				cvtColor(outputImg, outputImg, CV_GRAY2RGB);
				cv::line(outputImg, pointA, pointB, cv::Scalar(0,0,255), 5, 8);
				
				//output image
				std::ostringstream text;
				text<<"mask_"<<argv[i];
				cv::imwrite(text.str(), outputImg);	
			}		
		}
	}
	return 1;
}