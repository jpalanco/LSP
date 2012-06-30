

	//Indica TODOS los valores obligatorios de un paquete enviado para que si se cumplen todos se ignore la redirección.
	char * fDataRequired[2] = { "Via: 1.1 acFP (acFP/3.0)", "User-Agent:\0xA0" };
	//Indica las cadenas a buscar para interpretar si es un protocolo HTTP
	char * fDataDetectHttp[1] = { "GET" };
	//Indica las IP's a ignorar en la redirección.
	char * fNoRedirectIP[1] = { "127.0.0.1" };
	//Indica rango de IP's a ignorar en la redirección.
	char * fNoRedirectRangeIP[3][2] = { { "192.168.1.0" , "192.168.1.255" }, { "192.168.2.0" , "192.168.2.255"}, { "192.168.3.0" , "192.168.3.255"} } ;

	