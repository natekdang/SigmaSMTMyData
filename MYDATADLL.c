#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <direct.h>


#include "global.h"
#include "log.h"
#include "bomlist.h"


typedef struct {
	char						szName[ 32 ];

	int							iLayer;

	int							X;
	int							Y;

	int							Orientation;

	int							bPlace;


} PART_DATA_TYPE;






static int						TopF2X;
static int						TopF2Y;
static int						BotF2X;
static int						BotF2Y;


static int						No_Part_Data;
static PART_DATA_TYPE *			Part_Data;





void _stdcall MYDATAAPI_Init(
	void
)
{
	No_Part_Data = 0;
	Part_Data = ( PART_DATA_TYPE * ) malloc( 65536 * sizeof( PART_DATA_TYPE ) );
}


void _stdcall MYDATAAPI_Shutdown(
	void
)
{
	free( Part_Data );
}

void _stdcall MYDATAAPI_Version(
	int *						piMajor,
	int *						piMinor,
	int *						piRelease
)
{
	* piMajor = VERSION_MAJOR;
	* piMinor = VERSION_MINOR;
	* piRelease = VERSION_RELEASE;

}


static int ToMM(
	int					Inch
)
{
	return( ( int )( ( double ) Inch / 10000.0 * 25400.0 ) );
}


static int ReadMyData(
	char *						szFileName,
	int							iLayer
)
{
	FILE *						fhd;

	char						szBuffer[ 256 ];
	char *						szPtr;

	int							length;




	// Open file
	if( ( fhd = fopen( szFileName, "rb" ) ) == NULL )
	{
		return( -1 );
	}

	// Skip 5 line
	fgets( szBuffer, sizeof( szBuffer ), fhd );
	fgets( szBuffer, sizeof( szBuffer ), fhd );
	fgets( szBuffer, sizeof( szBuffer ), fhd );
	fgets( szBuffer, sizeof( szBuffer ), fhd );

	// Do until done
	while( fgets( szBuffer, sizeof( szBuffer ), fhd ) )
	{
		length = strlen( szBuffer );
		if( length >= 2 )
		{
			szBuffer[ length - 2 ] = 0;
		}

		if( ( szBuffer[ 0 ] == 'C' ) &&
			( szBuffer[ 1 ] == '1' ) )
		{
			szPtr = strtok( szBuffer, "\t \r\n" );
			szPtr = strtok( NULL, "\t \r\n" );
			szPtr = strtok( NULL, "\t \r\n" );

			szPtr = strtok( NULL, "\t \r\n" );
			if( iLayer == 0 )
			{
				TopF2X = atoi( szPtr );
			}
			
			else
			{
				BotF2X = atoi( szPtr );
			}

			szPtr = strtok( NULL, "\t \r\n" );
			if( iLayer == 0 )
			{
				TopF2Y = atoi( szPtr );
			}
			
			else
			{
				BotF2Y = atoi( szPtr );
			}
		}

		else if( ( szBuffer[ 0 ] == 'M' ) &&
			     ( szBuffer[ 1 ] == '1' ) )
		{
			// Forget the first one M1
			szPtr = strtok( szBuffer, "\t \r\n" );

			// Layer
			Part_Data[ No_Part_Data ].iLayer = iLayer;

			// X
			szPtr = strtok( NULL, "\t \r\n" );
			Part_Data[ No_Part_Data ].X = ToMM( ( int ) atof( szPtr ) );

			// Y
			szPtr = strtok( NULL, "\t \r\n" );
			Part_Data[ No_Part_Data ].Y = ToMM( ( int ) atof( szPtr ) );

			// Orientation
			szPtr = strtok( NULL, "\t \r\n" );
			Part_Data[ No_Part_Data ].Orientation = atoi( szPtr );

		}

		else if( ( szBuffer[ 0 ] == 'M' ) &&
				 ( szBuffer[ 1 ] == '2' ) )
		{
			strcpy( Part_Data[ No_Part_Data ].szName, &szBuffer[ 2 ] );
			No_Part_Data++;
		}


	}


	fclose( fhd );

	return( 0 );
}



