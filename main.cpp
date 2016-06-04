#include "cv.h"
#include "highgui.h"
#include <random>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;
using namespace cv;

Point warpPoint(Point a, Mat r)
{
	return Point(r.at<double>(0) * a.x + r.at<double>(1) * a.y + r.at<double>(2),
		r.at<double>(3) * a.x + r.at<double>(4) * a.y + r.at<double>(5));
}

int triangle_area_2(Point a, Point b, Point c)
{
	return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

double triangle_area(Point a, Point b, Point c)
{
	return abs(triangle_area_2(a, b, c)) / 2.0;
}

bool clockwise(Point a, Point b, Point c)
{
	return triangle_area_2(a, b, c) < 0;
}

bool counter_clockwise(Point a, Point b, Point c)
{
	return triangle_area_2(a, b, c) > 0;
}

bool isInside(Point x, Point a, Point b, Point c, Point d)
{

	if (clockwise(a, b, x))
		return false;
	if (clockwise(b, c, x))
		return false;
	if (clockwise(c, d, x))
		return false;
	if (clockwise(d, a, x))
		return false;
	return true;
}

double getSquareError(Mat &img, Mat &test, Point a, Point b, Point c, Point d)
{
	double res = 0;
	int i, j;
	int k = 0;
	for (i = 0; i < img.rows; i++)
	{
		for (j = 0; j < img.cols; j++)
		{
			if (isInside(Point(j, i), d, c, b, a))
			{
				//circle(img,Point(j,i),3,Scalar(255,0,0));
				res += (img.at<unsigned char>(i, j) - test.at<unsigned char>(i, j))*
					(img.at<unsigned char>(i, j) - test.at<unsigned char>(i, j));

				k++;
			}

		}
	}
	return res / (k + 0.0);
}

bool outsideBounds(Point a, Mat &img)
{
	return a.x < 0 || a.y < 0 || a.x > img.cols || a.y > img.rows;
}

long long getMillis()
{
	chrono::milliseconds ms = chrono::duration_cast< chrono::milliseconds >(chrono::high_resolution_clock::now().time_since_epoch());
	return ms.count();
}

Point ma, mb, mc, md;
double minError = 100000000;

Mat originalImg, originalTest;
Mat img, test;
Mat displayedImage;
mutex mtx;

double processFPS = 0;

Point originalPoint1, originalPoint2;
double alpha = 1;
double prevAlpha = 1;
double Temperature = 1;
random_device rd;
mt19937 gen(rd());

uniform_real_distribution<> mrand(0, 1);
uniform_real_distribution<> rand01(-1, 1);

int numSincePrev = 0;
int numSinceStart = 0;

void doCalculations()
{

	img = originalImg.clone();
	Point randPoint1, randPoint2;

	for (;;)
	{
		long long frameStartTime = getMillis();
		//mtx.lock();
		//img = originalImg.clone();
		//test = originalTest.clone();
		randPoint1 = originalPoint1 + Point(rand01(gen) * img.cols * alpha, rand01(gen) * img.rows * alpha);
		randPoint2 = originalPoint2 + Point(rand01(gen) * img.cols * alpha, rand01(gen) * img.rows * alpha);


		Point a, b, c, d;
		a = randPoint1;
		b = randPoint2;

		if (norm(a - b) < 50)
			continue;

		c = Point(a.x + a.y - b.y, a.y + b.x - a.x);
		d = Point(b.x + a.y - b.y, b.y + b.x - a.x);

		Mat r = getRotationMatrix2D((a + b + c + d)*0.25, 180 - 180 * atan2(b.y - a.y, b.x - a.x) / 3.1415, norm(a - b) / originalTest.cols);

		//cout <<a.x<<" "<<a.y<<endl;

		a = warpPoint(Point(0, 0), r);
		b = warpPoint(Point(0, originalTest.rows), r);
		c = warpPoint(Point(originalTest.cols, originalTest.rows), r);
		d = warpPoint(Point(originalTest.cols, 0), r);



		if (outsideBounds(a, img) || outsideBounds(b, img) || outsideBounds(c, img) || outsideBounds(d, img))
			continue;


		warpAffine(originalTest, test, r, originalImg.size());

		double newError = getSquareError(img, test, a, b, c, d);

		double P = exp((minError - newError) / Temperature);
		double newP = mrand(gen);

		if (P > newP)
		{
			cout << "P: " << P << " newP: " << newP << endl;
		}

		if (newError < minError || P > newP)
		{
			minError = newError;
			ma = a;
			mb = b;
			mc = c;
			md = d;

			displayedImage = img.clone();
			absdiff(displayedImage, test, displayedImage);
			line(displayedImage, ma, mb, Scalar(255, 0, 0), 2);
			line(displayedImage, mb, mc, Scalar(255, 0, 0), 2);
			line(displayedImage, mc, md, Scalar(255, 0, 0), 2);
			line(displayedImage, md, ma, Scalar(255, 0, 0), 2);
			originalPoint1 = randPoint1;
			originalPoint2 = randPoint2;
			cout << "Error: " << minError << endl;

			alpha *= 0.9;
			numSincePrev = 0;
		}


		//mtx.unlock();
		processFPS = 1000.0 / (getMillis() - frameStartTime);

		numSincePrev++;
		numSinceStart++;
		if (numSinceStart % 50000 == 0)
			alpha *= 0.9;
		if (numSinceStart % 500 == 0)
		{
			Temperature *= 0.9;
		}
		//if (numSincePrev % 400 == 0)
		//    alpha *= 1.5;
		if (alpha*img.rows < 2)
			alpha = 2.0 / img.rows;
		if (alpha > 1)
			alpha = 1;
	}

}

void displayImage()
{
	for (;;)
	{
		//mtx.lock();
		//absdiff(img, test, img);
		imshow("Annealing Image Search", displayedImage);
		if (alpha != prevAlpha)
		{
			cout << "alpha: " << alpha << endl;
			prevAlpha = alpha;
		}

		//cout << processFPS << endl;
		//mtx.unlock();
		if (waitKey(30) >= 0) break;

	}
}

int main(int, char**)
{


	namedWindow("Annealing Image Search", 1);
	originalImg = imread("test.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	originalTest = imread("testcut.jpg", CV_LOAD_IMAGE_GRAYSCALE);

	displayedImage = originalImg.clone();

	originalPoint1 = Point(mrand(gen) * originalImg.cols, mrand(gen) * originalImg.rows);
	originalPoint2 = Point(mrand(gen) * originalImg.cols, mrand(gen) * originalImg.rows);

	thread calcLoop(doCalculations);
	//thread displayLoop(displayImage);

	//calcLoop.join();
	displayImage();
	//while(true){}

	return 0;
}
