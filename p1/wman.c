#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LENGTH 100

void access_file(FILE *fptr, char *file){
    char c[MAX_LENGTH];
    if(fptr == NULL){ // unable to open
        printf("cannot open file\n");
        exit(1);
    }
    while( fgets (c, MAX_LENGTH, fptr)!=NULL ) {
      printf("%s", c);
   }
}

int main(int argc, char **argv){
    if(argc == 1){
       printf("What manual page do you want?\nFor example, try 'wman wman'\n");
       return 0;
    }

    else if(argc == 2){
        char *page;
        page = argv[1];

        for(int i = 1; i < 10; i++){
            char fileName[50];
            sprintf(fileName, "man_pages/man%d/%s.%d", i, page, i);

            if (access(fileName, F_OK) == 0){ // file exists
                FILE *fp = fopen(fileName, "r");
                access_file(fp, fileName);
                fclose(fp);
                exit(0);
            }
        }
        printf("No manual entry for %s\n", page);
    }

    else if(argc == 3){
        char *page;
        page = argv[2];
        char fileName[50], *empty;
		int errno = 0;
        int dir = strtol(argv[1], &empty, 10); // avoiding problems
		if(errno || dir < 1 || dir > 9){
			printf("invalid section\n");
			exit(1);
		}
		
        sprintf(fileName, "man_pages/man%d/%s.%d", dir, page, dir);
        
        if (access(fileName, F_OK) == 0){ // file exists
            FILE *fp = fopen(fileName, "r");
            access_file(fp, fileName);
            fclose(fp);
            exit(0);
        }
        else{
            printf("No manual entry for %s in section %d\n", page, dir);
        }
    }
    return 0;
}