static int GetCom(
	char *				szName
)
{
	int					i;



	for( i = 0; i < No_Part_Data; i++ )
	{
		if( stricmp( szName, Part_Data[ i ].szName ) == 0 )
		{
			return( i );
		}

	}

	return( -1 );
}


int GetGroupNo(
	char *				szPartType
)
{


	if( strstr( szPartType, "RES-0201" ) != NULL )
	{
		return( 10 );
	}

	else if( strstr( szPartType, "CAP-0201" ) != NULL )
	{
		return( 12 );
	}

	else if( strstr( szPartType, "RES-0402" ) != NULL )
	{
		return( 20 );
	}

	else if( strstr( szPartType, "CAP-0402" ) != NULL )
	{
		return( 22 );
	}

	else if( strstr( szPartType, "0402" ) != NULL )
	{
		return( 24 );
	}

	else if( strstr( szPartType, "RES-0603" ) != NULL )
	{
		return( 30 );
	}

	else if( strstr( szPartType, "0603" ) != NULL )
	{
		return( 32 );
	}

	else if( strstr( szPartType, "RES" ) != NULL )
	{
		return( 40 );
	}

	else if( strstr( szPartType, "CAP" ) != NULL )
	{
		return( 50 );
	}

	else if( strstr( szPartType, "CASE" ) )
	{
		return( 60 );
	}


	else 
	{
		return( 100 );
	}

	return( 0 );
}




static int WriteMyData(
	BOM_LIST_TYPE *		pBomList,
	char *				szKitName,
	char *				szKitDescription,
	double				X,
	double				Y,
	char *				szTopFile,
	char *				szBotFile
)
{
	FILE *				Topfhd;
	FILE *				Botfhd;
	
	char				szBuffer[ 256 ];
	int					length;

	BOM_TYPE *			pB;
	REF_TYPE *			pR;

	int					ComIndex;
	int					i;




	// Reset the placement data
	for( i = 0; i < No_Part_Data; i++ )
	{
		Part_Data[ i ].bPlace = 0;
	}


	// Open the file
	Botfhd = fopen( szBotFile, "wb" );
	if( Botfhd == NULL )
	{
		LogWrite( "Unable to open mydata bottom file to write." );
		return( -1 );
	}

	length = sprintf( szBuffer, "F1 %s BOTTOM\r\n", szKitName );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F2 %s\r\n", szKitDescription );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F3 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F3 %d %d\r\n", ToMM( BotF2X ), ToMM( BotF2Y ) );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F4 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F5 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F6 %d %d\r\n", ( int ) X, (int ) Y );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F7 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	

	// Open top
	Topfhd = fopen( szTopFile, "wb" );
	if( Topfhd == NULL )
	{
		LogWrite( "Unable to open mydata top file to write." );
		return( -2 );
	}

	length = sprintf( szBuffer, "F1 %s TOP\r\n", szKitName );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F2 %s\r\n", szKitDescription );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F3 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F3 %d %d\r\n", ToMM( TopF2X ), ToMM( TopF2Y ) );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F4 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F5 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F6 %d %d\r\n", ( int ) X, (int ) Y );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F7 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );



	pB = pBomList->pHead;
	while( pB != NULL )
	{
		pR = pB->RefList.pHead;
		while( pR != NULL )
		{
			// Search 
			ComIndex = GetCom( pR->szRef );
			if( ComIndex >= 0 )
			{
				Part_Data[ ComIndex ].bPlace = -1;
				if( Part_Data[ ComIndex ].iLayer == 1 )
				{
					if( strlen( pB->szInternalPN ) > 2 )
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N %s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											GetGroupNo( pB->szPartType ),
											pB->szInternalPN );
					}

					else
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N L%s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											GetGroupNo( pB->szPartType ),
											pB->szLineNo );
					}

					fwrite( szBuffer, 1, length, Botfhd );

					length = sprintf( szBuffer, "F9 %s\r\n", pR->szRef );
					fwrite( szBuffer, 1, length, Botfhd );
				}

				else if( Part_Data[ ComIndex ].iLayer == 0 )
				{
					if( strlen( pB->szInternalPN ) > 2 )
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N %s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											GetGroupNo( pB->szPartType ),
											pB->szInternalPN );
					}

					else
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N L%s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											GetGroupNo( pB->szPartType ),
											pB->szLineNo );
					}

					fwrite( szBuffer, 1, length, Topfhd );

					length = sprintf( szBuffer, "F9 %s\r\n", pR->szRef );
					fwrite( szBuffer, 1, length, Topfhd );

				}
			}

			else
			{
				sprintf( szBuffer, "Item %s, <%s> - NOT ON XY DATA FILE", pB->szLineNo, pR->szRef );
				LogWrite( szBuffer );

			}

			pR = pR->pNext;
		}

		pB = pB->pNext;
	}


	fclose( Botfhd );
	fclose( Topfhd );


	return( 0 );

}






