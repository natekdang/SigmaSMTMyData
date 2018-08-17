#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "global.h"
#include "bomlist.h"





REF_TYPE * NewRef(
	void
)
{
	REF_TYPE *					p;


	p = ( REF_TYPE * ) malloc( sizeof( REF_TYPE ) );
	if( p == NULL )
	{
		// Log

		// Close log

	}

	else
	{
		memset( p, 0, sizeof( REF_TYPE ) );
	}

	return( p );

}


void FreeRefList(
	REF_LIST_TYPE *				pList
)
{
	REF_TYPE *					p;


	if( ( pList->pHead == NULL ) || ( pList->pTail == NULL ) )
	{
		pList->pHead = pList->pTail = NULL;
		pList->iCount = 0;
	}

	else if( pList->pHead == pList->pTail )
	{
		free( pList->pHead );

		pList->pHead = pList->pTail = NULL;
		pList->iCount = 0;
	}

	else
	{
		p = pList->pHead;
		while( p->pNext != NULL )
		{
			pList->pHead = pList->pHead->pNext;

			free( p );

			p = pList->pHead;
		}

		pList->pHead = pList->pTail = NULL;
		pList->iCount = 0;


	}

}


void InsertRefList(
	REF_LIST_TYPE *				pList,
	REF_TYPE *					p
)
{
	if( pList->pHead == NULL )
	{
		p->pNext = NULL;
		p->pPrev = NULL;
		pList->pHead = pList->pTail = p;
		pList->iCount++;

	}

	else
	{
		p->pNext = NULL;
		p->pPrev = pList->pTail;
		pList->pTail->pNext = p;
		pList->pTail = p;
		pList->iCount++;
	}

}

void ReplaceRefList(
	REF_LIST_TYPE *				pList,
	REF_TYPE *					p,
	REF_LIST_TYPE *				pNewList

)
{
	// Single element
	if( ( p->pNext == NULL ) && ( p->pPrev == NULL ) )
	{
		pList->pHead = pNewList->pHead;
		pList->pTail = pNewList->pTail;
		pList->iCount = pNewList->iCount;
		free( p );
	}

	// First
	else if( p->pPrev == NULL )
	{
		pNewList->pTail->pNext = p->pNext;
		p->pNext->pPrev = pNewList->pTail;
		pList->pHead = pNewList->pHead;
		pList->iCount += ( pNewList->iCount - 1 );
		free( p );
	}

	// Last
	else if( p->pNext == NULL )
	{
		pNewList->pHead->pPrev = p->pPrev;
		p->pPrev->pNext = pNewList->pHead;
		pList->pTail = pNewList->pTail;
		pList->iCount += ( pNewList->iCount - 1 );
		free( p );
	}

	// Middle
	else
	{
		pNewList->pHead->pPrev = p->pPrev;
		p->pPrev->pNext = pNewList->pHead;
		pNewList->pTail->pNext = p->pNext;
		p->pNext->pPrev = pNewList->pTail;
		pList->iCount += ( pNewList->iCount - 1 );
		free( p );
	}

}


void AppendRefList(
	REF_LIST_TYPE *				pList,
	REF_LIST_TYPE *				pNewList

)
{
	// Check if the append list is empty
	if( pNewList->pHead == NULL || pNewList->pTail == NULL )
	{
		return;
	}

	// Empty
	if( pList->pHead == NULL )
	{
		pList->pHead = pNewList->pHead;
		pList->pTail = pNewList->pTail;
		pList->iCount = pNewList->iCount;

	}

	else
	{
		pNewList->pHead->pPrev = pList->pTail;
		pList->pTail->pNext = pNewList->pHead;
		pList->pTail = pNewList->pTail;
		pList->iCount += pNewList->iCount;
	}

	pNewList->pHead = NULL;
	pNewList->pTail = NULL;
	pNewList->iCount = 0;


}




