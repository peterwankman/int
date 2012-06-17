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

#endif