static int WriteMyDataOrder(
	BOM_LIST_TYPE *		pBomList,
	char *				szKitName,
	char *				szKitDescription,
	double				X,
	double				Y,
	char *				szTopFile,
	char *				szBotFile
)
{
	FILE *				Topfhd;
	FILE *				Botfhd;
	
	char				szBuffer[ 256 ];
	int					length;

	BOM_TYPE *			pB;
	REF_TYPE *			pR;

	int					ComIndex;
	int					i;

	int					GroupNo;




	// Reset the placement data
	for( i = 0; i < No_Part_Data; i++ )
	{
		Part_Data[ i ].bPlace = 0;
	}


	// Open the file
	Botfhd = fopen( szBotFile, "wb" );
	if( Botfhd == NULL )
	{
		LogWrite( "Unable to open mydata bottom file to write." );
		return( -1 );
	}

	length = sprintf( szBuffer, "F1 %s BOTTOM\r\n", szKitName );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F2 %s\r\n", szKitDescription );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F3 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F3 %d %d\r\n", ToMM( BotF2X ), ToMM( BotF2Y ) );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F4 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F5 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F6 %d %d\r\n", ( int ) X, (int ) Y );
	fwrite( szBuffer, 1, length, Botfhd );

	length = sprintf( szBuffer, "F7 0 0\r\n" );
	fwrite( szBuffer, 1, length, Botfhd );

	

	// Open top
	Topfhd = fopen( szTopFile, "wb" );
	if( Topfhd == NULL )
	{
		LogWrite( "Unable to open mydata top file to write." );
		return( -2 );
	}

	length = sprintf( szBuffer, "F1 %s TOP\r\n", szKitName );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F2 %s\r\n", szKitDescription );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F3 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F3 %d %d\r\n", ToMM( TopF2X ), ToMM( TopF2Y ) );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F4 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F5 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F6 %d %d\r\n", ( int ) X, (int ) Y );
	fwrite( szBuffer, 1, length, Topfhd );

	length = sprintf( szBuffer, "F7 0 0\r\n" );
	fwrite( szBuffer, 1, length, Topfhd );


	// Start highest
	GroupNo = 1;

	// Reset iGroupNo
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		pB->iGroupNo = 250;
		pB = pB->pNext;
	}


	// Search for res-0105
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "RES-0105" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for cap-0105
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "CAP-0105" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for 0105
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "0105" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}


	// Search for res-0402
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "RES-0402" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for cap-0402
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "CAP-0402" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for 0402
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "0402" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for res-0603
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "RES-0603" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for cap-0603
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "CAP-0603" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for 0603
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "0603" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}


	// Search for res
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "RES" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for cap
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "CAP" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for IND
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "IND" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}

	// Search for FER
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "FER" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}


	// Search for SOT
	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( ( pB->iGroupNo == 250 ) && ( strstr( pB->szPartType, "SOT" ) != NULL ) )
		{
			pB->iGroupNo = GroupNo++;
			if( GroupNo > 250 )
			{
				GroupNo = 250;
			}
		}

		pB = pB->pNext;
	}



	pB = pBomList->pHead;
	while( pB != NULL )
	{
		pR = pB->RefList.pHead;
		while( pR != NULL )
		{
			// Search 
			ComIndex = GetCom( pR->szRef );
			if( ComIndex >= 0 )
			{
				Part_Data[ ComIndex ].bPlace = -1;
				if( Part_Data[ ComIndex ].iLayer == 1 )
				{
					if( strlen( pB->szInternalPN ) > 2 )
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N %s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											pB->iGroupNo,
											pB->szInternalPN );
					}

					else
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N L%s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											pB->iGroupNo,
											pB->szLineNo );
					}

					fwrite( szBuffer, 1, length, Botfhd );

					length = sprintf( szBuffer, "F9 %s\r\n", pR->szRef );
					fwrite( szBuffer, 1, length, Botfhd );
				}

				else if( Part_Data[ ComIndex ].iLayer == 0 )
				{
					if( strlen( pB->szInternalPN ) > 2 )
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N %s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											pB->iGroupNo,
											pB->szInternalPN );
					}

					else
					{
						length = sprintf( szBuffer, "F8 %d %d %d %d N N L%s\r\n",
											Part_Data[ ComIndex ].X,
											Part_Data[ ComIndex ].Y,
											1000 * Part_Data[ ComIndex ].Orientation,
											pB->iGroupNo,
											pB->szLineNo );
					}

					fwrite( szBuffer, 1, length, Topfhd );

					length = sprintf( szBuffer, "F9 %s\r\n", pR->szRef );
					fwrite( szBuffer, 1, length, Topfhd );

				}
			}

			else
			{
				sprintf( szBuffer, "Item %s, <%s> - NOT ON XY DATA FILE", pB->szLineNo, pR->szRef );
				LogWrite( szBuffer );

			}

			pR = pR->pNext;
		}

		pB = pB->pNext;
	}


	fclose( Botfhd );
	fclose( Topfhd );


	return( 0 );

}






