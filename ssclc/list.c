/* SSCL - Symbion Simple Class Library
 * Copyright (C) 2001-2005 Szilard Hajba
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

#include <sscl/ssclc.h>
#include <string.h>

List *list_init(List *list)
{
    list->first=NULL;
    list->last=NULL;
    list->num=0;
    return list;
}

List *list_new()
{
    List *list=NEW(List);
    list->first=NULL;
    list->last=NULL;
    list->num=0;
    return list;
}

void list_done(List *list)
{
    ListItem *p, *prev;
    for (p=list->first; p; (prev=p, p=p->next, free(prev)));
    list_init(list);
}

void list_done_with_data(List *list)
{
    ListItem *p, *prev;
    for (p=list->first; p; (prev=p, p=p->next, free(prev->data), free(prev)));
    list_init(list);
}

void list_free(List *list)
{
    list_done(list);
    free(list);
}

void list_free_with_data(List *list)
{
    list_done_with_data(list);
    free(list);
}

void list_clear(List *list)
{
    list_done(list);
}

void list_clear_with_data(List *list)
{
    list_done_with_data(list);
}

void list_append(List *list, void *data)
{
    if (list->first) {
	list->last->next=NEW(ListItem);
	list->last->next->prev=list->last;
	list->last=list->last->next;
    } else {
	list->first=NEW(ListItem);
	list->last=list->first;
	list->last->prev=NULL;
    }
    list->last->data=data;
    list->last->next=NULL;
    list->num++;
}

void list_push(List *list, void *data)
{
    if (list->first) {
	list->first->prev=NEW(ListItem);
	list->first->prev->next=list->first;
	list->first=list->first->prev;
    } else {
	list->first=NEW(ListItem);
	list->last=list->first;
	list->first->next=NULL;
    }
    list->first->data=data;
    list->first->prev=NULL;
    list->num++;
}

void *list_pop(List *list)
{
    void *ret;
    ListItem *p=list->first;
    if (p) {
	list->first=p->next;
	if (list->first) list->first->prev=NULL;
	else list->last=NULL;
	ret=p->data;
	free(p);
	list->num--;
    } else ret=NULL;
    return ret;
}

void *list_get(List *list, int num)
{
    ListItem *p;
    for (p=list->first; p && num>0; p=p->next) num--;
    if (p) return p->data;
    else return NULL;
}

void list_foreach(List *list, void (*func)(int, void *))
{
    int i=0;
    ListItem *p;
    for (p=list->first; p; p=p->next, i++) func(i, p->data);
}

//=============================================================================
//	SList
//=============================================================================

SList *slist_init(SList *list)
{
    list->first=NULL;
    list->last=NULL;
    list->num=0;
    return list;
}

SList *slist_new()
{
    SList *p=NEW(SList);
    bzero(p, sizeof(SList));
    p->first=NULL;
    p->last=NULL;
    p->num=0;
    return p;
}

void slist_done(SList *list)
{
    SListItem *p, *prev;
    for (p=list->first; p; (prev=p, p=p->next, free(prev)));
    slist_init(list);
}

void slist_done_with_data(SList *list)
{
    SListItem *p, *prev;
    for (p=list->first; p; (prev=p, p=p->next, free(prev->data), free(prev)));
    slist_init(list);
}

void slist_free(SList *list)
{
    SListItem *p, *prev;
    for (p=list->first; p; (prev=p, p=p->next, free(prev)));
    free(list);
}

void slist_free_with_data(SList *list)
{
    SListItem *p, *prev;
    for (p=list->first; p; (prev=p, p=p->next, free(prev->data), free(prev)));
    free(list);
}

void slist_append(SList *list, void *data)
{
    if (list->first) {
	list->last->next=NEW(SListItem);
	list->last=list->last->next;
    } else {
	list->first=NEW(SListItem);
	list->last=list->first;
    }
    list->last->data=data;
    list->last->next=NULL;
    list->num++;
}

void slist_push(SList *list, void *data)
{
    SListItem *li=NEW(SListItem);
    li->next=list->first;
    list->first=li;
    if (!list->last) {
	list->last=list->first;
    }
    li->data=data;
    list->num++;
}

void *slist_pop(SList *list)
{
    void *ret;
    SListItem *p=list->first;
    if (p) {
	list->first=p->next;
	if (!list->first) list->last=NULL;
	ret=p->data;
	free(p);
	list->num--;
    } else ret=NULL;
    return ret;
}

void *slist_get(SList *list, int num)
{
    SListItem *p;
    for (p=list->first; p && num>0; p=p->next);
    return p;
}

void slist_foreach(SList *list, void (*func)(int, void *))
{
    int i=0;
    SListItem *p;
    for (p=list->first; p; p=p->next, i++) func(i, p->data);
}

int list_get_num(List *list)
{
    return list->num;
}
