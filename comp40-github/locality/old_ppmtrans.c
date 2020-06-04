#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "uarray2b.h"
#include "uarray2.h"
#include "pnm.h"
#include "cputiming.h"


A2Methods_UArray2 rotate_ppm(Pnm_ppm orig, A2Methods_T methods, A2Methods_mapfun map,
                    int rotation);

void rotate_90(int col, int row, A2Methods_UArray2 arr, void *elem, void *img);
void rotate_180(int col, int row, A2Methods_UArray2 arr, void *elem, void *img);


#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (0)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

int main(int argc, char *argv[]) 
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;
        //(void) time_file_name;
        FILE *img_file = fopen(argv[1], "r");
        if (img_file == NULL) {
                fprintf(stderr, "Error: could not open image. Program exiting.\n");
                exit(1);
        }
        
        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        for (i = 2; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
				    "row-major");
                        //printf("changing to row-major mapping\n");
                        map = methods->map_row_major;
                        // change map to row-major

                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
				    "column-major");
                        //printf("changing to col-major mapping\n");
                        map = methods->map_col_major;
                        // change map to col-major

                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                        //printf("changing to block-major mapping\n");
                        methods = uarray2_methods_blocked;
                        map = methods->map_block_major;

                        // change methods to uarray2_methods_blocked
                        // change map to block-major

                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
					"Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-time") == 0) {
                        time_file_name = argv[++i];
                        //printf("writing time data to file: %s\n", time_file_name);
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
				argv[i]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {
                        break;
                }
        }

        Pnm_ppm orig_image = Pnm_ppmread(img_file, methods);
    
        //printf("image width: %d pixels\n", orig_image->width);
        //printf("image height: %d pixels\n", orig_image->height);

        // (only do if rotation != 0 )
        // also if time_file_name != "" start timer before and after rotation

        CPUTime_T timer = CPUTime_New();
        double time_spent;

        if (time_file_name) {
                CPUTime_Start(timer);
        }

        if (rotation != 0) {

                A2Methods_UArray2 rotated_img = rotate_ppm(orig_image, methods, map, rotation);

                methods->free(&(orig_image->pixels));
                orig_image->pixels = rotated_img;

                orig_image->width = methods->width(orig_image->pixels);
                orig_image->height = methods->height(orig_image->pixels);

        }

        //printf("rotation width: %d\n", orig_image->width);
        //printf("rotation height: %d\n", orig_image->height);

        if (time_file_name) {
                time_spent = CPUTime_Stop(timer);
                FILE *time_file;

                if ( (time_file = fopen(time_file_name, "w") ) == NULL ) {
                        fprintf(stderr, "Error: can't create file\n");
                        exit(1);
                } else {
                        //char time_arr[sizeof(time_spent)];
                        //sprintf(time_arr, "%lf", time_spent);
                        //memcpy(time_arr, &time_spent, sizeof(time_spent));
                        //fwrite(time_arr, time_file);

                        fprintf(time_file, "%f\n", time_spent);
                        fclose(time_file);
                        //printf(" time in nanoseconds: %s\n", time_arr);
                }
                //write nanoseconds to time file
        }

        Pnm_ppmwrite(stdout, orig_image);

        Pnm_ppmfree(&orig_image);
        CPUTime_Free(&timer);
        

        // create pnm_ppm object

        // have apply_transformation return the time spent
        // pass to apply_trasformation(), along with methods, map, rotation

        fclose(img_file);


        //assert(0);              // the rest of this function is not yet implemented
}

struct copy_closure {
        void *blank_a2;
        void *methods_obj;
};

A2Methods_UArray2 rotate_ppm(Pnm_ppm orig, A2Methods_T methods, A2Methods_mapfun map,
                    int rotation)
{

        A2Methods_UArray2 array_copy; 

        // HERE we call new assuming unblocked????
        int rgb_size = sizeof(**(Pnm_rgb *)(orig->methods->at(orig->pixels, 0, 0)));
        // printf("size: %d\n", rgb_size);
        // printf("one unsigned size: %ld\n", sizeof(unsigned));
        // printf("pointer size: %ld\n", sizeof( unsigned *));

        if (rotation == 90) {
                array_copy = methods->new(orig->height, orig->width,
                                                    rgb_size);
                struct copy_closure my_copy = { &array_copy, &methods };
                map(orig->pixels, &rotate_90, &my_copy);
        }
        if (rotation == 180) {
                array_copy = methods->new(orig->width, orig->height,
                                                    rgb_size);
                struct copy_closure my_copy = { &array_copy, &methods };
                map(orig->pixels, &rotate_180, &my_copy);
        }

        // map thru array copy and


    //methods->free(&array_copy);
        //(void) map;
    //(void) rotation;
    return array_copy;
}

void rotate_90(int col, int row, A2Methods_UArray2 arr, void *elem, void *cl)
{

        struct copy_closure *mycopy = cl;
        A2Methods_UArray2 *transformed_array = mycopy->blank_a2;
        A2Methods_T *methods = mycopy->methods_obj;

        //printf("rotation 90 degrees\n");
        int new_col = ((*methods)->height(arr)) - row - 1;
        int new_row = col;

        //printf(" [%d, %d] to [%d, %d] \n", col, row, new_col, new_row);

        Pnm_rgb new_pix = (*methods)->at((*transformed_array), new_col, new_row);
        Pnm_rgb curr_pix = (Pnm_rgb)elem;
        *new_pix = *curr_pix;

        // printf("size of new struct: %ld\n", sizeof(**new_pix));
        // printf("size of old struct: %ld\n", sizeof(**curr_pix));

        // printf("old:    red: %u\n", *(*curr_pix->red));
        // printf("        green: %u\n", *(*curr_pix->green));
        // printf("        blue: %u\n", *(*curr_pix->red));
        // printf("new:    red: %u\n", *(*new_pix->red));
        // printf("        green: %u\n", *(*new_pix->green));
        // printf("        blue: %u\n", *(*new_pix->red));

        // (*new_pix)->red = (*curr_pix)->red;
        // (*new_pix)->green = (*curr_pix)->green;
        // (*new_pix)->blue = (*curr_pix)->blue;

        // Pnm_ppm *orig_img = img;
        // elem = orig_img->methods->at(orig_img->pixels, orig_col, orig_row);

        //(void) transformed_array;
    // //img->methods
    // (void) col;
    // (void) row;
    // (void) arr;
        //(void) elem;
    // (void) img;
}

void rotate_180(int col, int row, A2Methods_UArray2 arr, void *elem, void *img)
{
        //printf("rotation 180 degrees\n");

    //img->methods
    (void) col;
    (void) row;
    (void) arr;
    (void) elem;
    (void) img;
}

// {

//     start timer;

//     if (methods == plain ) new Uarray2 = array_copy;
//     else if (methods == blocked) new UArray2b = array_copy;

//     copy pnm_ppm->pixels into array_copy using map;


//     OR start timer here????

     //if (rotation==90) map(array_copy, rotate_90, &pnm);
     //if (rotation==180) map(array_copy, rotate_180, &pnm);

//     time = stop timer;

//     free Uarray2;


//     return (time / (width * height));


// }

// void rotate_90(array_copy, i , j, *elem, void *cl)
// {
//     get height and width from closure;
//     cl->uarray_at(array_copy, height - j - 1, i) = *elem;
// }

