#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"
#include    <vector>
#include    <algorithm>
#include	<iostream>
#include	<cmath>
using namespace std;

#define PI (float) 3.14159265358979323846

float dot_product(GzCoord a, GzCoord b, int size) {
	float sum = 0;

	for (int i = 0; i < size; i++) {
		sum += a[i] * b[i];
	}

	return sum;
}

void normalize(GzCoord& vector, int size) {
	float total = 0.0;
	for (int i = 0; i < size; i++) {
		total += vector[i] * vector[i];
	}

	total = sqrt(total);

	for (int i = 0; i < size; i++) {
		vector[i] /= total;
	}
}


bool isRotation(GzMatrix matrix) {

	if (matrix[0][3] != 0 || matrix[1][3] != 0 || matrix[2][3] != 0 || matrix[3][0] != 0 || matrix[3][1] != 0 || matrix[3][2] != 0 || matrix[3][3] != 1) {
		return false;
	}

	if (matrix[0][0] == matrix[1][1] && matrix[0][0] == matrix[2][2] && matrix[1][1] == matrix[2][2]) {
		return false;
	}
	return true;
}


int GzRender::GzRotXMat(float degree, GzMatrix mat)
{
	/* HW 3.1
	// Create rotate matrix : rotate along x axis
	// Pass back the matrix using mat value
	*/
	memset(mat, 0, sizeof(mat));
	float radian = degree * (PI / 180.0);

	mat[0][0] = 1.0;
	mat[1][1] = cos(radian);
	mat[1][2] = -sin(radian);
	mat[2][1] = sin(radian);
	mat[2][2] = cos(radian);
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}

int GzRender::GzRotYMat(float degree, GzMatrix mat)
{
	/* HW 3.2
	// Create rotate matrix : rotate along y axis
	// Pass back the matrix using mat value
	*/
	memset(mat, 0, sizeof(mat));
	float radian = degree * (PI / 180.0);

	mat[0][0] = cos(radian);
	mat[0][2] = sin(radian);
	mat[1][1] = 1.0;
	mat[2][0] = -sin(radian);
	mat[2][2] = cos(radian);
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}

int GzRender::GzRotZMat(float degree, GzMatrix mat)
{
	/* HW 3.3
	// Create rotate matrix : rotate along z axis
	// Pass back the matrix using mat value
	*/
	memset(mat, 0, sizeof(mat));
	float radian = degree * (PI / 180.0);

	mat[0][0] = cos(radian);
	mat[0][1] = -sin(radian);
	mat[1][0] = sin(radian);
	mat[1][1] = cos(radian);
	mat[2][2] = 1.0;
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}

int GzRender::GzTrxMat(GzCoord translate, GzMatrix mat)
{
	/* HW 3.4
	// Create translation matrix
	// Pass back the matrix using mat value
	*/
	memset(mat, 0, sizeof(mat));

	mat[0][3] = translate[0];
	mat[1][3] = translate[1];
	mat[2][3] = translate[2];
	mat[0][0] = 1.0;
	mat[1][1] = 1.0;
	mat[2][2] = 1.0;
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}


int GzRender::GzScaleMat(GzCoord scale, GzMatrix mat)
{
	/* HW 3.5
	// Create scaling matrix
	// Pass back the matrix using mat value
	*/
	memset(mat, 0, sizeof(mat));

	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2];
	mat[3][3] = 1.0;

	return GZ_SUCCESS;
}


