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
 * Generated from $Id: avltree.cc,v 1.1 2005/03/18 12:32:51 szilu Exp $
 *
 * The author of AVLTree can be reached at the following address:
 * Michael H. Buselli
 * 30051 N. Waukegan Rd. Apt. 103
 * Lake Bluff, IL  60044-5412
 *
 * Or you can send email to cosine at cosine dot org.
 * The official web page for this product is:
 * http://www.cosine.org/project/AVLTree/
 */

#include <sscl/avltree.h>

#define MAX(x, y)      ((x) > (y) ? (x) : (y))
#define MIN(x, y)      ((x) < (y) ? (x) : (y))
#define L_DEPTH(n)     ((n)->left ? (n)->left->depth : 0)
#define R_DEPTH(n)     ((n)->right ? (n)->right->depth : 0)
#define CALC_DEPTH(n)  (MAX(L_DEPTH(n), R_DEPTH(n)) + 1)

namespace SSCL {

AVLTree::AVLTree(char *nam, bool master): Container(nam, master)
{
    top=NULL;
    count=0;
}

AVLTree::~AVLTree()
{
    if (top) delete top;
}

int AVLTree::insert(Object *item, AVLKey key)
{
    AVLNode *newnode;
    AVLNode *node;
    AVLNode *balnode;
    AVLNode *nextbalnode;

    newnode = new AVLNode;
    if (newnode == NULL)
	return -1;

    newnode->key = key?key:item->get_name();
    newnode->item = item;
    newnode->depth = 1;
    newnode->left = NULL;
    newnode->right = NULL;
    newnode->parent = NULL;

    if (top != NULL) {
	node = close_search_node(newnode->key);

	if (!key_cmp(node->key, newnode->key)) {
	    free(newnode);
	    return 3;
	}

	newnode->parent = node;

	if (key_cmp(newnode->key, node->key) < 0) {
	    node->left = newnode;
	    node->depth = CALC_DEPTH(node);
	}

	else {
	    node->right = newnode;
	    node->depth = CALC_DEPTH(node);
	}

	for (balnode = node->parent; balnode; balnode = nextbalnode) {
	    nextbalnode = balnode->parent;
	    rebalance_node(balnode);
	}
    }

    else {
	top = newnode;
    }

    count++;
    return 0;
}

Object *AVLTree::get(AVLKey key)
{
    AVLNode *node;
    node=close_search_node(key);
    if (node && !key_cmp(node->key, key))
	return node->item;
    return NULL;
}

int AVLTree::remove(AVLKey key)
{
    AVLNode *avlnode;
    AVLNode *origparent;
    AVLNode **superparent;

    avlnode = close_search_node(key);
    if (avlnode == NULL || key_cmp(avlnode->key, key))
	return -1;

    origparent = avlnode->parent;

    if (origparent) {
	if (key_cmp(avlnode->key, avlnode->parent->key) < 0)
	    superparent = &(avlnode->parent->left);
	else
	    superparent = &(avlnode->parent->right);
    }
    else
	superparent = &(top);

    fill_vacancy(origparent, superparent, avlnode->left, avlnode->right);
    free(avlnode);
    count--;
    return 0;
}

AVLTree::Iterator::Iterator()
{
    curnode=NULL;
}

AVLTree::Iterator::Iterator(AVLNode *node)
{
    curnode=node;
}

Object *AVLTree::Iterator::get()
{
    return curnode ? curnode->item : NULL;
}

/*
 * first:
 * Initializes an AVLTree::Iterator object and returns the item with the lowest
 * key in the AVLTree.
 */
AVLTree::Iterator AVLTree::first()
{
    AVLTree::Iterator i;
    if (!this->top) {
	i.curnode = NULL;
    } else {
	for (i.curnode = top; i.curnode->left; i.curnode=i.curnode->left);
    }
    return i;
}


/*
 * next:
 * Called after an first() call, this returns the item with the least
 * key that is greater than the last item returned either by first()
 * or a previous invokation of this function.
 */
Object *AVLTree::Iterator::next()
{
    AVLNode *avlnode;
    avlnode = curnode;
    if (avlnode->right != NULL) {
	for (avlnode = avlnode->right;
	       avlnode->left != NULL;
	       avlnode = avlnode->left);
	curnode = avlnode;
	return avlnode->item;
    }
    while (avlnode->parent && avlnode->parent->left != avlnode) {
	avlnode = avlnode->parent;
    }
    if (avlnode->parent == NULL) {
	curnode = NULL;
	return NULL;
    }
    curnode = avlnode->parent;
    return avlnode->parent->item;
}


/*
 * close_search_node:
 * Return a pointer to the node closest to the given key.
 * Returns NULL if the AVL tree is empty.
 */
AVLTree::AVLNode *AVLTree::close_search_node(AVLKey key)
{
    AVLNode *node;
    node = top;
    if (!node)
	return NULL;
    for (;;) {
	if (!key_cmp(node->key, key))
	    return node;

	if (key_cmp(node->key, key) < 0) {
	    if (node->right)
	      node = node->right;
	    else
	      return node;
	}

	else {
	    if (node->left)
	      node = node->left;
	    else
	      return node;
	}
    }
}


/*
 * rebalance_node:
 * Rebalances the AVL tree if one side becomes too heavy.  This function
 * assumes that both subtrees are AVL trees with consistant data.  This
 * function has the additional side effect of recalculating the depth of
 * the tree at this node.  It should be noted that at the return of this
 * function, if a rebalance takes place, the top of this subtree is no
 * longer going to be the same node.
 */
void AVLTree::rebalance_node(AVLNode *avlnode)
{
    long depthdiff;
    AVLNode *child;
    AVLNode *gchild;
    AVLNode *origparent;
    AVLNode **superparent;

    origparent = avlnode->parent;

    if (origparent) {
	if (key_cmp(avlnode->key, avlnode->parent->key) < 0)
	    superparent = &(avlnode->parent->left);
	else
	    superparent = &(avlnode->parent->right);
    }
    else
	superparent = &top;

    depthdiff = R_DEPTH(avlnode) - L_DEPTH(avlnode);

    if (depthdiff <= -2) {
	child = avlnode->left;

	if (L_DEPTH(child) >= R_DEPTH(child)) {
	    avlnode->left = child->right;
	    if (avlnode->left != NULL)
	      avlnode->left->parent = avlnode;
	    avlnode->depth = CALC_DEPTH(avlnode);
	    child->right = avlnode;
	    if (child->right != NULL)
	      child->right->parent = child;
	    child->depth = CALC_DEPTH(child);
	    *superparent = child;
	    child->parent = origparent;
	}

	else {
	    gchild = child->right;
	    avlnode->left = gchild->right;
	    if (avlnode->left != NULL)
	      avlnode->left->parent = avlnode;
	    avlnode->depth = CALC_DEPTH(avlnode);
	    child->right = gchild->left;
	    if (child->right != NULL)
	      child->right->parent = child;
	    child->depth = CALC_DEPTH(child);
	    gchild->right = avlnode;
	    if (gchild->right != NULL)
	      gchild->right->parent = gchild;
	    gchild->left = child;
	    if (gchild->left != NULL)
	      gchild->left->parent = gchild;
	    gchild->depth = CALC_DEPTH(gchild);
	    *superparent = gchild;
	    gchild->parent = origparent;
	}
    }

    else if (depthdiff >= 2) {
	child = avlnode->right;

	if (R_DEPTH(child) >= L_DEPTH(child)) {
	    avlnode->right = child->left;
	    if (avlnode->right != NULL)
	      avlnode->right->parent = avlnode;
	    avlnode->depth = CALC_DEPTH(avlnode);
	    child->left = avlnode;
	    if (child->left != NULL)
	      child->left->parent = child;
	    child->depth = CALC_DEPTH(child);
	    *superparent = child;
	    child->parent = origparent;
	}

	else {
	    gchild = child->left;
	    avlnode->right = gchild->left;
	    if (avlnode->right != NULL)
	      avlnode->right->parent = avlnode;
	    avlnode->depth = CALC_DEPTH(avlnode);
	    child->left = gchild->right;
	    if (child->left != NULL)
	      child->left->parent = child;
	    child->depth = CALC_DEPTH(child);
	    gchild->left = avlnode;
	    if (gchild->left != NULL)
	      gchild->left->parent = gchild;
	    gchild->right = child;
	    if (gchild->right != NULL)
	      gchild->right->parent = gchild;
	    gchild->depth = CALC_DEPTH(gchild);
	    *superparent = gchild;
	    gchild->parent = origparent;
	}
    }

    else {
	avlnode->depth = CALC_DEPTH(avlnode);
    }
}


/*
 * Free memory used by this node and its item.  If the freeitem argument
 * is not NULL, then that function is called on the items to free their
 * memory as well.  In other words, the freeitem function is a
 * destructor for the items in the tree.
 */
AVLTree::AVLNode::~AVLNode()
{
    if (left) delete left;
    if (right) delete right;
    delete item;
}


/*
 * fill_vacancy:
 * Given a vacancy in the AVL tree by it's parent, children, and parent
 * component pointer, fill that vacancy.
 */
void AVLTree::fill_vacancy(AVLNode *origparent, AVLNode **superparent,
	AVLNode *left, AVLNode *right)
{
    AVLNode *avlnode;
    AVLNode *balnode;
    AVLNode *nextbalnode;

    if (left == NULL) {
	if (right)
	    right->parent = origparent;

	*superparent = right;
	balnode = origparent;
    }

    else {
	for (avlnode = left; avlnode->right != NULL; avlnode = avlnode->right);

	if (avlnode == left) {
	    balnode = avlnode;
	}
	else {
	    balnode = avlnode->parent;
	    balnode->right = avlnode->left;
	    if (balnode->right != NULL)
	      balnode->right->parent = balnode;
	    avlnode->left = left;
	    left->parent = avlnode;
	}

	avlnode->right = right;
	if (right != NULL)
	    right->parent = avlnode;
	*superparent = avlnode;
	avlnode->parent = origparent;
    }

    for (; balnode; balnode = nextbalnode) {
	nextbalnode = balnode->parent;
	rebalance_node(balnode);
    }
}

} /* namespace SSCL */
