#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highgui.h"
#include <stdlib.h>
#include <stdio.h>

using namespace cv;

/// Global variables
Mat src, erosion_dst, dilation_dst;

int erosion_elem = 0;
int erosion_size = 0;
int dilation_elem = 0;
int dilation_size = 0;
int const max_elem = 2;
int const max_kernel_size = 21;

/** Function Headers */
void Demo( int, void* );

/** @function main */
int main( int argc, char** argv )
{
  /// Load an image
  src = imread( argv[1] );

  if( !src.data )
  { return -1; }

  /// Create windows
  namedWindow( "Demo", CV_WINDOW_NORMAL );
  cvMoveWindow( "Dilation Demo", src.cols, 0 );

  /// Create Erosion Trackbars
  createTrackbar( "Erode kernel size:\n 2n +1", "Demo",
                  &erosion_size, max_kernel_size,
                  Demo );
  createTrackbar( "Dilate kernel size:\n 2n +1", "Demo",
                  &dilation_size, max_kernel_size,
                  Demo );

  /// Default start
  Demo( 0, 0 );

  waitKey(0);
  return 0;
}

/** @function Dilation */
void Demo( int, void* )
{
  Mat element = getStructuringElement( MORPH_RECT,
                                       Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                       Point( dilation_size, dilation_size ) );
  /// Apply the dilation operation
  dilate( src, dilation_dst, element );
  erode( dilation_dst, erosion_dst, element );
  imshow( "Demo", erosion_dst );
}