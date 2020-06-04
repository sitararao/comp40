#include <stdio.h>
#include "uarray2b.h"
void apply(int col, int row, UArray2b_T array2b,void *elem, void *cl);
int main()
{
    UArray2b_T test = UArray2b_new(5, 5, sizeof(int), 2);

    // printf("width: %d\n", UArray2b_width(test));
    // printf("height: %d\n", UArray2b_height(test));
    // printf("size: %d\n", UArray2b_size(test));
    // printf("blocksize: %d\n", UArray2b_blocksize(test));

  //  printf("before at\n");

    int *store = UArray2b_at(test, 4, 3);

  //  printf("after at\n");
    
    *store = 7;

    printf("storing value: %d\n", *store);

    int *get = (int *)UArray2b_at(test, 4, 3);
    printf("value at 4, 3: %d\n", *get);
    UArray2b_map(test, apply, NULL);
    UArray2b_free(&test);

    UArray2b_height(NULL);
    //(void) test;

    return 0;
}

void apply(int col, int row, UArray2b_T array2b,void *elem, void *cl){
    (void)col;
    (void)row;
    (void)array2b;
    (void)elem;
    (void)cl;
    printf("in apply\n");
    int *store = UArray2b_at(array2b, col, row);
    *store = row*UArray2b_width(array2b)+col;
    int *get = (int *)UArray2b_at(array2b, col, row);
    printf("value at [%i][%i]: %d\n", col, row,*get);
}