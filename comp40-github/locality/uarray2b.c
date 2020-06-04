#include <stdio.h>
#include <stdlib.h>
#include "uarray2b.h"
#include "uarray2.h"        // need to find these files
#include "uarray.h"         // need to find these files
#include "math.h"
#include "assert.h"
#include "except.h"

Except_T Mem_Fail = {"Error: memory could not be allocated."};

#define T UArray2b_T

struct T {
        int height;
        int width;
        int size;
        int blocksize;
        UArray2_T block_array;
};

struct init_closure {
        void *num_cells;
        void *cell_size;
};

void init_blocks(int col, int row, UArray2_T arr, void *elem, void *cl);
void free_blocks(int col, int row, UArray2_T arr, void *elem, void *cl);


T UArray2b_new(int w, int h, int sz, int bsize)
{
        assert((w >= 0) && (h >= 0) && (sz > 0) && (bsize > 0));

        T arr2b = malloc(sizeof(*arr2b) + (4 * sizeof(int)));
        if (arr2b == NULL)
                RAISE(Mem_Fail);

        arr2b->width = w;
        arr2b->height = h;
        arr2b->blocksize = bsize;
        arr2b->size = sz;

        int block_width = ceil((double)w/bsize);
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



T UArray2b_new_64K_block(int w, int h, int sz)
{
        assert((w >= 0) && (h >= 0) && (sz > 0));

        T arr2b = malloc(sizeof(*arr2b) + (4 * sizeof(int)));
        if (arr2b == NULL)
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

        int block_width = ceil((double)w/bsize);
        int block_height = ceil((double)h/bsize);
        int block_sz = bsize * bsize * sz;

        arr2b->block_array = UArray2_new(block_width, block_height, block_sz);
        if (arr2b->block_array == NULL)
                RAISE(Mem_Fail);

        // assert 
        int num_cells = bsize * bsize;
        struct init_closure myinit = { &num_cells, &sz };
        UArray2_map_row_major(arr2b->block_array, &init_blocks, &myinit);

        return arr2b;
}

void UArray2b_free (T *array2b)
{
        assert(array2b && *array2b);
        UArray2_map_row_major((*array2b)->block_array, &free_blocks, NULL);
        UArray2_free(&((*array2b)->block_array));
        free(*array2b);

}

void free_blocks(int col, int row, UArray2_T arr, void *elem, void *cl)
{
        
        UArray_free((UArray_T *)elem);
        (void) col;
        (void) row;
        (void) arr;
        (void) cl;
}

int UArray2b_width(T array2b)
{
        assert(array2b);
        return array2b->width;
}

int UArray2b_height(T array2b)
{
        assert(array2b);
        return array2b->height;
}

int UArray2b_size(T array2b)
{
        assert(array2b);
        return array2b->size;
}

int UArray2b_blocksize(T array2b)
{
        assert(array2b);
        return array2b->blocksize;
}

void *UArray2b_at(T array2b, int column, int row)
{       
        assert(array2b);
        assert(column < array2b->width);
        assert(row < array2b->height);
        int block_size = array2b->blocksize;

        int uarr2_c = (column / block_size);
        int uarr2_r = (row / block_size);
        UArray_T* my_block = UArray2_at(array2b->block_array, uarr2_c, uarr2_r);
        int block_row = (row%block_size);
        int block_col = (column%block_size);
        void *my_elem = UArray_at(*my_block, ((block_row * block_size) + block_col));
        return my_elem;
}

void  UArray2b_map(T array2b, void apply(int col, int row, T array2b, 
                                         void *elem, void *cl),
                   void *cl)
{
        assert(array2b);
    //iterate through each block in the Uarray2b
    //within each block, iterate through all of the block's elements 
    //print out indices so we know we are traversing properly
    //call apply function on each element
        int block_size = array2b->blocksize;

        int width = array2b->width;
        int height = array2b->height;
        
        int num_blockw = ceil((double)width/(block_size));
        int num_blockr = ceil((double)height/(block_size));
        for(int j = 0; j < num_blockr; j++){
            for(int i = 0; i < num_blockw; i++){
                UArray_T* my_block = UArray2_at(array2b->block_array, i, j);
                (void) my_block;
                for(int m = 0; m < UArray_length(*my_block); m++){
                    int c = i*block_size+(m%block_size);
                    int r = j*block_size + (m/block_size);

                    
                    if((c < width)&&(r < height)){

                        apply(c, r, array2b, UArray2b_at(array2b, c, r), cl);
                    }
                }
            }
        }


        (void) array2b;
        (void) apply;
       (void) cl;

}