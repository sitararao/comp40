#ifndef BUCKET_H
#define BUCKET_H

#include "Location.h"
#include "list.h"

typedef struct Bucket {
    const char *cleaned_line;
    List_T loc_list;
} Bucket;

#endif