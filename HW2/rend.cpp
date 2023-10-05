#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include    <vector>
#include    <algorithm>
#include	<iostream>


GzRender::GzRender(int xRes, int yRes)
{
	/* HW1.1 create a framebuffer for MS Windows display:
	 -- set display resolution
	 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
	 -- allocate memory for pixel buffer
	 */
	xres = xRes;
	yres = yRes;

	framebuffer = new char[3 * sizeof(char)*xres * yres];
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

		pixelbuffer[i].alpha = 1;
		pixelbuffer[i].blue = 1542;
		pixelbuffer[i].green = 1799;
		pixelbuffer[i].red = 2056;
		pixelbuffer[i].z = MAXINT;
	}


	return GZ_SUCCESS;

}


int GzRender::GzPut(int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
	/* HW1.4 write pixel values into the buffer */
	if (i >= 0 && i <= xres - 1 && j >= 0 && j <= yres - 1) {

		int pixel_index = ARRAY(i, j);
		if (z < pixelbuffer[pixel_index].z) {
			pixelbuffer[pixel_index].alpha = a;
			pixelbuffer[pixel_index].blue = b;
			pixelbuffer[pixel_index].green = g;
			pixelbuffer[pixel_index].red = r;
			pixelbuffer[pixel_index].z = z;
		}
		

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

	//fclose(outfile);

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



/***********************************************/
/* HW2 methods: implement from here */

void sort_vertices(GzCoord* vertices) {
	int current_lowest;
	float highest_height;

	for (int i = 0; i < 3; i++) {
		current_lowest = i;
		highest_height = vertices[i][1];

		for (int k = i+1; k < 3; k++) {
			if (highest_height > vertices[k][1]) {
				current_lowest = k;
				highest_height = vertices[k][1];
				
			}
			if (highest_height == vertices[k][1] && i > k) {
				current_lowest = k;
			}
		}

		if (current_lowest != i) {
			std::swap(vertices[i], vertices[current_lowest]);
		}
	}
}


int GzRender::GzPutAttribute(int numAttributes, GzToken	*nameList, GzPointer *valueList) 
{
/* HW 2.1
-- Set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
-- In later homeworks set shaders, interpolaters, texture maps, and lights
*/
	for (int i = 0; i < numAttributes; i++) {

		if (nameList[i] == GZ_RGB_COLOR) {
        std::copy(((GzColor*)valueList[i])[0], ((GzColor*)valueList[i])[0] + 3, flatcolor);
		}
	}
	return GZ_SUCCESS;
}

int GzRender::GzPutTriangle(int	numParts, GzToken *nameList, GzPointer *valueList) 
/* numParts - how many names and values */
{
/* HW 2.2
-- Pass in a triangle description with tokens and values corresponding to
      GZ_NULL_TOKEN:		do nothing - no values
      GZ_POSITION:		3 vert positions in model space
-- Invoke the rastrizer/scanline framework
-- Return error code
*/
	for (int i = 0; i < numParts; i++) {
		
		if (nameList[i] == GZ_POSITION) {
			GzCoord vertices[3];

			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 3; k++) {
					vertices[j][k] = ((GzCoord*)(valueList[i]))[j][k];
				}
			}

			sort_vertices(vertices);
			GzCoord V1 = { vertices[0][0], vertices[0][1],vertices[0][2] };
			GzCoord V2 = { vertices[1][0], vertices[1][1],vertices[1][2] };
			GzCoord V3 = { vertices[2][0], vertices[2][1],vertices[2][2] };


			GzCoord start = { V1[0], V1[1], V1[2] };
			GzCoord end = { V2[0], V2[1], V2[2] };
			GzCoord end_V3 = { V3[0], V3[1], V3[2] };
			GzCoord current = { V1[0], V1[1], V1[2] };
			GzCoord current_V3 = { V1[0], V1[1], V1[2] };
			char edge;
			float slopex, slopez, slopex_V3, slopez_V3, slopez_span, deltay, deltax;
			int x, y, z;
			GzIntensity r, g, b;
			float start_span[2], end_span[2], current_span[2];
			int made_switch = 0;

			slopex = (end[0] - start[0]) / (end[1] - start[1]);
			slopez = (end[2] - start[2]) / (end[1] - start[1]);
			deltay = ceil(start[1]) - start[1];

			slopex_V3 = (end_V3[0] - start[0]) / (end_V3[1] - start[1]);
			slopez_V3 = (end_V3[2] - start[2]) / (end_V3[1] - start[1]);

			if (slopex >= slopex_V3) {
				edge = 'R';
			}

			else {
				edge = 'L';
			}

			current[0] = current[0] + (slopex * deltay);
			current[1] = current[1] + deltay;
			current[2] = current[2] + (slopez * deltay);

			current_V3[0] = current_V3[0] + (slopex_V3 * deltay);
			current_V3[1] = current_V3[1] + deltay;
			current_V3[2] = current_V3[2] + (slopez_V3 * deltay);

			while (current[1] <= end[1]) {

				//SPAN
				if (edge == 'R') {
					start_span[0] = current_V3[0];
					start_span[1] = current_V3[2];
					current_span[0] = start_span[0];
					current_span[1] = start_span[1];
					end_span[0] = current[0];
					end_span[1] = current[2];
				}

				else {
					start_span[0] = current[0];
					start_span[1] = current[2];
					current_span[0] = start_span[0];
					current_span[1] = start_span[1];
					end_span[0] = current_V3[0];
					end_span[1] = current_V3[2];
				}

				deltax = ceil(start_span[0]) - start_span[0];
				slopez_span = (end_span[1] - start_span[1]) / (end_span[0] - start_span[0]);
				current_span[0] = current_span[0] + deltax;
				current_span[1] = current_span[1] + (slopez_span * deltax);

				while (current_span[0] <= end_span[0]) {

					x = std::ceil(current_span[0]);
					y = std::ceil(current[1]);
					z = std::ceil(current_span[1]);
					r = ctoi(flatcolor[0]);
					g = ctoi(flatcolor[1]);
					b = ctoi(flatcolor[2]);

					GzPut(x, y, r, g, b, 1, z);
						
					deltax = 1;
					current_span[0] = current_span[0] + deltax;
					current_span[1] = current_span[1] + (slopez_span * deltax);
				}
		
				deltay = 1;
				current[0] = current[0] + (slopex * deltay);
				current[1] = current[1] + deltay;
				current[2] = current[2] + (slopez * deltay);

				current_V3[0] = current_V3[0] + (slopex_V3 * deltay);
				current_V3[1] = current_V3[1] + deltay;
				current_V3[2] = current_V3[2] + (slopez_V3 * deltay);
				/*
				if (current[1] > end[1] && made_switch == 0) {
					made_switch++;
					
					start[0] = V2[0];
					start[1] = V2[1];
					start[2] = V2[2];
					current[0] = start[0];
					current[1] = start[1];
					current[2] = start[2];
					end[0] = V3[0];
					end[1] = V3[1];
					end[2] = V3[2];
					slopex = (end[0] - start[0]) / (end[1] - start[1]);
					slopez = (end[2] - start[2]) / (end[1] - start[1]);

					deltay = ceil(start[1]) - start[1];
					current[0] = current[0] + (slopex * deltay);
					current[1] = current[1] + deltay;
					current[2] = current[2] + (slopez * deltay);
				}*/
			}
			//Swapping edges
			
			start[0] = V2[0];
			start[1] = V2[1];
			start[2] = V2[2];
			current[0] = V2[0];
			current[1] = V2[1];
			current[2] = V2[2];
			end[0] = V3[0];
			end[1] = V3[1];
			end[2] = V3[2];
			slopex = (end[0] - start[0]) / (end[1] - start[1]);
			slopez = (end[2] - start[2]) / (end[1] - start[1]);

			deltay = ceil(start[1]) - start[1];
			current[0] = current[0] + (slopex * deltay);
			current[1] = current[1] + deltay;
			current[2] = current[2] + (slopez * deltay); 
			while (current[1] <= end[1]) {

				//SPAN
				if (edge == 'R') {
					start_span[0] = current_V3[0];
					start_span[1] = current_V3[2];
					current_span[0] = start_span[0];
					current_span[1] = start_span[1];
					end_span[0] = current[0];
					end_span[1] = current[2];
				}

				else {
					start_span[0] = current[0];
					start_span[1] = current[2];
					current_span[0] = start_span[0];
					current_span[1] = start_span[1];
					end_span[0] = current_V3[0];
					end_span[1] = current_V3[2];

				}

				deltax = ceil(start_span[0]) - start_span[0];
				slopez_span = (end_span[1] - start_span[1]) / (end_span[0] - start_span[0]);
				current_span[0] = current_span[0] + deltax;
				current_span[1] = current_span[1] + (slopez_span * deltax);

				while (current_span[0] <= end_span[0]) {

					x = std::ceil(current_span[0]);
					y = std::ceil(current[1]);
					z = std::ceil(current_span[1]);
					r = ctoi(flatcolor[0]);
					g = ctoi(flatcolor[1]);
					b = ctoi(flatcolor[2]);

					GzPut(x, y, r, g, b, 0, z);

					deltax = 1;
					current_span[0] = current_span[0] + deltax;
					current_span[1] = current_span[1] + (slopez_span * deltax);
				}

				deltay = 1;
				current[0] = current[0] + (slopex * deltay);
				current[1] = current[1] + deltay;
				current[2] = current[2] + (slopez * deltay);

				current_V3[0] = current_V3[0] + (slopex_V3 * deltay);
				current_V3[1] = current_V3[1] + deltay;
				current_V3[2] = current_V3[2] + (slopez_V3 * deltay);
			}



		}
		
	}

	return GZ_SUCCESS;
}



