/******************************************************************************
 *
 *     float_mod.h
 *
 *     Assignment: Comp40 HW4 (Arith)
 *     Authors:  Sitara Rao (srao03) and Era Iyer (eiyer01)
 *     Date: 10/23/2019
 *
 *     This file contains the interface for the float_mod module for both 
 *     compression and decompression. This module is responsible for converting
 *     from a Pnm_ppm image to an A2Methods_UArray2 containing component video
 *     values during compression, and for converting from an A2Methods_UArray2
 *     of component video values to a Pnm_ppm object during decompression.
 *
 *****************************************************************************/

#ifndef FLOAT_MOD_H
#define FLOAT_MOD_H

#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"

/*****************************************************************************
 *                           COMPRESSION FUNCTIONS                           *
 *****************************************************************************/
/* 
 * Function: ppm_to_component()
 * Purpose: reads in a Pnm_ppm image and converts each pixel's rgb values 
 *                      to component video color space values
 * Parameters: a Pnm_ppm image 
 * Returns: an A2Methods_UArray2 object containing structs of component video
 *                      values
 */
A2Methods_UArray2 ppm_to_component(Pnm_ppm image);



/*****************************************************************************
 *                          DECOMPRESSION FUNCTIONS                          *
 *****************************************************************************/
/* 
 * Function: component_to_ppm()
 * Purpose: reads in an A2Methods_UArray2 object containing structs of 
 *                      component video values and creates a Pnm_ppm object
 *                      containing an A2Methods_UArary2 object of rgb structs
 * Parameters: an A2Methods_UArray2 object containing structs of component 
 *                         video values
 * Returns: a Pnm_ppm object containing 
 */
Pnm_ppm component_to_ppm(A2Methods_UArray2 ybr_float_arr);

#endif