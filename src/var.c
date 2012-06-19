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

#include <stdlib.h>
#include <string.h>

#include "errlist.h"
#include "eval.h"
#include "help.h"
#include "var.h"

static int evalsubs(char *array, char **ident, int *subs, int *status) {
	char *subsstr;
	int len;

	if((*ident = delwhite(array)) == NULL) {
		*status = ERROR_MALLC;
		return 0;
	}

	if((subsstr = strchr(*ident, '(')) == NULL) {
		free(*ident);
		*status = ERROR_TYPE;
		return 0;
	}

	if(subsstr[strlen(subsstr) - 1] != ')') {
		free(*ident);
		*status = ERROR_SYNTX;
		return 0;
	}

	len = strlen(subsstr) - 2;
	if(len < 1) {
		free(*ident);
		*status = ERROR_SYNTX;
		return 0;
	}

	subsstr[0] = '\0';
	subsstr++;
	subsstr[len] = '\0';
		
	*subs = (int)eval(subsstr, status);
	if(*status != ERROR_NONE) {
		free(*ident);
		return 0;
	}

	if(*subs < 0) {
		free(*ident);
		*status = ERROR_SUBST;
		return 0;
	}

	return 1;
}

static varentry_t *findlastvar(void) {
	varentry_t *curr = varlist;

	if(curr == NULL)
		return NULL;

	while(curr->next)
		curr = curr->next;

	return curr;
}

static varentry_t *mknewvar(char *ident, float dval, int ival, char *str, int size, vartype_t type) {
	varentry_t *out;
	int i, j;

	if((out = malloc(sizeof(varentry_t))) == NULL)
		return NULL;

	if((out->ident = malloc(strlen(ident) + 1)) == NULL) {
		free(out);
		return NULL;
	}

	strncpy(out->ident, ident, strlen(ident) + 1);

	switch(type) {
		case integer:
			out->type = integer;
			out->val.integer = ival;
			break;

		case decimal:			
			out->type = decimal;
			out->val.decimal = dval;
			break;

		case string:
			out->type = string;
			if((out->val.string = malloc(strlen(str) + 1)) == NULL) {
				free(out->ident);
				free(out);
				return NULL;
			}
			strncpy(out->val.string, str, strlen(str) + 1);
			break;
		
		case intarr:
			out->type = intarr;
			out->val.array.type = integer;
			out->val.array.size = size;

			if((out->val.array.val.integer = malloc(size * sizeof(int))) == NULL) {
				free(out);
				return NULL;
			}
			
			for(i = 0; i < size; i++)
				out->val.array.val.integer[i] = ival;
			break;

		case decarr:
			out->type = decarr;
			out->val.array.type = decimal;
			out->val.array.size = size;

			if((out->val.array.val.decimal = malloc(size * sizeof(int))) == NULL) {
				free(out);
				return NULL;
			}
			
			for(i = 0; i < size; i++)
				out->val.array.val.decimal[i] = dval;
			break;

		case strarr:
			if((out->val.array.val.string = malloc(size * sizeof(char**))) == NULL) {
				free(out);
				return NULL;
			}
			out->type = strarr;
			out->val.array.type = string;
			out->val.array.size = size;

			for(i = 0; i < size; i++) {
				if((out->val.array.val.string[i] = malloc(2)) == NULL) {
					for(j = 0; j < i; j++)
						free(out->val.array.val.string[i]);
					free(out->val.array.val.string);
					free(out);
					return NULL;
				}
				strncpy(out->val.array.val.string[i], "", 1);
			}
			break;
	}	
	out->next = NULL;

	return out;
}

static void addtovarlist(varentry_t *item) {
	varentry_t *last = findlastvar();

	if(last == NULL) {
		varlist = item;
		return;
	}

	item->next = NULL;
	last->next = item;
}

static varentry_t *lookupvar(char *ident, vartype_t type) {
	varentry_t *curr = varlist;

	while(curr) {
		if((curr->type == type) && (!strcmp(curr->ident, ident)))
			return curr;
		curr = curr->next;
	}
	return NULL;
}