static int WriteComponentFileClear(
	BOM_LIST_TYPE *		pBomList,
	char *				szFileName
)
{
	FILE *				fhd;
	char				szBuffer[ 256 ];
	int					length;

	BOM_TYPE *			pB;


	fhd = fopen( szFileName, "wb" );
	if( fhd == NULL )
	{
		LogWrite( "Unable to open component Clear file to write." );
		return( -1 );
	}


	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( pB->RefList.iCount > 0 )
		{
			if( strlen( pB->szInternalPN ) > 2 )
			{
				length = sprintf( szBuffer, "C00 %s\r\n", pB->szInternalPN );
				fwrite( szBuffer, 1, length, fhd );
			}

			else
			{
				length = sprintf( szBuffer, "C00 L%s\r\n", pB->szLineNo );
				fwrite( szBuffer, 1, length, fhd );
			}


			if( strlen( pB->szPartType ) == 0 )
			{
				length = sprintf( szBuffer, "C01 000-SMT\r\n" );
			}

			else
			{
				length = sprintf( szBuffer, "C01 000-%s\r\n", pB->szPartType );
			}
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C02 %s, %s\r\n", pB->szPartName, pB->szPartDescription );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C03 TRUE FALSE\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C04 1 1\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C05 FALSE 0 BIN_2\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C08 8mm\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C081 TAPE_MAG\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "#\r\n" );
			fwrite( szBuffer, 1, length, fhd );
		}

		pB = pB->pNext;
	}


	fclose( fhd );

	return( 0 );
}




