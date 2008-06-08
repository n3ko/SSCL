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
#ifndef _SSCL_SDT_H
#define _SSCL_SDT_H

#include <sscl/ssclc.h>
#include <sscl/object.h>
#include <sscl/stream.h>
#include <sscl/error.h>

namespace SSCL {

class SDTConn: public Object {
    public:
	SDTConn(const int len=SSCL_BUF_LEN);
	~SDTConn();
	virtual void log(SDTLogLevel level, const char *format,...);
	virtual void set_state(SDTState stat);
	virtual int check_event();
	// Event methods, called by check_event()
	// some of them are abstract, should be overwritten
	virtual void start_event();
	virtual void quit_event()=0;
	virtual void status_event();
    protected:
	virtual void send(const char *format, ...);
    private:
	SDTState state;
	InStream in;
	OutStream out;
};

} /* namespace SSCL */

#endif /* _SSCL_SDT_H */
