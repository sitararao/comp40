#ifndef SUDOKU_H
#define SUDOKU_H


#include "uarray2.h"

UArray2_T pgmread(FILE *inputfp);
void pnmrdr_exception(char * message, FILE *fp);

void check_sudoku(UArray2_T pix_array);
bool check_rows(UArray2_T pix_array);
bool check_cols(UArray2_T pix_array);
bool check_subarrays(UArray2_T pix_array);
bool subarray(UArray2_T pix_array, int col, int row);


#endif