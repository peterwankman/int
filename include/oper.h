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

#ifndef OPER_H_
#define OPER_H_

#define OPER_ADD	1
#define OPER_SUB	2
#define OPER_MUL	3
#define OPER_DIV	4
#define OPER_MOD	5
#define OPER_AND	6
#define OPER_XOR	7
#define OPER_OR		8

#define OPER_LEFT	1
#define OPER_RIGHT	2

#define NUM_OPER	8

int operlist[NUM_OPER][3];

#endif
