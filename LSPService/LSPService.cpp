//Gestiona el servicio

#include "socketsDef.h"
#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

#define SVCNAME TEXT("LSPService")  //Contiene el nombre del servicio

#define SVC_ERROR ((DWORD)0xC0020001L)	//Contiene la definicion de erro del servicio.
SERVICE_STATUS          gSvcStatus;		//Contiene el estado del servicio.
SERVICE_STATUS_HANDLE   gSvcStatusHandle;  //Contiene el identificador del estado del servicio
HANDLE                  ghSvcStopEvent = NULL; //Contiene el identificador del servicio.

//Prototipos

VOID WINAPI SvcCtrlHandler( DWORD ); 
VOID WINAPI SvcMain( DWORD, LPTSTR * ); 

VOID ReportSvcStatus( DWORD, DWORD, DWORD );
VOID SvcInit( DWORD, LPTSTR * ); 
VOID SvcReportEvent( LPTSTR );
VOID SvcInstall();



//Punto de inicio de la aplicación.
void __cdecl _tmain(int argc, TCHAR *argv[]) 
{ 
	//Si el parametro es "install" instalamos el serivcio.
    if( lstrcmpi( argv[1], TEXT("install")) == 0 )
    {
        SvcInstall();
        return;
    }

    SERVICE_TABLE_ENTRY DispatchTable[] = 
    { 
        { SVCNAME, (LPSERVICE_MAIN_FUNCTION) SvcMain }, 
        { NULL, NULL } 
    }; 

    if (!StartServiceCtrlDispatcher( DispatchTable )) 
    { 
        SvcReportEvent(TEXT("StartServiceCtrlDispatcher")); 
    } 
} 

//Instala el servicio.
VOID SvcInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;
    TCHAR szPath[MAX_PATH];

    if( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
    {
        dbgprint("Cannot install service (%d)\n", GetLastError());
        return;
    }
 
    schSCManager = OpenSCManager( 
        NULL,                    
        NULL,                     
        SC_MANAGER_ALL_ACCESS); 
 
    if (NULL == schSCManager) 
    {
        dbgprint("OpenSCManager failed (%d)\n", GetLastError());
        return;
    }

    schService = CreateService( 
        schSCManager,             
        SVCNAME,                  
        SVCNAME,                  
        SERVICE_ALL_ACCESS,     
        SERVICE_WIN32_OWN_PROCESS, 
        SERVICE_AUTO_START,     
        SERVICE_ERROR_NORMAL,    
        szPath,                    
        NULL,                  
        NULL,                    
        NULL,                    
        NULL,                    
        NULL);                    
 
    if (schService == NULL) 
    {
        printf("CreateService failed (%d)\n", GetLastError()); 
        CloseServiceHandle(schSCManager);
        return;
    }
    else printf("Service installed successfully\n"); 

    CloseServiceHandle(schService); 
    CloseServiceHandle(schSCManager);
}

//Punto de inicio del servicio.
VOID WINAPI SvcMain( DWORD dwArgc, LPTSTR *lpszArgv )
{
    gSvcStatusHandle = RegisterServiceCtrlHandler( 
        SVCNAME, 
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
    { 
        SvcReportEvent(TEXT("RegisterServiceCtrlHandler")); 
        return; 
    } 
	
    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; 
    gSvcStatus.dwServiceSpecificExitCode = 0;    

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );


    SvcInit( dwArgc, lpszArgv );
}

//Inicio del servicio
VOID SvcInit( DWORD dwArgc, LPTSTR *lpszArgv)
{
    ghSvcStopEvent = CreateEvent(
                         NULL,   
                         TRUE,  
                         FALSE,  
                         NULL);  

    if ( ghSvcStopEvent == NULL)
    {
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }

    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

	SocketStartup();
	SocketServer  * server = new SocketServer(fListenPort);
	server->Listen();

	//Esperamos al cierre del servicio
	while (WaitForSingleObject(ghSvcStopEvent, INFINITE) == WAIT_TIMEOUT) {}
	SocketCleanup();
    ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
    return;
}

//Reporta el estado del servicio
VOID ReportSvcStatus( DWORD dwCurrentState,
                      DWORD dwWin32ExitCode,
                      DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;


    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
}

//Controla el servicio
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   switch(dwCtrl) 
   {  
      case SERVICE_CONTROL_STOP: 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
         SetEvent(ghSvcStopEvent);
         ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);         
         return;

      case SERVICE_CONTROL_INTERROGATE: 
         break; 
 
      default: 
         break;
   } 
   
}


//Registra un mensaje en el log del sistema
VOID SvcReportEvent(LPTSTR szFunction) 
{ 
    HANDLE hEventSource;
    LPCTSTR lpszStrings[2];
    TCHAR Buffer[80];

    hEventSource = RegisterEventSource(NULL, SVCNAME);

    if( NULL != hEventSource )
    {
        StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

        lpszStrings[0] = SVCNAME;
        lpszStrings[1] = Buffer;

        ReportEvent(hEventSource,        
                    EVENTLOG_ERROR_TYPE,
                    0,                  
                    SVC_ERROR,          
                    NULL,              
                    2,                   
                    0,                   
                    lpszStrings,         
                    NULL);              

        DeregisterEventSource(hEventSource);
    }
}
