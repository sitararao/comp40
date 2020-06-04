/******************************************************************************
 *
 *     word_mod.c
 *
 *     Assignment: Comp40 HW4 (Arith)
 *     Authors:    Sitara Rao (srao03) and Era Iyer (eiyer01)
 *     Date:       10/23/2019
 *
 *     This file implements the word_mod module for both compression and 
 *     decompression. This module is responsible for scaling the chroma values,
 *     packing them into uint64_ts, and unpacking them from uint64_ts to 
 *     dct_floats
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "assert.h"
#include "math.h"

#include "word_mod.h"
#include "arith40.h"
#include "bitpack.h"


/* Struct containing chroma values for a block of pixels */
typedef struct dct_floats {
        float a;
        float b;
        float c;
        float d;
        float avg_pb;
        float avg_pr;
} *dct_floats;

/* Properties of scaled chroma values for bitpacking purposes */
const unsigned A_WIDTH  = 6;
const unsigned B_WIDTH  = 6;
const unsigned C_WIDTH  = 6;
const unsigned D_WIDTH  = 6;
const unsigned PB_WIDTH = 4;
const unsigned PR_WIDTH = 4;

const unsigned A_LSB  = 26;
const unsigned B_LSB  = 20;
const unsigned C_LSB  = 14;
const unsigned D_LSB  = 8;
const unsigned PB_LSB = 4;
const unsigned PR_LSB = 0;

const bool A_SIGN  = false;
const bool B_SIGN  = true;
const bool C_SIGN  = true;
const bool D_SIGN  = true;
const bool PB_SIGN = false;
const bool PR_SIGN = false;

/* Formula to scale between a, b, c, d floats and bitpacked values */ 
#define scale(signed, width, max) (signed ?                                  \
                                   (floor((pow(2, width  - 1) - 1) / max)) : \
                                   (floor((pow(2, width) - 1)      / max)))

/* Maximum absolute values of floats, pre-scaling */
const float MAX_A = 1;
const float MAX_B = 0.3;
const float MAX_C = 0.3;
const float MAX_D = 0.3;




/*****************************************************************************
*                           COMPRESSION FUNCTIONS                            *
******************************************************************************/

/*
 *  Compression helper functions --> apply() functions for UArray2 mapping
 */
void scale_chroma(int col, int row, A2Methods_UArray2 arr, void *elem,
                           void *cl);
void pack_chroma(int col, int row, A2Methods_UArray2 arr, void *elem,
                           void *cl);

/*
 * pack_array()
 * Takes an A2Methods_UArray2 containing dct_floats as arg
 * Returns an A2Methods_UArray2 containing uint64_t words 
 * Each word in returned UArray2 packs scaled dct floats into 32 bits
 * Frees memory associated with input chroma_array
 */
A2Methods_UArray2 pack_array(A2Methods_UArray2 chroma_array)
{
        assert(chroma_array != NULL);
        A2Methods_T methods_b = uarray2_methods_blocked;

        /* scale a, b, c, d to min/max value as needed */
        methods_b->map_default(chroma_array, &scale_chroma, NULL);

        int width = methods_b->width(chroma_array);
        int height = methods_b->height(chroma_array);        
        A2Methods_T methods_p = uarray2_methods_plain;

        /* 
         * Scale pb,pr values to unsigneds, pack them along w a, b, c, d into
         * 32-bit words, then store each word (a uint64_t) in word_array
         */
        A2Methods_UArray2 word_array = methods_p->new(width, height,
                                                      sizeof(uint64_t));
        assert(word_array != NULL);
        methods_b->map_default(chroma_array, &pack_chroma, word_array);

        methods_b->free(&chroma_array);
        return word_array;
}

/*
 * scale_chroma()
 * apply() function called in pack_array() that scales a, b, c, and d
 * values to be within the appropriate range as defined by consts
 */
