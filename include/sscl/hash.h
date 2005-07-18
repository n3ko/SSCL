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

#ifndef _SSCL_HASH_H_
#define _SSCL_HASH_H_

#include <sscl/ssclc.h>

namespace SSCL {

// ============================================================ Hash
class Hash: public Container {
    public:
	Hash(int size=16, HashFunc hash_func=NULL) {hash_init(&cs, size, hash_func);}
	~Hash() {hash_done(&cs);}
	void set(const char *key, void *data) {hash_set(&cs, key, data);}
	const void *get(const char *key) {return hash_get(&cs, key);}
	const void *del(const char *key) {return hash_delete(&cs, key);}
	const void *operator[](const char *key) {return hash_get(&cs, key);}
	void foreach(void (*func)(const char *, void *, void *), void *data) {
	    return hash_foreach(&cs, func, data);
	}
    private:
	::Hash cs;
};

} /* namespace SSCL */

#endif /* _SSCL_HASH_H_ */
