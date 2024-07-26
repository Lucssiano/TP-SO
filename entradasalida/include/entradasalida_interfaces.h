#ifndef IO_INTERFACE_H
#define IO_INTERFACE_H

#include "./entradasalida_gestor.h"

void interfaz_generica(int cant_unidades_trabajo);
// void interfaz_stdin(char* registro_direccion, char* registro_tamanio);
void interfaz_stdin(t_list *lista_direcciones, int pid);
// void enviar_texto_a_memoria(char* direccion_fisica, char* texto, int tamanio);
// void interfaz_stdout(char* registro_direccion, char* registro_tamanio);
void interfaz_stdout(t_list* lista_direcciones, int cant_direcciones, int pid);
// void solicitar_datos_a_memoria(char* direccion_fisica, int tamanio);
// char* recibir_datos_de_memoria(int tamanio);
void interfaz_fs_create(char* filename, int pid);
void interfaz_fs_delete(char* filename, int pid);

#endif