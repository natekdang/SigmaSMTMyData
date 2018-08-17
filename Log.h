
#ifndef __H_LOG__
#define __H_LOG__



int LogInit(
	char *						szFileName
);


void LogClose(
	void
);



void LogWrite(
	char *						szMsg
);





#endif
