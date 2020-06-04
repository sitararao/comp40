/*
 * Simlines.c
 * Created for Comp40, Fall 2019
 *
 * Authors: Sitara Rao (srao03) and Era Iyer (eiyer01)
 * 
 * Purpose: Implementation of Simlines program. 
 *          Reads in files, detects and prints out all "similar" lines,
 *          along with the file names and line numbers where they appear.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "simlines.h"

Except_T Inval_File = { "Invalid file name" };


int main(int argc, char *argv[]){
    //if argc <= 1, there are no files to read, and therefore, no similar lines
    if (argc > 1) {
        Table_T lines = Table_new(5000, NULL, NULL); 

        //iterate through all files and call read_file, which calls readaline,
        //clean_string, and store_table
        for (int i = 0; i < (argc-1); i++) {
            read_file(argv[i+1], &lines);  
        }
        //print out similar lines in table
        print_simlines(&lines);
        Table_free(&lines);
    }

    exit(EXIT_SUCCESS);
    return 0;
}

//Function: read_file
//Parameters: char * to the filename, Table_T * to the table in which to hash
//Returns: nothing
//Purpose: opens the file given, reads each line, creates a new string atom of
//     the cleaned line, uses the string atom to call store_table 
void read_file(char *fname, Table_T *lines) {
    FILE *f = fopen(fname, "r");

    if (f == NULL) RAISE(Inval_File);

    char *curr_line = "hi";
    size_t line_chars = 0;
    int line_number = 0;

    //line_chars stores the number of bytes in the line given
    line_chars = readaline(f, &curr_line);

    //continuously calls readaline until no more lines left to read
    while (line_chars != 0 && curr_line != NULL) {
        line_number++;
        //'cleans' the line given and creates an atom string out of it
        clean_string(&curr_line, line_chars);
        const char *cleaned_string = Atom_string(curr_line);
        Location *loc = (Location *) malloc(sizeof(Location));
        loc->origin_file = fname;
        loc->line_num = line_number;

        if (curr_line[0] != '\0') {
            store_table(lines, loc, cleaned_string);
        } else {
            free(loc);
        }
        free(curr_line);
        line_chars = readaline(f, &curr_line);
    }

    fclose(f);
}

//Function: clean_string
//Parameters: char **, size_t
//Returns: nothing
//Purpose: strips the char aray given of any special characters and places a 
//     single space in between each word
void clean_string(char **array, size_t arr_size) {
    char *arr = *array;
    char *new_arr = malloc(arr_size * sizeof(char));
    int arr_index = 0;
    int new_arr_index = 0;
    //iterates through original array
    while (arr_index < (int)arr_size) {
     //if character is a word character, then inserts it into new array
        if(is_word_char(arr[arr_index])){
            new_arr[new_arr_index] = arr[arr_index];
            new_arr_index++;
        }
        //not a word character
        else{
            if(new_arr_index>0){
                //if prev char is a word char, current char should be a space
                if(is_word_char(new_arr[new_arr_index-1])){
                    new_arr[new_arr_index]=' ';
                    new_arr_index++;
                }
            }
        }
       arr_index++;
    }
    //if last element in new_array is a space, then replace it with '\0'
    if (new_arr_index>0 && new_arr[new_arr_index-1]==' ')
        new_arr[new_arr_index-1]='\0';
    //if last element in new_array is a character, set next element to '\0'
    else new_arr[new_arr_index]='\0';

    free(*array);
    *array = new_arr;
}

//Function: is_word_char
//Parameters:char c
//Returns: boolean, returns true or false
//Purpose: checks if the character given is alphanumeric or a '_'. If yes, 
//     returns true, else return false
bool is_word_char(char c) {
    return (isalnum(c) || c == '_');
}

//Function: store_table
//Parameters: Table_T *, Location *, const char *
//Returns: nothing
//Purpose: inserts into the table by using Atom string as key. Creates a Bucket
//     struct or appends to existing Bucket struct 
void store_table(Table_T *Table, Location *loc, const char *line_atom) {
//if line_atom is empty, do not store 
    // Try getting atom to see if it's already in table
    void *get_result = Table_get(*Table, line_atom);
    
    //atom in table
    if (get_result != NULL) {
        Bucket *result = get_result;
        result->loc_list = List_push(result->loc_list, loc);
    } else {
        Bucket *new_bucket = malloc(sizeof(Bucket));
        new_bucket->cleaned_line = line_atom;
        new_bucket->loc_list = NULL;
        new_bucket->loc_list = List_push(new_bucket->loc_list, loc);
      
        Table_put(*Table, line_atom, new_bucket);
    }

}

//Function: print_simlines
//Parameters: Table_T *
//Returns: nothing
//Purpose: primary function to print out the entire table. Calls Table_map
//     to iterate through the entire table & print out each key value pair
void print_simlines(Table_T *Table) {

    bool printed_a_line = false;
    void *closure = &printed_a_line;
    Table_map(*Table, &print_table, closure);

}

//Function: print_table
//Parameters: const void *, void **, void *
//Returns: nothing
//Purpose: used as apply function for Table_map. Prints out all information 
//     corresponding to the key given. Calls List_map function
//     to print the location list associated with each Bucket value
void print_table(const void *key, void **value, void *cl) {
    
    const char *string = key;
    Bucket *val = *value;
    List_T list_pointer = val->loc_list;
    void *popped = NULL;

    // line stored in table but not similar (<2 locations)
    // free location list memory and return without printing
    if (List_length(list_pointer) <= 1) {
        while (list_pointer != NULL) {
            list_pointer = List_pop(list_pointer, &popped);
            free(popped);
            popped = NULL;
        }

        free(val);
        return;
    }
    
    // else: at least one match group to print

    // print newline in between if we have already printed a match group
    bool *printed_line = cl;
    if (*printed_line) printf("\n");

    printf("%s\n", string);
    list_pointer = List_reverse(list_pointer);
    
    // print each filename and line number for match groups
    // then deallocate Location memory
    while (list_pointer != NULL) {
        list_pointer = List_pop(list_pointer, &popped);
        printf("%-20s %7u\n", ((Location *)(popped))->origin_file,
                              ((Location *)popped)->line_num);
        free(popped);
        popped = NULL;
    }

    //set *cl to true (print \n in between match groups)
    *printed_line = true;

    free(val);
}

