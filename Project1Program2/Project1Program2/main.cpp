#include "opencv2/highgui.hpp"
#include <iostream>
#include <tuple>
using namespace cv;
using namespace std;

tuple<double, double, double> RGBtoXYZ(int R, int G, int B);
tuple<double, double, double> XYZtoLuv(double X, double Y, double Z);
tuple<double, double, double> LuvToXYZ(double L, double u, double v);
tuple<int, int, int> XYZtoRGB(double X, double Y, double Z);
double invgamma(double v);

void runOnWindow(int W1, int H1, int W2, int H2, Mat inputImage, char *outName) {
	int rows = inputImage.rows;
	int cols = inputImage.cols;

	vector<Mat> i_planes;
	split(inputImage, i_planes);
	Mat iB = i_planes[0];
	Mat iG = i_planes[1];
	Mat iR = i_planes[2];

	// dynamically allocate RGB arrays of size rows x cols
	int** R = new int*[rows];
	int** G = new int*[rows];
	int** B = new int*[rows];
	for (int i = 0; i < rows; i++) {
		R[i] = new int[cols];
		G[i] = new int[cols];
		B[i] = new int[cols];
	}

	for (int i = 0; i < rows; i++)
	for (int j = 0; j < cols; j++) {
		R[i][j] = iR.at<uchar>(i, j);
		G[i][j] = iG.at<uchar>(i, j);
		B[i][j] = iB.at<uchar>(i, j);
	}


	//	   The transformation should be based on the
	//	   historgram of the pixels in the W1,W2,H1,H2 range.
	//	   The following code goes over these pixels

	int LHistogram[101];
	int LLookupTable[101];
	int min = 102; //the values for linear scaling
	int max = -1;

	for (int i = 0; i < 101; i++)
		LHistogram[i] = 0;

	for (int i = H1; i <= H2; i++)
	for (int j = W1; j <= W2; j++) {
		double r = R[i][j];
		double g = G[i][j];
		double b = B[i][j];

		double X, Y, Z;
		double L, u, v;

		tie(X, Y, Z) = RGBtoXYZ(r, g, b);
		tie(L, u, v) = XYZtoLuv(X, Y, Z);

		R[i][j] = L;
		G[i][j] = u;
		B[i][j] = v;

		LHistogram[(int)round(L)]++;

		//Keep track of which is the min an max so we don't need to compute it later
		if (L < min)
			min = L;
		else if (L > max)
			max = L;
	}

	for (int i = 0; i < 101; i++)
	{
		LLookupTable[i] = ((i - min) * 100) / (max - min);
	}

	for (int i = H1; i <= H2; i++)
	for (int j = W1; j <= W2; j++) {

		double L = (double)LLookupTable[R[i][j]];
		double u = G[i][j];
		double v = B[i][j];

		double X, Y, Z;
		double r, g, b;

		tie(X, Y, Z) = LuvToXYZ(L, u, v);
		tie(r, g, b) = XYZtoRGB(X, Y, Z);

		R[i][j] = r;
		G[i][j] = g;
		B[i][j] = b;
	}

	Mat oR(rows, cols, CV_8UC1);
	Mat oG(rows, cols, CV_8UC1);
	Mat oB(rows, cols, CV_8UC1);
	for (int i = 0; i < rows; i++)
	for (int j = 0; j < cols; j++) {
		oR.at<uchar>(i, j) = R[i][j];;
		oG.at<uchar>(i, j) = G[i][j];;
		oB.at<uchar>(i, j) = B[i][j];;
	}

	Mat o_planes[] = { oB, oG, oR };
	Mat outImage;
	merge(o_planes, 3, outImage);

	namedWindow("output", CV_WINDOW_AUTOSIZE);
	imshow("output", outImage);
	imwrite(outName, outImage);
}

