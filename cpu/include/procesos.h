#ifndef PROCESOS_H_
#define PROCESOS_H_


#include "cpu_gestor.h"



void inicializarPCBAleatorio(t_pcb *pcb);
void copiarContexto(t_pcb pcb_recibido);
void mostrarPCB(t_pcb auxiliar);
void inicializarPCB(t_pcb *pcb);
int *obtener_registro(char *registro_string);


#endif