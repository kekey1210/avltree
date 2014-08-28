#include "avltree.h"
#include <malloc.h>
#include <stdlib.h>
#include <crtdbg.h>

typedef struct AvlNode	AvlNode;

struct AvlNode
{
	void*		pKey;
	void*		pValue;
	AvlNode*	pParent;
	AvlNode*	pLeftChild;
	AvlNode*	pRightChild;
	AvlNode*	pPrevNode;
	AvlNode*	pNextNode;
	int			nHeight;
};

struct AvlTree
{
	AvlNode*	pTreeRoot;//根节点
	unsigned int	nCount;//节点总数
	keyCompare		fpKeyCompare;
};


//内部调用函数声明
static void AvlTree_destroyNode(AvlTree* pTree, AvlNode** pNode);
static void AvlNode_singleRotateWithLeft(AvlNode** ppNode);
static void AvlNode_singleRotateWithRight(AvlNode** ppNode);
static void AvlNode_doubleRotateWithLeft(AvlNode** ppNode);
static void AvlNode_doubleRotateWithRight(AvlNode** ppNode);
static BOOL AvlTree_insertNode(AvlTree* pTree, AvlNode** ppStartPos,AvlNode* pInsertNode);
static AvlNode** AvlTree_findNodeByKey(keyCompare func, AvlNode** ppRoot, void* pKey);
static AvlNode** AvlTree_findNodeByIt(AvlNode** pRoot, AvlIterator pIterator);
static BOOL AvlTree_deleteByNode(AvlTree* pTree, AvlNode** ppNode);
static BOOL AvlNode_check(AvlNode* pNode);


#define AvlNode_height(node)	( (node) ? ((node)->nHeight) : (-1) )

static void AvlTree_destroyNode(AvlTree* pTree, AvlNode** pNode)
{
	if ( pNode && *pNode )
	{
		AvlTree_destroyNode(pTree, &(*pNode)->pLeftChild);
		AvlTree_destroyNode(pTree, &(*pNode)->pRightChild);
		free(*pNode);
		*pNode = NULL;
		pTree->nCount--;
		_ASSERT( pTree->nCount >= 0 );
	}
}

AvlTree	*AvlTree_create(keyCompare keyCompareFunc)
{
	if ( keyCompareFunc )
	{
		AvlTree* pTree = malloc(sizeof(AvlTree));
		pTree->pTreeRoot = NULL;
		pTree->nCount = 0;
		pTree->fpKeyCompare = keyCompareFunc;
		return pTree;
	}
	return NULL;
}

void AvlTree_destroy(AvlTree* pTree)
{
	_ASSERT(pTree);
	if ( pTree )
	{
		AvlTree_clear(pTree);
		free(pTree);
	}
}

void AvlTree_clear(AvlTree* pTree)
{
	_ASSERT(pTree);
	AvlTree_destroyNode(pTree, &pTree->pTreeRoot);
}

static void AvlNode_singleRotateWithLeft(AvlNode** ppNode)
{
	AvlNode* pNewRoot;
	AvlNode* pNode;
	_ASSERT(ppNode && *ppNode);
	pNode = *ppNode;

	pNewRoot = pNode->pLeftChild;
	pNode->pLeftChild = pNewRoot->pRightChild;
	if ( pNewRoot->pRightChild )
		pNewRoot->pRightChild->pParent = pNode;
	pNewRoot->pRightChild = pNode;

	pNewRoot->pParent = pNode->pParent;
	if ( pNewRoot->pParent )
	{
		if ( pNode->pParent->pLeftChild==pNode )
			pNewRoot->pParent->pLeftChild = pNewRoot;
		else
			pNewRoot->pParent->pRightChild = pNewRoot;
	}
	pNode->pParent = pNewRoot;

	pNode->nHeight = max(AvlNode_height(pNode->pLeftChild),
		AvlNode_height(pNode->pRightChild)) + 1;

	pNewRoot->nHeight = max(AvlNode_height(pNewRoot->pLeftChild), pNode->nHeight) + 1;

	*ppNode = pNewRoot;
}

