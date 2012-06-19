#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errlist.h"
#include "oper.h"
#include "var.h"

vartype_t getvartype(char *var) {
	int i;
	char c = var[strlen(var) - 1];

	for(i = 0; i < strlen(var) - 1; i++) {
		if((i > 0) && (var[i] == '(') && isalpha(var[i - 1]))
			return decarr;
		if((var[i] == '%') && (var[i + 1] == '('))
			return intarr;
		if((var[i] == '$') && (var[i + 1] == '('))
			return strarr;
		if(!isalpha(var[i]))
			return err;
	}

	if(c == '$')
		return string;
	else if(c == '%')
		return integer;
	else if(isalpha(c))
		return decimal;
	else
		return err;
}

int isoper(char c) {
	int i;

	for(i = 0; i < NUM_OPER; i++) {
		if(operlist[i][0] == c)
			return i;
	}
	return -1;
}

int isint(char *exp) {
	int i = 0;
	if(exp[0] == '-') i++;
	for(; i < strlen(exp); i++)
		if((exp[i] < '0') || (exp[i] > '9'))
			return 0;
	return 1;
}

int isdec(char *exp) {
	int i = 0, dot = 0;
	if(exp[0] == '-') i++;
	for(; i < strlen(exp); i++) {
		if(exp[i] == '.')
			dot++;
		else if((exp[i] < '0') || (exp[i] > '9'))
			return 0;
	}

	return (dot==1)?1:0;
}

int isarray(vartype_t type) {
	switch(type) {
		case intarr:
		case decarr:
		case strarr: return 1;
	}
	return 0;
}

int isscalar(vartype_t type) {
	switch(type) {
		case integer:
		case decimal:
		case string: return 1;
	}
	return 0;
}

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
		case ERROR_SYNTX: printf("ERROR: Syntax error.\n"); break;
		case ERROR_REDIM: printf("ERROR: Array redimensioned.\n"); break;
		case ERROR_SUBST: printf("ERROR: Bad subscript.\n"); break;
		case ERROR_TYPE:  printf("ERROR: Type mismatch.\n"); break;
		case ERROR_DIVZE: printf("ERROR: Division by zero.\n"); break;
		case ERROR_MALLC: printf("ERROR: malloc() failed.\n"); break;
		case ERROR_CMMND: printf("ERROR: Unknown command.\n"); break;
		default:		  printf("ERROR: unknown error.\n");
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