GzRender::GzRender(int xRes, int yRes)
{
	/* HW1.1 create a framebuffer for MS Windows display:
	 -- set display resolution
	 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
	 -- allocate memory for pixel buffer
	 */
	xres = xRes;
	yres = yRes;

	framebuffer = new char[3 * sizeof(char) * xres * yres];
	pixelbuffer = new GzPixel[xres * yres];
	numlights = 0;


	/* HW 3.6
	- setup Xsp and anything only done once
	- init default camera
	*/
	matlevel = -1;
	memset(Xsp, 0, sizeof(Xsp));
	Xsp[0][0] = xres / 2.0;
	Xsp[0][3] = xres / 2.0;
	Xsp[1][1] = -yres / 2.0;
	Xsp[1][3] = yres / 2.0;
	Xsp[2][2] = MAXINT;
	Xsp[3][3] = 1.0;

	m_camera.FOV = DEFAULT_FOV;
	m_camera.position[0] = DEFAULT_IM_X;
	m_camera.position[1] = DEFAULT_IM_Y;
	m_camera.position[2] = DEFAULT_IM_Z;

	memset(m_camera.lookat, 0, sizeof(m_camera.lookat));
	memset(m_camera.worldup, 0, sizeof(m_camera.worldup));
	m_camera.worldup[1] = 1.0;

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

int GzRender::GzBeginRender()
{
	/* HW 3.7
	- setup for start of each frame - init frame buffer color,alpha,z
	- compute Xiw and projection xform Xpi from camera definition
	- init Ximage - put Xsp at base of stack, push on Xpi and Xiw
	- now stack contains Xsw and app can push model Xforms when needed
	*/
	int status = 0;
	status |= GzPushMatrix(Xsp);

	//Setting Xpi
	memset(m_camera.Xpi, 0, sizeof(m_camera.Xpi));
	m_camera.Xpi[0][0] = 1.0;
	m_camera.Xpi[1][1] = 1.0;
	//1/d = Ttan(FOV/2)
	m_camera.Xpi[2][2] = tan((m_camera.FOV * PI / 180) / 2.0);
	m_camera.Xpi[3][2] = tan((m_camera.FOV * PI / 180) / 2.0);
	m_camera.Xpi[3][3] = 1;

	status |= GzPushMatrix(m_camera.Xpi);

	//Setting Xiw
	GzCoord Xiw_Z, cl;
	for (int i = 0; i < 3; i++) {
		cl[i] = m_camera.lookat[i] - m_camera.position[i];
	}

	float magnitude = 0;
	for (int i = 0; i < 3; i++) {
		magnitude += cl[i] * cl[i];
	}

	for (int i = 0; i < 3; i++) {
		Xiw_Z[i] = cl[i] / sqrt(magnitude);
	}

	GzCoord Xiw_Y, up_prime;
	float dot = dot_product(m_camera.worldup, Xiw_Z, 3);
	for (int i = 0; i < 3; i++) {
		up_prime[i] = m_camera.worldup[i] - (dot * Xiw_Z[i]);
	}

	magnitude = 0;
	for (int i = 0; i < 3; i++) {
		magnitude += up_prime[i] * up_prime[i];
	}

	for (int i = 0; i < 3; i++) {
		Xiw_Y[i] = up_prime[i] / sqrt(magnitude);
	}

	GzCoord Xiw_X;
	memset(m_camera.Xiw, 0, sizeof(m_camera.Xiw));
	Xiw_X[0] = (Xiw_Y[1] * Xiw_Z[2]) - (Xiw_Y[2] * Xiw_Z[1]);
	Xiw_X[1] = (Xiw_Y[2] * Xiw_Z[0]) - (Xiw_Y[0] * Xiw_Z[2]);
	Xiw_X[2] = (Xiw_Y[0] * Xiw_Z[1]) - (Xiw_Y[1] * Xiw_Z[0]);

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == 0) {
				m_camera.Xiw[i][j] = Xiw_X[j];
			}
			else if (i == 1) {
				m_camera.Xiw[i][j] = Xiw_Y[j];
			}
			else {
				m_camera.Xiw[i][j] = Xiw_Z[j];
			}
		}
	}

	dot = -1.0 * dot_product(Xiw_X, m_camera.position, 3);
	m_camera.Xiw[0][3] = dot;
	dot = -1.0 * dot_product(Xiw_Y, m_camera.position, 3);
	m_camera.Xiw[1][3] = dot;
	dot = -1.0 * dot_product(Xiw_Z, m_camera.position, 3);
	m_camera.Xiw[2][3] = dot;
	m_camera.Xiw[3][3] = 1.0;

	status |= GzPushMatrix(m_camera.Xiw);


	if (status == 1) {
		return GZ_FAILURE;
	}



	return GZ_SUCCESS;
}

int GzRender::GzPutCamera(GzCamera camera)
{
	/* HW 3.8
	/*- overwrite renderer camera structure with new camera definition
	*/
	m_camera.FOV = camera.FOV;
	copy(begin(camera.position), end(camera.position), begin(m_camera.position));
	copy(begin(camera.lookat), end(camera.lookat), begin(m_camera.lookat));
	copy(begin(camera.worldup), end(camera.worldup), begin(m_camera.worldup));

	for (int i = 0; i <= 3; i++) {
		copy(begin(camera.Xiw[i]), end(camera.Xiw[i]), begin(m_camera.Xiw[i]));
		copy(begin(camera.Xpi[i]), end(camera.Xpi[i]), begin(m_camera.Xpi[i]));
	}

	return GZ_SUCCESS;
}