static void AvlNode_doubleRotateWithLeft(AvlNode** ppNode)
{
	_ASSERT(ppNode && *ppNode);
	AvlNode_singleRotateWithRight(&(*ppNode)->pLeftChild);
	AvlNode_singleRotateWithLeft(ppNode);
}

static void AvlNode_singleRotateWithRight(AvlNode** ppNode)
{
	AvlNode* pNewRoot;
	AvlNode* pNode;
	_ASSERT(ppNode && *ppNode);
	pNode = *ppNode;

	pNewRoot = pNode->pRightChild;
	pNode->pRightChild = pNewRoot->pLeftChild;
	if ( pNewRoot->pLeftChild )
		pNewRoot->pLeftChild->pParent = pNode;
	pNewRoot->pLeftChild = pNode;

	pNewRoot->pParent = pNode->pParent;
	if ( pNewRoot->pParent )
	{
		if ( pNode->pParent->pLeftChild==pNode )
			pNewRoot->pParent->pLeftChild = pNewRoot;
		else
			pNewRoot->pParent->pRightChild = pNewRoot;
	}
	pNode->pParent = pNewRoot;

	pNode->nHeight = max(AvlNode_height(pNode->pLeftChild),
		AvlNode_height(pNode->pRightChild)) + 1;

	pNewRoot->nHeight = max(AvlNode_height(pNewRoot->pRightChild), pNode->nHeight) + 1;

	*ppNode = pNewRoot;
}

static void AvlNode_doubleRotateWithRight(AvlNode** ppNode)
{
	_ASSERT(ppNode && *ppNode);
	AvlNode_singleRotateWithLeft(&(*ppNode)->pRightChild);
	AvlNode_singleRotateWithRight(ppNode);
}

static BOOL AvlTree_insertNode(AvlTree* pTree, AvlNode** ppStartPos,AvlNode* pInsertNode)
{
	AvlNode* pStartPos;
	int cmpVal;
	_ASSERT(ppStartPos && *ppStartPos);
	pStartPos = *ppStartPos;
	cmpVal = pTree->fpKeyCompare(pInsertNode->pKey, pStartPos->pKey);
	if ( cmpVal < 0 )
	{
		if ( !pStartPos->pLeftChild )
		{
			pInsertNode->pParent = *ppStartPos;
			pInsertNode->pNextNode = *ppStartPos;
			pInsertNode->pPrevNode = (*ppStartPos)->pPrevNode;
			if ( pInsertNode->pPrevNode )
				pInsertNode->pPrevNode->pNextNode = pInsertNode;
			(*ppStartPos)->pLeftChild = pInsertNode;
			(*ppStartPos)->pPrevNode = pInsertNode;
			pTree->nCount++;
			pStartPos = *ppStartPos;
		}
		else if ( !AvlTree_insertNode(pTree, &pStartPos->pLeftChild, pInsertNode) )
		{
			return FALSE;
		}
		if ( ( AvlNode_height(pStartPos->pLeftChild) - AvlNode_height(pStartPos->pRightChild) ) == 2 )
		{
			if ( pTree->fpKeyCompare(pInsertNode->pKey, pStartPos->pLeftChild->pKey) < 0 )
				AvlNode_singleRotateWithLeft(ppStartPos);
			else
				AvlNode_doubleRotateWithLeft(ppStartPos);
		}
	}
	else if ( cmpVal > 0 )
	{
		if ( !pStartPos->pRightChild )
		{
			pInsertNode->pParent = *ppStartPos;
			pInsertNode->pPrevNode = *ppStartPos;
			pInsertNode->pNextNode = (*ppStartPos)->pNextNode;
			if ( pInsertNode->pNextNode )
				pInsertNode->pNextNode->pPrevNode = pInsertNode;
			(*ppStartPos)->pRightChild = pInsertNode;
			(*ppStartPos)->pNextNode = pInsertNode;
			pTree->nCount++;
			pStartPos = *ppStartPos;
		}
		else if ( !AvlTree_insertNode(pTree, &pStartPos->pRightChild, pInsertNode) )
		{
			return FALSE;
		}
		if ( ( AvlNode_height(pStartPos->pRightChild) - AvlNode_height(pStartPos->pLeftChild) ) == 2 )
		{
			if ( pTree->fpKeyCompare(pInsertNode->pKey, pStartPos->pRightChild->pKey) > 0 )
				AvlNode_singleRotateWithRight(ppStartPos);
			else
				AvlNode_doubleRotateWithRight(ppStartPos);
		}
	}
	else
	{
		return FALSE;//已经存在
	}

	pStartPos = *ppStartPos;
	pStartPos->nHeight = max(AvlNode_height(pStartPos->pLeftChild),
		AvlNode_height(pStartPos->pRightChild)) + 1;
	return TRUE;
}

