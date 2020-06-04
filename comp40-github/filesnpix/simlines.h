#include "readaline.h"
#include "Bucket.h"
#include "Location.h"
#include "table.h"
#include "atom.h"

void read_file(char *fname, Table_T *lines);
void clean_string(char **, size_t);  
bool is_word_char(char c);
void store_table(Table_T *Table, Location *loc, const char *line_atom);
void print_simlines(Table_T *lines);
void print_table(const void *key, void **value, void *cl);