int main(int argc, char** argv) {
	/*
	if (argc != 7) {
		cerr << argv[0] << ": "
			<< "got " << argc - 1
			<< " arguments. Expecting six: w1 h1 w2 h2 ImageIn ImageOut."
			<< endl;
		cerr << "Example: proj1b 0.2 0.1 0.8 0.5 fruits.jpg out.bmp" << endl;
		return(-1);
	}
	double w1 = atof(argv[1]);
	double h1 = atof(argv[2]);
	double w2 = atof(argv[3]);
	double h2 = atof(argv[4]);
	char *inputName = argv[5];
	char *outputName = argv[6];
	*/
	double w1 = .3;
	double h1 = .3;
	double w2 = .8;
	double h2 = .8;
	char *inputName = "Dark_forest.jpg";
	char *outputName = "output.jpg";

	if (w1<0 || h1<0 || w2 <= w1 || h2 <= h1 || w2>1 || h2>1) {
		cerr << " arguments must satisfy 0 <= w1 < w2 <= 1"
			<< " ,  0 <= h1 < h2 <= 1" << endl;
		return(-1);
	}

	Mat inputImage = imread(inputName, CV_LOAD_IMAGE_UNCHANGED);
	if (inputImage.empty()) {
		cout << "Could not open or find the image " << inputName << endl;
		return(-1);
	}

	string windowInput("input: ");
	windowInput += inputName;

	namedWindow(windowInput, CV_WINDOW_AUTOSIZE);
	imshow(windowInput, inputImage);

	if (inputImage.type() != CV_8UC3) {
		cout << inputName << " is not a standard color image  " << endl;
		return(-1);
	}

	int rows = inputImage.rows;
	int cols = inputImage.cols;
	int W1 = (int)(w1*(cols - 1));
	int H1 = (int)(h1*(rows - 1));
	int W2 = (int)(w2*(cols - 1));
	int H2 = (int)(h2*(rows - 1));

	runOnWindow(W1, H1, W2, H2, inputImage, outputName);

	waitKey(0); // Wait for a keystroke
	return(0);
}

tuple<double, double, double> LuvToXYZ(double L, double u, double v)
{
	double Xw = 0.95;
	double Yw = 1.0;
	double Zw = 1.09;

	double uw = (4.0 * Xw) / (Xw + 15.0 * Yw + 3.0 * Zw);
	double vw = (9.0 * Yw) / (Xw + 15.0 * Yw + 3.0 * Zw);

	double uprime = (u + 13.0 * uw * L) / (13.0 * L);
	double vprime = (v + 13.0 * vw * L) / (13.0 * L);
	double Y;
	double X;
	double Z;

	if (L > 7.9996)
		Y = pow(((L + 16.0) / 116.0), 3.0) * Yw;
	else
		Y = (L / 903.3) * Yw;

	if (vprime == 0)
	{
		X = 0;
		Z = 0;
	}
	else
	{
		X = Y * 2.25 * (uprime / vprime);
		Z = (Y * (3.0 - (.75 * uprime) - (5.0 * vprime))) / vprime;
	}

	return make_tuple(X, Y, Z);
}

tuple<double, double, double> XYZtoLuv(double X, double Y, double Z)
{
	double Xw = 0.95;
	double Yw = 1.0;
	double Zw = 1.09;

	double uw = (4.0 * Xw) / (Xw + (15.0 * Yw) + (3.0 * Zw));
	double vw = (9.0 * Yw) / (Xw + (15.0 * Yw) + (3.0 * Zw));

	double t = Y / Yw;
	double L;

	if (t > 0.008856)
		L = (116.0 * pow(t, 1.0 / 3.0)) - 16.0;
	else
		L = 903.3 * t;

	double d = X + (15.0 * Y) + (3.0 * Z);

	double uprime = (4.0 * X) / d;
	double vprime = (9.0 * Y) / d;

	double u = (13.0 * L) * (uprime - uw);
	double v = (13.0 * L) * (vprime - vw);

	return make_tuple(L, u, v);
}

tuple<double, double, double> RGBtoXYZ(int R, int G, int B)
{
	double convert[3][3] =
	{
		{ 0.412453, 0.35758, 0.180423 },
		{ 0.212671, 0.71516, 0.072169 },
		{ 0.019334, 0.119193, 0.950227 }
	};

	double Rprime = (double)R / 255.0;
	double Gprime = (double)G / 255.0;
	double Bprime = (double)B / 255.0;

	double Rgamma = invgamma(Rprime);
	double Ggamma = invgamma(Gprime);
	double Bgamma = invgamma(Bprime);

	double XYZ[3];

	for (int n = 0; n < 3; n++)
	{
		XYZ[n] = convert[n][0] * Rgamma + convert[n][1] * Ggamma + convert[n][2] * Bgamma;
		if (XYZ[n] < 0)
			XYZ[n] = 0;
		if (XYZ[n] > 1)
			XYZ[n] = 1;
	}

	return make_tuple(XYZ[0], XYZ[1], XYZ[2]);

}

tuple<int, int, int> XYZtoRGB(double X, double Y, double Z)
{
	double convert[3][3] =
	{
		{ 3.240479, -1.53715, -0.498535 },
		{ -0.969256, 1.875991, 0.041556 },
		{ 0.055648, -0.204043, 1.057311 }
	};

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

	int r = (int)(sRGB[0] * 255);
	int g = (int)(sRGB[1] * 255);
	int b = (int)(sRGB[2] * 255);

	return make_tuple(r, g, b);
}

double invgamma(double v)
{
	if (v < 0.03928)
		return (v / 12.92);
	else
		return (pow((v + .055) / 1.055, 2.4));
}
