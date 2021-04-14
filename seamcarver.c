#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <omp.h>
#include "c_img.h"
#include "seamcarving.h"

const int BYTES_PER_PIXEL = 3; /// red, green, & blue
const int FILE_HEADER_SIZE = 14;
const int INFO_HEADER_SIZE = 40;
uint8_t *image_raster;
uint8_t *grad_raster;

void generateBitmapImage(unsigned char* image, int height, int width, char* imageFileName);
unsigned char* createBitmapFileHeader(int height, int stride);
unsigned char* createBitmapInfoHeader(int height, int width);
void write_bmp_img(struct rgb_img *im, char *filename, unsigned char* image);

#define IMG(i,j,k) (image[(im->width*BYTES_PER_PIXEL+paddingSize)*i + BYTES_PER_PIXEL*j + k])

void write_bmp_img(struct rgb_img *im, char *filename, unsigned char* image)
{ // image is only allocated once and as we're going down in size it'll never need to be resized.
    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (im->width * BYTES_PER_PIXEL) % 4) % 4;

	#pragma omp parallel for
    for (int i = 0; i < im->height; i++)
    {
        for (int j = 0; j < im->width; j++)
        {   // bmp is read from bottom-to-top UNLIKE every other image format.
            IMG(i,j,2) = (unsigned char) get_pixel(im, (im->height)-1-i, /*(im->width)-1-*/j, 0); // R
            IMG(i,j,1) = (unsigned char) get_pixel(im, (im->height)-1-i, /*(im->width)-1-*/j, 1); // G
            IMG(i,j,0) = (unsigned char) get_pixel(im, (im->height)-1-i, /*(im->width)-1-*/j, 2); // B
        }
		// adding padding. https://stackoverflow.com/a/36262260
	    for (int pad = 0; pad < paddingSize; pad++)
		{
			IMG(i, im->width, pad) = 0x00;
		}
    }
//	#pragma omp parallel private(image, im, filename)
	generateBitmapImage(image, im->height, im->width, filename);
}

int initialize(size_t height, size_t width)
{
    int paddingSize = (4 - (width * BYTES_PER_PIXEL) % 4) % 4;
    *image_raster = (uint8_t*)malloc(width*height*BYTES_PER_PIXEL+paddingSize*height);
    *grad_raster = (uint8_t*)malloc(width*height*BYTES_PER_PIXEL+paddingSize*height);
    return 0;
}

unsigned char* getNextFrame(uint8_t img_raster)
{
    struct rgb_img *im;
    struct rgb_img *grad;
	// 'tis a shame but there must be 2 mallocs.
	create_img(grad, im->height, im->width);
}
// slightly modified from https://stackoverflow.com/a/47785639
void generateBitmapImage (unsigned char* image, int height, int width, char* imageFileName)
{
    int widthInBytes = width * BYTES_PER_PIXEL;

    unsigned char padding[3] = {0, 0, 0};
    int paddingSize = (4 - (widthInBytes) % 4) % 4;

    int stride = (widthInBytes) + paddingSize;

    FILE* imageFile = fopen(imageFileName, "wb");

    unsigned char* fileHeader = createBitmapFileHeader(height, stride);
    fwrite(fileHeader, 1, FILE_HEADER_SIZE, imageFile);

    unsigned char* infoHeader = createBitmapInfoHeader(height, width);
    fwrite(infoHeader, 1, INFO_HEADER_SIZE, imageFile);

	fwrite(image, 1, height*width*BYTES_PER_PIXEL+paddingSize*height, imageFile);
    fclose(imageFile);
}

unsigned char* createBitmapFileHeader (int height, int stride)
{
    int fileSize = FILE_HEADER_SIZE + INFO_HEADER_SIZE + (stride * height);

    static unsigned char fileHeader[] = {
        0,0,     /// signature
        0,0,0,0, /// image file size in bytes
        0,0,0,0, /// reserved
        0,0,0,0, /// start of pixel array
    };

    fileHeader[ 0] = (unsigned char)('B');
    fileHeader[ 1] = (unsigned char)('M');
    fileHeader[ 2] = (unsigned char)(fileSize      );
    fileHeader[ 3] = (unsigned char)(fileSize >>  8);
    fileHeader[ 4] = (unsigned char)(fileSize >> 16);
    fileHeader[ 5] = (unsigned char)(fileSize >> 24);
    fileHeader[10] = (unsigned char)(FILE_HEADER_SIZE + INFO_HEADER_SIZE);

    return fileHeader;
}

unsigned char* createBitmapInfoHeader (int height, int width)
{
    static unsigned char infoHeader[] = {
        0,0,0,0, /// header size
        0,0,0,0, /// image width
        0,0,0,0, /// image height
        0,0,     /// number of color planes
        0,0,     /// bits per pixel
        0,0,0,0, /// compression
        0,0,0,0, /// image size
        0,0,0,0, /// horizontal resolution
        0,0,0,0, /// vertical resolution
        0,0,0,0, /// colors in color table
        0,0,0,0, /// important color count
    };

    infoHeader[ 0] = (unsigned char)(INFO_HEADER_SIZE);
    infoHeader[ 4] = (unsigned char)(width      );
    infoHeader[ 5] = (unsigned char)(width >>  8);
    infoHeader[ 6] = (unsigned char)(width >> 16);
    infoHeader[ 7] = (unsigned char)(width >> 24);
    infoHeader[ 8] = (unsigned char)(height      );
    infoHeader[ 9] = (unsigned char)(height >>  8);
    infoHeader[10] = (unsigned char)(height >> 16);
    infoHeader[11] = (unsigned char)(height >> 24);
    infoHeader[12] = (unsigned char)(1);
    infoHeader[14] = (unsigned char)(BYTES_PER_PIXEL*8);

    return infoHeader;
}
