//Puerto de escucha del serivcio donde se conectara el LSP
#define fServicePort 6565
//Ip del servicio donde estara escuchando para conectarse el LSP
#define fServiceAddr INADDR_LOOPBACK
//Lista de ips a ignorar en la redirección del LSP
char * fNoRedirectIP[1] = { "127.0.0.1" };
//Nombre del proceso del servicio del LSP el cual no se redireccionara la conexión
char * fNoRedirectProgram[1] = { "LSPService.exe" };