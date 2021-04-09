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
//			image[i*(im->width*BYTES_PER_PIXEL + paddingSize) + 3*im->width + pad] = 0x00;
			IMG(i, im->width, pad) = 0x00;
		}
    }
//	#pragma omp parallel private(image, im, filename)
	generateBitmapImage(image, im->height, im->width, filename);
}

int main(int argc, char* argv[])
{
    char binName[255] = "";
	if (argc == 1) {
		// Nothing passed in.
		printf("Enter source bin file (include file extension): ");
		scanf("%s", binName);
	} else if (argc > 2) {
		printf("This program only accepts 1 argument (the filepath). Perhaps you want to enclose what you've given in quotes?\n");
		exit(0);
	} else if (strcmp(argv[1], "help") == 0) {
		printf("This program can either be invoked with one argument (a valid filepath to an image file) or without any argument (in which case it will ask)\n");
		exit(0);
	} else {
		sprintf(binName, "%s", argv[1]);
	}

	FILE *fp = fopen(binName, "rb");
	while (fp == NULL) {
		printf("Invalid filepath passed.\n");
		printf("Enter source bin file (include file extension): ");
		scanf("%s", binName);
	}
	fclose(fp);

    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

#if defined(_WIN32)
    mkdir("cropped_images");
#else
    mkdir("cropped_images", 0777);
#endif

    read_in_img(&im, binName);
    int WIDTH = im->width;

    printf("Carving Seams and saving as BMP...\n");
    int paddingSize = (4 - (im->width * BYTES_PER_PIXEL) % 4) % 4;
    unsigned char *image = (unsigned char*)malloc(im->width*im->height*BYTES_PER_PIXEL+paddingSize*im->height);
    write_bmp_img(im, "cropped_images/img0.bmp", image); // write the original file out too.
    for (int i = 1; i < WIDTH; i++)
    {
        calc_energy(im, &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "cropped_images/img%d.bmp", i);
        write_bmp_img(im, filename, image);

        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
        if (((int)(((float)i/(float)WIDTH)*100)) % 5 == 0) printf("\r%d%% done.", (int)(((float)i/(float)WIDTH)*100)); fflush(stdout);
    }
    destroy_image(im);
    free(image);
    printf("\r100%% done!\n");

    return 0;
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
