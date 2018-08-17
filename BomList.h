
#ifndef __H_BOMLIST__
#define __H_BOMLIST__



typedef struct _REF_STRUCT {

	struct _REF_STRUCT *		pNext;
	struct _REF_STRUCT *		pPrev;

	char						szRef[ 64 ];

} REF_TYPE;


typedef struct {
	REF_TYPE *					pHead;
	REF_TYPE *					pTail;

	int							iCount;

} REF_LIST_TYPE;



typedef struct _BOM_STRUCT {

	struct _BOM_STRUCT *		pNext;
	struct _BOM_STRUCT *		pPrev;

	char						szLineNo[ 16 ];

	int							NoParts;

	char						szInternalPN[ 64 ];

	char						szPartName[ 64 ];

	char						sz5[ 64 ];

	char						sz6[ 64 ];

	char						szPartDescription[ 256 ];

	char						szPartType[ 32 ];

	int							iGroupNo;

	REF_LIST_TYPE				RefList;


} BOM_TYPE;



typedef struct {
	BOM_TYPE *					pHead;
	BOM_TYPE *					pTail;

} BOM_LIST_TYPE;






REF_TYPE * NewRef(
	void
);

void FreeRefList(
	REF_LIST_TYPE *				pList
);

void InsertRefList(
	REF_LIST_TYPE *				pList,
	REF_TYPE *					p
);


void ReplaceRefList(
	REF_LIST_TYPE *				pList,
	REF_TYPE *					p,
	REF_LIST_TYPE *				pNewList

);


void AppendRefList(
	REF_LIST_TYPE *				pList,
	REF_LIST_TYPE *				pNewList

);

BOM_TYPE * NewBom(
	void
);

void FreeBomList(
	BOM_LIST_TYPE *				pList
);

void InsertBomList(
	BOM_LIST_TYPE *				pList,
	BOM_TYPE *					p
);

void MergeBomList(
	BOM_LIST_TYPE *				pList,
	BOM_TYPE *					p1,
	BOM_TYPE *					p2
);


void RemoveBomList(
	BOM_LIST_TYPE *				pList,
	BOM_TYPE *					p
);


int ReadBOM(
	BOM_LIST_TYPE *				pBomList,
	char *						szFileName
);


#endif
