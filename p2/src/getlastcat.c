#include "types.h"
#include "user.h"

int main(void) {
	char *buf = malloc(sizeof(char) * 200);
	if(getlastcat(buf) != -1){
		printf(1, "XV6_TEST_OUTPUT Last catted filename: %s\n", buf);
	}
	else{
		printf(1, "XV6_TEST_OUTPUT Last catted filename: -1\n");
	}
	exit();
}

