#include "opencv2/highgui.hpp"
#include <iostream>
#include <cmath>
using namespace cv;
using namespace std;

int main(int argc, char** argv) {

	if (argc != 3) {
		cout << argv[0] << ": "
			<< "got " << argc - 1 << " arguments. Expecting two: width height."
			<< endl;
		return(-1);
	}


	int width = atoi(argv[1]);
	int height = atoi(argv[2]);
	//int width = 500;
	//int height = 500;
	int** RED1 = new int*[height];
	int** GREEN1 = new int*[height];
	int** BLUE1 = new int*[height];
	int** RED2 = new int*[height];
	int** GREEN2 = new int*[height];
	int** BLUE2 = new int*[height];

	for (int i = 0; i < height; i++) {
		RED1[i] = new int[width];
		GREEN1[i] = new int[width];
		BLUE1[i] = new int[width];
		RED2[i] = new int[width];
		GREEN2[i] = new int[width];
		BLUE2[i] = new int[width];
	}

	for (int i = 0; i < height; i++)
	for (int j = 0; j < width; j++)
	{
		double convert[3][3] = 
		{
			{3.240479, -1.53715, -0.498535},
			{-0.969256, 1.875991, 0.041556},
			{0.055648, -0.204043, 1.057311}
		};

		int r1, g1, b1;
		int r2, g2, b2;

		double x = (double)j / (double)width;
		double y = (double)i / (double)height;
		double Y = 1.0;

		double L = 90;
		double u = x * 512 - 255;
		double v = y * 512 - 255;


		/* Your code should be placed here
		It should translate xyY to byte sRGB
		and Luv to byte sRGB
		*/

		//Convert xyY to XYZ then to sRGB
		double X = (x / y) * Y;
		double Z = ((1 - x - y) / y) * Y;

		double linearRGB[3];
		double sRGB[3];

		for (int n = 0; n < 3; n++)
		{
			linearRGB[n] = convert[n][0] * X + convert[n][1] * Y + convert[n][2] * Z;
			if (linearRGB[n] < 0)
				linearRGB[n] = 0;
			if (linearRGB[n] > 1)
				linearRGB[n] = 1;
			if (linearRGB[n] < 0.00304)
				sRGB[n] = linearRGB[n] * 12.92;
			else
				sRGB[n] = (1.055 * pow(linearRGB[n], (1.0 / 2.4))) - 0.055;
		}

		r1 = (int)(sRGB[0] * 255);
		g1 = (int)(sRGB[1] * 255);
		b1 = (int)(sRGB[2] * 255);

		//Convert Luv to XYZ the to sRGB

		double Xw = 0.95;
		double Yw = 1.0;
		double Zw = 1.09;

		double uw = (4.0 * Xw) / (Xw + 15.0 * Yw + 3.0 * Zw);
		double vw = (9.0 * Yw) / (Xw + 15.0 * Yw + 3.0 * Zw);

		double uprime = (u + 13.0 * uw * L) / (13.0 * L);
		double vprime = (v + 13.0 * vw * L) / (13.0 * L);
		double Y2;
		double X2;
		double Z2;

		if (L > 7.9996)
			Y2 = pow(((L + 16.0) / 116.0), 3.0) * Yw;
		else
			Y2 = (L / 903.3) * Yw;

		if (vprime == 0)
		{
			X2 = 0;
			Z2 = 0;
		}
		else
		{
			X2 = Y2 * 2.25 * (uprime / vprime);
			Z2 = (Y2 * (3.0 - (.75 * uprime) - (5.0 * vprime))) / vprime;
		}

		for (int n = 0; n < 3; n++)
		{
			linearRGB[n] = convert[n][0] * X2 + convert[n][1] * Y2 + convert[n][2] * Z2;
			if (linearRGB[n] < 0)
				linearRGB[n] = 0;
			if (linearRGB[n] > 1)
				linearRGB[n] = 1;
			if (linearRGB[n] < 0.00304)
				sRGB[n] = linearRGB[n] * 12.92;
			else
				sRGB[n] = (1.055 * pow(linearRGB[n], (1.0 / 2.4))) - 0.055;
		}

		r2 = (int)(sRGB[0] * 255);
		g2 = (int)(sRGB[1] * 255);
		b2 = (int)(sRGB[2] * 255);

		// this is the end of your code

		RED1[i][j] = r1;
		GREEN1[i][j] = g1;
		BLUE1[i][j] = b1;
		RED2[i][j] = r2;
		GREEN2[i][j] = g2;
		BLUE2[i][j] = b2;
	}


	Mat R1(height, width, CV_8UC1);
	Mat G1(height, width, CV_8UC1);
	Mat B1(height, width, CV_8UC1);

	Mat R2(height, width, CV_8UC1);
	Mat G2(height, width, CV_8UC1);
	Mat B2(height, width, CV_8UC1);

	for (int i = 0; i < height; i++)
	for (int j = 0; j < width; j++) {

		R1.at<uchar>(i, j) = RED1[i][j];
		G1.at<uchar>(i, j) = GREEN1[i][j];
		B1.at<uchar>(i, j) = BLUE1[i][j];

		R2.at<uchar>(i, j) = RED2[i][j];
		G2.at<uchar>(i, j) = GREEN2[i][j];
		B2.at<uchar>(i, j) = BLUE2[i][j];
	}

	Mat xyY;
	Mat xyY_planes[] = { B1, G1, R1 };
	merge(xyY_planes, 3, xyY);
	namedWindow("xyY", CV_WINDOW_AUTOSIZE);
	imshow("xyY", xyY);

	Mat Luv;
	Mat Luv_planes[] = { B2, G2, R2 };
	merge(Luv_planes, 3, Luv);
	namedWindow("Luv", CV_WINDOW_AUTOSIZE);
	imshow("Luv", Luv);
	waitKey(0); // Wait for a keystroke
	return(0);
}
