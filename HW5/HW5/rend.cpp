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

void sort_vertices(GzCoord* vertices, GzCoord* normals, GzTextureIndex* uvs) {
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
			swap(uvs[i], uvs[current_lowest]);
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

		else if (nameList[i] == GZ_TEXTURE_MAP) {
			tex_fun = (GzTexture)valueList[i];
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
			GzTextureIndex uvs[3];

			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					vertices_in_4D[i][j] = ((GzCoord*)(valueList[0]))[i][j];
					normals_in_4D[i][j] = ((GzCoord*)(valueList[1]))[i][j];

					if (j < 2) {
						uvs[i][j] = ((GzTextureIndex*)(valueList[2]))[i][j];
					}
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



			GzColor C[3], specular[3], diffuse[3], ambient[3];
			GzColor C_N, specular_N, diffuse_N, ambient_N;
			memset(C, 0, sizeof(C));
			memset(specular, 0, sizeof(specular));
			memset(diffuse, 0, sizeof(diffuse));
			memset(ambient, 0, sizeof(ambient));
			memset(C_N, 0, sizeof(C_N));
			memset(specular_N, 0, sizeof(specular_N));
			memset(diffuse_N, 0, sizeof(diffuse_N));
			memset(ambient_N, 0, sizeof(ambient_N));
			GzCoord E = { 0,0,-1 };
			GzCoord R;
			float RdotE, NdotL, NdotE;
			float VzPrime;

			//if GZ_FLAT color will only be calculated on the first vertex given from the file
			int n;
			if (interp_mode == GZ_FLAT) {
				n = 1;
			}

			else {
				n = 3;
				sort_vertices(vertices, normals, uvs);
			}

			if (tex_fun != NULL && interp_mode == GZ_COLOR) {
				fill(Ks, Ks + 3, 1.0);
				fill(Kd, Kd + 3, 1.0);
				fill(Ka, Ka + 3, 1.0);
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
									GzCoord normal_copy[3];
									for (int m = 0; m < 3; m++) {
										normal_copy[i][m] = normals[i][m] * -1;
									}
									NdotL = dot_product(normal_copy[i], lights[j].direction, 3);
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

			if (tex_fun != NULL) {

				for (int i = 0; i < 3; i++) {
					VzPrime = (vertices[i][2] / (MAXINT - vertices[i][2]));

					for (int j = 0; j < 2; j++) {
						uvs[i][j] /= (VzPrime + 1);
					}
				}
			}

			//For GZ_FLAT vertices are sorted after Color is computed
			if (interp_mode == GZ_FLAT) {
				sort_vertices(vertices, normals, uvs);
			}

			double slopex_V2 = (vertices[1][0] - vertices[0][0]) / (vertices[1][1] - vertices[0][1]);
			double slopex_V3 = (vertices[2][0] - vertices[0][0]) / (vertices[2][1] - vertices[0][1]);

			if (slopex_V3 > slopex_V2) {
				for (int i = 0; i < 3; i++) {
					swap(vertices[1][i], vertices[2][i]);
					swap(normals[1][i], normals[2][i]);
					swap(C[1][i], C[2][i]);

					if (i < 2) {
						swap(uvs[1][i], uvs[2][i]);
					}
				}
			}

			GzCoord V1 = { vertices[0][0], vertices[0][1],vertices[0][2] };
			GzCoord V2 = { vertices[1][0], vertices[1][1],vertices[1][2] };
			GzCoord V3 = { vertices[2][0], vertices[2][1],vertices[2][2] };
			GzColor interpolated_color;
			GzCoord interpolated_normal;
			GzTextureIndex interpolated_uv;

			float edge1, edge2, edge3;
			float edge1_A, edge1_B, edge1_C, edge2_A, edge2_B, edge2_C, edge3_A, edge3_B, edge3_C;
			float Z_interpolated, Z_A, Z_B, Z_C, Z_D, Z_v1_v2, Z_v1_v3;
			float edge1x, edge1y, edge1z, edge2x, edge2y, edge2z,
				edge1redColor, edge2redColor, red_A, red_B, red_C, red_D,
				edge1greenColor, edge2greenColor, green_A, green_B, green_C, green_D,
				edge1blueColor, edge2blueColor, blue_A, blue_B, blue_C, blue_D,
				edge1normalx, edge2normalx, normalx_A, normalx_B, normalx_C, normalx_D,
				edge1normaly, edge2normaly, normaly_A, normaly_B, normaly_C, normaly_D,
				edge1normalz, edge2normalz, normalz_A, normalz_B, normalz_C, normalz_D,
				edge1U, edge2U, U_A, U_B, U_C, U_D,
				edge1V, edge2V, V_A, V_B, V_C, V_D;

			edge1x = V2[0] - V1[0];
			edge1y = V2[1] - V1[1];
			edge1z = V2[2] - V1[2];

			edge2x = V3[0] - V1[0];
			edge2y = V3[1] - V1[1];
			edge2z = V3[2] - V1[2];


			edge1_A = V2[1] - V1[1];
			edge2_A = V3[1] - V2[1];
			edge3_A = V1[1] - V3[1];

			edge1_B = -1.0 * (V2[0] - V1[0]);
			edge2_B = -1.0 * (V3[0] - V2[0]);
			edge3_B = -1.0 * (V1[0] - V3[0]);

			edge1_C = (-1.0 * edge1_B * V1[1]) - (edge1_A * V1[0]);
			edge2_C = (-1.0 * edge2_B * V2[1]) - (edge2_A * V2[0]);
			edge3_C = (-1.0 * edge3_B * V3[1]) - (edge3_A * V3[0]);

			Z_A = edge1y * edge2z - edge1z * edge2y;
			Z_B = edge1z * edge2x - edge1x * edge2z;
			Z_C = edge1x * edge2y - edge1y * edge2x;
			Z_D = -(Z_A * V1[0] + Z_B * V1[1] + Z_C * V1[2]);

			if (interp_mode == GZ_COLOR) {

				edge1redColor = C[1][0] - C[0][0];
				edge2redColor = C[2][0] - C[0][0];

				red_A = edge1y * edge2redColor - edge1redColor * edge2y;
				red_B = edge1redColor * edge2x - edge1x * edge2redColor;
				red_C = edge1x * edge2y - edge1y * edge2x;
				red_D = -(red_A * V1[0] + red_B * V1[1] + red_C * C[0][0]);


				edge1greenColor = C[1][1] - C[0][1];
				edge2greenColor = C[2][1] - C[0][1];

				green_A = edge1y * edge2greenColor - edge1greenColor * edge2y;
				green_B = edge1greenColor * edge2x - edge1x * edge2greenColor;
				green_C = red_C;
				green_D = -(green_A * V1[0] + green_B * V1[1] + green_C * C[0][1]);

				edge1blueColor = C[1][2] - C[0][2];
				edge2blueColor = C[2][2] - C[0][2];

				blue_A = edge1y * edge2blueColor - edge1blueColor * edge2y;
				blue_B = edge1blueColor * edge2x - edge1x * edge2blueColor;
				blue_C = red_C;
				blue_D = -(blue_A * V1[0] + blue_B * V1[1] + blue_C * C[0][2]);

			}

			if (interp_mode == GZ_NORMALS) {

				edge1normalx = normals[1][0] - normals[0][0];
				edge2normalx = normals[2][0] - normals[0][0];

				normalx_A = edge1y * edge2normalx - edge1normalx * edge2y;
				normalx_B = edge1normalx * edge2x - edge1x * edge2normalx;
				normalx_C = edge1x * edge2y - edge1y * edge2x;
				normalx_D = -(normalx_A * V1[0] + normalx_B * V1[1] + normalx_C * normals[0][0]);

				edge1normaly = normals[1][1] - normals[0][1];
				edge2normaly = normals[2][1] - normals[0][1];

				normaly_A = edge1y * edge2normaly - edge1normaly * edge2y;
				normaly_B = edge1normaly * edge2x - edge1x * edge2normaly;
				normaly_C = normalx_C;
				normaly_D = -(normaly_A * V1[0] + normaly_B * V1[1] + normaly_C * normals[0][1]);

				edge1normalz = normals[1][2] - normals[0][2];
				edge2normalz = normals[2][2] - normals[0][2];

				normalz_A = edge1y * edge2normalz - edge1normalz * edge2y;
				normalz_B = edge1normalz * edge2x - edge1x * edge2normalz;
				normalz_C = normalx_C;
				normalz_D = -(normalz_A * V1[0] + normalz_B * V1[1] + normalz_C * normals[0][2]);
			}

			if (tex_fun != NULL) {

				edge1U = uvs[1][0] - uvs[0][0];
				edge2U = uvs[2][0] - uvs[0][0];

				U_A = edge1y * edge2U - edge1U * edge2y;
				U_B = edge1U * edge2x - edge1x * edge2U;
				U_C = edge1x * edge2y - edge1y * edge2x;
				U_D = -(U_A * V1[0] + U_B * V1[1] + U_C * uvs[0][0]);

				edge1V = uvs[1][1] - uvs[0][1];
				edge2V = uvs[2][1] - uvs[0][1];

				V_A = edge1y * edge2V - edge1V * edge2y;
				V_B = edge1V * edge2x - edge1x * edge2V;
				V_C = U_C;
				V_D = -(V_A * V1[0] + V_B * V1[1] + V_C * uvs[0][1]);
			}

			float y_min, y_max, x_min, x_max;
			int r, g, b, x, y, z;

			y_min = V1[1];
			y_max = V1[1];
			x_min = V1[0];
			x_max = V1[0];

			for (int i = 0; i < 3; i++) {

				if (vertices[i][1] > y_max) {
					y_max = vertices[i][1];
				}
				if (vertices[i][1] < y_min) {
					y_min = vertices[i][1];
				}
				if (vertices[i][0] > x_max) {
					x_max = vertices[i][0];
				}
				if (vertices[i][0] < x_min) {
					x_min = vertices[i][0];
				}

			}


			for (int i = round(x_min); i <= round(x_max); i++) {
				for (int j = round(y_min); j <= round(y_max); j++) {

					edge1 = edge1_A * i + edge1_B * j + edge1_C;
					edge2 = edge2_A * i + edge2_B * j + edge2_C;
					edge3 = edge3_A * i + edge3_B * j + edge3_C;

					if (((edge1 > 0 && edge2 > 0 && edge3 > 0 && Z_C != 0) || (edge1 < 0 && edge2 < 0 && edge3 < 0 && Z_C != 0) || (edge1 == 0 || edge2 == 0 || edge3 == 0))) {

						Z_interpolated = -(Z_A * i + Z_B * j + Z_D) / Z_C;
						z = round(Z_interpolated);

						if (z >= 0) {

							if (interp_mode == GZ_FLAT) {
								r = ctoi(C[0][0]);
								g = ctoi(C[0][1]);
								b = ctoi(C[0][2]);
							}

							if (interp_mode == GZ_COLOR) {

								if (red_C != 0 && green_C != 0 && blue_C != 0) {
									interpolated_color[0] = -(red_A * i + red_B * j + red_D) / red_C;
									interpolated_color[1] = -(green_A * i + green_B * j + green_D) / green_C;
									interpolated_color[2] = -(blue_A * i + blue_B * j + blue_D) / blue_C;

									if (tex_fun != NULL) {
										GzColor UV_Color;
										interpolated_uv[0] = -(U_A * i + U_B * j + U_D) / U_C;
										interpolated_uv[1] = -(V_A * i + V_B * j + V_D) / V_C;

										VzPrime = z / ((float)MAXINT - z);

										for (int i = 0; i < 2; i++) {
											interpolated_uv[i] *= (VzPrime + 1.0);
										}

										tex_fun(interpolated_uv[0], interpolated_uv[1], UV_Color);

										for (int i = 0; i < 3; i++) {
											interpolated_color[i] *= UV_Color[i];
										}

										r = ctoi(interpolated_color[0]);
										g = ctoi(interpolated_color[1]);
										b = ctoi(interpolated_color[2]);

									}

								}

								if (interp_mode == GZ_NORMALS) {

									if (normalx_C != 0 && normaly_C != 0 && normalz_C != 0) {

										interpolated_normal[0] = -(normalx_A * i + normalx_B * j + normalx_D) / normalx_C;
										interpolated_normal[1] = -(normaly_A * i + normaly_B * j + normaly_D) / normaly_C;
										interpolated_normal[2] = -(normalz_A * i + normalz_B * j + normalz_D) / normalz_C;

										for (int j = 0; j < numlights; j++) {

											NdotL = dot_product(interpolated_normal, lights[j].direction, 3);
											NdotE = dot_product(interpolated_normal, E, 3);

											if ((NdotL > 0 && NdotE > 0) || (NdotL < 0 && NdotE < 0)) {

												for (int k = 0; k < 3; k++) {
													R[k] = 2.0 * NdotL * interpolated_normal[k] - lights[j].direction[k];
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
													specular_N[k] += Ks[k] * pow(RdotE, spec) * lights[j].color[k];

													if (NdotE < 0 || NdotL < 0) {
														GzCoord normal_copy;
														for (int m = 0; m < 3; m++) {
															normal_copy[m] = interpolated_normal[m] * -1;
														}
														NdotL = dot_product(normal_copy, lights[j].direction, 3);
													}

													diffuse_N[k] += Kd[k] * NdotL * lights[j].color[k];
												}

											}
										}

										for (int i = 0; i < 3; i++) {
											ambient_N[i] += Ka[i] * ambientlight.color[i];
										}


										for (int i = 0; i < 3; i++) {
											C_N[i] = specular_N[i] + diffuse_N[i] + ambient_N[i];
											if (C_N[i] > 1.0) {
												C_N[i] = 1.0;
											}
											if (C_N[i] < 0) {
												C_N[i] = 0;
											}
										}

										r = ctoi(C_N[0]);
										g = ctoi(C_N[1]);
										b = ctoi(C_N[2]);

										memset(specular_N, 0, sizeof(specular_N));
										memset(diffuse_N, 0, sizeof(diffuse_N));
										memset(ambient_N, 0, sizeof(ambient_N));
									}

								}

								x = i;
								y = j;
								GzPut(x, y, r, g, b, 1, z);
							}
						}

					}
				}

			}

		}
	}


	return GZ_SUCCESS;
}