static BOOL AvlTree_deleteByNode(AvlTree* pTree, AvlNode** ppNode)
{
	AvlNode* pKeyNode;
	AvlNode *pTmp, *pStartPos;
	if ( !ppNode || !*ppNode )
		return FALSE;

	pKeyNode = *ppNode;
	pTmp = pStartPos = NULL;
	if ( !pKeyNode->pLeftChild && !pKeyNode->pRightChild )
	{
		//叶子节点
		AvlNode *pPrev,*pNext;
		pPrev = pKeyNode->pPrevNode;
		pNext = pKeyNode->pNextNode;
		if ( pPrev )
			pPrev->pNextNode = pNext;
		if ( pNext )
			pNext->pPrevNode = pPrev;
		pStartPos = pKeyNode->pParent;
		pTree->nCount--;
		_ASSERT(pTree->nCount>=0);
		free(*ppNode);
		*ppNode = NULL;
	}
	else if ( pKeyNode->pLeftChild && !pKeyNode->pRightChild )
	{
		//只有左子树
		AvlNode *p = pKeyNode->pLeftChild;
		AvlNode *pPrev,*pNext;

		pPrev = pKeyNode->pPrevNode;
		pNext = pKeyNode->pNextNode;
		if ( pPrev )
			pPrev->pNextNode = pNext;
		if ( pNext )
			pNext->pPrevNode = pPrev;
		
		pStartPos = pKeyNode->pParent;
		p->pParent = pKeyNode->pParent;
		pTree->nCount--;
		_ASSERT(pTree->nCount>=0);
		free(*ppNode);
		*ppNode = p;
	}
	else if ( !pKeyNode->pLeftChild && pKeyNode->pRightChild )
	{
		//只有右子树
		AvlNode *p = pKeyNode->pRightChild;
		AvlNode *pPrev,*pNext;

		pPrev = pKeyNode->pPrevNode;
		pNext = pKeyNode->pNextNode;
		if ( pPrev )
			pPrev->pNextNode = pNext;
		if ( pNext )
			pNext->pPrevNode = pPrev;

		pStartPos = pKeyNode->pParent;
		p->pParent = pKeyNode->pParent;
		pTree->nCount--;
		_ASSERT(pTree->nCount>=0);
		free(*ppNode);
		*ppNode = p;
	}
	else
	{
		//左右子树都有
		AvlNode** pBeyondNode;
		AvlNode *pNodeTmp;
		void *pKeyTmp, *pValueTmp;
		if ( AvlNode_height(pKeyNode->pLeftChild) > 
			AvlNode_height(pKeyNode->pRightChild ))
		{
			//找到pDeletePos的左子树中最右边的节点，即中序遍历的前驱节点
			pBeyondNode = &pKeyNode->pLeftChild;
			while ( (*pBeyondNode)->pRightChild )pBeyondNode = &(*pBeyondNode)->pRightChild;
		}
		else
		{
			//找到pDeletePos右子树中最左边的节点，即中序遍历的后续节点
			pBeyondNode = &pKeyNode->pRightChild;
			while ( (*pBeyondNode)->pLeftChild )pBeyondNode = &(*pBeyondNode)->pLeftChild;
		}

		pNodeTmp = (*pBeyondNode)->pParent;
		pKeyTmp = (*pBeyondNode)->pKey;
		pValueTmp = (*pBeyondNode)->pValue;
		if ( !AvlTree_deleteByNode(pTree, pBeyondNode) )
			return FALSE;

		pKeyNode->pKey = pKeyTmp;
		pKeyNode->pValue = pValueTmp;

		pStartPos = pNodeTmp;
	}

	//更新高度
	if ( pStartPos )
	{
		pTmp = pStartPos;
		while ( pTmp )
		{
			if ( ( AvlNode_height(pTmp->pRightChild) - AvlNode_height(pTmp->pLeftChild) ) == 2 )
			{
				_ASSERT(pTmp->pRightChild);
				if ( AvlNode_height(pTmp->pRightChild->pRightChild)
					>= AvlNode_height(pTmp->pRightChild->pLeftChild) )
					AvlNode_singleRotateWithRight(&pTmp);
				else
					AvlNode_doubleRotateWithRight(&pTmp);
			}
			else if ( ( AvlNode_height(pTmp->pLeftChild) - AvlNode_height(pTmp->pRightChild) ) == 2 )
			{
				_ASSERT(pTmp->pLeftChild);
				if ( AvlNode_height(pTmp->pLeftChild->pLeftChild)
					>= AvlNode_height(pTmp->pLeftChild->pRightChild) )
					AvlNode_singleRotateWithLeft(&pTmp);
				else
					AvlNode_doubleRotateWithLeft(&pTmp);
			}
			pTmp->nHeight = max(AvlNode_height(pTmp->pLeftChild),
				AvlNode_height(pTmp->pRightChild)) + 1;
			if ( pTmp->pParent == NULL )
			{
				//修改树根
				pTree->pTreeRoot = pTmp;
			}
			pTmp = pTmp->pParent;
		}
	}

	return TRUE;
}

