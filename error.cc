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

#include <sscl/error.h>
#include <sscl/strfunc.h>
#include <stdio.h>

/*
Error::Error(const char *lvl, const char *i, const char *fnc, const char *txt)
{
    level=lvl; id=i; errn=0; func=fnc;
    text=txt;
}
*/
Error::Error(const char *lvl, const char *i, const char *txt,...)
{
    char s[257];
    va_list args;
    va_start(args, txt);
    str_printv(s, txt, 256, args);
    va_end(args);
    level=lvl; id=strdup(i); func=NULL; text=strdup(s);
}
/*
Error::Error(const char *lvl, int num, const char *fnc, const char *txt)
{
    level=lvl; errn=num; func=fnc; text=txt; id=NULL;
}
*/
Error::Error(const char *lvl, int num, const char *txt,...)
{
    char s[257], sid[7];
    va_list args;
    va_start(args, txt);
    str_printv(s, txt, 256, args);
    va_end(args);
//    str_itoa(sid, num, 7, '0');
    snprintf(sid, 7, "%06d", num);
    level=lvl; id=strdup(sid); func=NULL; text=strdup(s);
}

Error::Error(Error &error)
{
    level=error.level; id=str_dup(error.id); func=error.func; text=str_dup(error.text);
}

Error::~Error()
{
    free(text); free(id);
}

static char errstr[1024];
char *Error::format() {
    char *lev;
    switch (level[0]) {
	case 'F': lev="FATAL"; break;
	case 'E': lev="ERROR"; break;
	case 'W': lev="WARNING"; break;
	default: lev="???"; break;
    }
    sprintf(errstr, "!%s-%6s %s: %s%s%s", level, id, lev,
	    func?func:"", func?"(): ":"", text);
    return errstr;
}
