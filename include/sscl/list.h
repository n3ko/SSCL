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

#include <sscl/object.h>

// ============================================================ List
class List: public Container {
    public:
	class Iterator;
	class Item {
	    public:
		Item(Object *obj);
		Object *get();
	    private:
		Object *item;
		Item *next;
		Item *prev;
		friend class List;
		friend class Iterator;
	};
	class Iterator {
	    public:
		Iterator();
		Iterator(Item *it);
		Object *next();
		Object *get();
		Object *operator++() {return next();};
	    private:
		Item *curitem;
		friend class List;
	};
	List(char *nam="", bool master=true);
	~List();
	Item *append(Object *item);
	Object *get(int num);
	int get_num();
    private:
	Item *first;
	Item *last;
	Item *cursor;
	int num;
};

#endif /* _SSCL_LIST_H_ */
