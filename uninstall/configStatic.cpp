
#ifndef CONFIG_STATIC_UNINSTALL
#define CONFIG_STATIC_UNINSTALL 1

//Contiene el nombre del LSP que se asignara.
#define DEFAULT_LSP_NAME "drainware-LSP"

#ifdef _WIN64
//Contiene el númro de parámetros utilizados.
#define STATIC_COMMAND_LSP_LEN 2
//Contiene el comando a utilizar en el instalador.
#define STATIC_COMMAND_LSP {"null", "-f"}


#else

//Contiene el númro de parámetros utilizados.
#define STATIC_COMMAND_LSP_LEN 2
//Contiene el comando a utilizar en el instalador.
#define STATIC_COMMAND_LSP {"null", "-f"}

#endif

#endif