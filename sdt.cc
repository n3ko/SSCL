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

#include <errno.h>

#include <sscl/sdt.h>
#include <sscl/strfunc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

/*
 * Symbion Deamon Tools communication commands:
 *   QUERY:	Ask information about supported commands
 *   START:	Start the program
 *   QUIT:	Stop the daemon and quit
 *   STATUS:	Status request, answer can be:
 *			I: initializing, R: running, Q: quitting
 */

SDTConn::SDTConn(const int len):
	in(STDIN_FILENO, len), out(STDOUT_FILENO)
{
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

SDTConn::~SDTConn()
{
}

void SDTConn::send(char *format,...)
{
    char buf[1024];
    va_list args;
    va_start(args, format);
    str_printv(buf, format, 1000, args);
    va_end(args);
    out.write(buf);
    out.put_c('\n');
}

void SDTConn::log(SDTLogLevel level, char *format,...)
{
    char buf[1024];
    va_list args;
    va_start(args, format);
    str_printv(buf, format, 1000, args);
    va_end(args);
    send(".LOG %c \"%e\"", level, buf);
}

void SDTConn::set_state(SDTState stat)
{
    state=stat;
    // Send our changed state
    status_event();
}

int SDTConn::check_event()
{
    int i;
    char buf[256];
    if ((i=in.get_s(buf, 250))>0) {
	try {
	    switch (buf[0]) {
		case '?': // Command
		    if (!strncmp(buf+1, "QUERY ", 6)) {
			if (!str_cmp(buf+1+6, "CMDS")) {
			    send(".ANS ?START .QUIT ?STATUS");
			} else
			    throw Error("E-SDT", "QUECMD", "SDTConn::check_event()", "QUERY command error");
		    }
		    else if (!str_cmp(buf+1, "START")) start_event();
		    else if (!str_cmp(buf+1, "STATUS")) status_event();
		    else throw Error("E-SDT", "E-UNKCMD", "SDTConn::check_event()", "Unknown command");
		case '.': // Command
		    if (!str_cmp(buf+1, "QUIT")) quit_event();
		    break;
		default:
		    throw Error("E-SDT", "PROTOC", "SDTConn::check_event()", "Protocol error");
	    }
	} catch (Error &err) {
	    send("%s", err.format());
	}
	return 1;
    }
    return 0;
}

void SDTConn::start_event()
{
    send(".OK");
}

void SDTConn::status_event()
{
    switch (state) {
	case st_init:
	    send(".STATUS I \"Initializing\""); break;
	case st_run:
	    send(".STATUS R \"Running\""); break;
	case st_quit:
	    send(".STATUS Q \"Quiting\""); break;
	default:;
    }
}
