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
#ifndef _SSCL_OBJECT_H_
#define _SSCL_OBJECT_H_

#include <stdlib.h>
#include <string.h>

// ============================================================= Object
class Object {
    public:
	Object(char *nam="");
	virtual ~Object();
	char *get_name();
    private:
	char *name;
};

// ============================================================ Container
class Container: public Object {
    public:
	// The master flag controls whether the contained objects should be
	// destructed on container destruction.
	Container(char *nam="", bool master=true);
    protected:
	bool master;
};

#endif /* _SSCL_OBJECT_H_ */
