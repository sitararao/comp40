/******************************************************************************
 *
 *     float_mod.c
 *
 *     Assignment: Comp40 HW4 (Arith)
 *     Authors:  Sitara Rao (srao03) and Era Iyer (eiyer01)
 *     Date: 10/23/2019
 *
 *     This file implements the float_mod module for both compression and 
 *     decompression. This module is responsible for converting between a
 *     Pnm_ppm object and an A2Methods_UArray2 of float_ybr structs.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include "ppm_mod.h"

/* Factor we use to scale up floats to unsigneds */
const unsigned DECOMP_DENOM = 255;

/* Max and min values of scaled RGB floats */
const float MAX_RGB_FLOAT = 1;
const float MIN_RGB_FLOAT = 0;

/* Blocksize of 2 used for compression steps */
#define blocksize 2

/* Struct that contains floats of Pnm_rgb numerators / Pnm_ppm->denominator */
typedef struct float_rgb {
        float red;
        float green;
        float blue;
} *float_rgb;

/* Struct containing component video values for each pixel */
typedef struct float_ybr {
        float y;
        float pb;
        float pr;
} *float_ybr;

/*****************************************************************************
 *                           COMPRESSION FUNCTIONS                           *
 *****************************************************************************/

/*
 *  Compression helper functions --> apply() functions for UArray2 mapping
 */
void ppm_to_rgb(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl);
void rgb_to_comp(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl);


/*
 * ppm_to_component()
 * Function to create an A2Methods_UArray2 object containing float_ybr structs
 * from input Pnm_ppm object
 * Does so through an intermediate A2Methods_UArray2 containing float_rgb
 * structs
 * Calls apply() functions ppm_to_rgb() and rgb_to_comp()
 */
A2Methods_UArray2 ppm_to_component(Pnm_ppm image)
{
        assert(image != NULL);
        assert(image->methods == uarray2_methods_blocked);

        A2Methods_T methods = uarray2_methods_blocked;
        A2Methods_mapfun *map = methods->map_default;
        unsigned width = image->width;
        unsigned height = image->height;

        /***************************************
         * Step 1: RGB unsigneds -> RGB floats *
         **************************************/

        /* New array to hold float_rgb structs */
        A2Methods_UArray2 rgb_float_array = methods->new_with_blocksize(width,
                                                     height, 
                                                     sizeof(struct float_rgb),
                                                     blocksize);
        assert(rgb_float_array != NULL);

        /* Convert Pnm_ppm unsigneds RGB into floats, store in new array */
        map(rgb_float_array, &ppm_to_rgb, image);

        Pnm_ppmfree(&image);

        /****************************************
         * Step 2: RGB floats -> Y/Pb/Pr floats *
         ***************************************/
        A2Methods_UArray2 ybr_float_array = methods->new_with_blocksize(width,
                                                     height,
                                                     sizeof(struct float_ybr),
                                                     blocksize);
        assert(ybr_float_array != NULL);

        /* Convert RGB floats, store in new array of YBR floats */
        map(ybr_float_array, &rgb_to_comp, rgb_float_array);

        methods->free(&rgb_float_array);
        return ybr_float_array;
}

/*
 * ppm_to_rgb()
 * Apply() function called in ppm_to_component(), Pnm_ppm passed as closure
 * Divides Pnm_ppm RGB numerators by Pnm_ppm denom to yield RGB floats
 * Stores in new array at current index
 */