char *setstr(char *ident, char *str, int *status) {
	char *newstr;
	varentry_t *var = lookupvar(ident, string);

	*status = ERROR_NONE;
	if(var) {
		if((newstr = malloc(strlen(str) + 1)) == NULL) {
			*status = ERROR_MALLC;
			return NULL;
		}
		strncpy(newstr, str, strlen(str) + 1);
		free(var->val.string);
		var->val.string = newstr;
	} else {
		var = mknewvar(ident, 0, 0, str, 0, string);
		if(var == NULL)
			*status = ERROR_SUBST; /* ASDF */
		addtovarlist(var);
	}
	
	return var->val.string;
}

char *setstrarr(char *arrstr, char *str, int *status) {
	varentry_t *var;
	array_t arr;
	char *ident;
	int subs;

	if(evalsubs(arrstr, &ident, &subs, status) == 0)
		return NULL;

	var = lookupvar(ident, strarr);	
	if(var == NULL) {		
		*status = ERROR_SUBST;
		return NULL;
	}
	
	arr = var->val.array;
	if(arr.size < subs + 1) {
		*status = ERROR_SUBST;
		return NULL;
	}

	if((ident = realloc(ident, strlen(str) + 1)) == NULL) { /* var reuse */
		*status = ERROR_MALLC; /* ASDF */
		return NULL;
	}

	strncpy(ident, str, strlen(str) + 1);
	free(arr.val.string[subs]);
	arr.val.string[subs] = ident;

	*status = ERROR_NONE;
	return ident;
}

char *getstr(char *ident, int *status) {
	varentry_t *var = lookupvar(ident, string);
	char *out;

	*status = ERROR_NONE;
	if(var == NULL) {
		out = setstr(ident, "", status);
	} else {
		out = var->val.string;
	}

	return out;
}

char *getstrarr(char *arrstr, int *status) {
	varentry_t *var;
	array_t arr;
	char *ident;
	int subs;

	if(evalsubs(arrstr, &ident, &subs, status) == 0)
		return 0;

	if((var = lookupvar(ident, strarr)) == NULL) {	
		free(ident);
		*status = ERROR_SUBST;
		return NULL;
	}
	free(ident);

	arr = var->val.array;
	if(arr.size < subs + 1) {		
		*status = ERROR_SUBST;
		return NULL;
	}

	*status = ERROR_NONE;
	return arr.val.string[subs];
}

int dimstr(char *ident, int size, int *status) {
	varentry_t *var = lookupvar(ident, strarr);

	*status = ERROR_NONE;
	if(var) {
		*status = ERROR_REDIM;
		return 0;
	}
	var = mknewvar(ident, 0, 0, "", size, strarr);

	if(var == NULL) {
		*status = ERROR_REDIM; /* ASDF */
		return 0;
	}
	addtovarlist(var);

	return 1;
}

int setint(char *ident, char *exp, int *status) {
	int i;
	varentry_t *var = lookupvar(ident, integer);

	*status = ERROR_NONE;
	i = eval(exp, status);
	if(var) {
		var->val.integer = i;
	} else {
		var = mknewvar(ident, 0, i, NULL, 0, integer);
		if(var == NULL)
			*status = ERROR_REDIM; /* ASDF */
		addtovarlist(var);
	}

	return var->val.integer;
}

int setintarr(char *arrstr, char *exp, int *status) {
	varentry_t *var;
	array_t arr;
	char *ident;
	int subs, i;

	if(evalsubs(arrstr, &ident, &subs, status) == 0)
		return 0;

	if((var = lookupvar(ident, intarr)) == NULL) {
		free(ident);
		*status = ERROR_SUBST;
		return 0;
	}
	free(ident);

	arr = var->val.array;
	if(arr.size < subs + 1) {
		*status = ERROR_SUBST;
		return 0;
	}

	i = eval(exp, status);
	arr.val.integer[subs] = i;

	*status = ERROR_NONE;
	return i;
}

int getint(char *ident, int *status) {
	varentry_t *var = lookupvar(ident, integer);
	int out;

	*status = ERROR_NONE;
	if(var == NULL) {
		out = setint(ident, "0", status);
	} else {
		out = var->val.integer;
	}

	return out;
}

