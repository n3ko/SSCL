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
 *
 * avltree.cc and avltree.h is based on AVLTree
 * Copyright (C) 1998,2001  Michael H. Buselli
 * zAVLTree.c: Source code for zAVLTrees.
 * This is version 0.1.3 (alpha).
 * Generated from $Id: avltree.h,v 1.3 2005/03/18 12:32:51 szilu Exp $
 *
 * The author of AVLTree can be reached at the following address:
 * Michael H. Buselli
 * 30051 N. Waukegan Rd. Apt. 103
 * Lake Bluff, IL  60044-5412
 *
 * Or you can send email to <cosine@cosine.org>.
 * The official web page for this product is:
 * http://www.cosine.org/project/AVLTree/
 */
#ifndef _AVLTREE_H_
#define _AVLTREE_H_

#include <sscl/object.h>

namespace SSCL {

// ============================================================ AVLTree
/* typedef the keytype */
typedef const char *AVLKey;

/* Comparison function for strings is strcmp(). */
#define key_cmp(a, b) (strcmp((a), (b)))

class AVLTree: public Container {
    public:
	class Iterator;
	class AVLNode {
	    public:
		~AVLNode();
	    private:
		AVLKey key;
		long depth;
		Object *item;
		AVLNode *parent;
		AVLNode *left;
		AVLNode *right;
		friend class AVLTree;
		friend class Iterator;
		// Private methods
	};
	class Iterator {
	    public:
		Iterator();
		Iterator(AVLNode *node);
		Object *next();
		AVLKey key();
		Object *get();
		Object *operator++() {return next();};
		Object *operator++(int) {Object *o=get(); return next(), o;};
		Object *operator*() {return get();};
	    private:
		AVLNode *curnode;
		friend class AVLTree;
	};
	AVLTree(char *nam="", bool master=true);
	~AVLTree();
	int insert(Object *item, AVLKey key=(AVLKey)NULL);
	Object *get(AVLKey key);
	int remove(AVLKey key);
	AVLTree::Iterator first();
	AVLTree::Iterator search();
	Object *operator[](AVLKey key) {return get(key);};
    private:
	AVLNode *top;
	long count;
	// Private methods
	AVLNode *close_search_node(AVLKey key);
	void rebalance_node (AVLNode *avlnode);
	void fill_vacancy(AVLNode *origparent, AVLNode **superparent,
		AVLNode *left, AVLNode *right);
};

} /* namespace SSCL */
#endif /* _AVLTREE_H_ */
