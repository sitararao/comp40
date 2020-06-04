#include <stdio.h>
#include <stdlib.h>
#include "pnmrdr.h"
#include <stdbool.h>
#include <setjmp.h>
#include "assert.h"
#include "except.h"
#define T Except_T

float calc_avg_frac(unsigned *numerators, unsigned denominator, int len);
void error_exit();
bool read_pnm(FILE *fp);

int main(int argc, char *argv[]){

	//ask clarification for reading from stdin (same line or hit enter)
	
	// check that we can open file
	// check that file given = pgm format; if so check pixel count > 0


	//PNMrdr function

	//take in png and jpg formats??
	FILE *fp=NULL;
	
	//char file_name[100];
	//take in argc of size 1
	//read from stdin (jpg and png format)
	//errors (throw an error and exit)
	//check PNM reader for errors and exceptions
	//limit output to 3 decimal spaces  
	
	if(argc > 2){
		error_exit();
	}
	//read from stdin
	else if(argc == 1){	
		// char c;
		// //https://stackoverflow.com/questions/42722707/reading-a-string-from-stdin-in-c 
		// int i = 0;
  // 		while (scanf("%c",&c) && c != '\n'){
  // 			file_name[i]=c;
  // 			i++;
  // 		}
  // 		file_name[i]='\0';
  		//fp = fopen(file_name, "r");
  		if(!read_pnm(stdin))
  			error_exit();
	}

	else if(argc == 2) {
		fp = fopen(argv[1], "rb");
		if(fp == NULL) error_exit();
		if(!read_pnm(fp)) error_exit();
		fclose(fp);
   }
   return 0;
}

bool read_pnm(FILE *fp){
	//check filetype
	
	void *img;
	Pnmrdr_mapdata img_info;
	int size = 0;
	unsigned denominator = 1;
	TRY
		img = Pnmrdr_new(fp);
	EXCEPT(Pnmrdr_Badformat)
		error_exit();
	END_TRY;

	// if(setjmp()){
	// 	printf("can't read\n");
	// 	return false;
	// }
	//check exceptions (bad format)

	img_info = Pnmrdr_data(img);
	size = img_info.height * img_info.width;
	if(size == 0){
		error_exit();
	}
	denominator = img_info.denominator;
	unsigned numerators[size];
	for(int j = 0; j < size; ++j){
		//check exceptions (count)
		TRY
			numerators[j] = Pnmrdr_get(img);
		EXCEPT(Pnmrdr_Count)
			error_exit();
		END_TRY;
		
	}
	float avg_brightness = 0;
	avg_brightness = calc_avg_frac(numerators, denominator, size);
	printf("%0.3f\n",avg_brightness);
	//Pnmrdr_free(img);

	return true;
}

float calc_avg_frac(unsigned *numerators, unsigned denominator, int len){
	unsigned sum = 0;
	//int len = sizeof(&numerators)/sizeof(numerators[0]);
	for(int j = 0; j < len; ++j){
		sum+=numerators[j];
	}
	float x = sum/(float)len;
	float y = x/(float)denominator;
	return y;
}

void error_exit(){
	fprintf(stderr, "Error, program exiting\n");
	exit(EXIT_FAILURE);
}