AvlIterator AvlTree_delete(AvlTree* pTree, AvlIterator pIterator)
{
	AvlNode** ppKeyNode;
	AvlIterator next;
	_ASSERT(pTree);
	ppKeyNode = AvlTree_findNodeByIt(&pTree->pTreeRoot, pIterator);
	if ( !ppKeyNode || !*ppKeyNode )
		return NULL;

	next = AvlTree_next(pIterator);
	if ( AvlTree_deleteByNode(pTree, ppKeyNode) )
		return next;

	return NULL;
}

BOOL AvlTree_deleteByKey(AvlTree* pTree, void* pKey)
{
	AvlNode** ppKeyNode;
	_ASSERT(pTree);
	ppKeyNode = AvlTree_findNodeByKey(pTree->fpKeyCompare, &pTree->pTreeRoot, pKey);
	if ( !ppKeyNode || !*ppKeyNode )
		return FALSE;

	return AvlTree_deleteByNode(pTree, ppKeyNode);
}

BOOL AvlTree_insert(AvlTree* pTree, void* pKey, void* pValue)
{
	AvlNode* pInsertNode;
	_ASSERT(pTree);
	pInsertNode = (AvlNode*)malloc(sizeof(AvlNode));
	pInsertNode->pLeftChild = pInsertNode->pRightChild = NULL;
	pInsertNode->pPrevNode = pInsertNode->pNextNode = NULL;
	pInsertNode->nHeight = 0;
	pInsertNode->pKey = pKey;
	pInsertNode->pValue = pValue;

	if ( !pTree->pTreeRoot )//空树
	{
		pInsertNode->pParent = NULL;
		pTree->pTreeRoot = pInsertNode;
		pTree->nCount = 1;
		return TRUE;
	}

	if ( !AvlTree_insertNode(pTree, &pTree->pTreeRoot, pInsertNode) )
	{
		free(pInsertNode);
		return FALSE;
	}
	return TRUE;
}

