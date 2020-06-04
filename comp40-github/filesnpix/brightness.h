#include "pnmrdr.h"


float calc_avg_frac(unsigned *numerators, unsigned denominator, int len);
void pnmrdr_exception(char *message, FILE *fp);
float read_pnm(FILE *fp);