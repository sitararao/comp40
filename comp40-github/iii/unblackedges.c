#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "unblackedges.h"
#include "pnmrdr.h"


int main(int argc, char *argv[])
{
        if (argc > 2) {
                fprintf(stderr, "Error: please provide one argument.\n");
                exit(EXIT_FAILURE);
        }

        FILE *fp = NULL;
        Bit2_T file_bits;

        /* open appropriate file, store info in bit array */
        if(argc == 1)
                file_bits = pbmread(stdin);   
        else if(argc == 2) {
                fp = fopen(argv[1], "rb");
                assert(fp != NULL);
                file_bits = pbmread(fp);
                fclose(fp);
        }

        /* 2D bit array to store whether each pixel is a black edge */
        /* 1 = black edge; initialize all to 0 */
        Bit2_T black_edges = Bit2_new(Bit2_width(file_bits),
                                      Bit2_height(file_bits));
        init_edge_array(black_edges);

        /* Mark all black edges with 1s, calls check_nbrs to iterate */
        mark_edges(file_bits, black_edges);

        invert_edge_pixels(file_bits, black_edges);
        pbmwrite(stdout, file_bits);

        /* free bit arrays and exit program */
        Bit2_free(&black_edges);
        Bit2_free(&file_bits);

        exit(EXIT_SUCCESS);
        return 0;
}

Bit2_T pbmread (FILE *inputfp)
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

        /* check that image type is pbm and size > 0 */
        if (img_info.type != Pnmrdr_bit) {
                Pnmrdr_free(&img);
                pnmrdr_exception("Image is not in pbm format", inputfp);
        }
        int size = img_info.height * img_info.width;
        if (size == 0) {
                Pnmrdr_free(&img);
                pnmrdr_exception("Image not readable", inputfp);
        }

        /* create 2D bit array to store pixel values */
        Bit2_T pixels = Bit2_new(img_info.width, img_info.height);

        /* store each pixel in 2d bit array */
        /* here count is raised if we try to read past last pixel */
        TRY
                void *pbm_file = img;
                Bit2_map_row_major(pixels, store_pixels, pbm_file);
        EXCEPT(Pnmrdr_Count)
                Pnmrdr_free(&img);
                pnmrdr_exception("Too many pixels read", inputfp);
        END_TRY;

        /* here count is raised if we try to free without reading all pixels */
        TRY
                Pnmrdr_free(&img);
        EXCEPT(Pnmrdr_Count)
                pnmrdr_exception("Too few pixels read", inputfp);
        END_TRY;

        return pixels;
}

void store_pixels(int col, int row, Bit2_T arr, int pix, void *pbm)
{
        Bit2_put(arr, col, row, Pnmrdr_get(pbm));
        (void) pix;
}

/* Given a 2D bit array, itialize all bits to 0s */
void init_edge_array(Bit2_T black_edges)
{
        Bit2_map_row_major(black_edges, set_to_zero, NULL);
}

/* Initialize a bit at the current index in a 2D bit array to 0 */
void set_to_zero(int col, int row, Bit2_T arr, int pix, void *cl)
{
        (void) pix;
        (void) cl;
        Bit2_put(arr, col, row, 0);
}

/*
 * Given a 2D bit array representing black (1s) and white (0s) pixels,
 * mark all black edge pixels in another 2D bit array with 1s.
 */
void mark_edges(Bit2_T file_bits, Bit2_T black_edges)
{
        int max_rows = Bit2_height(file_bits);
        int max_cols = Bit2_width(file_bits);
        Seq_T stack = Seq_new((max_rows * max_cols / 2));

        /* check topmost row, L -> R */
        for (int col = 0; col < max_cols; col++) {
                if (Bit2_get(file_bits, col, 0) == 1) {
                        add_pos_to_stack(col, 0, stack);
                }
        }

        /* check leftmost column, top -> bottom */
        for (int row = 0; row < max_rows; row++) {
                if (Bit2_get(file_bits, 0, row) == 1) {
                        add_pos_to_stack(0, row, stack);
                }
        }

        /* check bottom row, L -> R */
        for (int col2 = 0; col2 < max_cols; col2++) {
                if (Bit2_get(file_bits, col2, max_rows - 1) == 1) {
                        add_pos_to_stack(col2, max_rows - 1, stack);
                }
        }

        /* check rightmost column, top -> bottom */
        for (int row2 = 0; row2 < max_rows; row2++) {
                if (Bit2_get(file_bits, max_cols - 1, row2) == 1) {
                        add_pos_to_stack(max_cols - 1, row2, stack);
                }
        }

        pop_and_mark(file_bits, black_edges, stack);
        Seq_free(&stack);
}