static AvlNode** AvlTree_findNodeByIt(AvlNode** pRoot, AvlIterator pIterator)
{
	if ( pRoot && *pRoot )
	{
		AvlNode* pNode = (AvlNode*)pIterator;
		if ( *pRoot == pNode )
		{
			return pRoot;
		}
		else
		{
			AvlNode** ppNode = AvlTree_findNodeByIt(&(*pRoot)->pLeftChild, pIterator);
			if ( ppNode )
				return ppNode;
			ppNode = AvlTree_findNodeByIt(&(*pRoot)->pRightChild, pIterator);
			if ( ppNode )
				return ppNode;
			return NULL;
		}
	}
	return NULL;
}

static AvlNode** AvlTree_findNodeByKey(keyCompare func, AvlNode** ppRoot, void* pKey)
{
	if ( ppRoot && *ppRoot )
	{
		int cmpVal = func(pKey, (*ppRoot)->pKey);
		if ( cmpVal < 0 )
		{
			return AvlTree_findNodeByKey(func, &(*ppRoot)->pLeftChild, pKey);
		}
		else if ( cmpVal > 0 )
		{
			return AvlTree_findNodeByKey(func, &(*ppRoot)->pRightChild, pKey);
		}
		else
		{
			return ppRoot;
		}
	}
	return NULL;
}

AvlIterator AvlTree_find(AvlTree* pTree, void* pKey)
{
	AvlNode** pNode;
	_ASSERT(pTree);
	pNode = AvlTree_findNodeByKey(pTree->fpKeyCompare, &pTree->pTreeRoot, pKey);
	if (!pNode)return NULL;
	return (AvlIterator)(*pNode);
}

int	AvlTree_getCount(AvlTree* pTree)
{
	_ASSERT(pTree);
	return pTree->nCount;
}

static BOOL AvlNode_check(AvlNode* pNode)
{
	if ( pNode )
	{
		if( abs(AvlNode_height(pNode->pLeftChild)-AvlNode_height(pNode->pRightChild)) >= 2 )
			return FALSE;

		if ( pNode->pLeftChild && pNode->pLeftChild->pKey >= pNode->pKey )
			return FALSE;

		if ( pNode->pRightChild && pNode->pRightChild->pKey <= pNode->pKey )
			return FALSE;
		
		return AvlNode_check(pNode->pLeftChild) && AvlNode_check(pNode->pRightChild);
	}

	return TRUE;
}

BOOL AvlTree_isValid(AvlTree* pTree)
{
	_ASSERT(pTree);
	return AvlNode_check(pTree->pTreeRoot);
}

//return the 'smallest' or the left most item
AvlIterator AvlTree_first(AvlTree *pTree)
{
	_ASSERT(pTree);
	if ( !pTree->pTreeRoot )
	{
		return NULL;
	}
	else
	{
		AvlNode* pNode = pTree->pTreeRoot;
		while ( pNode->pLeftChild )pNode = pNode->pLeftChild;
		return (AvlIterator)pNode;
	}
}

//return the 'largest' or right most item
AvlIterator AvlTree_last(AvlTree *pTree)
{
	_ASSERT(pTree);
	if ( !pTree->pTreeRoot )
	{
		return NULL;
	}
	else
	{
		AvlNode* pNode = pTree->pTreeRoot;
		while ( pNode->pRightChild )pNode = pNode->pRightChild;
		return (AvlIterator)pNode;
	}
}

//return the next item of pIterator
AvlIterator AvlTree_next(AvlIterator pIterator)
{
	_ASSERT(pIterator);
	return (AvlIterator)(((AvlNode*)pIterator)->pNextNode);
}

//return the previous item of pIterator
AvlIterator AvlTree_prev(AvlIterator pIterator)
{
	_ASSERT(pIterator);
	return (AvlIterator)(((AvlNode*)pIterator)->pPrevNode);
}