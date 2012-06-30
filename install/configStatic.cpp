

#ifndef CONFIG_STATIC_INSTALL
#define CONFIG_STATIC_INSTALL 1

#ifndef STATIC_COMMAND_LSP



/*
	Obtiene el directorio de instalación con su fichero concatenados desde el registro de windows
	Parametros:
		- fileName: Nombre del fichero a concatenar
	Valor devuelto:
		Devuelve un char* de 10240 de longitud alojados en memoria (se debe liberar una vez deja de usarse con free())
*/
	char * GetPathInstallation(char * fileName);



	#ifdef _WIN64
	//Contiene el númro de parámetros utilizados.
	#define STATIC_COMMAND_LSP_LEN 5
	//Contiene el comando a utilizar en el instalador.
	
	#define STATIC_COMMAND_LSP {"null", "-i" ,"-a", "-d" , GetPathInstallation("nonifslsp64.dll")}

	#else
	//Contiene el númro de parámetros utilizados.
	#define STATIC_COMMAND_LSP_LEN 5
	//Contiene el comando a utilizar en el instalador.
	#define STATIC_COMMAND_LSP {"null", "-i" ,"-a", "-d" , GetPathInstallation("nonifslsp32.dll")}

	#endif
#endif
//Contiene el nombre del LSP que se asignara.
#define DEFAULT_LSP_NAME "drainware-LSP"


#endif