/* 
 * Create a Position struct from the column and row passed in as
 * arguments, and push it onto the stack argument.
 */
void add_pos_to_stack(int pos_col, int pos_row, Seq_T stack)
{
        Position *p = malloc(sizeof(Position));
        p->row = pos_row;
        p->col = pos_col;
        Seq_addhi(stack, p);
}

/* 
 * Pop a black edge pixel's position from the stack, mark it in the 2D
 * edge array, and call check_nbrs() on its 4 neighbor pixels.
 */
void pop_and_mark(Bit2_T file_bits, Bit2_T black_edges, Seq_T stack)
{
        Position *p;
        while(Seq_length(stack) > 0) {
                p = Seq_remhi(stack);
                        Bit2_put(black_edges, p->col, p->row, 1);
                        check_nbrs(file_bits, black_edges, (p->col) - 1, 
                                   p->row, stack);
                        check_nbrs(file_bits, black_edges, p->col,
                                   (p->row) - 1, stack);
                        check_nbrs(file_bits, black_edges, (p->col) + 1,
                                   p->row, stack);
                        check_nbrs(file_bits, black_edges, p->col,
                                   (p->row) + 1, stack);
                        free(p);
        }
}

/*
 * Visit a neighbor of a black edge pixel and, if it is also a black edge
 * pixel, push it onto the stack.
 */
void check_nbrs(Bit2_T file_bits, Bit2_T black_edges, int curr_col,
                int curr_row, Seq_T stack) {
        if (curr_col >= 0 && curr_col < Bit2_width(file_bits) && curr_row >= 0
            && curr_row < Bit2_height(file_bits)) {
                if (Bit2_get(file_bits, curr_col, curr_row) == 1
                    && Bit2_get(black_edges, curr_col, curr_row) != 1) {
                        Position *p = malloc(sizeof(Position));
                        p->row = curr_row;
                        p->col = curr_col;
                        Seq_addhi(stack, p);
                }
        }
}

/*
 * Rewrite the bit values in a 2D bit array to stdout, changing black edge
 * pixels to white in the process.
 */
void invert_edge_pixels(Bit2_T file_bits, Bit2_T black_edges)
{
    Bit2_map_row_major(file_bits, black_to_white, &black_edges);
}

/* 
 * Invert the pixel value at the current index in a 2D bit array 
 * if it is marked as a black edge in another parallel 2D bit array
 */
void black_to_white(int col, int row, Bit2_T file_bits, int pix, void *edges)
{
        (void) pix;
        Bit2_T *black_edges = edges;
        if (Bit2_get(*black_edges, col, row) == 1) {
                Bit2_put(file_bits, col, row, 0);
        }
}

/* Write the values in a 2D bitmap into a pbm outfile */
void pbmwrite(FILE *outputfp, Bit2_T bitmap)
{
        fprintf(outputfp, "P1\n");
        fprintf(outputfp, "%d %d\n", Bit2_width(bitmap), Bit2_height(bitmap));
        Bit2_map_row_major(bitmap, print_bit, outputfp);
}

/* Write the pixel value at the curr index in a 2D bitmap into pbm outfile */
void print_bit(int col, int row, Bit2_T bitmap, int pix, void *outputfp)
{
        FILE *outfile = outputfp;
        fprintf(outfile, "%d", pix);
        (void) row;

        if (col == (Bit2_width(bitmap) - 1)) {
                fprintf(outfile, "\n");
        }
        else {
                fprintf(outfile, " ");
        }
}

void pnmrdr_exception(char *message, FILE *fp)
{
        if (fp != stdin) {
                fclose(fp);
        }
        fprintf(stderr, "Error: %s. Program exiting.\n", message);
        exit(EXIT_FAILURE);
}
