/*
 * sockets.c
 *
 *  Created on: 21/10/2017
 *      Author: utnso
 */

#include "sockets.h"

/* Crea un socket servidor, lo setea para escuchar a varios al mismo tiempo y lo pone a escuchar */

int crearSocketServidor(char* puerto){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, puerto, &hints, &serverInfo);

	int listenningSocket;
	if((listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0) {
		perror("Error al crear socket");
		exit(EXIT_FAILURE);
	};

	int enable = 1;
	if(setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
		perror("Error al setear socket");
		exit(EXIT_FAILURE);
	};

	if(bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen)== -1){
		perror("Error al bindear socket");
		exit(EXIT_FAILURE);
	};

	freeaddrinfo(serverInfo);

	if (listen(listenningSocket, BACKLOG) == -1) {
		perror("Error al poner a escuchar al socket");
		exit(EXIT_FAILURE);
	}

	return listenningSocket;
}

/* Crea un socket cliente y lo conecta al server */

int crearSocketCliente(char* ip, char* puerto){

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ip, puerto, &hints, &serverInfo);

	int serverSocket;
	if((serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol)) < 0) {
		perror("Error al crear socket");
		exit(EXIT_FAILURE);
	};


	if(connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen) == -1){
		perror("Error al conectar al server");
		exit(EXIT_FAILURE);
	};

	freeaddrinfo(serverInfo);

	return serverSocket;
}

/* Acepta una nueva conexión y devuelve el nuevo socket */

int aceptarConexion(int socketServidor) {
	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(socketServidor, (struct sockaddr *) &addr, &addrlen);
	if (socketCliente < 0) {

		perror("Error al aceptar cliente");
		return -1;
	}

	return socketCliente;
}

/* Devuelve la IP de un socket */

char* getIpSocket(int fd){
	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	int res = getpeername(fd, (struct sockaddr *)&addr, &addr_size);
	if(res == -1){
		return NULL;
	}
	char ipNodo[20];
	strcpy(ipNodo, inet_ntoa(addr.sin_addr));
	return strdup(ipNodo);
}

/* Cierra un socket */

void cerrarSocket(int socket){
	close(socket);
}

/* Función para manejar múltiples conexiones. Recibe un puerto y una función "procesar_mensaje" que recibe como argumento un socket */


void multiplexar(char * puerto, int (*procesar_mensaje)(int)) {
	fd_set master;
	fd_set read_fds;
	int fdmax;
	int listener;
	int i;
	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	listener = crearSocketServidor(puerto);
	FD_SET(listener, &master);
	fdmax = listener;
	for (;;) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listener) {
					int newfd = aceptarConexion(listener);
					FD_SET(newfd, &master);
					if (newfd > fdmax) {
						fdmax = newfd;
					}
					//printf("Nueva conexion con fd: %d\n",newfd);
				} else {

					if (procesar_mensaje(i) <= 0) {
						cerrarSocket(i);
						FD_CLR(i, &master);
					}
				}
			}
		}
	}
}

int enviarHeader(int fd, int32_t header){
	return enviarInt(fd, header);
}

int32_t recibirHeader(int fd){
	return recibirInt(fd);
}

int enviarString(int fd ,char* mensaje){
	int total =0;
	int tamanio = string_length(mensaje);
	int pendiente = tamanio;
	char* msj = string_duplicate(mensaje);

	int tamanioEnviado = enviarInt(fd, tamanio);
			if (tamanioEnviado < 0) {
				perror("Error al enviar tamaño");
				free(msj);
				return -1;
			}

	while (total < pendiente) {

		int enviado = send(fd, msj, tamanio, MSG_NOSIGNAL);
		if (enviado < 0) {
			free(msj);
			return -1;
		}
		total += enviado;
		pendiente -= enviado;
	}
    free(msj);
    return total;
}

char* recibirString(int fd) {
	int tamanio = recibirInt(fd);
	char* buffer;

	if (tamanio < 0) {
		perror("Error al recibir tamaño");
		return NULL;
	}
	if(tamanio == 0 ){
		return NULL;
	}

	buffer = malloc(tamanio+1);

	if (recv(fd, buffer,tamanio, MSG_WAITALL) < 0) {
		perror("Error al recibir string");
		free(buffer);
		return NULL;
	}

	buffer[tamanio] = '\0';

	return buffer;
}

int enviarInt(int fd, int32_t numero){
	return (send(fd, &numero, sizeof(int32_t),MSG_NOSIGNAL));
}

int32_t recibirInt(int fd){
	int32_t numero = 0;
	if(recv(fd, &numero, sizeof(int32_t),MSG_WAITALL) < 0){
		perror("Error al recibir numero");
		return -1;
	}
	return numero;
}
