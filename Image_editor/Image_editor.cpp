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

#include "ipa_tool.h"


#define WIN_WIDTH 800.0
#define PROJECT_NAME "IPA - graficky editor 2017"
#define PROJECT_NAME_WIN_IN "IPA - graficky editor 2017-IN"
#define PROJECT_NAME_WIN_OUT "IPA - graficky editor 2017-OUT"


#define DEG2RAD 0.017453292

#define swap_(a, b) do { double tmp = a; a = b; b = tmp; } while(0)

#define round_(X) ((int) (((double) (X)) + 0.5))

#define ipart_(X) ((int) (X))

#define fpart_(X) (((double) (X)) - (double) ipart_(X))

#define rfpart_(X) (1.0 - fpart_(X))


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


unsigned int get_ofset(unsigned int x, unsigned int y, unsigned int width)
{
	return 3 * ((y * width) + x);
}


void to_grayscale(unsigned char *input_data, unsigned char *output_data, unsigned int width, unsigned int height)
{
	unsigned int ofs;
	double rc, gc, bc, gray;

	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			ofs = get_ofset(x, y, width);
			bc = (double) input_data[ofs];
			gc = (double) input_data[ofs + 1];
			rc = (double) input_data[ofs + 2];
			gray = (bc + gc + rc) / 3;
			output_data[ofs] = output_data[ofs + 1] = output_data[ofs + 2] = gray;
		}
	}
}


int xGradient(unsigned char *image, unsigned int x, unsigned int y, unsigned int width)
{
	return image[get_ofset(x - 1, y - 1, width)]
		+ 2 * image[get_ofset(x - 1, y , width)]
		+ image[get_ofset(x - 1, y + 1, width)]
		- image[get_ofset(x + 1, y - 1, width)]
		- 2 * image[get_ofset(x + 1, y, width)]
		- image[get_ofset(x + 1, y + 1, width)];
}


int yGradient(unsigned char *image, unsigned int x, unsigned int y, unsigned int width)
{
	return image[get_ofset(x - 1, y - 1, width)]
		+ 2 * image[get_ofset(x, y - 1, width)]
		+ image[get_ofset(x + 1, y - 1, width)]
		- image[get_ofset(x - 1, y + 1, width)]
		- 2 * image[get_ofset(x, y + 1, width)]
		- image[get_ofset(x + 1, y + 1, width)];
}


void sobel_edge_detection(unsigned char *input_data, unsigned char *output_data, unsigned int width, unsigned int height)
{
	unsigned int ofs;

	unsigned char *gray_image;
	unsigned int size = width * height * 3;
	gray_image = (unsigned char*) malloc((size + 1) * sizeof(unsigned char));
	gray_image[size] = '\0';


	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			ofs = get_ofset(x, y, width);
			output_data[ofs] = output_data[ofs + 1] = output_data[ofs + 2] = 0;
		}
	}

	to_grayscale(input_data, gray_image, width, height);

	int gx, gy, sum;
	for (unsigned int x = 1; x < width - 1; x++)
	{
		for (unsigned int y = 1; y < height - 1; y++)
		{
			gx = xGradient(input_data, x, y, width);
			gy = yGradient(input_data, x, y, width);

			sum = abs(gx) + abs(gy);
			sum = sum > 255 ? 255 : sum;
			sum = sum < 0 ? 0 : sum;

			ofs = get_ofset(x, y, width);
			output_data[ofs] = output_data[ofs + 1] = output_data[ofs + 2] = sum;
		}
	}

	free(gray_image);
}


void hough_transform(unsigned char *image, unsigned int width, unsigned int height, unsigned int *accu_width, unsigned int *accu_height, unsigned int **accu)
{
	double hough_h = ((sqrt(2.0) * (double) (height > width ? height : width)) / 2.0);
	*accu_height = hough_h * 2.0;
	*accu_width = 180;

	*accu = (unsigned int*) calloc(*accu_height * *accu_width, sizeof(unsigned int));

	double center_x = width / 2;
	double center_y = height / 2;

	unsigned int ofs;
	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			ofs = get_ofset(x, y, width);
			if (image[ofs] > 250)
			{
				for (int t = 0; t < 180; t++)
				{
					double r = (((double) x - center_x) * cos((double) t * DEG2RAD)) + (((double) y - center_y) * sin((double) t * DEG2RAD));
					(*accu)[(int) ((round(r + hough_h) * 180.0)) + t]++;
				}
			}
		}
	}
}


