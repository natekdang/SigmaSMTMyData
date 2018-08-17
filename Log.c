#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "log.h"




static FILE *					LogFhd = NULL;





int LogInit(
	char *						szLogFileName
)
{
	// Open the message file
	LogFhd = fopen( szLogFileName, "wb" );

	if( LogFhd == NULL )
	{
		return( -1 );
	}

	else
	{
		return( 0 );
	}
}



void LogClose(
	void
)
{
	if( LogFhd )
	{
		fclose( LogFhd );

		LogFhd = NULL;
	}
}



void LogWrite(
	char *						szMsg
)
{
	if( LogFhd == NULL )
	{
		return;
	}


	fprintf( LogFhd, "%s\r\n", szMsg );

}







