#include <stdio.h>
#include <stdlib.h>
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "assert.h"

void trim_dims(Pnm_ppm image);
A2Methods_UArray2 scale_to_float(Pnm_ppm image);
void store_float_structs(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl);
void store_int_structs(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl);
Pnm_ppm float_to_scale(A2Methods_UArray2 float_arr);

const int DECOMP_DENOM = 255;


extern void compress40(FILE *input)
{
        A2Methods_T methods = uarray2_methods_plain;
        Pnm_ppm image = Pnm_ppmread(input, methods);


        /* dimension trim = working */
        //printf("img height: %d\nimg width: %d\n", image->height,
                                                  //image->width);
        trim_dims(image);

        //printf("trimmed height: %d\ntrimmed width: %d\n", image->height,
                                                          //image->width);



        //conv from scaled ints to floats
        A2Methods_UArray2 comp = scale_to_float(image);
        //(void) comp;

        //printf("done compressing. decompressing and writing to stdout\n");

        Pnm_ppm decomp = float_to_scale(comp);

        Pnm_ppmwrite(stdout, decomp);
        (void) decomp;


}

extern void decompress40(FILE *input)
{
        // conv from float --> scaled ints





        // write image to ppm format stdout
        (void) input;
        //printf("decompressing\n");
}

void trim_dims(Pnm_ppm image)
{
        if ((image->height % 2) != 0)
                image->height -= 1;
        if ((image->width % 2) != 0)
                image->width -= 1;
}

struct float_rgb {
        float red;
        float green;
        float blue;
};

Pnm_ppm float_to_scale(A2Methods_UArray2 float_arr)
{
        A2Methods_T methods = uarray2_methods_plain;
        int w = methods->width(float_arr);
        int h = methods->height(float_arr);

        Pnm_ppm new_img = malloc( w * h * 3 * sizeof(unsigned) );

        new_img->width = w;
        new_img->height = h;
        new_img->denominator = DECOMP_DENOM;
        new_img->methods = methods;

        new_img->pixels = methods->new(w, h, sizeof(Pnm_rgb));

        methods->map_default(float_arr, &store_int_structs, new_img);


        //(void) new_img;
        return new_img;
}


void store_int_structs(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{
        Pnm_ppm image = cl;

        struct float_rgb *pix_floats = elem;
        Pnm_rgb uints = image->methods->at(image->pixels, col, row);

        uints->red = (unsigned)(pix_floats->red * DECOMP_DENOM);
        uints->green = (unsigned)(pix_floats->green * DECOMP_DENOM);
        uints->blue = (unsigned)(pix_floats->blue * DECOMP_DENOM);

        (void) arr;
}




A2Methods_UArray2 scale_to_float(Pnm_ppm image)
{
        
        A2Methods_UArray2 float_array = image->methods->new(image->width,
                                                            image->height,
                                                            sizeof(struct float_rgb));

        // go through image->pixels, getting value, storing corresponding float in float_array
        

        assert(image);
        assert(image->methods);
        assert(image->methods->map_default);
        assert(float_array);


        A2Methods_mapfun *map = image->methods->map_default;
        
        map(float_array, &store_float_structs, image);

        //printf("image denom %d\n", image->denominator);

        return float_array;

}

void store_float_structs(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{       
        Pnm_ppm image = cl;

        struct float_rgb *pix_floats = elem;
        int denom = ((Pnm_ppm)cl)->denominator;


        Pnm_rgb old_pix = image->methods->at(image->pixels, col, row);

        pix_floats->red = (float)(old_pix->red) / denom;
        pix_floats->green = (float)(old_pix->green) / denom;
        pix_floats->blue = (float)(old_pix->blue) / denom;

        (void) arr;

}