void dla_plot(int x, int y, float br, unsigned char *output_data, double bc, double gc, double rc, unsigned int width, unsigned int height)
{
	br = br > 1.0 ? 1.0 : br;
	bc = br * (float) bc;
	gc = br * (float) gc;
	rc = br * (float) rc;

	unsigned int ofs = get_ofset(x, y, width);
	unsigned int size = width * height * 3;

	if (ofs > size)
	{
		return;
	}

	output_data[ofs] = bc;
	output_data[ofs + 1] = gc;
	output_data[ofs + 2] = rc;
}


void draw_line_antialias(unsigned char *output_data, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, double bc, double gc, double rc, unsigned int width, unsigned int height)
{
	double dx = (double) x2 - (double) x1;
	double dy = (double) y2 - (double) y1;

	if (fabs(dx) > fabs(dy))
	{
		if (x2 < x1) 
		{
			swap_(x1, x2);
			swap_(y1, y2);
		}

		double gradient = dy / dx;
		double xend = round_(x1);
		double yend = y1 + gradient * (xend - x1);
		double xgap = rfpart_(x1 + 0.5);
		int xpxl1 = xend;
		int ypxl1 = ipart_(yend);

		dla_plot(xpxl1, ypxl1, rfpart_(yend) * xgap, output_data, bc, gc, rc, width, height);
		dla_plot(xpxl1, ypxl1 + 1, fpart_(yend) * xgap, output_data, bc, gc, rc, width, height);

		double intery = yend + gradient;

		xend = round_(x2);
		yend = y2 + gradient * (xend - x2);
		xgap = fpart_(x2 + 0.5);
		int xpxl2 = xend;
		int ypxl2 = ipart_(yend);

		dla_plot(xpxl2, ypxl2, rfpart_(yend) * xgap, output_data, bc, gc, rc, width, height);
		dla_plot(xpxl2, ypxl2 + 1, fpart_(yend) * xgap, output_data, bc, gc, rc, width, height);

		for (int x = xpxl1 + 1; x <= (xpxl2 - 1); x++) 
		{
			dla_plot(x, ipart_(intery), rfpart_(intery), output_data, bc, gc, rc, width, height);
			dla_plot(x, ipart_(intery) + 1, fpart_(intery), output_data, bc, gc, rc, width, height);
			intery += gradient;
		}
	}
	else
	{
		if (y2 < y1) 
		{
			swap_(x1, x2);
			swap_(y1, y2);
		}

		double gradient = dx / dy;
		double yend = round_(y1);
		double xend = x1 + gradient * (yend - y1);
		double ygap = rfpart_(y1 + 0.5);
		int ypxl1 = yend;
		int xpxl1 = ipart_(xend);

		dla_plot(xpxl1, ypxl1, rfpart_(xend)*ygap, output_data, bc, gc, rc, width, height);
		dla_plot(xpxl1, ypxl1 + 1, fpart_(xend)*ygap, output_data, bc, gc, rc, width, height);

		double interx = xend + gradient;

		yend = round_(y2);
		xend = x2 + gradient * (yend - y2);
		ygap = fpart_(y2 + 0.5);
		int ypxl2 = yend;
		int xpxl2 = ipart_(xend);

		dla_plot(xpxl2, ypxl2, rfpart_(xend) * ygap, output_data, bc, gc, rc, width, height);
		dla_plot(xpxl2, ypxl2 + 1, fpart_(xend) * ygap, output_data, bc, gc, rc, width, height);

		for (int y = ypxl1 + 1; y <= (ypxl2 - 1); y++) 
		{
			dla_plot(ipart_(interx), y, rfpart_(interx), output_data, bc, gc, rc, width, height);
			dla_plot(ipart_(interx) + 1, y, fpart_(interx), output_data, bc, gc, rc, width, height);
			interx += gradient;
		}
	}
}


