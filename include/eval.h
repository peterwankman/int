#ifndef EVAL_H_
#define EVAL_H_

typedef enum {
	oper, number
} exptype_t;

typedef struct expr_t {
	exptype_t type;
	int val;
	struct expr_t *left;
	struct expr_t *right;
} expr_t;

int eval(char *exp, int *status);

#endif
