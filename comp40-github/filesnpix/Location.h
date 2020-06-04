#ifndef LOCATION_H
#define LOCATION_H


typedef struct Location {
    char *origin_file;
    unsigned line_num;
} Location;

#endif