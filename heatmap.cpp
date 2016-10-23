// The "Square Detector" program.
// It loads several images sequentially and tries to find squares in
// each image

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace cv;
using namespace std;

// Image matrices for the pattern matching templates
Mat four, three, two, one, clicked, unclicked, rightclick;

// Image matrices for helper images while template matching
Mat r1, r2, r3, r4, rClick, rUnclick, rRight;

// Image matrix for the cropped grid from the image
Mat cropped;

double risk[16][16];
double ret_ind;
int matchMethod = 0;
int click = 0;

/**
 * @function MatchingMethod
 * @brief Trackbar callback
 */
void MatchingMethod(int, void*) {
	Mat img_display;
	cropped.copyTo( img_display );

	/// Create the resulting matrices
	int result_cols_four =  -cropped.cols + four.cols + 1;
	int result_rows_four = -cropped.rows + four.rows + 1;
	int result_cols_three =  -cropped.cols + three.cols + 1;
	int result_rows_three = -cropped.rows + three.rows + 1;
	int result_cols_two =  -cropped.cols + two.cols + 1;
	int result_rows_two = -cropped.rows + two.rows + 1;
	int result_cols_one =  -cropped.cols + one.cols + 1;
	int result_rows_one = -cropped.rows + one.rows + 1;
	int result_cols_clicked =  -cropped.cols + clicked.cols + 1;
	int result_rows_clicked = -cropped.rows + clicked.rows + 1;
	int result_cols_unclicked =  -cropped.cols + unclicked.cols + 1;
	int result_rows_unclicked = -cropped.rows + unclicked.rows + 1;
	int result_rows_rightclick = -cropped.rows + rightclick.rows + 1;
	int result_cols_rightclick = -cropped.cols + rightclick.cols + 1;

 	r4.create( result_rows_four, result_cols_four, CV_32FC1 );
 	r3.create( result_rows_three, result_cols_three, CV_32FC1 );
 	r2.create( result_rows_two, result_cols_two, CV_32FC1 );
 	r1.create( result_rows_one, result_cols_one, CV_32FC1 );
 	rClick.create( result_rows_clicked, result_cols_clicked, CV_32FC1 );
 	rUnclick.create( result_rows_unclicked, result_cols_unclicked, CV_32FC1 );
	rRight.create( result_rows_rightclick, result_cols_rightclick, CV_32FC1 );
	
	/// Do the Matching and Normalize
  	matchTemplate( cropped, four, r4, matchMethod);
  	matchTemplate( cropped, three, r3, matchMethod); 
	matchTemplate( cropped, two, r2, matchMethod); 
	matchTemplate( cropped, one, r1, matchMethod); 
	matchTemplate( cropped, clicked, rClick, matchMethod); 
	matchTemplate( cropped, unclicked, rUnclick, matchMethod);
	matchTemplate( cropped, rightclick, rRight, matchMethod);

  	/// Localizing the best match with minMaxLoc
  	double minVal4; double maxVal4; Point minLoc4; Point maxLoc4;
  	double minVal3; double maxVal3; Point minLoc3; Point maxLoc3;
	double minVal2; double maxVal2; Point minLoc2; Point maxLoc2;
	double minVal1; double maxVal1; Point minLoc1; Point maxLoc1;
	double minValC; double maxValC; Point minLocC; Point maxLocC;
	double minValU; double maxValU; Point minLocU; Point maxLocU;
	double minValR; double maxValR; Point minLocR; Point maxLocR;

	// minVal* and maxVal* doubles are populate with similarity values
  	minMaxLoc( r4, &minVal4, &maxVal4, &minLoc4, &maxLoc4, Mat() );
    minMaxLoc( r3, &minVal3, &maxVal3, &minLoc3, &maxLoc3, Mat() ); 
    minMaxLoc( r2, &minVal2, &maxVal2, &minLoc2, &maxLoc2, Mat() ); 
    minMaxLoc( r1, &minVal1, &maxVal1, &minLoc1, &maxLoc1, Mat() ); 
    minMaxLoc( rClick, &minValC, &maxValC, &minLocC, &maxLocC, Mat() ); 
    minMaxLoc( rUnclick, &minValU, &maxValU, &minLocU, &maxLocU, Mat() ); 
	minMaxLoc( rRight, &minValR, &maxValR, &minLocR, &maxLocR, Mat() );

	int ret_val;
	ret_ind = 0;
  	if (matchMethod  == CV_TM_SQDIFF || matchMethod == CV_TM_SQDIFF_NORMED) { 
		// Lowest value is best fit
		double minVals[7];
		ret_val = minValU;
		minVals[0] = minValU;
		minVals[1] = minVal1;
		minVals[2] = minVal2;
		minVals[3] = minVal3;
		minVals[4] = minVal4;
		minVals[5] = minValC;
		minVals[6] = minValR;
		
		for (int d = 0; d < 6; d++) {
			if (ret_val > minVals[d]) {
				ret_val = minVals[d];
				ret_ind = d;
			}
		}

	} else {
		// Highest value is best fit
		double maxVals[7];
		ret_val = maxValU;
		maxVals[0] = maxValU;
		maxVals[1] = maxVal1;
		maxVals[2] = maxVal2;
		maxVals[3] = maxVal3;
		maxVals[4] = maxVal4;
		maxVals[5] = maxValC;
		maxVals[6] = maxValR;
		
		for (int d = 0; d < 6; d++) {
			if (ret_val < maxVals[d]) {
				ret_val = maxVals[d];
				ret_ind = d;
			}
		}
	}
	return;
}

