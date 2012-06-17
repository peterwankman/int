#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errno.h"
#include "eval.h"
#include "help.h"
#include "oper.h"
#include "var.h"

static expr_t *makenum(int i) {
	expr_t *out = malloc(sizeof(expr_t));

	if(out == NULL)
		return NULL;

	out->left = out->right = NULL;
	out->type = number;
	out->val = i;

	return out;
}

static expr_t *makeoper(int op, expr_t *left, expr_t *right) {
	expr_t *out = malloc(sizeof(expr_t));

	if(out == NULL)
		return NULL;

	out->left = left;
	out->right = right;
	out->type = oper;
	out->val = op;

	return out;
}

static int isint(char *exp) {
	int i;
	for(i = 0; i < strlen(exp); i++)
		if((exp[i] < '0') || (exp[i] > '9'))
			return 0;
	return 1;
}

static int isvar(char *exp) {
	int i;
	for(i = 0; i < strlen(exp); i++)
		if((exp[i] < 'A') || (exp[i] > 'Z'))
			return 0;
	return 1;
}

static int isarray(char *exp) {
	char *oparen, *c;
	int parbal = 0;

	if((oparen = strchr(exp, '(')) == NULL)		
		return 0;

	if(oparen == exp)
		return 0;

	c = oparen;
	do {
		if(*c == '(')
			parbal++;
		if(*c == ')')
			parbal--;
		c++;
	} while(parbal && *c);

	if(*c)
		return 0;
	
	for(c = exp; c < oparen; c++)
		if ((*c < 'A') || (*c > 'Z'))
			return 0;
	
	return 1;
}

static int isoper(char c) {
	int i;

	for(i = 0; i < NUM_OPER; i++) {
		if(operlist[i][0] == c)
			return i;
	}
	return -1;
}

static int getoper(char *exp, int *pos) {
	int len = strlen(exp);
	int i, oper, skip = 0, ret = ERROR_SYNTX;
	int lowest = INT_MAX;
	char c;

	*pos = -1;
	for(i = len - 1; i > -1; i--) {
		c = exp[i];

		if(c == ')')
			skip++;
		if(c == '(')
			skip--;

		if((skip == 0) && (oper = isoper(exp[i])) > -1) {
			if(operlist[oper][2] < lowest) {
				lowest = operlist[oper][2];
				*pos = i;
				ret = oper;
			}
		}
	}

	if(skip != 0) {
		ret = ERROR_SYNTX;
		*pos = -1;
	}

	return ret;
}

static char *getoperand(char *exp, int pos, int lr, int *status) {
	char *ret;
	int len;

	if(lr == OPER_LEFT) {
		len = pos + 1;
		if(len == 1) {
			*status = ERROR_SYNTX;
			return NULL;
		}
		if((ret = malloc(len)) == NULL) {
			*status = ERROR_MALLC;
			return NULL;
		}
		strncpy(ret, exp, pos);
		ret[pos] = '\0';
	} else {
		len = strlen(exp) - pos;
		if(len == 1) {
			*status = ERROR_SYNTX;
			return NULL;
		}
		if((ret = malloc(len)) == NULL) {
			*status = ERROR_MALLC;
			return NULL;
		}
		strncpy(ret, exp + pos + 1, len - 1);
		ret[len - 1] = '\0';
	}

	return ret;
}

static expr_t *buildtree(char *exp, int *status) {
	int opr, oprpos;
	char *left, *right;
	expr_t *tree_left, *tree_right;
	expr_t *out;

	*status = ERROR_NONE;
		
	if(isint(exp))
		return makenum(atoi(exp));
	if(isvar(exp))
		return makenum(getint(exp, status));
	if(isarray(exp))
		return makenum(getintarr(exp, status));
	
	opr = getoper(exp, &oprpos);
	if(oprpos < 1) {				/* CHECK FOR UNARY '-' */
		if((oprpos == 0) && (operlist[opr][1] == OPER_SUB)) {
			if((right = getoperand(exp, oprpos, OPER_RIGHT, status)) == NULL)
				return NULL;

			if((tree_right = buildtree(right, status)) == NULL)
				return NULL;

			out = makeoper(OPER_SUB, makenum(0), tree_right);
			free(right);
			return out;
		}
		*status = ERROR_SYNTX;
		return NULL;		
	}

	left = getoperand(exp, oprpos, OPER_LEFT, status);
	right = getoperand(exp, oprpos, OPER_RIGHT, status);

	if(left && right) {
		if((tree_left = buildtree(left, status)) == NULL) {
			free(left);
			free(right);
			return NULL;
		}
		if((tree_right = buildtree(right, status)) == NULL) {
			free(left);
			free(right);
			return NULL;
		}
		out = makeoper(operlist[opr][1], tree_left, tree_right);
		free(left);
		free(right);
		return out;
	} else {
		if(left) free(left);
		if(right) free(right);
		return NULL;
	}
}

