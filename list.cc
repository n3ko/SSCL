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

#include <sscl/list.h>

//=============================================================================
//	List
//=============================================================================

List::Item::Item(Object *obj)
{
    item=obj;
}

Object *List::Item::get()
{
    return item;
}

List::List(char *nam, bool master): Container(nam, master)
{
    first=NULL;
    last=NULL;
    cursor=NULL;
    num=0;
}

List::~List()
{
    Item *p, *prev;
    p=first;
    while (p) {
	prev=p; p=p->next;
	if (master) delete prev->item;
	delete prev;
    }
}

List::Item *List::append(Object *item)
{
    if (first) {
	last->next=new List::Item(item);
	last->next->prev=last;
	last=last->next;
    } else {
	last=first=new List::Item(item);
	last->prev=NULL;
    }
    last->next=NULL;
    num++;
    return last;
}

Object *List::get(int num)
{
    Item *p;
    for (p=first; p && num>0; p=p->next) num--;
    if (p) return p->item;
    else return NULL;
}

int List::get_num()
{
    return num;
}

List::Iterator::Iterator()
{
    curitem=NULL;
}

List::Iterator::Iterator(Item *it)
{
    curitem=it;
}

Object *List::Iterator::next()
{
    if (curitem) {
	Object *o=curitem->get();
	curitem=curitem->next;
	return o;
    } else return NULL;
}
