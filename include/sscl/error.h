/* SSCL - Symbion Simple Class Library
 * Copyright (C) 2001 Szilard Hajba
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#ifndef _SSCL_BASE_H
#define _SSCL_BASE_H

#include <sscl/stream.h>
#include <stdarg.h>

class Error {
    public:
//	Error(const char *lvl, const char *i, const char *fnc, const char *txt);
	Error(const char *lvl, const char *i, const char *txt,...);
//	Error(const char *lvl, int num, const char *fnc, const char *txt);
	Error(const char *lvl, int num, const char *txt,...);
	Error(Error &error);
	virtual ~Error();
	char *format();
    private:
	const char *level;
	char *id;
	const char *func;
	char *text;
};

#define SSCL_SYSERR(func) throw new Error("F-SYS", errno, func, sys_errlist[errno])
#define IS_ALNUM(c) (((c)>='a'&&(c)<='z')||((c)>='A'&&(c)<='Z')||((c)>='0'&&(c)<='9')||(c)=='_')
#define IS_ALPHA(c) (((c)>='a'&&(c)<='z')||((c)>='A'&&(c)<='Z')||(c)=='_')
#define IS_SPACE(c) ((c)==' '||(c)=='\t')

#endif /* _SSCL_BASE_H */
