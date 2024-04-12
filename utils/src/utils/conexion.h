#ifndef CONEXION_H_
#define CONEXION_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<assert.h>
#include<commons/log.h>
#include<commons/collections/list.h>

typedef enum{
	MENSAJE,
	PAQUETE
}op_code;

typedef struct{
	int size;
	void* stream;
} t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

//CLIENTE
int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

// SERVIDOR
extern t_log* logger;

void* recibir_buffer(int*, int);

int servidor(char* puerto)
{
	// Inicializar el servidor utilizando el puerto proporcionado
    printf("Inicializando servidor en el puerto %s\n", puerto);

    // Lógica de negocio del servidor (por ejemplo, aceptar conexiones y procesar solicitudes)

    return 0; // Devolver un código de estado exitoso
};
void iterator(char*);

int iniciar_servidor(int puerto);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);

#endif