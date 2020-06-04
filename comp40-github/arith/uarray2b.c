/*
 *     uarray2b.c
 *
 *     Assignment: Comp40 HW3 (Locality)
 *     Authors:  Sitara Rao (srao03) and Era Iyer (eiyer01)
 *     Date: 10/9/2019
 *
 *     This file contains the implementation of a UArray2b object.
 *     It defines functions for creating, destroying, manipulating, and
 *     getting info about UArray2b objects.
 *     
 */ 

#include <stdio.h>
#include <stdlib.h>
#include "uarray2b.h"
#include "uarray2.h"        
#include "uarray.h"        
#include "math.h"
#include "assert.h"
#include "except.h"

Except_T Mem_Fail = {"Error: memory could not be allocated."};

#define T UArray2b_T

/* 
 * init_blocks()
 * Purpose: initialization apply() function
 *          at the elem passed in, initializes a new UArray object of the
 *          appropriate size as specified by the closure 
 */
void init_blocks(int col, int row, UArray2_T arr, void *elem, void *cl);

/* 
 * free_blocks()
 * Purpose: memory-freeing apply() function
 *          frees the UArray object at *elem.
 */
void free_blocks(int col, int row, UArray2_T arr, void *elem, void *cl);

/*
 * UArray2b_T struct containing information about the UArray2b_T as well as
 * the underlying representation of a UArray2b_T object (A UArray2_T containing
 * several UArray_T objects.)
 */
struct T {
        int height;
        int width;
        int size;
        int blocksize;
        UArray2_T block_array;
};

/*
 * Passed as closure argument to map function
 * when initializing a new UArray2b
 */
struct init_closure {
        void *num_cells;
        void *cell_size;
};

/*
 * Function: UArray2b_new()
 * Parameters: int w, int h, int sz, int bsize
 * Returns: UArray2b_T object
 * Purpose: Initializes and returns a new UArray2b_T object with width, height,
 *          elem size, and blocksize specified in parameter
 * Expectations: width and height are >= 0, cell size and blocksize are > 0
 */
T UArray2b_new(int w, int h, int sz, int bsize)
{
        assert((w >= 0) && (h >= 0) && (sz > 0) && (bsize > 0));

        T arr2b = malloc(sizeof(*arr2b) + (4 * sizeof(int)));
        if (arr2b == NULL)              /* checks that malloc'd correctly */
                RAISE(Mem_Fail);

        arr2b->width = w;
        arr2b->height = h;
        arr2b->blocksize = bsize;
        arr2b->size = sz;

        /* 
         * Number of blocks across width of uarray2b is upper limit of total
         * width divided by blocksize 
         */
        int block_width = ceil((double)w / bsize);  

        /* 
         * Number of blocks across height of uarray2b is upper limit of total
         * height divided by blocksize 
         */
        int block_height = ceil((double)h / bsize);

        int block_sz = bsize * bsize * sz;

        arr2b->block_array = UArray2_new(block_width, block_height, block_sz);
        if (arr2b->block_array == NULL) 
                RAISE(Mem_Fail);

        int num_cells = bsize * bsize;
        struct init_closure myinit = { &num_cells, &sz };
        UArray2_map_row_major(arr2b->block_array, &init_blocks, &myinit);
        return arr2b;
}


/*
 * Function: init_blocks()
 * Parameters: int col, int row, UArray2_T arr, void *elem, void *cl
 * Returns: nothing
 * Purpose: initializes each block to be a new UArray object of a certain size
 *          specified by the closure
 */
void init_blocks(int col, int row, UArray2_T arr, void *elem, void *cl)
{
        struct init_closure *myinit = cl;
        int *n_cells = myinit->num_cells;
        int *c_size = myinit->cell_size;

        *(UArray_T*) elem = UArray_new(*n_cells, *c_size);

        if (elem == NULL)
                RAISE(Mem_Fail);
        
        (void) col;
        (void) row;
        (void) arr;
}

/*
 * Function: UArray2b_new_64K_block()
 * Parameters: int w, int h, int sz
 * Returns: UArray2b_T object
 * Purpose: Initializes and returns a new UArray2b_T object with width, height,
 *          and elem size specified in parameter. Chooses a blocksize that is
 *          as large as possible while still allowing a block to fit in 64KB 
 * Expectations: width and height >= 0, cell size is > 0 
 */
T UArray2b_new_64K_block(int w, int h, int sz)
{
        assert((w >= 0) && (h >= 0) && (sz > 0));

        T arr2b = malloc(sizeof(*arr2b) + (4 * sizeof(int)));
        if (arr2b == NULL)          /* checks that malloc'd correctly */
                RAISE(Mem_Fail);

        arr2b->width = w;
        arr2b->height = h;
        arr2b->size = sz;
        int bsize;

        if (sz > 64000)
                bsize = 1;
        else {
                int max_b_squared = (64000 / sz);
                int max_bsize = floor(sqrt(max_b_squared));

                bsize = max_bsize;
        }
        arr2b->blocksize = bsize;

        /* width of each block is upper limit of total w divided by blksz */
        int block_width = ceil((double)w/bsize);
        
        /* height of each block is upper limit of total h divided by blksz */
        int block_height = ceil((double)h/bsize);

        int block_sz = bsize * bsize * sz;

        arr2b->block_array = UArray2_new(block_width, block_height, block_sz);
        if (arr2b->block_array == NULL)
                RAISE(Mem_Fail);

        int num_cells = bsize * bsize;
        struct init_closure myinit = { &num_cells, &sz };
        UArray2_map_row_major(arr2b->block_array, &init_blocks, &myinit);

        return arr2b;
}