static int evaltree(expr_t *tree, int *status) {
	int left, right;

	*status = ERROR_NONE;
	if(tree->type == number)
		return tree->val;

	left = evaltree(tree->left, status);
	right = evaltree(tree->right, status);
	switch(tree->val) {
		case OPER_ADD: return left + right;
		case OPER_SUB: return left - right;
		case OPER_MUL: return left * right;
		case OPER_MOD: return left % right;
		case OPER_AND: return left & right;
		case OPER_XOR: return left ^ right;
		case OPER_OR:  return left | right;
		case OPER_DIV:
			if(right != 0)
				return left / right;
			else
				*status = ERROR_DIVZE;
		default: return 0;
	}
}

static void freetree(expr_t *tree) {
	if(tree->left)
		freetree(tree->left);
	if(tree->right)
		freetree(tree->right);
	free(tree);
}

static int evalparen(char* exp, int** result, int** pos, int** len, int *status) {
	int i, j, n, nest;
	char *subexp;

	n = 0;
	*result = *pos = *len = NULL;

	for(i = 0; i < strlen(exp); i++) {
		if(exp[i] == '(') {
			if((i > 0) && (exp[i - 1] == '$')) {
				*status = ERROR_TYPE;
				return 0;
			} else if((i > 0) && (isalpha(exp[i - 1]))) {				
				nest = 1;
				while(nest && (i < strlen(exp))) {
					i++;
					if(exp[i] == '(')
						nest++;
					if(exp[i] == ')')
						nest--;					
				}
				if(nest) {
					*status = ERROR_SYNTX;
					return 0;
				}					
			} else {
				n++;
				
				if(*pos)
					*pos = realloc(*pos, n * sizeof(int));
				else
					*pos = malloc(n * sizeof(int));
				if(*pos == NULL) {
					*status = ERROR_MALLC;
					return 0;
				}
			
				if(*len)
					*len = realloc(*len, n * sizeof(int));
				else
					*len = malloc(n * sizeof(int));
				if(*len == NULL) {
					*status = ERROR_MALLC;
					return 0;
				}
								
				(*pos)[n - 1] = i;
				nest = 1;
				while(nest && (i < strlen(exp))) {
					i++;
					if(exp[i] == '(')
						nest++;
					if(exp[i] == ')')
						nest--;					
				}
				if(nest) {
					free(*pos);
					free(*len);
					*status = ERROR_SYNTX;
					return 0;
				}			
				(*len)[n - 1] = i - (*pos)[n - 1] + 1;
			}
		}
	}

	if(n) *result = malloc(n * sizeof(int));
	
	for(i = 0; i < n; i++) {
		if((subexp = malloc((*len)[i] - 1)) == NULL) {
			*status = ERROR_MALLC;
			return 0;
		}

		if((*len)[i] > 2) {
			for(j = 1; j < (*len)[i] - 1; j++)
				subexp[j - 1] = exp[(*pos)[i] + j];
			subexp[(*len)[i] - 2] = '\0';			

			(*result)[i] = eval(subexp, status);			
		} else
			(*result)[i] = 0;

		free(subexp);
	}	
	return n;
}

static char *buildnewexp(char* exp, int num, int* result, int* pos, int* len, int* status) {
	char *out;
	int i, newlen, n;
	
	newlen = (num * 10) + strlen(exp) + 1;

	for(i = 0; i < num; i++) {
		newlen -= len[i];
	}

	if((out = malloc(newlen)) == NULL) {
		*status = ERROR_MALLC;
		return NULL;
	}
	memset(out, '\0', newlen);

	n = 0;
	for(i = 0; i < strlen(exp); i++) {
		if(i == pos[n]) {
			sprintf(out, "%s%d", out, result[n]);
			i += len[n] - 1;
			if(n < num)
				n++;
		} else {			
			out[strlen(out)] = exp[i];
		}
	}
	return out;
}

int eval(char *exp, int *status) {
	expr_t *exptree;
	char *buf, *buf2;
	int ret = 0;

	int numpar;
	int *resultpar, *pospar, *lenpar;

	if((buf = delwhite(exp)) == NULL) {
		*status = ERROR_MALLC;
		return 0;
	}

	*status = ERROR_NONE;

	numpar = evalparen(buf, &resultpar, &pospar, &lenpar, status);
	if(*status != ERROR_NONE)
		return 0;

	if(numpar) {
		buf2 = buildnewexp(buf, numpar, resultpar, pospar, lenpar, status);
		if(*status != ERROR_NONE)
			return 0;
		free(buf);
		free(resultpar);
		free(pospar);	
		free(lenpar);
		buf = buf2;
	}

	exptree = buildtree(buf, status);
	free(buf);	

	if(exptree) {
		ret = evaltree(exptree, status);
		freetree(exptree);
	}

	return ret;
}
