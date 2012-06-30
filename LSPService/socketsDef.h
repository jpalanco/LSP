#ifndef DEFINE_DEF
#define DEFINE_DEF 1
#include <WinSock2.h>
#include <list>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "ws2_32.lib")

//Estructura que contiene la información que envia el LSP en cada conexión.
typedef struct _LSP_INFO_SEND
{
	sockaddr address;   //Dirección real donde iva a conectarse la conexión
	DWORD processId;	//Proceso del que procede la conexión.
} LSP_INFO_SEND;

//Contiene el buffer establecido para las sockets
#define SOCKET_SIZE_BUFFER_RECV 8064 

//Inicializa las sockets
static void SocketStartup()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2), &wsaData);
}
//Limpia las conexiones
static void SocketCleanup()
{
	WSACleanup();
}

#ifdef _DEBUG

//Guarda un log.
void 
dbgprint(
        char *format,
        ...
        );

#else

#define dbgprint

#endif

//Clase gestiona las conexiones cliente (salen del Servicio hacia fuera)
class SocketClient
{

	public:
	SocketClient(SOCKET socketLSP, void * SocketServer); //Constructor que inicia la clase
	void Close();
	void InitializeClient();
	void StartManager();
	void ManagerSockets();

	private:
	SOCKET _socketLSP; //Contiene la socket del LSP
	SOCKET _socketHost; //Contiene la socket del Host donde se conecta
	LSP_INFO_SEND _lspInfo; //Contiene la dirección a donde debe conectarse la socket
	bool _redirectChecked; //Contiene si se ha comprobado la redirección de la socket
	bool _close; //Contiene si debe cerrarse la conexion
	HANDLE _threadManager; //Hilo encargado de gestionar la conexion y los paquetes del LSP
	void * _socketServer;
	void PrivateClose();

};
//Gestiona las conexiones del servidor (escucha en un puerto y acepta las conexiones)
class SocketServer
{
public:
	void WaitForConnectionAsync();
	SocketServer(unsigned short port);
	~SocketServer();
	void Listen();
	void EventCloseClient(void * socketClient);
private:
	
	unsigned short _port;
	SOCKET _socketId;
	CRITICAL_SECTION _crit;	
	HANDLE _threadListen;
	bool _close;
	std::list<void*> _listClients;
};

bool NeedRedirectSocket(SOCKET socketLsp, char * buffer, int bufferCount, LSP_INFO_SEND * lspInfo);
bool charCompare(char * data, DWORD dataCount, char * findText, DWORD findCount);
unsigned long reverseIp(char * ip);


//Indica el puerto destino de la redirección
#define fRedirectPort 8080
//Indica la IP destino de la redirección
#define fRedirectIp "127.0.0.1"
//Indica el puerto de escucha del servicio
#define fListenPort 6565

#endif