/*
 * Function: UArray2b_free()
 * Parameters: a pointer to a UArray2b_T object 
 * Returns: nothing
 * Purpose: frees all memory associated with the UArray2b_T object
 * Expectations: array2b and pointer to array2b are non-null
 */
void UArray2b_free (T *array2b)
{
        assert(array2b && *array2b);
        UArray2_map_row_major((*array2b)->block_array, &free_blocks, NULL);
        UArray2_free(&((*array2b)->block_array));
        free(*array2b);
}

/*
 * Function: free_blocks()
 * Parameters: int col, int row, UArray2_T arr, void *elem, void *cl
 * Returns: nothing
 * Purpose: frees all memory associated with the current block
 */
void free_blocks(int col, int row, UArray2_T arr, void *elem, void *cl)
{
        
        UArray_free((UArray_T *)elem);
        (void) col;
        (void) row;
        (void) arr;
        (void) cl;
}

/*
 * Function: UArray2b_width()
 * Parameters: UArray2b_T array2b
 * Returns: int
 * Purpose: returns the width of the UArray2b_T object
 * Expectations: array2b is non-null
 */
int UArray2b_width(T array2b)
{
        assert(array2b);
        return array2b->width;
}

/*
 * Function: UArray2b_height()
 * Parameters: UArray2b_T array2b
 * Returns: int
 * Purpose: returns the height of the UArray2b_T object
 * Expectations: array2b is non-null
 */
int UArray2b_height(T array2b)
{
        assert(array2b);
        return array2b->height;
}

/*
 * Function: UArray2b_size()
 * Parameters: UArray2b_T array2b
 * Returns: int
 * Purpose: returns the cell size of the UArray2b_T object
 * Expectations: array2b is non-null
 */
int UArray2b_size(T array2b)
{
        assert(array2b);
        return array2b->size;
}

/*
 * Function: UArray2b_blocksize()
 * Parameters: UArray2b_T array2b
 * Returns: int
 * Purpose: returns the blocksize of the UArray2b_T object
 * Expectations: array2b is non-null
 */
int UArray2b_blocksize(T array2b)
{
        assert(array2b);
        return array2b->blocksize;
}

/*
 * Function: UArray2b_at()
 * Parameters: UArray2b_T array2b, int column, int row
 * Returns: void *
 * Purpose: returns a void pointer to the element at the location specified
 *          by the column and row given int the parameter
 * Expectations: array2b is non-null, indices column and row are within
 *               array2b dimensions
 */
void *UArray2b_at(T array2b, int column, int row)
{       
        assert(array2b);
        assert(column < array2b->width && row < array2b->height);
        int block_size = array2b->blocksize;

        int uarr2_c = (column / block_size);
        int uarr2_r = (row / block_size);
        UArray_T *my_block = UArray2_at(array2b->block_array, uarr2_c,
                                        uarr2_r);
        int block_row = (row % block_size);
        int block_col = (column % block_size);
        void *my_elem = UArray_at(*my_block,
                                  ((block_row * block_size) + block_col));
        return my_elem;
}

/*
 * Function: UArray2b_map()
 * Parameters: UArray2b_T array2b, void apply, void *cl
 * Returns: nothing
 * Purpose: mapping function that iterates through every element in the 
 *          UArray2b_T
 * Expectations: array2b is non-null
 */
void  UArray2b_map(T array2b, void apply(int col, int row, T array2b, 
                                         void *elem, void *cl),
                   void *cl)
{
        assert(array2b);

        int block_size = array2b->blocksize;
        int width = array2b->width;
        int height = array2b->height;
        
        /* 
         * Number of blocks across width of uarray2b is upper limit of total
         * width divided by blocksize 
         */ 
        int num_blocks_width = ceil((double)width / (block_size));

        /* 
         * Number of blocks across height of uarray2b is upper limit of total
         * height divided by blocksize 
         */
        int num_blocks_height = ceil((double)height / (block_size));


        /* Note: we switched to 4-space tabs for these nested loops */
        /* Iterate through each block in UArray2b */
        for (int j = 0; j < num_blocks_height; j++){
            for (int i = 0; i < num_blocks_width; i++){

                /* Within each block */
                UArray_T* my_block = UArray2_at(array2b->block_array,
                                                i, j);

                /* Iterate through each cell in the block */
                for (int cell = 0; cell < UArray_length(*my_block); cell++) {
                        int arr2b_col = (i * block_size) + (cell % block_size);
                        int arr2b_row = (j * block_size) + (cell / block_size);

                        /* make sure indices are not out-of-bounds, and pass */
                        /* the current col and row indices to apply func */
                        if ((arr2b_col < width) && (arr2b_row < height)) {
                            apply(arr2b_col, arr2b_row, array2b,
                                  UArray2b_at(array2b, arr2b_col, arr2b_row),
                                              cl);
                    }
                }
            }
        }
}