int GzRender::GzPushMatrix(GzMatrix	matrix)
{
	/* HW 3.9
	- push a matrix onto the Ximage stack
	- check for stack overflow
	*/
	bool isRot;
	GzMatrix identity_matrix;
	memset(identity_matrix, 0, sizeof(identity_matrix));
	identity_matrix[0][0] = 1.0;
	identity_matrix[1][1] = 1.0;
	identity_matrix[2][2] = 1.0;
	identity_matrix[3][3] = 1.0;


	if (matlevel >= MATLEVELS) {
		return GZ_FAILURE;
	}

	if (matlevel == -1) {
		matlevel++;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				Ximage[matlevel][i][j] = matrix[i][j];
				Xnorm[matlevel][i][j] = identity_matrix[i][j];
			}
		}

	}
	else {
		matlevel++;
		isRot = isRotation(matrix);

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				Ximage[matlevel][i][j] = 0;
				Xnorm[matlevel][i][j] = 0;
				if (matlevel < 2) {
					Xnorm[matlevel][i][j] = identity_matrix[i][j];
				}

				else if (matlevel == 2) {
					Xnorm[matlevel][i][j] = m_camera.Xiw[i][j];
				}

				else {
					Ximage[matlevel][i][j] = 0;
				}

				for (int k = 0; k < 4; k++) {
					Ximage[matlevel][i][j] += Ximage[matlevel - 1][i][k] * matrix[k][j];

					if (isRot && matlevel > 2) {
						Xnorm[matlevel][i][j] += Xnorm[matlevel - 1][i][k] * matrix[k][j];
					}
					else if (!isRot && matlevel > 2) {
						Xnorm[matlevel][i][j] += Xnorm[matlevel - 1][i][k] * identity_matrix[k][j];
					}
				}
			}
		}
		Xnorm[matlevel][0][3] = 0;
		Xnorm[matlevel][1][3] = 0;
		Xnorm[matlevel][2][3] = 0;

	}

	return GZ_SUCCESS;
}

