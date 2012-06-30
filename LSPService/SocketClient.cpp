#include "socketsDef.h"

//Redirecciona la llamada a la instancia enviada por parametro.
static DWORD WINAPI CallAsyncManager(LPVOID lpParam)
{
	((SocketClient *)lpParam)->StartManager();
	return 0;
}
//Inicializa el cliente.
void SocketClient::InitializeClient()
{
	DWORD threadId;
	this->_threadManager = CreateThread(NULL, NULL, &CallAsyncManager, this, NULL, &threadId);
}

//Gestor de las sockets donde se comprueban los paquetes y si es necesaria uan redirección.
void SocketClient::ManagerSockets()
{
	int bytesRecv;
	char * recvBuffer = new char[SOCKET_SIZE_BUFFER_RECV];
	memset(recvBuffer, 0, SOCKET_SIZE_BUFFER_RECV); //Limpiamos el buffer
	int err;
	FD_SET fdRead;
	FD_SET fdErr;
	TIMEVAL timeout = {0 , 50};



	while (!this->_close)
	{
		FD_ZERO(&fdRead);
		FD_ZERO(&fdErr);

		FD_SET(this->_socketLSP, &fdRead);
		FD_SET(this->_socketLSP, &fdErr);
		FD_SET(this->_socketHost, &fdRead);
		FD_SET(this->_socketHost, &fdErr);

		err = select(0, &fdRead, NULL, &fdErr, &timeout);
		if (FD_ISSET(this->_socketLSP, &fdErr))
		{						
			err = WSAGetLastError();
			dbgprint("Client LSP socket(%d) Host Socket(%d) LSP select fdErr err(%d)", this->_socketLSP, this->_socketHost, err);
			break;
		}
		if (FD_ISSET(this->_socketHost, &fdErr))
		{						
			err = WSAGetLastError();
			dbgprint("Client LSP socket(%d) Host Socket(%d) Host select fdErr err(%d)", this->_socketLSP, this->_socketHost, err);
			break;
		}
		if (err == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			dbgprint("Client LSP socket(%d) Host Socket(%d) select err(%d)", this->_socketLSP, this->_socketHost, err);
			break;
		}
		else if(err > 0) //Hay sockets para leer
		{
			if (FD_ISSET(this->_socketLSP, &fdRead))
			{
				bytesRecv = recv(this->_socketLSP, recvBuffer, SOCKET_SIZE_BUFFER_RECV, 0);
				if (bytesRecv > 0)
				{				
					if (this->_redirectChecked != true)
					{
						//SI debe redireccionarse al proxy...
						if (NeedRedirectSocket(this->_socketLSP, recvBuffer, bytesRecv, &this->_lspInfo))
						{
							err = closesocket(this->_socketHost);
							if (err == SOCKET_ERROR)
							{
								err = WSAGetLastError();
								dbgprint("REDIRECTING closing host err(%d)", err);
							}
							this->_socketHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
							if (this->_socketHost == INVALID_SOCKET)
							{			
								err = WSAGetLastError();
								dbgprint("REDIRECTING Client socket(%d) create socket err(%d)", this->_socketLSP, err);
								break;
							}
							else
							{
								sockaddr_in redirectAddr;

								redirectAddr.sin_family = AF_INET;
								redirectAddr.sin_addr.s_addr = inet_addr(fRedirectIp);
								redirectAddr.sin_port = htons(fRedirectPort);

								dbgprint("REDIRECTING Client socket host created(%d)", this->_socketHost);
								err = connect(this->_socketHost, (sockaddr*)&redirectAddr, sizeof(redirectAddr));
								if (err == SOCKET_ERROR)
								{
									err = WSAGetLastError();
									dbgprint("REDIRECTING Client socket(%d) host Socket(%d) connect err(%d)", this->_socketLSP, this->_socketHost, err);
									break;
								}
								else
								{		
									FD_ZERO(&fdRead);
									FD_ZERO(&fdErr);
								}
							}							
						}
						this->_redirectChecked = true;
					}
					dbgprint("Client LSP socket(%d) Host Socket(%d) LSP RE-SENDING %d bytes", this->_socketLSP,  this->_socketHost, bytesRecv);
					err = send(this->_socketHost, recvBuffer, bytesRecv, 0);
					if (err == SOCKET_ERROR)
					{
						err = WSAGetLastError();
						dbgprint("Client LSP socket(%d) Host Socket(%d) LSP SEND err(%d)",  this->_socketLSP,  this->_socketHost, bytesRecv, err);
						break;
					}
				}
				else if (bytesRecv == SOCKET_ERROR)
				{
					err = WSAGetLastError();
					dbgprint("Client LSP socket(%d) Host Socket(%d) LSP recv result(%d) err(%d)",  this->_socketLSP,  this->_socketHost, bytesRecv, err);
					break;
				}
				else //recv Socket closed
				{							
					dbgprint("Client LSP socket(%d) Host Socket(%d) LSP recv socket closed", this->_socketLSP,  this->_socketHost);
					break;
				}
			}
			if (FD_ISSET(this->_socketHost, &fdRead))
			{
				bytesRecv = recv(this->_socketHost, recvBuffer, SOCKET_SIZE_BUFFER_RECV, 0);
				if (bytesRecv > 0)
				{				
					dbgprint("Client LSP socket(%d) Host Socket(%d) Host RE-SENDING %d bytes", this->_socketLSP,  this->_socketHost, bytesRecv);
					err = send(this->_socketLSP, recvBuffer, bytesRecv, 0);
					if (err == SOCKET_ERROR)
					{
						err = WSAGetLastError();
						dbgprint("Client LSP socket(%d) Host Socket(%d) Host SEND err(%d)",  this->_socketLSP,  this->_socketHost, bytesRecv, err);
						break;
					}
				}
				else if (bytesRecv == SOCKET_ERROR)
				{
					err = WSAGetLastError();
					dbgprint("Client LSP socket(%d) Host Socket(%d) Host recv result(%d) err(%d)",  this->_socketLSP,  this->_socketHost, bytesRecv, err);
					break;
				}
				else //recv Socket closed
				{							
					dbgprint("Client LSP socket(%d) Host Socket(%d) Host recv socket closed", this->_socketLSP,  this->_socketHost);
					break;
				}
			}
			
		}
	}
	dbgprint("Client LSP socket(%d) Host Socket(%d) Exit While", this->_socketLSP, this->_socketHost);
	delete[] recvBuffer;
}
//Inicializa el gestor.
void SocketClient::StartManager()
{
	int bytesRecv;
	int err;

	//Esperamos a que el LSP envie la información de la socket real
	bytesRecv = recv(this->_socketLSP, (char*)&this->_lspInfo, sizeof(this->_lspInfo), 0);
	dbgprint("Client socket(%d) recv LSP INFO bytes recv(%d)", this->_socketLSP, bytesRecv);
	if (bytesRecv > 0)
	{
		this->_socketHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (this->_socketHost == INVALID_SOCKET)
		{			
			err = WSAGetLastError();
			dbgprint("Client socket(%d) create socket err(%d)", this->_socketLSP, err);
		}
		else
		{
			dbgprint("Client socket host created(%d)", this->_socketHost);
			err = connect(this->_socketHost, &this->_lspInfo.address, sizeof(this->_lspInfo.address));
			if (err == SOCKET_ERROR)
			{
				err = WSAGetLastError();
				dbgprint("Client socket(%d) host Socket(%d) connect err(%d)", this->_socketLSP, this->_socketHost, err);
			}
			else
			{
				this->ManagerSockets();
			}
		}
	}
	dbgprint("Client socket(%d) host Socket(%d) EndManager", this->_socketLSP, this->_socketHost);
	this->PrivateClose();
}
//Cierra la conexión lanzando el evento de cierre.
void SocketClient::PrivateClose()
{
	int err;
	dbgprint("Client socket(%d) host Socket(%d) PrivateClose", this->_socketLSP, this->_socketHost);
	err = closesocket(this->_socketLSP);
	if (err == SOCKET_ERROR)
	{
		err = WSAGetLastError();
		dbgprint("Client LSP socket(%d) Host Socket(%d) LSP CloseSocket err(%d)", this->_socketLSP, this->_socketHost, err);
	}
	if (this->_socketHost != NULL)
	{
		err = closesocket(this->_socketHost);
		if (err == SOCKET_ERROR)
		{
			err = WSAGetLastError();
			dbgprint("Client LSP socket(%d) Host Socket(%d) HOST CloseSocket err(%d)", this->_socketLSP, this->_socketHost, err);
		}
	}
	((SocketServer*)this->_socketServer)->EventCloseClient(this);
}
//Indica al gestor que debe cerrar la conexión y se espera a que el gestor finalize el cierre.
void SocketClient::Close()
{
	dbgprint("Client socket(%d) host Socket(%d) close", this->_socketLSP, this->_socketHost);
	this->_close = true;
	WaitForSingleObject(this->_threadManager, INFINITE);
}
//Constructor que inicialzia las variables.
SocketClient::SocketClient(SOCKET socketLSP, void * socketServer)
{
	this->_socketHost = NULL;
	this->_threadManager = NULL;
	this->_socketServer = socketServer;
	this->_close = false;
	this->_socketHost = NULL;
	this->_socketLSP = socketLSP;
	this->_redirectChecked = false;
}