void find_and_draw_lines_antialias(int threshold, unsigned char *output_data, unsigned int width, unsigned int height, unsigned int accu_width, unsigned int accu_height, unsigned int *accu)
{
	if (accu == 0)
	{
		return;
	}

	unsigned int ofs;
	for (unsigned int t = 0; t < accu_width; t++)
	{
		for (unsigned int r = 0; r < accu_height; r++)
		{
			if ((int) accu[(r * accu_width) + t] >= threshold)
			{
				int max = accu[(r * accu_width) + t];
				for (int ly = -4; ly <= 4; ly++)
				{
					for (int lx = -4; lx <= 4; lx++)
					{
						if ((ly + r >= 0 && ly + r < accu_height) && (lx + t >= 0 && lx + t < accu_width))
						{
							if ((int) accu[((r + ly) * accu_width) + (t + lx)] > max)
							{
								max = accu[((r + ly) * accu_width) + (t + lx)];
								ly = lx = 5;
							}
						}
					}
				}

				if (max > (int) accu[(r * accu_width) + t])
				{
					continue;
				}

				int x1, y1, x2, y2;
				x1 = y1 = x2 = y2 = 0;

				if (t >= 45 && t <= 135)
				{
					x1 = 0;
					y1 = (((double) r - (accu_height / 2)) - (((double) x1 - (width / 2)) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (height / 2);
					x2 = width - 0;
					y2 = (((double) r - (accu_height / 2)) - (((double) x2 - (width / 2)) * cos(t * DEG2RAD))) / sin(t * DEG2RAD) + (height / 2);
				}
				else
				{
					y1 = 0;
					x1 = (((double) r - (accu_height / 2)) - (((double) y1 - (height / 2)) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (width / 2);
					y2 = height - 0;
					x2 = (((double) r - (accu_height / 2)) - (((double) y2 - (height / 2)) * sin(t * DEG2RAD))) / cos(t * DEG2RAD) + (width / 2);
				}

				x1 = x1 < 0 ? 0 : x1;
				y1 = y1 < 0 ? 0 : y1;
				x2 = x2 < 0 ? 0 : x2;
				y2 = y2 < 0 ? 0 : y2;

				ofs = get_ofset(t, r, width);
				//draw_line_antialias(output_data, x1, y1, x2, y2, output_data[ofs], output_data[ofs + 1], output_data[ofs + 2], width, height);
				draw_line_antialias(output_data, x1, y1, x2, y2, 0, 0, 0, width, height);
			}
		}
	}
}


void algorithm(unsigned char *input_data, unsigned char *output_data, unsigned int width, unsigned int height, int argc, char **argv)
{
	sobel_edge_detection(input_data, output_data, width, height);

	unsigned int accu_width, accu_height, *accu;

	hough_transform(output_data, width, height, &accu_width, &accu_height, &accu);

	unsigned int ofs;
	for (unsigned int x = 0; x < width; x++)
	{
		for (unsigned int y = 0; y < height; y++)
		{
			ofs = get_ofset(x, y, width);
			output_data[ofs] = input_data[ofs];
			output_data[ofs + 1] = input_data[ofs + 1];
			output_data[ofs + 2] = input_data[ofs + 2];
		}
	}

	find_and_draw_lines_antialias(300, output_data, width, height, accu_width, accu_height, accu);

	free(accu);
}


int main(int argc, char **argv)
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
	//if (!hInstLibrary)
	//{
	//	std::cout << "DLL Failed To Load!" << std::endl;
	//	return EXIT_FAILURE;
	//}
	//else
	//{
	//	//Algoritmus
	//	InstructionCounter counter;
	//	Ipa_algorithm f = (Ipa_algorithm) GetProcAddress(hInstLibrary, "ipa_algorithm");
	//	if (f)
	//	{
	//		counter.start();
	//		f(image.data, output.data, image.cols, image.rows, argc, argv);
	//		counter.print();
	//	}
	//}

	algorithm(image.data, output.data, image.cols, image.rows, argc, argv);

	namedWindow(PROJECT_NAME_WIN_OUT, WINDOW_AUTOSIZE);
	cv::resize(output, output, cv::Size(q * image.cols, q * image.rows));
	double x = image.channels();
	imshow(PROJECT_NAME_WIN_OUT, output);

	waitKey(0);
	FreeLibrary(hInstLibrary);

	return 0;
}
