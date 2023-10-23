/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image=NULL;
int xs, ys;
int reset = 1;

int texture_look_up(int x, int y) {
    return y * xs + x;
}

/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }

/* bounds-test u,v to make sure nothing will overflow image array bounds */
/* determine texture cell corner values and perform bilinear interpolation */
/* set color to interpolated GzColor value and return */
  u = max(0.0, min(u, 1.0));
  v = max(0.0, min(v, 1.0));

  u *= (xs - 1);
  v *= (ys - 1);

  float s = u - floor(u);
  float t = v - floor(v);

  for (int i = 0; i < 3; i++) {
      color[i] = 
          s * t * image[texture_look_up(ceil(u), ceil(v))][i] 
          + (1 - s) * t * image[texture_look_up(floor(u), ceil(v))][i] 
          + s * (1 - t) * image[texture_look_up(ceil(u), floor(v))][i]
          + (1 - s) * (1 - t) * image[texture_look_up(floor(u), floor(v))][i];
  }
  
	return GZ_SUCCESS;
}

/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{
    u = max(0.0, min(u, 1.0));
    v = max(0.0, min(v, 1.0));
    int n = 6;
    int c;

    u *= n;
    v *= n;

    if (((int)u % 2 == 0 && (int)v % 2 == 0) || ((int)u % 2 != 0 && (int)v % 2 != 0)) {
        c = 1.0;
    }

    else {
        c = 0;
    }

    for (int i = 0; i < 3; i++) {
        color[i] = c;
    }

 


    return 1;
	return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture()
{
	if(image!=NULL)
		free(image);
	return GZ_SUCCESS;
}



