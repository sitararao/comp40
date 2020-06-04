/******************************************************************************
 *
 *     ppm_mod.c
 *
 *     Assignment: Comp40 HW4 (Arith)
 *     Authors:    Sitara Rao (srao03) and Era Iyer (eiyer01)
 *     Date:       10/23/2019
 *
 *     This file implements the ppm_mod module for both compression and 
 *     decompression. During compression, this module simply reads a ppm
 *     image file into a Pnm_ppm object and trims to an even height and
 *     width. In decompression, this module is responsible for writing a
 *     Pnm_ppm object to stdout.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include "ppm_mod.h"

/*****************************************************************************
 *                           COMPRESSION FUNCTIONS                           *
 *****************************************************************************/

/*
 * read_and_trim()
 * Function to create a Pnm_ppm object from input file
 * Trims image to even height and width if needed
 * Returns trimmed Pnm_ppm object
 */
Pnm_ppm read_and_trim(FILE *input)
{
    assert(input != NULL);
    A2Methods_T methods = uarray2_methods_blocked;
    Pnm_ppm image = Pnm_ppmread(input, methods);

    if ((image->height % 2) != 0)
            image->height -= 1;
     if ((image->width % 2) != 0)
            image->width -= 1;

    return image;
}


/*****************************************************************************
 *                          DECOMPRESSION FUNCTIONS                          *
 *****************************************************************************/

/*
 * print_ppm()
 * Function to print a Pnm_ppm object to stdout (ppm plain format)
 * Frees Pnm_ppm object after printing its data
 */
void print_ppm(Pnm_ppm decomp)
{
    assert(decomp != NULL);
    Pnm_ppmwrite(stdout, decomp);
    Pnm_ppmfree(&decomp);
}