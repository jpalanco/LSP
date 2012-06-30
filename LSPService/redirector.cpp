#include "socketsDef.h"
#include "configStatic.cpp"

//Transforma una ip a un numero y la gira.
unsigned long reverseIp(char * ip)
{
	unsigned long uIp = inet_addr(ip);
	unsigned long reverseNum = 0;
	 
	 for (int s = sizeof(uIp); s > 0; s--)
	 {
			reverseNum *= 0x100;
			if (uIp > 0)
			{
				reverseNum += (uIp % 0x100);
				uIp /= 0x100;
			}
	 }

	return reverseNum;
}
//Compara dos cadenas.
bool charCompare(char * data, DWORD dataCount, char * findText, DWORD findCount)
{
	bool exist = false;
	try 
	{
		for (DWORD c = 0; c < dataCount && (dataCount - c) > findCount; c++)
		{
			for (DWORD f = 0; f < findCount; f++)
			{
				if (data[f + c] != findText[f])
				{
					break;
				}
				else if(f == (findCount - 1))
				{
					exist = true;
					break;
				}
			}
			if (exist == true)
			{
				break;
			}	
		}
	}
	catch (char * ex)
	{
		dbgprint("cahrCompare EXCEPTION (%d): %s", GetLastError(), ex);
	}
	return exist;
}
//Comprueba si es necesario redireccionar la conexión.
bool NeedRedirectSocket(SOCKET socketLsp, char * buffer, int bufferCount, LSP_INFO_SEND * lspInfo)
{
	bool needRedirect = true;
	bool haveAllRequire = true;
	try
	{
		char * ip = inet_ntoa(((struct sockaddr_in*)&lspInfo->address)->sin_addr);
		dbgprint("Checking (%d) redirect socket for ip %s", socketLsp, ip);
		//Comprobamos si la IP esta en el rango de ips a ignorar.
		if (fNoRedirectRangeIP != NULL && needRedirect  == true)
		{
			int dataSize = sizeof(fNoRedirectRangeIP) / sizeof(*fNoRedirectRangeIP);
			for (int i = 0; i < dataSize; i++)
			{
				if (fNoRedirectRangeIP[i] != NULL)
				{
					if (reverseIp(ip) >= reverseIp(fNoRedirectRangeIP[i][0]) && reverseIp(ip) <= reverseIp(fNoRedirectRangeIP[i][1]))
					{
						dbgprint("Checking (%d)(%s) Ignored by range filter ip index(%d)", socketLsp, ip, i);
						needRedirect = false;
						break;
					}
				}					
			}				
		}

		//Comprobamos si la IP esta en la lista blanca de ips a ignorar.
		if (fNoRedirectIP != NULL && needRedirect  == true)
		{
			int dataSize = sizeof(fNoRedirectIP) / sizeof(*fNoRedirectIP);
			for (int i = 0; i < dataSize; i++)
			{
				if (fNoRedirectIP[i] != NULL)
				{
					if (inet_addr(fNoRedirectIP[i]) == inet_addr(ip))
					{
						dbgprint("Checking (%d)(%s) Ignored by filter ip", socketLsp, ip);
						needRedirect = false;
						break;
					}
				}					
			}				
		}

		//Comprobamos si el paquete contiene TODOS los campos necesarios para identificar que es un protocolo HTTP
		haveAllRequire = true;
		if (fDataDetectHttp != NULL && needRedirect  == true)
		{
			int dataSize = sizeof(fDataDetectHttp) / sizeof(*fDataDetectHttp);
			for (int i = 0; i < dataSize; i++)
			{
				if (fDataDetectHttp[i] != NULL)
				{
					haveAllRequire &= charCompare(buffer, bufferCount, fDataDetectHttp[i], strlen(fDataDetectHttp[i]) - 1);
				}
				if (haveAllRequire == false)
				{
					needRedirect &= haveAllRequire;
					break;
				}
			}				
		}
		//Comprobamos si el paquete contiene TODOS los indicios de que es un paquete especial del último proxy y debe ignorarse la redirección
		haveAllRequire = true;
		if (fDataRequired != NULL && needRedirect  == true)
		{
			int dataSize = sizeof(fDataRequired) / sizeof(*fDataRequired);
			for (int i = 0; i < dataSize; i++)
			{
				if (fDataRequired[i] != NULL)
				{
					haveAllRequire &= charCompare(buffer, bufferCount, fDataRequired[i], strlen(fDataRequired[i]) - 1);
				}
			}				
			if (haveAllRequire == true)
			{
				dbgprint("Checking (%d)(%s) Ignored by data required", socketLsp, ip);
				//Al contener todos los indicios se evita la redirección.
				needRedirect = false;
			}
		}		
	}
	catch (char* ex)
	{
		dbgprint("NeedRedirectSocket EXCEPTION (%d): %s", GetLastError(), ex);
	}
	return needRedirect;
}
