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

#ifndef EVAL_H_
#define EVAL_H_

typedef enum {
	oper, number
} exptype_t;

typedef struct expr_t {
	exptype_t type;
	int oper;
	float val;
	struct expr_t *left;
	struct expr_t *right;
} expr_t;

float eval(char *exp, int *status);

#endif
