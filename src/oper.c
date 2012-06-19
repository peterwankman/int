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

#include "oper.h"

int operlist[NUM_OPER][3] = {
	'+', OPER_ADD, 1,
	'-', OPER_SUB, 1,
	'*', OPER_MUL, 2,
	'/', OPER_DIV, 2,
	'%', OPER_MOD, 2,
	'&', OPER_AND, 3,
	'^', OPER_XOR, 4,
	'|', OPER_OR, 5
};
