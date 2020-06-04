#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "assert.h"
#include "math.h"

#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))

struct dimensions {
        int height;
        int width;
};

        // to trim h and w:                                             FOR 40IMAGE.C     ///////
        // if ((dim % 2) != 0)
        //      dim -= 1;


struct dimensions *smallest_dims(Pnm_ppm img_1, Pnm_ppm img_2);
double calc_pixel_diff(Pnm_ppm img_1, Pnm_ppm img_2, struct dimensions *dims);

int main(int argc, char *argv[])
{
        assert(argc == 3);
        (void) argc;
        FILE *fp1 = NULL;
        FILE *fp2 = stdin;


        char *filename_1 = argv[1];
        char *filename_2 = argv[2];

        fp1 = fopen(filename_1, "r");
        fp2 = fopen(filename_2, "r");
        
        if (fp1 == NULL || fp2 == NULL) {
                fprintf(stderr, "Error: could not open one or both images.");
                fprintf(stderr, " Program exiting.\n");
                exit(1);
        }

        A2Methods_T METHODS = uarray2_methods_plain;                  // find a way to make this const


        Pnm_ppm img_1 = Pnm_ppmread(fp1, METHODS);
        Pnm_ppm img_2 = Pnm_ppmread(fp2, METHODS);

        fclose(fp1);
        fclose(fp2);

        /* compare dimensions - get smaller h and w in a struct dimension */
        struct dimensions *small_hw = smallest_dims(img_1, img_2);

        fprintf(stdout, "smaller height: %d pixels\n", small_hw->height);
        fprintf(stdout, "smaller width: %d pixels\n", small_hw->width);


        double rmsd_pixels = calc_pixel_diff(img_1, img_2, small_hw);
        printf("root mean square diff: %f\n", rmsd_pixels);
        // compute root mean sq diff

        // print root mean sq diff

        Pnm_ppmfree(&img_1);
        Pnm_ppmfree(&img_2);

        return 0;
}

struct dimensions *smallest_dims(Pnm_ppm img_1, Pnm_ppm img_2)
{
        int h1 = img_1->height;
        int w1 = img_1->width;
        int h2 = img_2->height;
        int w2 = img_2->width;

        if ((pow((h1 - h2), 2) > 1) || (pow((w1 - w2), 2) > 1)) {
                fprintf(stderr, "Image dimensions differ by >1\n");
                fprintf(stdout, "%1.1f\n", 1.0);
                //exit(1);                                                // code 1 or 0?
        }


        struct dimensions *smallest_hw = malloc(2 * sizeof(int));
        smallest_hw->height = min(h1, h2);
        smallest_hw->width = min(w1, w2);

        return smallest_hw;
}

double calc_pixel_diff(Pnm_ppm img_1, Pnm_ppm img_2, struct dimensions *dims)
{
        int h = dims->height;
        int w = dims->width;


        double square_diff = 0.0;
        Pnm_rgb rgb_1;
        Pnm_rgb rgb_2;



        //Pnm_rgb *rgb = (Pnm_rgb *)(img_1->methods->at(img_1->pixels,
          //                                                 0, 0));

        for (int i = 0; i < w; i++) {
                for (int j = 0; j < h; j++) {
                        rgb_1 = (Pnm_rgb)(img_1->methods->at(img_1->pixels, i, j));
                        rgb_2 = (Pnm_rgb)(img_2->methods->at(img_2->pixels, i, j));

                        square_diff += pow((((double)(rgb_1->red) / (double)(img_1->denominator)) - ((double)(rgb_2->red) / (double)(img_2->denominator))), 2);
                        square_diff += pow((((double)(rgb_1->green) / (double)(img_1->denominator)) - ((double)(rgb_2->green) / (double)(img_2->denominator))), 2);
                        square_diff += pow((((double)(rgb_1->blue) / (double)(img_1->denominator)) - ((double)(rgb_2->blue) / (double)(img_2->denominator))), 2);
                }
        }


        //printf("before division: %f\n", square_diff);
        square_diff /= (3 * w * h);
        return sqrt(square_diff);
}