int GzRender::GzPopMatrix()
{
	/* HW 3.10
	- pop a matrix off the Ximage stack
	- check for stack underflow
	*/
	if (matlevel <= -1) {
		return GZ_FAILURE;
	}

	matlevel--;

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
	vector<GzIntensity> pixel;
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
	vector<GzIntensity> pixel;
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

void sort_vertices(GzCoord* vertices, GzCoord* normals) {
	int current_lowest;
	float highest_height;

	for (int i = 0; i < 3; i++) {
		current_lowest = i;
		highest_height = vertices[i][1];

		for (int k = i + 1; k < 3; k++) {
			if (highest_height > vertices[k][1]) {
				current_lowest = k;
				highest_height = vertices[k][1];

			}
			if (highest_height == vertices[k][1] && i > k) {
				current_lowest = k;
			}
		}

		if (current_lowest != i) {
			swap(vertices[i], vertices[current_lowest]);
			swap(normals[i], normals[current_lowest]);
		}
	}
}

int GzRender::GzPutAttribute(int numAttributes, GzToken* nameList, GzPointer* valueList)
{
	/* HW 2.1
	-- Set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
	-- In later homeworks set shaders, interpolaters, texture maps, and lights
	*/
	for (int i = 0; i < numAttributes; i++) {

		if (nameList[i] == GZ_RGB_COLOR) {
			std::copy(((GzColor*)valueList[i])[0], ((GzColor*)valueList[i])[0] + 3, flatcolor);
		}

		else if (nameList[i] == GZ_INTERPOLATE) {
			interp_mode = ((int*)valueList[i])[0];
		}

		else if (nameList[i] == GZ_DIRECTIONAL_LIGHT) {
			if (numlights > MAX_LIGHTS) {
				return GZ_FAILURE;
			}
			copy(((GzLight*)valueList[i])->direction, ((GzLight*)valueList[i])->direction + 3, lights[numlights].direction);
			copy(((GzLight*)valueList[i])->color, ((GzLight*)valueList[i])->color + 3, lights[numlights].color);
			numlights++;
		}

		else if (nameList[i] == GZ_AMBIENT_LIGHT) {
			copy(((GzLight*)valueList[i])->direction, ((GzLight*)valueList[i])->direction + 3, ambientlight.direction);
			copy(((GzLight*)valueList[i])->color, ((GzLight*)valueList[i])->color + 3, ambientlight.color);
		}

		else if (nameList[i] == GZ_AMBIENT_COEFFICIENT) {
			copy(((GzColor*)valueList[i])[0], ((GzColor*)valueList[i])[0] + 3, Ka);
		}

		else if (nameList[i] == GZ_DIFFUSE_COEFFICIENT) {
			copy(((GzColor*)valueList[i])[0], ((GzColor*)valueList[i])[0] + 3, Kd);
		}

		else if (nameList[i] == GZ_SPECULAR_COEFFICIENT) {
			copy(((GzColor*)valueList[i])[0], ((GzColor*)valueList[i])[0] + 3, Ks);
		}

		else if (nameList[i] == GZ_DISTRIBUTION_COEFFICIENT) {
			spec = ((float*)valueList[i])[0];
		}


	}

	return GZ_SUCCESS;
}

int GzRender::GzPutTriangle(int numParts, GzToken* nameList, GzPointer* valueList)
/* numParts - how many names and values */
{
	/* HW 2.2
	-- Pass in a triangle description with tokens and values corresponding to
		  GZ_NULL_TOKEN:		do nothing - no values
		  GZ_POSITION:		3 vert positions in model space
	-- Invoke the rastrizer/scanline framework
	-- Return error code
	*/
	for (int s = 0; s < numParts; s++) {

		if (nameList[s] == GZ_POSITION) {
			float vertices_in_4D[3][4], normals_in_4D[3][4], transformed_vertices[3][4], transformed_normals[3][4];
			GzCoord vertices[3], normals[3];

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					vertices_in_4D[i][j] = ((GzCoord*)(valueList[0]))[i][j];
					normals_in_4D[i][j] = ((GzCoord*)(valueList[1]))[i][j];
				}
				vertices_in_4D[i][3] = 1.0;
				normals_in_4D[i][3] = 1.0;
			}

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 4; j++) {
					transformed_vertices[i][j] = 0;
					transformed_normals[i][j] = 0;
					for (int k = 0; k < 4; k++) {
						transformed_vertices[i][j] += Ximage[matlevel][j][k] * vertices_in_4D[i][k];
						transformed_normals[i][j] += Xnorm[matlevel][j][k] * normals_in_4D[i][k];
					}
				}
			}

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					vertices[i][j] = transformed_vertices[i][j] / transformed_vertices[i][3];
					normals[i][j] = transformed_normals[i][j] / transformed_normals[i][3];
				}
			}
			//Check for negative screen z vertex
			if (vertices[0][2] >= 0 && vertices[1][2] >= 0 && vertices[2][2] >= 0) {

				GzColor C[3], specular[3], diffuse[3], ambient[3];
				memset(C, 0, sizeof(C));
				memset(specular, 0, sizeof(specular));
				memset(diffuse, 0, sizeof(diffuse));
				memset(ambient, 0, sizeof(ambient));
				GzCoord E = { 0,0,-1 };
				GzCoord R;
				float RdotE, NdotL, NdotE;

				//if GZ_FLAT color will only be calculated on the first vertex
				int n;
				if (interp_mode == GZ_FLAT) {
					n = 1;
				}

				else {
					n = 3;
					sort_vertices(vertices, normals);
				}


				if (interp_mode == GZ_FLAT || interp_mode == GZ_COLOR) {
					for (int i = 0; i < n; i++) {
						for (int j = 0; j < numlights; j++) {
							NdotL = dot_product(normals[i], lights[j].direction, 3);
							NdotE = dot_product(normals[i], E, 3);

							if ((NdotL > 0 && NdotE > 0) || (NdotL < 0 && NdotE < 0)) {

								for (int k = 0; k < 3; k++) {
									R[k] = 2.0 * NdotL * normals[i][k] - lights[j].direction[k];
								}

								normalize(R, 3);
								RdotE = dot_product(R, E, 3);

								if (RdotE < 0) {
									RdotE = 0;
								}

								else if (RdotE > 1) {
									RdotE = 1;
								}

								for (int k = 0; k < 3; k++) {
									specular[i][k] += Ks[k] * pow(RdotE, spec) * lights[j].color[k];

									if (NdotE < 0 || NdotL < 0) {
										for (int m = 0; m < 3; m++) {
											normals[i][m] *= -1;
										}
										NdotL = dot_product(normals[i], lights[j].direction, 3);
									}

									diffuse[i][k] += Kd[k] * NdotL * lights[j].color[k];
								}

							}
						}

						for (int j = 0; j < 3; j++) {
							ambient[i][j] += Ka[j] * ambientlight.color[j];
						}
					}

					for (int i = 0; i < n; i++) {
						for (int j = 0; j < 3; j++) {
							C[i][j] = specular[i][j] + diffuse[i][j] + ambient[i][j];

							if (C[i][j] < 0) {
								C[i][j] = 0;
							}

							else if (C[i][j] > 1.0) {
								C[i][j] = 1.0;
							}
						}
					}
				}
				
				//For GZ_FLAT vertices are sorted after Color is computed
				if (interp_mode == GZ_FLAT) {
					sort_vertices(vertices, normals);
				}
				
				GzCoord V1 = { vertices[0][0], vertices[0][1],vertices[0][2] };
				GzCoord V2 = { vertices[1][0], vertices[1][1],vertices[1][2] };
				GzCoord V3 = { vertices[2][0], vertices[2][1],vertices[2][2] };
				
				GzCoord start = { V1[0], V1[1], V1[2] };
				GzCoord end = { V2[0], V2[1], V2[2] };
				GzCoord end_V3 = { V3[0], V3[1], V3[2] };
				GzCoord current = { V1[0], V1[1], V1[2] };
				GzCoord current_V3 = { V1[0], V1[1], V1[2] };
				GzColor current_color = { C[0][0], C[0][1], C[0][2] };
				GzColor current_color_V3 = { C[0][0], C[0][1], C[0][2] };
				char edge;
				float slopex, slopez, slopex_V3, slopez_V3, slopez_span, deltay, deltax, slopeRy, 
					slopeGy, slopeBy, slopeRy_V3, slopeGy_V3, slopeBy_V3, slopeRx, slopeGx, slopeBx;
				int x, y, z;
				GzIntensity r, g, b;
				float start_span[2], end_span[2], current_span[2];
				GzColor start_color_span, end_color_span, current_span_color;
				int made_switch = 0;

				slopex = (end[0] - start[0]) / (end[1] - start[1]);
				slopez = (end[2] - start[2]) / (end[1] - start[1]);
				deltay = ceil(start[1]) - start[1];

				slopeRy = (C[1][0] - C[0][0]) / (end[1] - start[1]);
				slopeGy = (C[1][1] - C[0][1]) / (end[1] - start[1]);
				slopeBy = (C[1][2] - C[0][2]) / (end[1] - start[1]);

				slopex_V3 = (end_V3[0] - start[0]) / (end_V3[1] - start[1]);
				slopez_V3 = (end_V3[2] - start[2]) / (end_V3[1] - start[1]);

				slopeRy_V3 = (C[2][0] - C[0][0]) / (end_V3[1] - start[1]);
				slopeGy_V3 = (C[2][1] - C[0][1]) / (end_V3[1] - start[1]);
				slopeBy_V3 = (C[2][2] - C[0][2]) / (end_V3[1] - start[1]);

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

						start_color_span[0] = current_color_V3[0];
						start_color_span[1] = current_color_V3[1];
						start_color_span[2] = current_color_V3[2];
						current_span_color[0] = start_color_span[0];
						current_span_color[1] = start_color_span[1];
						current_span_color[2] = start_color_span[2];
						end_color_span[0] = current_color[0];
						end_color_span[1] = current_color[1];
						end_color_span[2] = current_color[2];
					}

					else {
						start_span[0] = current[0];
						start_span[1] = current[2];
						current_span[0] = start_span[0];
						current_span[1] = start_span[1];
						end_span[0] = current_V3[0];
						end_span[1] = current_V3[2];

						start_color_span[0] = current_color[0];
						start_color_span[1] = current_color[1];
						start_color_span[2] = current_color[2];
						current_span_color[0] = start_color_span[0];
						current_span_color[1] = start_color_span[1];
						current_span_color[2] = start_color_span[2];
						end_color_span[0] = current_color_V3[0];
						end_color_span[1] = current_color_V3[1];
						end_color_span[2] = current_color_V3[2];
					}

					deltax = ceil(start_span[0]) - start_span[0];
					slopez_span = (end_span[1] - start_span[1]) / (end_span[0] - start_span[0]);
					current_span[0] = current_span[0] + deltax;
					current_span[1] = current_span[1] + (slopez_span * deltax);

					while (current_span[0] <= end_span[0]) {

						x = std::ceil(current_span[0]);
						y = std::ceil(current[1]);
						z = std::ceil(current_span[1]);

						if (interp_mode == GZ_FLAT) {
							r = ctoi(C[0][0]);
							g = ctoi(C[0][1]);
							b = ctoi(C[0][2]);
						}

						else if (interp_mode == GZ_COLOR) {
							slopeRx = (end_color_span[0] - current_span_color[0]) / (end_span[0] - current_span[0]);
							slopeGx = (end_color_span[1] - current_span_color[1]) / (end_span[0] - current_span[0]);
							slopeBx = (end_color_span[2] - current_span_color[2]) / (end_span[0] - current_span[0]);

							r = ctoi(slopeRx * deltax + current_span_color[0]);
							g = ctoi(slopeGx * deltax + current_span_color[1]);
							b = ctoi(slopeBx * deltax + current_span_color[2]);

							current_span_color[0] += slopeRx * deltax;
							current_span_color[1] += slopeGx * deltax;
							current_span_color[2] += slopeBx * deltax;
						}

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

					current_color[0] += slopeRy * deltay;
					current_color[1] += slopeGy * deltay;
					current_color[2] += slopeBy * deltay;

					current_color_V3[0] += slopeRy_V3 * deltay;
					current_color_V3[1] += slopeGy_V3 * deltay;
					current_color_V3[2] += slopeBy_V3 * deltay;
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

				slopeRy = (C[2][0] - C[1][0]) / (end[1] - start[1]);
				slopeGy = (C[2][1] - C[1][1]) / (end[1] - start[1]);
				slopeBy = (C[2][2] - C[1][2]) / (end[1] - start[1]);

				current_color[0] = C[1][0];
				current_color[1] = C[1][1];
				current_color[2] = C[1][2];

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

						start_color_span[0] = current_color_V3[0];
						start_color_span[1] = current_color_V3[1];
						start_color_span[2] = current_color_V3[2];
						current_span_color[0] = start_color_span[0];
						current_span_color[1] = start_color_span[1];
						current_span_color[2] = start_color_span[2];
						end_color_span[0] = current_color[0];
						end_color_span[1] = current_color[1];
						end_color_span[2] = current_color[2];
					}

					else {
						start_span[0] = current[0];
						start_span[1] = current[2];
						current_span[0] = start_span[0];
						current_span[1] = start_span[1];
						end_span[0] = current_V3[0];
						end_span[1] = current_V3[2];

						start_color_span[0] = current_color[0];
						start_color_span[1] = current_color[1];
						start_color_span[2] = current_color[2];
						current_span_color[0] = start_color_span[0];
						current_span_color[1] = start_color_span[1];
						current_span_color[2] = start_color_span[2];
						end_color_span[0] = current_color_V3[0];
						end_color_span[1] = current_color_V3[1];
						end_color_span[2] = current_color_V3[2];
					}

					deltax = ceil(start_span[0]) - start_span[0];
					slopez_span = (end_span[1] - start_span[1]) / (end_span[0] - start_span[0]);
					current_span[0] = current_span[0] + deltax;
					current_span[1] = current_span[1] + (slopez_span * deltax);

					while (current_span[0] <= end_span[0]) {

						x = std::ceil(current_span[0]);
						y = std::ceil(current[1]);
						z = std::ceil(current_span[1]);

						if (interp_mode == GZ_FLAT) {
							r = ctoi(C[0][0]);
							g = ctoi(C[0][1]);
							b = ctoi(C[0][2]);
						}

						else if (interp_mode == GZ_COLOR) {
							slopeRx = (end_color_span[0] - current_span_color[0]) / (end_span[0] - current_span[0]);
							slopeGx = (end_color_span[1] - current_span_color[1]) / (end_span[0] - current_span[0]);
							slopeBx = (end_color_span[2] - current_span_color[2]) / (end_span[0] - current_span[0]);

							r = ctoi(slopeRx * deltax + current_span_color[0]);
							g = ctoi(slopeGx * deltax + current_span_color[1]);
							b = ctoi(slopeBx * deltax + current_span_color[2]);

							current_span_color[0] += slopeRx * deltax;
							current_span_color[1] += slopeGx * deltax;
							current_span_color[2] += slopeBx * deltax;
						}

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

					current_color[0] += slopeRy * deltay;
					current_color[1] += slopeGy * deltay;
					current_color[2] += slopeBy * deltay;

					current_color_V3[0] += slopeRy_V3 * deltay;
					current_color_V3[1] += slopeGy_V3 * deltay;
					current_color_V3[2] += slopeBy_V3 * deltay;
				}
			}
		}

	}

	return GZ_SUCCESS;
}