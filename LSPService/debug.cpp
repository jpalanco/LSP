#include "socketsDef.h"

//Si esta definido el modo debug se activara el log
#ifdef _DEBUG
//Guarda en "C:\logLspService.log"
void dbgprint(char *format, ...)
{
    static  DWORD pid=0;
    va_list vl;
    char    dbgbuf1[2048],
            dbgbuf2[2048];

    
    if ( 0 == pid )
    {
        pid = GetCurrentProcessId();
    }

    va_start(vl, format);
    wvsprintf(dbgbuf1, format, vl);
    wsprintf(dbgbuf2, TEXT("[%d] %lu: %s\r\n"), GetTickCount(), pid, dbgbuf1);
    va_end(vl);


	FILE * fLog; 
	fLog = fopen("C:\\logLspService.log", "a+");
	if (fLog != NULL)
	{
		fwrite(dbgbuf2, sizeof(char), strlen(dbgbuf2), fLog);
		fclose(fLog);
	}

}
#endif