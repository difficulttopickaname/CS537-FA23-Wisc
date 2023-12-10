#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#define MAX_LENGTH 200

bool isValidDate(char *date) {
	if(date == NULL || date[0] == '\0'){
		return false;
	}
	char yearS[10], monthS[10], dayS[10];
	char *datePtr = date;
	strncpy(yearS, datePtr, 4);	datePtr += 4; 
	if(*datePtr != '-'){
		return false;
	}
	datePtr++; 
	strncpy(monthS, datePtr, 2); datePtr += 2; 
	if(*datePtr != '-'){
		return false;
	}
	datePtr++; 
	strncpy(dayS, datePtr, 2);
	if(atoi(yearS) == 0 || 
		(atoi(monthS) <= 0 || atoi(monthS) > 12) ||
		(atoi(dayS) <= 0 || atoi(dayS) > 31)){
			return false;
		}
    return true;
}

void improper(int lineno){
	printf("Improper formatting on line %d\n", lineno);
	exit(0);
}

void groff(FILE *fptr){
	char c[MAX_LENGTH], outputPath[MAX_LENGTH];
	int lineno = 1;

	// first line
	if(fgets(c, MAX_LENGTH, fptr) == NULL){
		improper(lineno);
	}
	char command[MAX_LENGTH], date[MAX_LENGTH];
	int section;
	if (sscanf(c, ".TH %s %d %s", command, &section, date) == 3) {
        if (section < 1 || section > 9 || !isValidDate(date)){
			improper(lineno);
        }
    } else {
		improper(lineno);
    }

	sprintf(outputPath, "%.50s.%d", command, section);
	
	FILE *out = fopen(outputPath, "w");

	char firstLine[MAX_LENGTH], *fp = firstLine;
	// Calculate the number of spaces needed for right justification
    int spaces_needed = 80 - 2 * strlen(command) - 4 - 2; // 4 for parentheses, 2 for section

    // Print the formatted output
    sprintf(fp, "%s(%d)", command, section);
	fp += strlen(fp);

    for (int i = 0; i < spaces_needed; i++) {
        strcpy(fp, " ");
		fp++;
    }

    sprintf(fp, "%s(%d)\n", command, section);
	fp += strlen(fp);
	*fp=0;
	fprintf(out, "%s", firstLine);

	// the rest
	while(fgets(c, MAX_LENGTH, fptr) != NULL){
		char output[MAX_LENGTH];

		char *p = c; // Pointer to iterate through the input string
		char *q = output; // Pointer to store the modified string

		if(*p == '#'){
			continue;
		}

		else if(*p == '.' && *(p + 1) == 'S' && *(p + 2) == 'H'){
			p += 4; // skip .SH and space after it
			fprintf(out, "\n\033[1m");
			while(*p != '\n'){
				if(*p >= 'a' && *p <= 'z'){
					*q = *p - 32;
				}
				else{
					*q = *p;
				}
				q++; p++;
			}
			*q = 0;
			fprintf(out, "%s\033[0m\n", output);
		}
		else{
			strcpy(q, "       "); // indent 7 spaces
			q += 7;
			while (*p != '\0') {
				if(*p == '/'){
					if(*(p + 1) == '/'){
						strcpy(q, "/");
						p += 2; // jump through //
						q++; // jump through /
						continue;
					}
					else if(*(p + 1) == 'f'){
						switch (*(p + 2)) {
							case 'B':
								strcpy(q, "\033[1m");
								p += 3;
								q += 4;
								break;
							case 'I':
								strcpy(q, "\033[3m");
								p += 3;
								q += 4;
								break;
							case 'U':
								strcpy(q, "\033[4m");
								p += 3;
								q += 4;
								break;
							case 'P':
								strcpy(q, "\033[0m");
								p += 3;
								q += 4;
								break;
							default:
								*q++ = *p++; // copy /
						}
					}
					else{
						*q++ = *p++;
					}
				}
				else {
					*q++ = *p++;
				}
			}
			*q = '\0'; // end of string
			fprintf(out, "%s", output);
		}
	}

	char lastLine[MAX_LENGTH], *lp = lastLine;
	// last line
    int date_length = strlen(date);

    // Calculate the number of spaces needed on both sides
    int spaces = (80 - date_length) / 2;

    // Print the centered string with padding
    for (int i = 0; i < spaces; i++) {
        *lp=' ';
		lp++;
    }

    strcpy(lp, date);
	lp += strlen(date);

    for (int i = 0; i < spaces; i++) {
        *lp=' ';
		lp++;
    }

    strcpy(lp, "\n");
	lp+=2;
	*lp=0;

	fprintf(out, "%s", lastLine);
	fclose(out);
}

int main(int argc, char **argv){
	if(argc < 2){
		printf("Improper number of arguments\nUsage: ./wgroff <file>\n");
		exit(0);
	}

	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL){
		printf("File doesn't exist\n");
		exit(0);
	}
	groff(fp);	
	fclose(fp);
	return 0;
}
