#ifndef HELP_H_
#define HELP_H_

#include "var.h"

vartype_t getvartype(char *var);
int isoper(char c);
int isint(char *exp);
int isdec(char *exp);
int isarray(vartype_t type);
int isscalar(vartype_t type);
void strtoup(char *in);
void printstatus(int status);
char *delwhite(char *in);

#endif
