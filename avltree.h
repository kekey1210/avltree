
/********************************************************************
Created:	2010/06/18
Author:		zhangzl
Version:	1.0
History:	2010/06/18	created
	
Comment:	
	this is an interface of avl tree which is used to store
data those have a 'key-value' style, it looks like the data 
structure map of STL, but is simpler and more efficient, when 
you want search data in memery by a key, you can use this tree, 
the key type and value type are both 'void*' which can be any 
type you want, but be careful, the user is responsible for 
allocating or releasing the memery of the key or the value.

Example:	
//key compare function
int keyCompareFunc(void* pKey1, void* pKey2)
{
	if ( (int)pKey1 < (int)pKey2 )return -1;
	if ( (int)pKey1 > (int)pKey2 )return 1;
	return 0;
}

void test()
{
	AvlTree* pTree = AvlTree_create(keyCompareFunc);

	//insert items
	for ( int i = 0; i < 10000; i++ )
	{
		void* pKey = (void*)i;
		char* psData = malloc(10);
		psData[0] = 0;
		sprintf(psData, "key=%d", i);
		AvlTree_insert(pTree, pKey, (void*)psData);
	}

	//search an item
	AvlIterator it = AvlTree_find(pTree, (void*)168);

	//traverse the tree
	it = AvlTree_first(pTree);
	while ( it )
	{
		//do something else
		it = AvlTree_next(it);
	}

	//delete an item
	AvlTree_deleteByKey(pTree, (void*)168);

	//clear the tree, be careful! AvlTree_clear() dosen't in charge of the memory you allocated
	//so you have to free them before clear the tree, you can clear like this:
	it = AvlTree_first(pTree);
	while ( it )
	{
		free(it->pValue);
		it = AvlTree_delete(pTree, it);//the function AvlTree_delete() return the next iterator after delete
	}

	//or like this
	it = AvlTree_first(pTree);
	while ( it )
	{
		free(it->pValue);
		it = AvlTree_next(it);
	}
	AvlTree_clear(pTree);//then clear the tree

	//destroy the tree
	AvlTree_destroy(pTree);
}
*********************************************************************/

#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef int		BOOL;
typedef struct	AvlTree	AvlTree;

typedef struct tagAvlIterator
{
	void* const pKey;
	void* pValue;
}*AvlIterator;

typedef int (*keyCompare)(void* key1, void* key2);


//create a tree
AvlTree* AvlTree_create(keyCompare keyCompareFunc);

//destroy the tree after use it
void AvlTree_destroy(AvlTree* pTree);

//clear all item but not destroy the tree
void AvlTree_clear(AvlTree* pTree);

//return the total count of the tree items
int AvlTree_getCount(AvlTree* pTree);

//in general, pKey and pValue allocated by the user, but if the key or value type
//is for example 'int', you don't need to allocate memory for it, you can call like
//this:AvlTree_insert(pTree, (void*)nIntKey, (void*)pValue), nIntKey is an int type data.
BOOL AvlTree_insert(AvlTree* pTree, void* pKey, void* pValue);

//delete an item by key, in most situation we use AvlTree_delete(), because if necessary we can free
//the memory through a AvlIterator before delete the item
BOOL AvlTree_deleteByKey(AvlTree* pTree, void* pKey);

AvlIterator AvlTree_delete(AvlTree* pTree, AvlIterator pIterator);

//search an item by key, return an AvlIterator type value
AvlIterator AvlTree_find(AvlTree* pTree, void* pKey);

//return the 'smallest' or the left most item
AvlIterator AvlTree_first(AvlTree *pTree);

//return the 'largest' or right most item
AvlIterator AvlTree_last(AvlTree *pTree);

//return the next item of pIterator
AvlIterator AvlTree_next(AvlIterator pIterator);

//return the previous item of pIterator
AvlIterator AvlTree_prev(AvlIterator pIterator);

//check if the tree is a valid avl tree
BOOL AvlTree_isValid(AvlTree* pTree);

#ifdef __cplusplus
}
#endif

#endif