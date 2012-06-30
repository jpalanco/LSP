

#include <stdio.h>
#include <stdlib.h>
#include "lspdef.h"
//Comentar si se quiere log en varios ficheros
#define LOG_SINGLE_FILE

#ifdef _DEBUG
//Guarda un log de un paquete recibido
void logBufferRecv(char * strName, SOCKET s, SOCK_INFO * socketContext, LPWSABUF buff, DWORD count, DWORD size)
{
	if (count > 0 && size > 0 && buff != NULL)
	{		
		char fileName[500];
#ifdef LOG_SINGLE_FILE
		sprintf(fileName, "C:\\logLspHex%s-%d.log", strName, s);
#else
		sprintf(fileName, "C:\\logLspHex%s.log", strName);
#endif
		FILE * fLog = fopen(fileName, "a+");
		if (fLog != NULL) 
		{
			fwrite(buff->buf, sizeof(char), size, fLog);
			fclose(fLog);
		}
		
		dbgprint("%s (%d)(%d): BuffCount[%d] BuffCounTotal[%d]", strName, s, socketContext->ProviderSocket, count, size);
	}
}

//Guarda un log de un buffer de datos
void logBuffer(char * strName, SOCKET s, SOCK_INFO* socketContext, LPWSABUF buff, DWORD count)
{
	if (count > 0) 
	{
		DWORD countTotal = 0;

		for (DWORD b = 0; b < count; b++)
		{
			countTotal += buff[b].len;
		}
		if (countTotal > 0)
		{

			char* buffTxt = new char[countTotal];

			int txtInc = 0;
			for (DWORD b = 0; b < count; b++)
			{
				for (ULONG c = 0; c < buff[b].len; c++)
				{
					buffTxt[txtInc] = buff[b].buf[c];
					txtInc++;
				}
			}
			char fileName[500];
	#ifdef LOG_SINGLE_FILE
			sprintf(fileName, "C:\\logLspHex%s-%d.log", strName, s);
	#else
			sprintf(fileName, "C:\\logLspHex%s.log", strName);
	#endif
			FILE * fLog = fopen(fileName, "a+");
			if (fLog != NULL) 
			{
				fwrite(buffTxt, sizeof(char), countTotal, fLog);
				fclose(fLog);
			}
		
			dbgprint("%s (%d)(%d): BuffCount[%d] BuffCounTotal[%d]", strName, s, socketContext->ProviderSocket, count, countTotal);

			delete[] buffTxt;
		}
		else
		{
			dbgprint("logBuffer FAILED!: no buffer data (count is %d)" , countTotal);
		}
		
	}

}
#else
#define logBufferRecv
#define logBuffer
#endif
