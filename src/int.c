/*
 * int -- A script interpreter for boskop-ng
 * (C) 2012 Martin Wolters
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errlist.h"
#include "eval.h"
#include "help.h"
#include "var.h"

#define MAXBUF 8

void exec(char *cmd) {
	char *buf, *var;
	int status = ERROR_CMMND;
	vartype_t type;

	if(!strncmp(cmd, "DIM ", 4)) {
		cmd += 4;
		
		if((buf = strchr(cmd, ' '))) {
			*buf = '\0';
			buf++;

			var = malloc(buf - cmd + 1);
			strncpy(var, cmd, buf - cmd + 1);

			type = getvartype(var);
			if(type == integer)
				dimint(var, atoi(buf), &status);
			else if(type == decimal)
				dimdec(var, atoi(buf), &status);
			else if(type == string)
				dimstr(var, atoi(buf), &status);
			else
				status = ERROR_SYNTX;

			free(var);
		} else {
			status = ERROR_SYNTX;
		}
	} else if(!strncmp(cmd, "SET ", 4)) {
		cmd += 4;

		if((buf = strchr(cmd, ' '))) {
			*buf = '\0';
			buf++;

			var = malloc(buf - cmd + 1);
			strncpy(var, cmd, buf - cmd + 1);

			type = getvartype(var);
			if(type == intarr)
				setintarr(var, buf, &status);
			else if(type == integer)
				setint(var, buf, &status);
			else if(type == decarr)
				setdecarr(var, buf, &status);
			else if(type == decimal)
				setdec(var, buf, &status);
			else if(type == strarr)
				setstrarr(var, buf, &status);
			else if(type == string)
				setstr(var, buf, &status);
			else
				status = ERROR_SYNTX;

			free(var);
		} else {
			status = ERROR_SYNTX;
		}
	} else if(!strncmp(cmd, "PRINT ", 6)) {
		cmd += 6;

		type = getvartype(cmd);
		if(type == strarr)
			printf(" %s\n", getstrarr(cmd, &status));
		else if(type == string)
			printf(" %s\n", getstr(cmd, &status));
		else 
			printf(" %g\n", eval(cmd, &status));
	}

	printstatus(status);
}

int main(void) {
	int oldsize, size = 0;
	char buf[MAXBUF];
	char *cmd = NULL;

	newvarlist();

	do {
		size = 0;
		printf(">> ");
		do {
			fgets(buf, MAXBUF, stdin);
			oldsize = size;
			size += strlen(buf);
			if((cmd = realloc(cmd, size + 1)) == NULL) {
				fprintf(stderr, "ERROR: realloc() failed.\n");
				if(cmd)
					free(cmd);
				return 1;
			} else {
				memset(cmd + oldsize, 0, size - oldsize + 1);
			}
			strcat(cmd, buf);
		} while(buf[strlen(buf) - 1] != '\n');
		cmd[size - 1] = '\0';

		if(size == 1)
			break;

		strtoup(cmd);
		exec(cmd);
	} while(1);
	free(cmd);

	freevarlist();
	return EXIT_SUCCESS;
}
