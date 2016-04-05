#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	Mat img_color, img_lab, img_thresh, img_open, img_close, img_keypoints, img_BGR[3], img_red;

	// Load source image and convert it to gray
	img_color = imread(argv[1], IMREAD_ANYCOLOR );
	//Convert image to CIE Lab colorspace for better colour based segmentation
    cvtColor(img_color, img_lab, CV_BGR2Lab);
	
	split(img_color, img_BGR);
	img_red = img_BGR[2];
	
	namedWindow("win_thresh", WINDOW_NORMAL);
//     namedWindow("win_blob", WINDOW_NORMAL);

    //Using trackbar calculate the range of L,a,b values to seperate blobs
    int low_L = 40, low_A = 0, low_B = 141,
        high_L = 255, high_A = 255, high_B = 255;

    //*Use trackbars to calibrate colour thresholding
    createTrackbar("low_L", "win_thresh", &low_L, 255);
    createTrackbar("low_A", "win_thresh", &low_A, 255);
    createTrackbar("low_B", "win_thresh", &low_B, 255);
    createTrackbar("high_L", "win_thresh", &high_L, 255);
    createTrackbar("high_A", "win_thresh", &high_A, 255);
    createTrackbar("high_B", "win_thresh", &high_B, 255);

    int minArea = 35, maxArea = 172, minCircularity = 58, minConvexity = 87, minInertiaRatio = 21, minDistance = 20;

    //Use trackbar and set blob detector parameters
    createTrackbar("minArea", "win_blob", &minArea, 1000);
    createTrackbar("maxArea", "win_blob", &maxArea, 10000);
    createTrackbar("minCircular", "win_blob", &minCircularity, 99);
    createTrackbar("minConvex", "win_blob", &minConvexity, 99);
    createTrackbar("minInertia", "win_blob", &minInertiaRatio, 99);
	createTrackbar("minDistance", "win_blob", &minDistance, 100);

    SimpleBlobDetector::Params params;
    vector<KeyPoint> keypoints;
	
	while (waitKey(1) != 27) //press 'esc' to quit
    {
        //inRange thresholds basedon the Scalar boundaries provided
        inRange(img_lab, Scalar(low_L, low_A, low_B), Scalar(high_L, high_A, high_B), img_thresh);

// 		imshow("win_thresh", img_thresh);
				
        //Morphological filling
        Mat strucElement = getStructuringElement(CV_SHAPE_ELLIPSE, Size(5, 5), Point(2, 2));
        morphologyEx(img_thresh, img_close, MORPH_CLOSE, strucElement);

        imshow("win_thresh", img_close);

        //**SimpleBlobDetector works only in inverted binary images
        //i.e.blobs should be in black and background in white.
        bitwise_not(img_close, img_close); // inverts matrix

        //Code crashes if minArea or any miin value is set to zero
        //since trackbar starts from 0, it is adjusted here by adding 1
        params.filterByArea = true;
        params.minArea = minArea + 1;
        params.maxArea = maxArea + 1;

        params.filterByCircularity = true;
        params.filterByConvexity = true;
        params.filterByInertia = true;

        params.minCircularity = (minCircularity + 1);
        params.minConvexity = (minConvexity + 1);
        params.minInertiaRatio = (minInertiaRatio + 1);
		params.minDistBetweenBlobs = minDistance;

        SimpleBlobDetector detector(params);
        detector.detect(img_close, keypoints);
        drawKeypoints(img_color, keypoints, img_keypoints, Scalar(0, 0, 255), DrawMatchesFlags::DEFAULT);

//         stringstream displayText;
//         displayText = stringstream();
//         displayText << "Blob_count: " << keypoints.size();
//         putText(img_keypoints, displayText.str(), Point(0, 50), CV_FONT_HERSHEY_PLAIN, 2, Scalar(0, 0, 255), 2);

//         imshow("win_blob", img_keypoints);
    }
}