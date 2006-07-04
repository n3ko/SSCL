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

#ifndef _SSCL_LIST_H_
#define _SSCL_LIST_H_

#include <sscl/ssclc.h>

namespace SSCL {

// ============================================================ List
class List: public Container {
    public:
	class Iterator;
	class Iterator {
	    public:
		Iterator(ListItem *it) {curitem=it;}
		void *next() {curitem=curitem->next; return curitem->data;}
		void *get() {return curitem->data;}
		void *operator++() {return next();};
	    private:
		ListItem *curitem;
		friend class List;
	};
	List() {list_init(&cs);}
	~List() {list_done(&cs);}
	void append(void *ptr) {list_append(&cs, ptr);}
	void push(void *ptr) {list_push(&cs, ptr);}
	void *pop() {return list_pop(&cs);}
	void *get(int n) {return list_get(&cs, n);}
	int get_num() {return list_get_num(&cs);}
	Iterator *first() {return new Iterator(cs.first);}
	void clear() {list_clear(&cs);}
	void clear_with_data() {list_clear_with_data(&cs);}
    private:
	::List cs;
};

} /* namespace SSCL */

#endif /* _SSCL_LIST_H_ */