void ppm_to_rgb(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{       
        assert((arr != NULL) && (elem != NULL) && (cl != NULL));
        A2Methods_T methods = uarray2_methods_blocked;
        float_rgb pix_floats = elem;
        Pnm_ppm image = cl;
        unsigned denom = image->denominator;

        /* Get unsigned values from Pnm_ppm pixels field */
        Pnm_rgb old_pix = methods->at(image->pixels, col, row);

        /* Transform unsigneds into floats and store */
        pix_floats->red = (float)(old_pix->red) / denom;
        pix_floats->green = (float)(old_pix->green) / denom;
        pix_floats->blue = (float)(old_pix->blue) / denom;

        (void) arr;
}

/*
 * rgb_to_comp()
 * Apply() function called in ppm_to_component()
 * Closure argument is A2Methods_UArray2 of rgb_floats
 * Does arithmetic on rgb_float values to get component (y, pb, pr) vals
 * Stores in new array of ybr_floats at current index
 */
void rgb_to_comp(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{
        assert((arr != NULL) && (elem != NULL) && (cl != NULL));
        A2Methods_T methods = uarray2_methods_blocked;
        float_ybr ybr_vals = elem;
        A2Methods_UArray2 rgb_array = cl;

        /* Get corresponding rgb_float from closure UArray2 */
        float_rgb rgb_vals = methods->at(rgb_array, col, row);

        float r = rgb_vals->red;
        float g = rgb_vals->green;
        float b = rgb_vals->blue;

        /* Transform rgb into component and store */
        ybr_vals->y  =     (0.299 * r) +     (0.587 * g) +     (0.114 * b);
        ybr_vals->pb = (-0.168736 * r) + (-0.331264 * g) +       (0.5 * b);
        ybr_vals->pr =       (0.5 * r) + (-0.418688 * g) + (-0.081312 * b);

        (void) arr;
}

/*****************************************************************************
 *                          DECOMPRESSION FUNCTIONS                          *
 *****************************************************************************/

/*
 *  Decompression helper functions --> apply() functions for UArray2 mapping
 */
void comp_to_rgb(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl);
void rbg_to_ppm(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl);

/*
 * component_to_ppm()
 * Function takes in an A2Methods_UArray2 object containing float_ybr structs
 * and creates a Pnm_ppm by transforming from component to RGB color
 * Does so through an intermediate A2Methods_UArray2 containing float_rgb
 * structs
 * Calls apply() functions comp_to_rgb() and rgb_to_ppm()
 */
Pnm_ppm component_to_ppm(A2Methods_UArray2 ybr_float_array)
{
        assert(ybr_float_array != NULL);
        A2Methods_T methods = uarray2_methods_blocked;
        A2Methods_mapfun *map = methods->map_default;
        unsigned w = methods->width(ybr_float_array);
        unsigned h = methods->height(ybr_float_array);

        /* ybr floats --> new array containing rgb floats */
        A2Methods_UArray2 rgb_float_array = methods->new(w, h, 
                                                     sizeof(struct float_rgb));
        assert(rgb_float_array != NULL);
        map(rgb_float_array, &comp_to_rgb, ybr_float_array);
        methods->free(&ybr_float_array);

        /*  array containing rgb floats --> new pnm_ppm object */
        A2Methods_UArray2 pix = methods->new(w, h, sizeof(struct Pnm_rgb));
        Pnm_ppm new_img = malloc(sizeof(struct Pnm_ppm));
        assert(pix != NULL && new_img != NULL);

        new_img->width = w;
        new_img->height = h;
        new_img->denominator = DECOMP_DENOM;
        new_img->methods = methods;
        new_img->pixels = pix;

        /* Transform rgb_float data to rgb unsigneds, store in pix array */
        map(rgb_float_array, &rbg_to_ppm, new_img);

        methods->free(&rgb_float_array);
        return new_img;
}

/*
 * comp_to_rgb()
 * Apply() function called in component_to_ppm()
 * Closure argument is A2Methods_UArray2 of ybr_floats
 * Does arithmetic on ybr_float values to get rgb float vals
 * Stores in new array of rgb_floats at current index
 */
void comp_to_rgb(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{
        assert((arr != NULL) && (elem != NULL) && (cl != NULL));
        A2Methods_T methods = uarray2_methods_blocked;
        float_rgb rgb_vals = elem;
        A2Methods_UArray2 ybr_array = cl;

        float_ybr ybr_vals = methods->at(ybr_array, col, row);

        float y = ybr_vals->y;
        float pb = ybr_vals->pb;
        float pr = ybr_vals->pr;

        rgb_vals->red   =  (1.0 * y) +       (0.0 * pb) +     (1.402 * pr);
        rgb_vals->green =  (1.0 * y) + (-0.344136 * pb) + (-0.714136 * pr);
        rgb_vals->blue  =  (1.0 * y) +     (1.772 * pb) +       (0.0 * pr);

        (void) arr;
}


/*
 * rgb_to_ppm()
 * apply() function called in component_to_ppm()
 * Closure argument is Pnm_ppm object with uninitialized pixels field
 * Scales up rgb float vals to unsigneds by a factor of const DECOMP_DENOM
 * Stores in pixels array at current index
 */
void rbg_to_ppm(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{
        assert((arr != NULL) && (elem != NULL) && (cl != NULL));
        Pnm_ppm image = cl;
        float_rgb pix_floats = elem;
        Pnm_rgb uints = image->methods->at(image->pixels, col, row);

        /* if rgb float value is out-of bounds after decompression: */
        /* scale down to max or up to min value */
        if (pix_floats->red > MAX_RGB_FLOAT) {
                pix_floats->red = MAX_RGB_FLOAT;
        } else if (pix_floats->red < MIN_RGB_FLOAT) {
                pix_floats->red = MIN_RGB_FLOAT;
        }

        if (pix_floats->green > MAX_RGB_FLOAT) {
                pix_floats->green = MAX_RGB_FLOAT;
        } else if (pix_floats->green < MIN_RGB_FLOAT) {
                pix_floats->green = MIN_RGB_FLOAT;
        }
        
        if (pix_floats->blue > MAX_RGB_FLOAT) {
                pix_floats->blue = MAX_RGB_FLOAT;
        } else if (pix_floats->blue < MIN_RGB_FLOAT) {
                pix_floats->blue = MIN_RGB_FLOAT;
        }

        /* Scale by constant DECOMP_DENOM to get unsigneds, store in uints */
        uints->red   = (unsigned)(pix_floats->red   * DECOMP_DENOM);
        uints->green = (unsigned)(pix_floats->green * DECOMP_DENOM);
        uints->blue  = (unsigned)(pix_floats->blue  * DECOMP_DENOM);

        (void) arr;
}





