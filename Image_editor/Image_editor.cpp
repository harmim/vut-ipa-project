/* Sablona pro projekty do predmetu IPA, tema graficky editor
* Autor: Tomas Goldmann, igoldmann@fit.vutbr.cz
*
* LOGIN STUDENTA: xharmi00
* Dominik Harmim <xharmi00@stud.fit.vutbr.cz>
*/

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <intrin.h>
#include <inttypes.h>
#include <windows.h>

//#include "ipa_algorithm.h" ???
#include "ipa_tool.h"


#define WIN_WIDTH 800.0
#define PROJECT_NAME "IPA - graficky editor 2017"
#define PROJECT_NAME_WIN_IN "IPA - graficky editor 2017-IN"
#define PROJECT_NAME_WIN_OUT "IPA - graficky editor 2017-OUT"


using namespace cv;
using namespace std;


typedef void (*Ipa_algorithm)(
	unsigned char *input_data,
	unsigned char *output_data,
	unsigned int width,
	unsigned int height,
	int argc,
	char **argv
);


//int xGradient(Mat image, int x, int y)
//{
//	return image.at<uchar>(y - 1, x - 1) +
//		2 * image.at<uchar>(y, x - 1) +
//		image.at<uchar>(y + 1, x - 1) -
//		image.at<uchar>(y - 1, x + 1) -
//		2 * image.at<uchar>(y, x + 1) -
//		image.at<uchar>(y + 1, x + 1);
//}
//
//
//int yGradient(Mat image, int x, int y)
//{
//	return image.at<uchar>(y - 1, x - 1) +
//		2 * image.at<uchar>(y - 1, x) +
//		image.at<uchar>(y - 1, x + 1) -
//		image.at<uchar>(y + 1, x - 1) -
//		2 * image.at<uchar>(y + 1, x) -
//		image.at<uchar>(y + 1, x + 1);
//}


int xGradient(Mat image, int x, int y, int i)
{
	return image.at<cv::Vec3b>(y - 1, x - 1)[i] +
		2 * image.at<cv::Vec3b>(y, x - 1)[i] +
		image.at<cv::Vec3b>(y + 1, x - 1)[i] -
		image.at<cv::Vec3b>(y - 1, x + 1)[i] -
		2 * image.at<cv::Vec3b>(y, x + 1)[i] -
		image.at<cv::Vec3b>(y + 1, x + 1)[i];
}


int yGradient(Mat image, int x, int y, int i)
{
	return image.at<cv::Vec3b>(y - 1, x - 1)[i] +
		2 * image.at<cv::Vec3b>(y - 1, x)[i] +
		image.at<cv::Vec3b>(y - 1, x + 1)[i] -
		image.at<cv::Vec3b>(y + 1, x - 1)[i] -
		2 * image.at<cv::Vec3b>(y + 1, x)[i] -
		image.at<cv::Vec3b>(y + 1, x + 1)[i];
}


int main(int argc, char** argv)
{
	unsigned __int64 cycles_start = 0;

	if (argc != 2)
	{
		cout << " Usage: display_image ImageToLoadAndDisplay" << std::endl;
		return -1;
	}

	Mat output, window_img, image;

	image = imread(argv[1], CV_LOAD_IMAGE_COLOR);
	if (!image.data)
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	//koeficient pro prijatelne vykresleni
	float q = WIN_WIDTH / image.cols;

	//vytvoreni vystupniho obrazu
	image.copyTo(output);

	cv::resize(image, window_img, cv::Size(q * image.cols, q * image.rows));
	namedWindow(PROJECT_NAME_WIN_IN, WINDOW_AUTOSIZE);
	imshow(PROJECT_NAME_WIN_IN, window_img);

	HINSTANCE hInstLibrary = LoadLibrary("Student_DLL.dll");
	if (!hInstLibrary)
	{
		std::cout << "DLL Failed To Load!" << std::endl;
		return EXIT_FAILURE;
	}
	else
	{
		//Algoritmus
		InstructionCounter counter;
		Ipa_algorithm f = (Ipa_algorithm) GetProcAddress(hInstLibrary, "ipa_algorithm");
		if (f)
		{
			//counter.start();
			//f(image.data, output.data, image.cols, image.rows, argc, argv);
			//counter.print();
		}
	}

	for (int y = 0; y < image.rows; y++)
		for (int x = 0; x < image.cols; x++)
		{
			for (int i = 0; i < 3; i++)
			{
				output.at<cv::Vec3b>(y, x)[i] = 0.0;
			}
			//output.at<uchar>(y, x) = 0.0;
		}

	int gx, gy, sum;
	for (int y = 1; y < image.rows - 1; y++) {
		for (int x = 1; x < image.cols - 1; x++) {
			/*gx = xGradient(image, x, y);
			gy = yGradient(image, x, y);
			sum = abs(gx) + abs(gy);
			sum = sum > 255 ? 255 : sum;
			sum = sum < 0 ? 0 : sum;
			output.at<uchar>(y, x) = sum;*/
			for (int i = 0; i < 3; i++)
			{
				gx = xGradient(image, x, y, i);
				gy = yGradient(image, x, y, i);
				sum = abs(gx) + abs(gy);
				sum = sum > 255 ? 255 : sum;
				sum = sum < 0 ? 0 : sum;
				output.at<cv::Vec3b>(y, x)[i] = sum;
			}
		}
	}

	namedWindow(PROJECT_NAME_WIN_OUT, WINDOW_AUTOSIZE);
	cv::resize(output, output, cv::Size(q * image.cols, q * image.rows));
	imshow(PROJECT_NAME_WIN_OUT, output);

	waitKey(0);
	FreeLibrary(hInstLibrary);

	return 0;
}