// Processes the current gamestate grid[][] and determines risk[][] probabilities
static void calculateRisk(double game[][16]) {
	int x, y;
	// Corner Cases
	if (game[0][0] != 0) {
		risk[0][0] = 0;
	} else {
		risk[0][0] = game[1][0] + game[0][1] + game[1][1];
		risk[0][0] = (risk[0][0]/12)*70 + 30;
		risk[0][0] = round(risk[0][0]);
	}
	if (game[0][15] != 0) {
		risk[0][15] = 0;
	} else {
		risk[0][15] = game[1][15] + game[0][14] + game[1][14];
		risk[0][15] = (risk[0][15]/12)*70 + 30;
		risk[0][15] = round(risk[0][15]);
	}
	if (game[15][0] != 0) {
		risk[15][0] = 0;
	} else {
		risk[15][0] = game[15][1] + game[14][0] + game[14][1];
		risk[15][0] = (risk[15][0]/12)*70 + 30;
		risk[15][0] = round(risk[15][0]);  
	}
	if (game[15][15] != 0) {
		risk[15][15] = 0;
	} else {
		risk[15][15] = game[15][14] + game[14][15] + game[14][14];
		risk[15][15] = (risk[15][15]/12)*70 + 30;
		risk[15][15] = round(risk[15][15]); 
	}
	
	// Edge Cases
	for (y = 1; y < 15; y++) {
		if (game[y][0] != 0) {
			risk[y][0] = 0;
		} else {
			risk[y][0] = game[y-1][0] + game[y-1][1] + game[y][1] + game[y+1][1] + game[y+1][0];
			risk[y][0] = (risk[y][0]/20)*70 + 30;
			risk[y][0] = round(risk[y][0]); 
		}
	}

	for (y = 1; y < 15; y++) {
		if (game[y][15] != 0) {
			risk[y][15] = 0;
		} else {
			risk[y][15] = game[y-1][15] + game[y-1][14] + game[y][14] + game[y+1][14] + game[y+1][15];
			risk[y][15] = (risk[y][15]/20)*70 + 30;
			risk[y][15] = round(risk[y][15]); 
		}
	}

	for (x = 1; x < 15; x++) {
		if (game[0][x] != 0) {
			risk[0][x] = 0;
		} else {
			risk[0][x] = game[0][x-1] + game[1][x-1] + game[1][x] + game[1][x+1] + game[0][x+1];
			risk[0][x] = (risk[0][x]/20)*70 + 30;
			risk[0][x] = round(risk[0][x]); 
		}
	}

	for (x = 1; x < 15; x++) {
		if (game[15][x] != 0) {
			risk[15][x] = 0;
		} else {
			risk[15][x] = game[15][x-1] + game[14][x-1] + game[14][x] + game[14][x+1] + game[15][x+1];
			risk[15][x] = (risk[15][x]/20)*70 + 30;
			risk[15][x] = round(risk[15][x]); 
		}
	}

	// Normal Cases
	for (x = 1; x < 15; x++) {
		for (y = 1; y < 15; y++) {
			if (game[y][x] != 0) {
				risk[y][x] = 0;
			} else {
				risk[y][x] = game[y-1][x-1] + game[y-1][x] + game[y-1][x+1];
				risk[y][x] += game[y+1][x-1] + game[y+1][x] + game[y+1][x+1];
				risk[y][x] += game[y][x-1] + game[y][x+1];
				risk[y][x] = (risk[y][x]/32)*70 + 30;
				risk[y][x] = round(risk[y][x]); 
			}
		}
	}
}

