#ifndef KERNEL_PLANIFICADOR_H_
#define KERNEL_PLANIFICADOR_H_

#include "kernel_gestor.h"
#include "kernel_utils.h"
#include "kernel_pcb.h"
#include "kernel_estados.h"
#include "kernel_recursos.h"

// INICIALIZACION PLANIFICADORES
void iniciar_planificadores(void);
void inicializar_estructuras(void);
void inicializar_estructuras_estados(void);
void inicializar_estructuras_pid(void);
void inicializar_semaforos(void);
void inicializar_diccionarios(void);
void iniciar_planificacion(void);
void detener_planificacion(void);
void cambiar_grado_multiprogramacion_a(int nuevo_grado_multiprogramacion);
// PLANIFICADOR CORTO PLAZO
void planificador_corto_plazo(void);
void planificador_corto_plazo_fifo(void);
t_pcb *elegir_proceso_segun_fifo(void);
void planificador_corto_plazo_rr(void);
t_pcb *elegir_proceso_segun_rr(void);
void planificador_corto_plazo_vrr(void);
t_pcb *elegir_proceso_segun_vrr(void);
void enviar_contexto_de_ejecucion(t_pcb *pcb);
void recibir_contexto_de_ejecucion_actualizado();
// PLANIFICADOR LARGO PLAZO
void iniciar_proceso(char *path);
void finalizar_proceso(int pid);
void planificador_largo_plazo(void);
t_codigo_operacion pedir_a_memoria_iniciar_proceso(int pid, char *path);
void liberar_procesos_exit(void);
t_codigo_operacion pedir_a_memoria_finalizar_proceso(int pid);
void liberar_recursos_usados(t_pcb *pcb);
void liberar_recurso(t_pcb *pcb, char *nombre_recurso);
void ejecutar_instruccion_signal(t_pcb *pcb, char *nombre_recurso);
void ejecutar_instruccion_wait(t_pcb *pcb, char *nombre_recurso);
// PROCESO A ...
void proceso_a_ready(t_pcb *pcb);
void proceso_a_exec(t_pcb *pcb);
void proceso_a_blocked(t_pcb *pcb, char *motivo_bloqueo);
void proceso_a_exit(t_pcb *pcb, char *motivo_finalizacion);

#endif