static int WriteComponentFile(
	BOM_LIST_TYPE *		pBomList,
	char *				szFileName
)
{
	FILE *				fhd;
	char				szBuffer[ 256 ];
	int					length;

	BOM_TYPE *			pB;


	fhd = fopen( szFileName, "wb" );
	if( fhd == NULL )
	{
		LogWrite( "Unable to open component file to write." );
		return( -1 );
	}


	pB = pBomList->pHead;
	while( pB != NULL )
	{
		if( pB->RefList.iCount > 0 )
		{
			if( strlen( pB->szInternalPN ) > 2 )
			{
				length = sprintf( szBuffer, "C00 %s\r\n", pB->szInternalPN );
				fwrite( szBuffer, 1, length, fhd );
			}

			else
			{
				length = sprintf( szBuffer, "C00 L%s\r\n", pB->szLineNo );
				fwrite( szBuffer, 1, length, fhd );
			}


			if( strlen( pB->szPartType ) == 0 )
			{
				length = sprintf( szBuffer, "C01 000-SMT\r\n" );
			}

			else
			{
				length = sprintf( szBuffer, "C01 000-%s\r\n", pB->szPartType );
			}
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C02 %s, %s\r\n", pB->szPartName, pB->szPartDescription );
			fwrite( szBuffer, 1, length, fhd );

			if( strlen( pB->szPartType ) > 0 )
			{
				length = sprintf( szBuffer, "C03 FALSE FALSE\r\n" );
			}

			else
			{
				length = sprintf( szBuffer, "C03 TRUE FALSE\r\n" );
			}

			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C04 1 1\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C05 FALSE 0 BIN_2\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C08 8mm\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "C081 TAPE_MAG\r\n" );
			fwrite( szBuffer, 1, length, fhd );

			length = sprintf( szBuffer, "#\r\n" );
			fwrite( szBuffer, 1, length, fhd );
		}

		pB = pB->pNext;
	}


	fclose( fhd );




	return( 0 );
}




static int WriteDNI( 
	char *				szFileName
)
{
	FILE *				fhd;

	char				szBuffer[ 256 ];
	int					length;

	int					i;
	int					TopDniCnt = 0;
	int					BotDniCnt = 0;

	int					bFirst;



	fhd = fopen( szFileName, "wb" );
	if( fhd == NULL )
	{
		LogWrite( "Unable to open DNI file to write." );
		return( -1 );
	}


	TopDniCnt = 0;
	for( i = 0; i < No_Part_Data; i++ )
	{
		if( ( Part_Data[ i ].bPlace == 0 ) && ( Part_Data[ i ].szName[ 0 ] != '~' ) )
		{
			if( Part_Data[ i ].iLayer == 0 )
			{
				TopDniCnt++;
			}

			else
			{
				BotDniCnt++;
			}
		}
	}


	if( TopDniCnt )
	{
		length = sprintf( szBuffer, "1\t%d\t\tTOPDNI\t\t\tTOPDNI\t", TopDniCnt );
		fwrite( szBuffer, 1, length, fhd );

		bFirst = 0;
		for( i = 0; i < No_Part_Data; i++ )
		{
			if( ( Part_Data[ i ].bPlace == 0 ) && ( Part_Data[ i ].szName[ 0 ] != '~' ) )
			{
				if( Part_Data[ i ].iLayer == 0 )
				{
					if( bFirst )
					{
						length = sprintf( szBuffer, "\t\t\t\t\t\t\t", Part_Data[ i ].szName );
						fwrite( szBuffer, 1, length, fhd );
					}

					length = sprintf( szBuffer, "%s\r\n", Part_Data[ i ].szName );
					fwrite( szBuffer, 1, length, fhd );

					bFirst = -1;
				}
			}
		}
	}


	if( BotDniCnt )
	{
		length = sprintf( szBuffer, "2\t%d\t\tBOTDNI\t\t\tBOTDNI\t", BotDniCnt );
		fwrite( szBuffer, 1, length, fhd );

		bFirst = 0;
		for( i = 0; i < No_Part_Data; i++ )
		{
			if( ( Part_Data[ i ].bPlace == 0 ) && ( Part_Data[ i ].szName[ 0 ] != '~' ) )
			{
				if( Part_Data[ i ].iLayer == 1 )
				{
					if( bFirst )
					{
						length = sprintf( szBuffer, "\t\t\t\t\t\t\t", Part_Data[ i ].szName );
						fwrite( szBuffer, 1, length, fhd );
					}

					length = sprintf( szBuffer, "%s\r\n", Part_Data[ i ].szName );
					fwrite( szBuffer, 1, length, fhd );

					bFirst = -1;
				}
			}
		}
	}

	fclose( fhd );


	return( 0 );
}





