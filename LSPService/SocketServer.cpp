#include "socketsDef.h"
//Contiene el número máximo de conexiones en cola
#define SOCKETSERVER_MAX_CONNECTION_QUEUE 100

//Redirecciona la llamada a la instancia enviada por parametro.
static DWORD WINAPI CallAsync(LPVOID lpParam)
{
	((SocketServer*)lpParam)->WaitForConnectionAsync();
	return 0;
}
//Constructor que inicializa las variables y crea la socket de escucha.
SocketServer::SocketServer(unsigned short port)
{
	InitializeCriticalSection(&this->_crit);
	this->_close = false;
	int err;
	this->_port = port;
	this->_socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->_socketId == INVALID_SOCKET)
	{
		err = WSAGetLastError();
	}
}
//Pone en escucha la socket.
void SocketServer::Listen()
{
	DWORD threadId;
	this->_threadListen = CreateThread(NULL, NULL, &CallAsync, this, NULL, &threadId);
}

//Gestiona la recepcion de conexiones entrantes.
void SocketServer::WaitForConnectionAsync()
{
	int err;
	SOCKADDR_IN serverInfo;
	memset(&serverInfo, 0, sizeof(serverInfo));
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	serverInfo.sin_port = htons(this->_port);
	
	err = bind(this->_socketId, (LPSOCKADDR)&serverInfo, sizeof(serverInfo));
	if (err == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		dbgprint("Server socket bind error(%d)", err);
	}
	else
	{
		err = listen(this->_socketId, SOCKETSERVER_MAX_CONNECTION_QUEUE);

		if (err == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			dbgprint("Server socket listen error(%d)", err);
		}
		else
			{
			SOCKET newClient;
			SocketClient * newSocketClient;
			while (!this->_close)
			{
				newClient = accept(this->_socketId, NULL , NULL);
				dbgprint("Server socket(%d) accept", newClient);
				if (newClient == INVALID_SOCKET)
				{
					err = WSAGetLastError();
					dbgprint("Server socket accept error(%d)", err);
				}
				else
				{					
					EnterCriticalSection(&this->_crit);

					newSocketClient = new SocketClient(newClient, this);
					this->_listClients.push_back(newSocketClient);					
					dbgprint("Server clients current alive(%d)", this->_listClients.size());
					LeaveCriticalSection(&this->_crit);

					newSocketClient->InitializeClient();
				}
			}
			dbgprint("Server exit while");
		}
	}
	

}
//Evento lanzado por las instancias de SocketClient que gestiona la eliminacion de la conexion de la lista
void SocketServer::EventCloseClient(void * socketClient)
{
	if (!this->_close)
	{
		dbgprint("Server REMOVING Client");
		EnterCriticalSection(&this->_crit);
		_listClients.remove(socketClient);
		dbgprint("Server clients current alive(%d)", this->_listClients.size());
		delete socketClient;
		LeaveCriticalSection(&this->_crit);
	}
}
//Destructor que cierra las conexiones y elimina las memorias reservadas
SocketServer::~SocketServer()
{
	EnterCriticalSection(&this->_crit);
	dbgprint("Server destructor");
	this->_close = true;
	std::list<void*>::iterator it;
	for (it=this->_listClients.begin(); it!=this->_listClients.end(); it++)
	{
		((SocketClient*)(*it))->Close();
		delete (*it);
	}
	WaitForSingleObject(this->_threadListen, INFINITE);
	closesocket(this->_socketId);
	LeaveCriticalSection(&this->_crit);
}


