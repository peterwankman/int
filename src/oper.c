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