int _stdcall MYDATAAPI_Process(
	char *				szPathName,
	char *				szFileName,
	char *				szTopFileName,
	char *				szBotFileName,
	char *				szJobNumber,
	char *				szDescription

)
{
	char				szOutputName[ _MAX_PATH ];
	char				szDir[ _MAX_PATH ];
	char				szCmpFile[ _MAX_PATH ];
	char				szCmpClearFile[ _MAX_PATH ];
	char				szTopFile[ _MAX_PATH ];
	char				szBotFile[ _MAX_PATH ];
	char				szDNIFile[ _MAX_PATH ];
	char				szTopFile2[ _MAX_PATH ];
	char				szBotFile2[ _MAX_PATH ];

	BOM_LIST_TYPE 		BomList;
	

	
	strcpy( szDir, szPathName );
	strcat( szDir, "\\FILES" );
	mkdir( szDir );

	strcpy( szCmpFile, szDir );
	strcat( szCmpFile, "\\CMP.CMP" );

	strcpy( szCmpClearFile, szDir );
	strcat( szCmpClearFile, "\\CLEAR.CMP" );

	strcpy( szTopFile, szDir );
	strcat( szTopFile, "\\TOP.PCB" );

	strcpy( szBotFile, szDir );
	strcat( szBotFile, "\\BOT.PCB" );

	strcpy( szTopFile2, szDir );
	strcat( szTopFile2, "\\TOPORDER.PCB" );

	strcpy( szBotFile2, szDir );
	strcat( szBotFile2, "\\BOTORDER.PCB" );

	strcpy( szOutputName, szPathName );
	strcat( szOutputName, "\\summary.txt" );

	if( LogInit( szOutputName ) < 0 )
	{
		return( -1 );
	}

	// Read the Part data
	if( ReadMyData( szTopFileName, 0 ) )
	{
		LogWrite( "Error reading Top Mount file." );
		LogClose( );
		return( -1 );
	}

	if( ReadMyData( szBotFileName, 1 ) )
	{
		LogWrite( "Error reading Bottom Mount file." );
		LogClose( );
		return( -2 );
	}

	// Read the BOM data
	if( ReadBOM( &BomList, szFileName ) )
	{
		LogWrite( "Error reading BOM file." );
		LogClose( );
		return( -3 );
	}

	// Generate the component database
	if( WriteComponentFile( &BomList, szCmpFile ) < 0 )
	{
		LogClose( );
		return( -4 );
	}

	// Generate the component database
	if( WriteComponentFileClear( &BomList, szCmpClearFile ) < 0 )
	{
		LogClose( );
		return( -4 );
	}

	// Generate the Top and Bot mydata file
	if( WriteMyData( &BomList, szJobNumber, szDescription, 0, 0, szTopFile, szBotFile ) < 0 )
	{
		LogClose( );
		return( -5 );
	}

	// Generate the Top and Bot mydata file
	if( WriteMyDataOrder( &BomList, szJobNumber, szDescription, 0, 0, szTopFile2, szBotFile2 ) < 0 )
	{
		LogClose( );
		return( -6 );
	}

	// Write DNI
	strcpy( szDNIFile, szPathName );
	strcat( szDNIFile, "\\DNI.TXT" );
	WriteDNI( szDNIFile );

	// Close output file
	LogWrite( "Done..." );
	LogClose();

	// Free memory
	FreeBomList( &BomList );

	// Free Part Data
	No_Part_Data = 0;


	return( 0 );
}


