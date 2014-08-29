avltree
=======
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
