/*
 *     a2plain.c
 *
 *     Assignment: Comp40 HW3 (Locality)
 *     Authors:  Sitara Rao (srao03) and Era Iyer (eiyer01)
 *     Date: 10/9/2019
 *
 *     This file is one of 2 underlying implementations used by the A2Methods
 *     interface class. It exports a pointer to a struct of function pointers
 *     called uarray2_methods_plain, for use by clients. Each function pointer
 *     calls the appropriate UArray2 function, from the UArray2 interface.
 */ 

#include <string.h>
#include <a2plain.h>
#include "uarray2.h"

/************************************************/
/* Define a private version of each function in */
/* A2Methods_T that we implement.               */
/************************************************/

typedef A2Methods_UArray2 A2;

/*
 * Initializes and returns a new A2Methods_UArray2 object that has a width,
 * height, and elem size specified by the parameters
 */
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/*
 * Initializes and returns a new A2Methods_UArray2 object that has a width,
 * height, elem size specified by the parameters. Ignores blocksize.
 */
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        return UArray2_new(width, height, size);
        (void) blocksize;
}

/*
 * frees all memory associated with the A2Methods_UArray2 object created
 */
static void a2free(A2 * array2)
{
        UArray2_free((UArray2_T *) array2);
}

/*
 * returns width of A2Methods_UArray2 object provided
 */
static int width(A2 array2)
{
        return UArray2_width(array2);
}

/*
 * returns height of A2Methods_UArray2 object provided
 */
static int height(A2 array2)
{
        return UArray2_height(array2);
}

/*
 * returns elem size of A2Methods_UArray2 object provided
 */
static int size(A2 array2)
{
        return UArray2_size(array2);
}

/*
 * returns blocksize, which is always 1, of A2Methods_UArray2 object provided
 */
static int blocksize(A2 array2)
{
        (void)array2;
        return 1;
}

/*
 * returns pointer to value at A2Methods_UArray2 object at location specified
 * in parameter
 */
static A2Methods_Object *at(A2 array2, int i, int j)
{
        return UArray2_at(array2, i, j);
}

/*
 * calls UArray2_map_row_major with values specified in parameter
 */
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/*
 * calls UArray2_map_col_major with values specified in parameter
 */
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/*
 * creates a struct containing an A2Methods_smallapplyfun pointer and a void *
 */
struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void *cl;
};

/*
 * calling apply function on elem and closure within the void closure
 */
static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void) i;
        (void) j;
        (void) uarray2;
        cl->apply(elem, cl->cl);
}

/*
 * initializes small_closure struct values to values specified in parameter
 * and calls UArray2_map_row_major with initialized struct as closure
 */
static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

/*
 * initializes small_closure struct values to values specified in parameter
 * and calls UArray2_map_col_major with initialized struct as closure
 */
static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

/*
 * struct of all function pointers 
 */
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,
        map_col_major,
        NULL,           // map_block_major
        map_row_major,  // map_default
        small_map_row_major,
        small_map_col_major,
        NULL,           //small_map_block_major
        small_map_row_major,  // small_map_default
};

/* finally the payoff: here is the exported pointer to the struct */
A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