void scale_chroma(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{
        assert((arr != NULL) && (elem != NULL));
        dct_floats curr_chroma = elem;

        if (curr_chroma->a > MAX_A) {
                curr_chroma->a = MAX_A;
        } else if (A_SIGN && (curr_chroma->a < (-1 * MAX_A))) {
                curr_chroma->a = (-1 * MAX_A);
        }

        if (curr_chroma->b > MAX_B) {
                curr_chroma->b = MAX_B;
        } else if (B_SIGN && (curr_chroma->b < (-1 * MAX_B))) {
                curr_chroma->b = (-1 * MAX_B);
        }

        if (curr_chroma->c > MAX_C) {
                curr_chroma->c = MAX_C;
        } else if (C_SIGN && (curr_chroma->c < (-1 * MAX_C))) {
                curr_chroma->c = (-1 * MAX_C);
        }

        if (curr_chroma->d > MAX_D) {
                curr_chroma->d = MAX_D;
        } else if (D_SIGN && (curr_chroma->d < (-1 * MAX_D))) {
                curr_chroma->d = (-1 * MAX_D);
        }

        (void) col;
        (void) row;
        (void) arr;
        (void) cl;
}

/*
 * pack_chroma()
 * apply function called in pack_array() that bitpacks the dct_float chroma
 * values into a uint64_t word, and inserts the word into same index of an
 * A2Methods_UArray2 object containing uint64_ts
 */
void pack_chroma(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{
        assert((arr != NULL) && (elem != NULL) && (cl != NULL));
        dct_floats curr_chroma = elem;

        /* scale dct floats to integers that fit in width digits */
        uint64_t a = round(scale(A_SIGN, A_WIDTH, MAX_A) * curr_chroma->a);
        int64_t  b = round(scale(B_SIGN, B_WIDTH, MAX_B) * curr_chroma->b);
        int64_t  c = round(scale(C_SIGN, C_WIDTH, MAX_C) * curr_chroma->c);
        int64_t  d = round(scale(D_SIGN, D_WIDTH, MAX_D) * curr_chroma->d);
        uint64_t ind_pb = Arith40_index_of_chroma(curr_chroma->avg_pb);
        uint64_t ind_pr = Arith40_index_of_chroma(curr_chroma->avg_pr);

        uint64_t word = 0;

        /* pack scaled integers into 32 bits of word */
        word = Bitpack_newu(word, A_WIDTH,  A_LSB,  a);
        word = Bitpack_news(word, B_WIDTH,  B_LSB,  b);
        word = Bitpack_news(word, C_WIDTH,  C_LSB,  c);
        word = Bitpack_news(word, D_WIDTH,  D_LSB,  d);
        word = Bitpack_newu(word, PB_WIDTH, PB_LSB, ind_pb);
        word = Bitpack_newu(word, PR_WIDTH, PR_LSB, ind_pr);

        /* store word at same index in word array */
        A2Methods_UArray2 words = cl;
        A2Methods_T methods_p = uarray2_methods_plain;
        uint64_t *curr_word = methods_p->at(words, col, row);
        *curr_word = word;

        (void) arr;
}


/*****************************************************************************
*                           DECOMPRESSION FUNCTIONS                          *
******************************************************************************/

/*
 *  Decompression helper function --> apply() function for UArray2 mapping
 */
void unpack_word(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl);

/*
 * unpack_array()
 * Takes in an A2Methods_UArray2 containing uint64_t words as arg
 * Unpacks all 6 ints packed into each word, then converts them back to floats
 * Frees memory associated with input word_array
 * Returns an A2Methods_UArray2 containing dct_floats
 */
A2Methods_UArray2 unpack_array(A2Methods_UArray2 word_array)
{
        assert(word_array != NULL);
        A2Methods_T methods_p = uarray2_methods_plain;
        int width  = methods_p->width(word_array);
        int height = methods_p->height(word_array);

        A2Methods_T methods_b = uarray2_methods_blocked;
        A2Methods_UArray2 chroma_array = methods_b->new(width, height,
                                                sizeof(struct dct_floats));
        assert(chroma_array != NULL);

        methods_p->map_default(word_array, &unpack_word, chroma_array);

        methods_p->free(&word_array);
        return chroma_array;
}

/*
 * unpack_word()
 * apply() function for unpack_array that unpacks integer values from the
 * uint64_t word, scales them back to floats, then stores as floats in
 * closure A2Methods_UArray2 of chroma values
 */
void unpack_word(int col, int row, A2Methods_UArray2 arr, void *elem,
                         void *cl)
{
        assert((arr != NULL) && (elem != NULL) && (cl != NULL));
        uint64_t *curr_word  = elem;

        /* unpack values from current word */
        uint64_t a_bits      = Bitpack_getu(*curr_word, A_WIDTH,  A_LSB);
        int64_t  b_bits      = Bitpack_gets(*curr_word, B_WIDTH,  B_LSB);
        int64_t  c_bits      = Bitpack_gets(*curr_word, C_WIDTH,  C_LSB);
        int64_t  d_bits      = Bitpack_gets(*curr_word, D_WIDTH,  D_LSB);
        uint64_t ind_pb_bits = Bitpack_getu(*curr_word, PB_WIDTH, PB_LSB);
        uint64_t ind_pr_bits = Bitpack_getu(*curr_word, PR_WIDTH, PR_LSB);

        /* calculate dct_float values by reversing compression scaling */
        float a = ((float)a_bits / (float)scale(A_SIGN, A_WIDTH, MAX_A)); 
        float b = ((float)b_bits / (float)scale(B_SIGN, B_WIDTH, MAX_B));
        float c = ((float)c_bits / (float)scale(C_SIGN, C_WIDTH, MAX_C));
        float d = ((float)d_bits / (float)scale(D_SIGN, D_WIDTH, MAX_D));
        float avg_pb = Arith40_chroma_of_index(ind_pb_bits);
        float avg_pr = Arith40_chroma_of_index(ind_pr_bits);

        A2Methods_UArray2 chroma_array = cl;
        A2Methods_T methods_b = uarray2_methods_blocked;

        /* store values in dct_floats struct at curr index of chroma_array */
        dct_floats curr_chroma = methods_b->at(chroma_array, col, row);
        curr_chroma->a = a;
        curr_chroma->b = b;
        curr_chroma->c = c;
        curr_chroma->d = d;
        curr_chroma->avg_pb = avg_pb;
        curr_chroma->avg_pr = avg_pr;

        (void) arr;
}
