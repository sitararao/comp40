


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#include "simlines.h"


void dealloc_table(const void *key, void **value, void *cl);
void dealloc_list(void **, void *cl);

int main(int argc, char *argv[]){
    //if argc <= 1, there are no files to read, and therefore, no similar lines
    if (argc > 1) {
        Table_T lines = Table_new(2, NULL, NULL); 

        //iterate through all files and call read_file, which calls readaline,
        //clean_string, and store_table
        for (int i = 0; i < (argc-1); i++) {
            read_file(argv[i+1], &lines);  
        }
        //print out similar lines in table
        print_simlines(&lines);

        //free each value in the table, then free table itself
        Table_map(lines, &dealloc_table, NULL);
        Table_free(&lines);
    }
    exit(EXIT_SUCCESS);
    return 0;


    // (void) argc;
    // (void) argv;

    // FILE *fp = fopen("simtest.txt", "r");
    // size_t line_chars = 0;
    // char *curr_line = "hi";
    // line_chars = readaline(fp, &curr_line);

    // printf("readaline on line of size %li\n", line_chars);

    // clean_string(&curr_line, line_chars);

    // Location *loc = (Location *) malloc(sizeof(Location));
    // loc->origin_file = "simtest.txt";
    // loc->line_num = 1;

    // const char *atom = Atom_string(curr_line);

    // Bucket *new_bucket = malloc(sizeof(Bucket));
    //     new_bucket->cleaned_line = atom;
    //     new_bucket->loc_list = NULL;
    //     new_bucket->loc_list = List_push(new_bucket->loc_list, loc);


    // //Location *popped;
    // //new_bucket->loc_list = List_pop()


    // free(curr_line);
    // fclose(fp);
    // return 0;

}

//Function: read_file
//Parameters: char * to the filename, Table_T * to the table in which to hash
//Returns: nothing
//Purpose: opens the file given, reads each line, creates a new string atom of
//     the cleaned line, uses the string atom to call store_table 
void read_file(char *fname, Table_T *lines) {
    FILE *f = fopen(fname, "r");

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
        const char *cleaned_string = Atom_string(curr_line);            // do we ever need to free this?
        Location *loc = (Location *) malloc(sizeof(Location));
        loc->origin_file = fname;
        loc->line_num = line_number;


        // FREE LOCATION IF LINE IS EMPTY ??

        if (curr_line[0] != '\0') {
            store_table(lines, loc, cleaned_string);
        } else {
            free(loc);
        }   

        free(curr_line);
        line_chars = readaline(f, &curr_line);
    }

    //free(curr_line);        // FREE ARRAY ALLOCATED IN READALINE
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

    free(*array);           // FREE UNCLEANED ARRAY MALLOC'ED IN READALINE
    *array = new_arr;       // NEW ARRAY GETS FREED IN READFILE() AFTER ATOM CREATION
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

        // printf("storing line: %s\n", result->cleaned_line);
        // printf("List length: %i\n", List_length(result->loc_list));
        // printf("origin file: %s\n", ((Location *)(result->loc_list->first))->origin_file);
        // printf("origin line: %u\n\n", ((Location *)(result->loc_list->first))->line_num);

    } else {
        Bucket *new_bucket = malloc(sizeof(Bucket));
        new_bucket->cleaned_line = line_atom;
        new_bucket->loc_list = NULL;
        new_bucket->loc_list = List_push(new_bucket->loc_list, loc);

        // printf("storing line: %s\n", new_bucket->cleaned_line);
        // printf("pushed one onto list, curr size is %i\n", List_length(new_bucket->loc_list));
        // printf("origin file: %s\n", ((Location *)(new_bucket->loc_list->first))->origin_file);  // not pushing onto list
        // printf("origin line: %u\n\n", ((Location *)(new_bucket->loc_list->first))->line_num);
        
        void *collision = Table_put(*Table, line_atom, new_bucket);
        if (collision != NULL) {
            printf("collision!\n");
        }
    }

}

//Function: print_simlines
//Parameters: Table_T *
//Returns: nothing
//Purpose: primary function to print out the entire table. Calls Table_map
//     to iterate through the entire table & print out each key value pair
void print_simlines(Table_T *Table) {
//function pointer to print_table 
    void (*fp)(const void*, void**, void*);
    fp = &print_table;

    bool printed_a_line = false;
    void *closure = &printed_a_line;

    Table_map(*Table, fp, closure);

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
    //Location placeholder = {"origin", 0};
    //void *popped = &placeholder;
    //void *popped = NULL;

    // line stored in table but not similar (<2 locations)
    // free location list memory and return without printing
    if (List_length(list_pointer) <= 1) {
        return;
    }
    
    // else: match group

    //print newline in between if we have already printed a match group
    bool *printed_line = cl;
    if (*printed_line) printf("\n");

    // print string of matching line
    printf("%s\n", string);
    list_pointer = List_reverse(list_pointer);
    
    // print each filename and line number for match groups
    // while (list_pointer != NULL) {
    //     list_pointer = List_pop(list_pointer, &popped);
    //     printf("%-20s %u\n", ((Location *)(popped))->origin_file,
    //                           ((Location *)popped)->line_num);
    // }

    List_map(list_pointer, &print_list, NULL);

    //set *cl to true (print \n in between match groups)
    *printed_line = true;

}

void dealloc_table(const void *key, void **value, void *cl) {
    (void) key;
    (void) cl;
    Bucket *val = *value;
    //List_T list_pointer = val->loc_list;

    // list map, free each location.
    List_map(val->loc_list, &dealloc_list, NULL);
    
    // free list
    if (val->loc_list != NULL) {
        List_free(&(val->loc_list));

    }
    free(val);

}


void dealloc_list(void **item, void *cl) {
    (void)cl;
    Location *curr_loc = *item;

    printf("FREEING LOC:    %s, %u\n", curr_loc->origin_file,
                              curr_loc->line_num);


    free(curr_loc);
}





//Function: print_list
//Parameters: void **, void *cl
//Returns: nothing
//Purpose: used as apply function for List_map. Prints out the location info,
//     specifically the name of file and the line number 
void print_list(void **first, void *cl) {
    (void)cl;
    Location *loc = *first;
    printf("%-20s %u\n", loc->origin_file, loc->line_num);


}