#ifndef VAR_H_
#define VAR_H_

typedef enum {
	decimal, decarr,
	integer, intarr, 
	string, strarr, 
	err
} vartype_t;

typedef struct {
	vartype_t type;
	int size;

	union {
		char **string;
		int *integer;
		float *decimal;
	} val;
} array_t;

typedef struct varentry_t {
	char *ident;
	vartype_t type;
	struct varentry_t *next;

	union {
		int integer;
		float decimal;
		char *string;
		array_t array;
	} val;
} varentry_t;

varentry_t *varlist;

char *setstr(char *ident, char *str, int *status);
char *setstrarr(char *ident, char *str, int *status);
char *getstr(char *ident, int *status);
char *getstrarr(char *ident, int *status);
int dimstr(char *ident, int size, int *status);

int setint(char *ident, char *exp, int *status);
int setintarr(char *ident, char *exp, int *status);
int getint(char *ident, int *status);
int getintarr(char *ident, int *status);
int dimint(char *ident, int size, int *status);

float setdec(char *ident, char *exp, int *status);
float setdecarr(char *ident, char *exp, int *status);
float getdec(char *ident, int *status);
float getdecarr(char *ident, int *status);
int dimdec(char *ident, int size, int *status);

void freevarlist(void);
#define newvarlist() (varlist = NULL)

#endif
