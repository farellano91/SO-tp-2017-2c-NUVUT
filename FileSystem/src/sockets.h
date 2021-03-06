/*
 * sockets.h
 *
 *  Created on: 21/10/2017
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <commons/string.h>

#define BACKLOG 100


/* Funciones para sockets */

int crearSocketServidor(char* puerto);
int crearSocketCliente(char* ip, char* puerto);
int aceptarConexion(int socketServidor);
char* getIpSocket(int fd);
void cerrarSocket(int socket);
void multiplexar(char * puerto, int (*procesar_mensaje)(int));

/* Funciones para mandar/recibir mensajes */

int enviarHeader(int fd, int32_t header);
int32_t recibirHeader(int fd);
int enviarString(int fd ,char* mensaje);
char* recibirString(int fd);
int enviarInt(int fd, int32_t numero);
int32_t recibirInt(int fd);

#endif /* SOCKETS_H_ */
