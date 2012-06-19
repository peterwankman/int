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
