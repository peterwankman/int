#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errno.h"

void strtoup(char *in) {
	do {
		if((*in >= 'a') && (*in <= 'z'))
			*in -= 32;
	} while(*(++in));
}

void printstatus(int status) {
	printf(" ");
	switch(status) {
		case ERROR_NONE:  printf("OK.\n"); break;
		case ERROR_SYNTX: printf("Syntax error.\n"); break;
		case ERROR_REDIM: printf("Array redimensioned.\n"); break;
		case ERROR_SUBST: printf("Bad subscript.\n"); break;
		case ERROR_TYPE:  printf("Type mismatch.\n"); break;
		case ERROR_DIVZE: printf("Division by zero.\n"); break;
		case ERROR_MALLC: printf("malloc() failed.\n"); break;
		case ERROR_CMMND: printf("Unknown command.\n"); break;
		default:		  printf("unknown error.\n");
	}
}

char *delwhite(char *in) {	
	int j = 0, i = 0;
	char *out;
	
	if((out = malloc(strlen(in) + 1)) == NULL)
		return NULL;

	while(in[i]) {
		if(!isspace(in[i]))
			out[j++] = in[i];
		i++;
	}
	out[j] = '\0';

	return out;
}