// Takes an array as input and writes to a file in the current directory
static void makeText(double heatmap[][16]) {
	int k, l;
	ofstream myfile;
	myfile.open("hardcode.txt");
	for (k = 0; k < 16; k++) {
		for (l = 0; l < 16; l++) {
			myfile << heatmap[k][l];
			if (!(k == 15 && l == 15)) {
				myfile << "\n";
			}
		}
	}
	myfile.close();

	for (k = 0; k < 16; k++) {
		for (l = 0; l < 16; l++) {
			if (heatmap[k][l] > 49) {
				cout << heatmap[k][l] << "  ";
			} else {
				cout << heatmap[k][l] << "   ";
			}
		}
		cout << "\n";
	}
	cout << "\n\n";
}

// Detects whether the mouse has been clicked
static void onMouse(int event, int x, int y, int, void*) {
	if (event == EVENT_LBUTTONDOWN || event == EVENT_RBUTTONDOWN) {
		// Delete old screenshot, take new screenshot after 1sec delay, then repeat
		// Repeat by toggling mouseClick boolean on
		cout << "CLICK" << endl;
		click = 1;
	}
}

int main(int /*argc*/, char** /*argv*/) {
    vector<vector<Point> > squares;
	double grid[16][16];
	
	four = imread( "four.png", 1);
	three = imread( "three.png", 1);
	two = imread( "two.png", 1);
	one = imread( "one.png", 1);
	clicked = imread( "clicked.png", 1);
	unclicked = imread( "unclicked.png", 1);
	rightclick = imread( "rightclick.png", 1);
	
	// OPENCV Mouse Click detection (only on image window from OPENCV)
	//imshow("help", Mat(four));
	//setMouseCallback( "help", onMouse, 0);

	int d = 1;

	while(d) {
		// Code for actually detecting mouse clicks
		//while(click == 0) {}
		//click = 1;
		//if (!click) {
		//	continue;
		//} else {
		//	click = 0;
		//	cout << "CLICK" << endl;
		//}

		// Take a screenshot and move it into the current directory
		system("osascript -l JavaScript -e 'Application(\"System Events\").keystroke(\"#\", {using: \"command down\"})'");
		usleep(500000);
		system("mv ~/Desktop/screenshots/grid.png ~/Documents/research/opencv-master/samples/cpp/");
		usleep(1000000);
		Mat image = imread("grid.png", 1);	

		Rect rectang = Rect(795, 333, 961, 961);
		Mat crop = image(rectang);
		int tileW = 60;
		int tileH = 60;
		
		// Detect each Minesweeper tile as an ROI
		for (int w = 0; w < 16; w++) {
			for (int h = 0; h < 16; h++) {
				Rect roi(w*tileW, h*tileH, tileW, tileH);	
				cropped = crop(roi);
				MatchingMethod( 0, 0 );
				grid[h][w] = ret_ind;
				if (ret_ind == 6) {
					grid[h][w] = 0;
				}
			}
		}
		
		// Process the grid[][] game state to produce probality heatmap in risk[][]
		calculateRisk(grid);
		cout << "CURRENT GAME STATE:" << endl;		
		makeText(grid);
		cout << "\n" << "CURRENT HEATMAP:" << endl;
		makeText(risk);
	}
    return 0;
}
