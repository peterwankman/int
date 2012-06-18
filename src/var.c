#include <stdlib.h>
#include <string.h>

#include "errno.h"
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
		
	*subs = eval(subsstr, status);
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

static varentry_t *mknewvar(char *ident, int val, char *str, int size) {
    varentry_t *out;
    int i, j;

    if((out = malloc(sizeof(varentry_t))) == NULL)
        return NULL;

    if((out->ident = malloc(strlen(ident) + 1)) == NULL) {
        free(out);
        return NULL;
    }

    strcpy(out->ident, ident);

    if(str == NULL) {
        if(size == 0) {
            out->type = integer;
            out->val.integer = val;
        } else {
            if((out->val.array.val.integer = malloc(size * sizeof(int))) == NULL) {
                free(out);
                return NULL;
            }
            out->type = intarr;
            out->val.array.type = integer;
            out->val.array.size = size;
            for(i = 0; i < size; i++)
                out->val.array.val.integer[i] = val;
        }
    } else {
        if(size == 0) {
            out->type = string;
            if((out->val.string = malloc(strlen(str) + 1)) == NULL) {
                free(out->ident);
                free(out);
                return NULL;
            }
            strcpy(out->val.string, str);
        } else {
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
                strcpy(out->val.array.val.string[i], "");
            }
        }
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
        strcpy(newstr, str);
        free(var->val.string);
        var->val.string = newstr;
    } else {
        var = mknewvar(ident, 0, str, 0);
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

	strcpy(ident, str);
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
    var = mknewvar(ident, 0, "", size);

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
        var = mknewvar(ident, i, NULL, 0);
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

    var = mknewvar(ident, 0, NULL, size);
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
        } else if ((varlist->type == strarr) || (varlist->type == intarr)) {
            freearr(varlist->val.array);
        }
        next = varlist->next;
        free(varlist);
        varlist = next;
    } while(varlist);
}
