#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "sudoku.h"
#include "pnmrdr.h"


int main(int argc, char *argv[])
{
        if (argc > 2) {
                fprintf(stderr, "Error: please provide one argument.\n");
                exit(1);
        }

        FILE *fp = NULL;
        UArray2_T puzzle_file;

        /* open appropriate file, store info in bit array */
        if(argc == 1)
                puzzle_file = pgmread(stdin);   
        else if(argc == 2) {
                fp = fopen(argv[1], "rb");
                assert(fp != NULL);
                puzzle_file = pgmread(fp);
                fclose(fp);
        }

        check_sudoku(puzzle_file);

        UArray2_free(&puzzle_file);
        exit(0);
        return 0;
}

UArray2_T pgmread(FILE *inputfp)
{
        Pnmrdr_T img;
        Pnmrdr_mapdata img_info;

        /* Try opening the reader using the specified image */
        TRY
                img = Pnmrdr_new(inputfp);
        EXCEPT(Pnmrdr_Badformat)
                pnmrdr_exception("Not a readable image file", inputfp);
        END_TRY;

        /* get image info */
        img_info = Pnmrdr_data(img);

        /* check that image type is pbm */
        if (img_info.type != Pnmrdr_gray) {
                Pnmrdr_free(&img);
                pnmrdr_exception("Image is not in pgm format", inputfp);
        }

        /* check image denominator is 9 and height/width are 9 */
        if (img_info.denominator != 9 || img_info.height != 9 ||
                                         img_info.width != 9) {
                if (inputfp != stdin && inputfp != NULL)
                        fclose(inputfp);
                Pnmrdr_free(&img);
                exit(1);
        }

        /* create 2D uarray to store pixel values */
        UArray2_T pixels = UArray2_new(9, 9, sizeof(int));

        for (int r = 0; r < 9; r++) {
                for (int c = 0; c < 10; c++) {
                        TRY
                                int pix = Pnmrdr_get(img);
                                int *store_pix = UArray2_at(pixels, c, r);
                                *store_pix = pix;
                        EXCEPT(Pnmrdr_Count)
                                Pnmrdr_free(&img);
                                UArray2_free(&pixels);
                                pnmrdr_exception("Too many pixels read",
                                                 inputfp);
                        END_TRY;    
                }
        }

        /* free the reader, return the 2D Uarray */

        Pnmrdr_free(&img);
        return pixels;
}


void pnmrdr_exception(char *message, FILE *fp)
{
        if ( fp != stdin && fp != NULL)
                fclose(fp);
        fprintf(stderr, "Error: %s. Program exiting.\n", message);
        exit(1);
}

void check_sudoku(UArray2_T pix_array)
{
        if (check_rows(pix_array) && check_cols(pix_array) 
            && check_subarrays(pix_array)) {
                UArray2_free(&pix_array);                 //SHOULD WE RETURN TO MAIN
                exit(0);
        } else {
                UArray2_free(&pix_array);
                exit(1);
        }
}

bool check_rows(UArray2_T pix_array)
{
        for (int row = 0; row < 9; row++) {
                int subset[9] = {0};
                for (int col = 0; col < 9; col++) {
                        int *pixel = UArray2_at(pix_array, col, row);
                        if (*pixel == 0)
                                return false;
                        else {
                                for (int i = 0; i <= col; i++) {
                                        if  (*pixel == subset[i]) {
                                                return false;
                                        }
                                }
                                subset[col] = *pixel;
                        }
                }
        }
        return true;
}

bool check_cols(UArray2_T pix_array)
{
        for (int col = 0; col < 9; col++) {
                int subset[9] = {0};
                for (int row = 0; row < 9; row++) {
                        int *pixel = UArray2_at(pix_array, col, row);
                        if (*pixel == 0)
                                return false;
                        else {
                                for (int i = 0; i <= row; i++) {
                                        if  (*pixel == subset[i]) {
                                                return false;
                                        }
                                }
                                subset[row] = *pixel;
                        }
                }
        }
        return true;
}

bool check_subarrays(UArray2_T pix_array)
{
        for (int row = 0; row < 9; row += 3 ) {
                for (int col = 0; col < 9; col += 3) {
                        if (! subarray(pix_array, col, row)) {
                                return false;
                        }
                }
        }
        return true;
}

bool subarray(UArray2_T pix_array, int col, int row)
{
        int subset[9] = {0};
        for (int r = row; r < (row+3); r++) {
                for (int c = col; c < (col+3); c++) {
                        int *pixel = UArray2_at(pix_array, c, r);
                        if (*pixel == 0)
                                return false;
                        else {
                                int index = ((c - col) + (3*(r - row)));
                                for (int i = 0; i <= index; i++) {
                                        if  (*pixel == subset[i]) {
                                                return false;
                                        }
                                }
                                subset[index] = *pixel;
                        }
                }
        }
        return true;
}