BOM_TYPE * NewBom(
	void
)
{
	BOM_TYPE *					p;


	p = ( BOM_TYPE * ) malloc( sizeof( BOM_TYPE ) );
	if( p == NULL )
	{
		// Log

		// Close log

	}

	else
	{
		memset( p, 0, sizeof( BOM_TYPE ) );
	}

	return( p );

}


void FreeBomList(
	BOM_LIST_TYPE *				pList
)
{
	BOM_TYPE *					p;


	if( ( pList->pHead == NULL ) || ( pList->pTail == NULL ) )
	{
		pList->pHead = pList->pTail = NULL;
	}

	else if( pList->pHead == pList->pTail )
	{
		FreeRefList( &pList->pHead->RefList );
		free( pList->pHead );
		pList->pHead = pList->pTail = NULL;
	}

	else
	{
		p = pList->pHead;
		while( p->pNext != NULL )
		{
			pList->pHead = pList->pHead->pNext;

			FreeRefList( &p->RefList );
			free( p );

			p = pList->pHead;
		}

		pList->pHead = pList->pTail = NULL;


	}

}


void InsertBomList(
	BOM_LIST_TYPE *				pList,
	BOM_TYPE *					p
)
{
	if( pList->pHead == NULL )
	{
		p->pNext = NULL;
		p->pPrev = NULL;
		pList->pHead = pList->pTail = p;
	}

	else
	{
		p->pNext = NULL;
		p->pPrev = pList->pTail;
		pList->pTail->pNext = p;
		pList->pTail = p;
	}

}


void MergeBomList(
	BOM_LIST_TYPE *				pList,
	BOM_TYPE *					p1,
	BOM_TYPE *					p2
)
{
	// Remove p2 from the list
	// Last
	if( p2->pNext == NULL )
	{
		p2->pPrev->pNext = NULL;
		pList->pTail = p2->pPrev;
	}

	// Middle
	else
	{
		p2->pPrev->pNext = p2->pNext;
		p2->pNext->pPrev = p2->pPrev;
	}
	

	// Merge the count
	p1->NoParts += p2->NoParts;

	// Merge Ref List from p2 to p1
	AppendRefList( &p1->RefList, &p2->RefList );

	// free p2
	free( p2 );
}



void RemoveBomList(
	BOM_LIST_TYPE *				pList,
	BOM_TYPE *					p
)
{
	// One element
	if( p->pNext == NULL && p->pPrev == NULL )
	{
		pList->pHead = pList->pTail = NULL;
	}

	// First
	else if( p->pPrev == NULL )
	{
		p->pNext->pPrev = NULL;
		pList->pHead = p->pNext;
	}


	// Last
	else if( p->pNext == NULL )
	{
		p->pPrev->pNext = NULL;
		pList->pTail = p->pPrev;
	}

	// Middle
	else
	{
		p->pPrev->pNext = p->pNext;
		p->pNext->pPrev = p->pPrev;
	}
	

	// free p2
	free( p );
}