int getintarr(char *arrstr, int *status) {
	varentry_t *var;
	array_t arr;
	char *ident;
	int subs;

	if(evalsubs(arrstr, &ident, &subs, status) == 0)
		return 0;

	if((var = lookupvar(ident, intarr)) == NULL) {
		free(ident);
		*status = ERROR_SUBST;
		return 0;
	}
	free(ident);
	
	arr = var->val.array;
	if(arr.size < subs + 1) {
		*status = ERROR_SUBST;
		return 0;
	}

	*status = ERROR_NONE;
	return arr.val.integer[subs];
}

int dimint(char *ident, int size, int *status) {
	varentry_t *var = lookupvar(ident, intarr);

	if(var) {
		*status = ERROR_REDIM;
		return 0;
	}

	var = mknewvar(ident, 0, 0, NULL, size, intarr);
	if(!var) {
		*status = ERROR_MALLC;
		return 0;
	}
	addtovarlist(var);

	*status = ERROR_NONE;
	return 1;
}

float setdec(char *ident, char *exp, int *status) {
	float i;
	varentry_t *var = lookupvar(ident, decimal);

	*status = ERROR_NONE;
	i = eval(exp, status);	
	if(var) {
		var->val.decimal = i;
	} else {
		var = mknewvar(ident, i, 0, NULL, 0, decimal);
		if(var == NULL)
			*status = ERROR_REDIM; /* ASDF */
		addtovarlist(var);
	}

	return var->val.decimal;
}

float setdecarr(char *arrstr, char *exp, int *status) {
	varentry_t *var;
	array_t arr;
	char *ident;
	int subs;
	float i;

	if(evalsubs(arrstr, &ident, &subs, status) == 0)
		return 0;

	if((var = lookupvar(ident, decarr)) == NULL) {
		free(ident);
		*status = ERROR_SUBST;
		return 0;
	}
	free(ident);

	arr = var->val.array;
	if(arr.size < subs + 1) {
		*status = ERROR_SUBST;
		return 0;
	}

	i = eval(exp, status);
	arr.val.decimal[subs] = i;

	*status = ERROR_NONE;
	return i;
}

float getdec(char *ident, int *status) {
	varentry_t *var = lookupvar(ident, decimal);
	float out;

	*status = ERROR_NONE;
	if(var == NULL) {		
		out = setdec(ident, "0", status);
	} else {
		out = var->val.decimal;
	}

	return out;
}

float getdecarr(char *arrstr, int *status) {
	varentry_t *var;
	array_t arr;
	char *ident;
	int subs;

	if(evalsubs(arrstr, &ident, &subs, status) == 0)
		return 0;

	if((var = lookupvar(ident, decarr)) == NULL) {
		free(ident);
		*status = ERROR_SUBST;
		return 0;
	}
	free(ident);
	
	arr = var->val.array;
	if(arr.size < subs + 1) {
		*status = ERROR_SUBST;
		return 0;
	}

	*status = ERROR_NONE;
	return arr.val.decimal[subs];
}

int dimdec(char *ident, int size, int *status) {
	varentry_t *var = lookupvar(ident, decarr);
	if(var) {
		*status = ERROR_REDIM;
		return 0;
	}

	var = mknewvar(ident, 0, 0, NULL, size, decarr);
	if(!var) {
		*status = ERROR_MALLC;
		return 0;
	}
	addtovarlist(var);

	*status = ERROR_NONE;
	return 1;
}

static void freearr(array_t arr) {
	int i;
	if(arr.type == integer) {
		free(arr.val.integer);
	} else {
		for(i = 0; i < arr.size; i++) {
			free(arr.val.string[i]);
		}
		free(arr.val.string);
	}
}

void freevarlist(void) {
	varentry_t *next = varlist;

	if(next == NULL)
		return;

	do {
		free(varlist->ident);
		if(varlist->type == string) {
			free(varlist->val.string);
		} else if  ((varlist->type == strarr) ||
					(varlist->type == intarr) ||
					(varlist->type == decarr)) {
			freearr(varlist->val.array);
		}
		next = varlist->next;
		free(varlist);
		varlist = next;
	} while(varlist);
}
