#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
/*   CS580 HW   */
#include    "stdafx.h"  
#include	"Gz.h"
#include<vector>


GzRender::GzRender(int xRes, int yRes)
{
	/* HW1.1 create a framebuffer for MS Windows display:
	 -- set display resolution
	 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
	 -- allocate memory for pixel buffer
	 */
	xres = xRes;
	yres = yRes;

	framebuffer = new char[3 * xres * yres];
	pixelbuffer = new GzPixel[xres * yres];

}

GzRender::~GzRender()
{
	/* HW1.2 clean up, free buffer memory */
	delete[] framebuffer;
	delete[] pixelbuffer;

}

int GzRender::GzDefault()
{
	/* HW1.3 set pixel buffer to some default values - start a new frame */
	for (int i = 0; i < xres * yres; i++) {

		pixelbuffer[i].alpha = 0;
		pixelbuffer[i].blue = 0;
		pixelbuffer[i].green = 0;
		pixelbuffer[i].red = 0;
		pixelbuffer[i].z = 0;
	}


	return GZ_SUCCESS;

}


int GzRender::GzPut(int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
	/* HW1.4 write pixel values into the buffer */
	if (i >= 0 && i <= xres - 1 && j >= 0 && j <= yres - 1) {

		int pixel_index = ARRAY(i, j);

		pixelbuffer[pixel_index].alpha = a;
		pixelbuffer[pixel_index].blue = b;
		pixelbuffer[pixel_index].green = g;
		pixelbuffer[pixel_index].red = r;
		pixelbuffer[pixel_index].z = z;

	}

	return GZ_SUCCESS;

}


int GzRender::GzGet(int i, int j, GzIntensity* r, GzIntensity* g, GzIntensity* b, GzIntensity* a, GzDepth* z)
{
	/* HW1.5 retrieve a pixel information from the pixel buffer */

	if (i >= 0 && i <= xres - 1 && j >= 0 && j <= yres - 1) {

		int pixel_index = ARRAY(i, j);

		*a = pixelbuffer[pixel_index].alpha;
		*b = pixelbuffer[pixel_index].blue;
		*g = pixelbuffer[pixel_index].green;
		*r = pixelbuffer[pixel_index].red;
		*z = pixelbuffer[pixel_index].z;

	}

	return GZ_SUCCESS;

}


int GzRender::GzFlushDisplay2File(FILE* outfile)
{
	/* HW1.6 write image to ppm file -- "P6 %d %d 255\r" */
	std::vector<GzIntensity> pixel;
	char cpixel[3] = { 0,0,0 };

	fprintf(outfile, "P6 %d %d 255\r", xres, yres);

	for (int i = 0; i < xres * yres; i++) {

		pixel = { pixelbuffer[i].red, pixelbuffer[i].green, pixelbuffer[i].blue };

		for (int c = 0; c < pixel.size(); c++) {
			if (pixel[c] < 0) {
				pixel[c] = 0;
			}

			else if (pixel[c] > 4095) {
				pixel[c] = 4095;
			}

			pixel[c] = pixel[c] >> 4;
			cpixel[c] = (pixel[c] & 0xFF);
		}

		fwrite(cpixel, sizeof(char), sizeof(cpixel), outfile);

	}

	return GZ_SUCCESS;

}

int GzRender::GzFlushDisplay2FrameBuffer()
{
	/* HW1.7 write pixels to framebuffer:
		- put the pixels into the frame buffer
		- CAUTION: when storing the pixels into the frame buffer, the order is blue, green, and red
		- NOT red, green, and blue !!!
	*/
	std::vector<GzIntensity> pixel;
	char cpixel[3] = { 0,0,0 };
	for (int i = 0; i < xres * yres; i++) {
		pixel = { pixelbuffer[i].red, pixelbuffer[i].green, pixelbuffer[i].blue };

		for (int c = 0; c < pixel.size(); c++) {
			if (pixel[c] < 0) {
				pixel[c] = 0;
			}

			else if (pixel[c] > 4095) {
				pixel[c] = 4095;
			}

			pixel[c] = pixel[c] >> 4;
			cpixel[c] = (pixel[c] & 0xFF);
		}

		framebuffer[3 * i] = cpixel[2];
		framebuffer[(3 * i) + 1] = cpixel[1];
		framebuffer[(3 * i) + 2] = cpixel[0];
	}

	return GZ_SUCCESS;

}