static void ReadTokenTab(
	char *						szInput,
	char *						sz1,
	char *						sz2,
	char *						sz3,
	char *						sz4,
	char *						sz5,
	char *						sz6,
	char *						sz7,
	char *						sz8,
	char *						sz9
)
{
	char						szBuffer[ 32768 ];
	int							length;
	int							tab[ 9 ];
	int							i;
	int							count = 0;



	strcpy( sz1, "" );
	strcpy( sz2, "" );
	strcpy( sz3, "" );
	strcpy( sz4, "" );
	strcpy( sz5, "" );
	strcpy( sz6, "" );
	strcpy( sz7, "" );
	strcpy( sz8, "" );
	strcpy( sz9, "" );


	for( i = 0; i < 9; i++ )
	{
		tab[ 0 ] = -1;
	}

	strcpy( szBuffer, szInput );

	length = strlen( szBuffer );

	for( i = 0; i < length; i++ )
	{
		if( szBuffer[ i ] == '\t' )
		{
			tab[ count++ ] = i;
			szBuffer[ i ] = 0;
		}
	}

	strcpy( sz1, szBuffer );

	if( tab[ 0 ] >= 0 )
	{
		strcpy( sz2, &szBuffer[ tab[ 0 ] + 1 ] );
	}

	if( tab[ 1 ] >= 0 )
	{
		strcpy( sz3, &szBuffer[ tab[ 1 ] + 1 ] );
	}

	if( tab[ 2 ] >= 0 )
	{
		strcpy( sz4, &szBuffer[ tab[ 2 ] + 1 ] );
	}

	if( tab[ 3 ] >= 0 )
	{
		strcpy( sz5, &szBuffer[ tab[ 3 ] + 1 ] );
	}

	if( tab[ 4 ] >= 0 )
	{
		strcpy( sz6, &szBuffer[ tab[ 4 ] + 1 ] );
	}

	if( tab[ 5 ] >= 0 )
	{
		strcpy( sz7, &szBuffer[ tab[ 5 ] + 1 ] );
	}

	if( tab[ 6 ] >= 0 )
	{
		strcpy( sz8, &szBuffer[ tab[ 6 ] + 1 ] );
	}

	if( tab[ 7 ] >= 0 )
	{
		strcpy( sz9, &szBuffer[ tab[ 7 ] + 1 ] );
	}
}


static int ParseRefDes(
	REF_LIST_TYPE *		pRefList,
	char *				szRefDes
)
{
	char				szGlobalBuffer[ 32768 ];

	char *				Token;

	REF_TYPE *			pR;



	// Global buffer
	strcpy( szGlobalBuffer, szRefDes );
	
	// Parse
	Token = strtok( szGlobalBuffer, " ,;\"\t" );

	// Has more token
	while( Token != NULL )
	{
		if( ( pR = NewRef( ) ) == NULL )
		{
			return( -1 );
		}

		strcpy( pR->szRef, Token );
		InsertRefList( pRefList, pR );

		Token = strtok( NULL, " ,;\"\t" );
	}


	return( 0 );
}



int ReadBOM(
	BOM_LIST_TYPE *				pBomList,
	char *						szBomFileName
)
{
	FILE *						fhd;
	char						sz1[ 256 ];
	char						sz2[ 256 ];
	char						sz3[ 256 ];
	char						sz4[ 256 ];
	char						sz5[ 256 ];
	char						sz6[ 256 ];
	char						sz7[ 256 ];
	char						sz8[ 32768];
	char						sz9[ 256 ];

	BOM_TYPE *					pB;

	char						szBuffer[ 32768 ];
	int							length;



	// Init BomList
	pBomList->pHead = pBomList->pTail = NULL;


	// Open file
	if( ( fhd = fopen( szBomFileName, "rb" ) ) == NULL )
	{
		return( -1 );
	}

	while( fgets( szBuffer, sizeof( szBuffer ) - 1, fhd ) != NULL )
	{ 
		length = strlen( szBuffer ) - 2;
		szBuffer[ length ] = 0;

		ReadTokenTab( szBuffer, sz1, sz2, sz3, sz4, sz5, sz6, sz7, sz8, sz9 );

		if( strlen( sz1 ) > 0 )
		{
			pB = NewBom( );

			strcpy( pB->szLineNo, sz1 );
			pB->NoParts = atoi( sz2 );
			strcpy( pB->szInternalPN, sz3 );
			strcpy( pB->szPartName, sz4 );
			strcpy( pB->sz5, sz5 );
			strcpy( pB->sz6, sz6 );
			strcpy( pB->szPartDescription, sz7 );
			strcpy( pB->szPartType, sz9 );

			// Parse the reference designation
			if( ParseRefDes( &pB->RefList, sz8 ) < 0 )
			{
				return( -1 );
			}

			InsertBomList( pBomList, pB );
		}

		else if( strlen( sz8 ) )
		{
			// Parse the reference designation
			if( ParseRefDes( &pB->RefList, sz8 ) < 0 )
			{
				return( -1 );
			}
		}
	}


	return( 0 );
}


