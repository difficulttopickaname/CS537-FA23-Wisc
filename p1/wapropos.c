#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

#define MAX_LENGTH 200

bool check_existence(FILE *fptr, char *key, char *pg, int sect, char c[]){
	char *desc, storage[MAX_LENGTH];
    bool check = false, descFlag = false, nonEmpty = false;
    if(fptr == NULL){ // unable to open
        return false;
    }
    while( fgets (c, MAX_LENGTH, fptr)!=NULL ) {
        if(c[0] != ' ' && strstr(c, "NAME") != NULL){ 
            check = true;
            descFlag = (!nonEmpty);
            continue;
        }
        else if(c[0] != ' ' && strstr(c, "DESCRIPTION") != NULL){
            check = true;
            descFlag = false;
            continue;
        }
        else if(c[0] != ' ' && (strstr(c, "SYNOPSIS") != NULL || strstr(c, "INPUT") != NULL)){
            check = false;
            descFlag = false;
            continue;
        }
		
        // check if is in NAME/DESCRIPTION
        if(check){
            if(descFlag && !nonEmpty){
				// content after dash
				desc = strchr(c, '-'); // will have dash and new line
				if(desc != NULL){
					strcpy(storage, desc);
					descFlag = false;
					nonEmpty = true;
				}
			}
            if(strstr(c, key) != NULL){
                printf("%s (%i) %s", pg, sect, storage);
                return true;
            }
        }
		
    }
	return false;
}

int main(int argc, char **argv){
    if(argc != 2){
        printf("wapropos what?\n");
        exit(0);
    }
    
    char *keyword = argv[1];
    bool found = false;
    for(int i = 1; i < 10; i++){
        char dirname[20];
        sprintf(dirname, "man_pages/man%d", i);
        DIR *dir = opendir(dirname);
        if (dir == NULL) {
            continue;
        }

        struct dirent *entry;
        // loop through the directory
        while((entry = readdir(dir)) != NULL){
            char filename[50], page[30], *pageEnd, c[MAX_LENGTH];
            snprintf(page, sizeof(page), "%.29s", entry->d_name);
            snprintf(filename, sizeof(filename), "%.20s/%.29s", dirname, entry->d_name);
            FILE *fp = fopen(filename, "r");
			pageEnd = strchr(page, '.');
			*pageEnd= 0;
            found = check_existence(fp, keyword, page, i, c) || found;
            fclose(fp);
        }
    }
    if(!found){
        printf("nothing appropriate\n");
        exit(0);
    }
    return 0;
}
