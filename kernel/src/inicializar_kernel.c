#include "../include/inicializar_kernel.h"

void inicializar_kernel(){
    iniciar_logger();
    iniciar_config();
}

void iniciar_logger(){
	// Creo el kernel_logger
    t_log* kernel_logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_INFO);
	if(kernel_logger == NULL){
		perror("No se pudo crear el logger.");
		exit(1);
	}
    
    // ...
}

void iniciar_config(){
    // Creo el kernel_config
	t_config* kernel_config = config_create("/home/utnso/tp-2024-1c-GSN/kernel/kernel.config");
	if(kernel_config == NULL){
		perror("No se pudo crear el config.");
		exit(2);
	}

    // Obtengo valores del archivo kernel.config
    PUERTO_ESCUCHA = config_get_string_value(kernel_config, "PUERTO_ESCUCHA");
    IP_MEMORIA = config_get_string_value(kernel_config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(kernel_config, "PUERTO_MEMORIA");
    IP_CPU = config_get_string_value(kernel_config, "IP_CPU");
    PUERTO_CPU_DISPATCH = config_get_string_value(kernel_config, "PUERTO_CPU_DISPATCH");
    PUERTO_CPU_INTERRUPT = config_get_string_value(kernel_config, "PUERTO_CPU_INTERRUPT");
    ALGORITMO_PLANIFICACION = config_get_string_value(kernel_config, "ALGORITMO_PLANIFICACION");
    QUANTUM = config_get_int_value(kernel_config, "QUANTUM");
    RECURSOS = config_get_array_value(kernel_config, "RECURSOS");
    INSTANCIAS_RECURSOS = config_get_string_value(kernel_config, "INSTANCIAS_RECURSOS");
    GRADO_MULTIPROGRAMACION = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");
}
