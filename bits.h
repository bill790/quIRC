#pragma once

/*
	quIRC - simple terminal-based IRC client
	Copyright (C) 2010-13 Edward Cree

	See quirc.c for license information
	bits: general helper functions (chiefly string manipulation)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "colour.h"

// helper fn macros
#define max(a,b)	((a)>(b)?(a):(b))
#define min(a,b)	((a)<(b)?(a):(b))

#ifdef	NEED_STRNDUP
char *strndup(const char *s, size_t size);
#endif

int wordline(const char *msg, unsigned int x, char **out, size_t *l, size_t *i, colour lc); // prepares a string for printing, breaking lines in between words; returns new x
void crush(char **buf, unsigned int len);
void scrush(char **buf, unsigned int len);
char *mktag(char *fmt, char *from);
