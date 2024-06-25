#include "../include/inicializar_memoria.h"

void inicializar_memoria()
{
    iniciar_logger();
    iniciar_config();
	iniciar_variables();
}

void iniciar_logger(void)
{
	memoria_logger = log_create("memoria.log", "MEMORIA", 1, LOG_LEVEL_INFO);
	if(memoria_logger == NULL){
		printf("No se pudo crear el logger.");
		exit(1);
	}
}

void iniciar_config(void)
{
	memoria_config = config_create("/home/utnso/tp-2024-1c-GSN/memoria/memoria.config");
	if(memoria_config == NULL){
		printf("No se pudo crear el config.");
		exit(2);
	}

    PUERTO_ESCUCHA = config_get_string_value(memoria_config, "PUERTO_ESCUCHA");
	TAM_MEMORIA = config_get_int_value(memoria_config, "TAM_MEMORIA");
	TAM_PAGINA = config_get_int_value(memoria_config, "TAM_PAGINA");
    PATH_INSTRUCCIONES = config_get_string_value(memoria_config, "PATH_INSTRUCCIONES");
    RETARDO_REPUESTA =config_get_int_value(memoria_config, "RETARDO_RESPUESTA");
}

void iniciar_variables(void)
{
	espacio_usuario = malloc(TAM_MEMORIA);
    memset(espacio_usuario, 0, TAM_MEMORIA);
    puntero_espacio_usuario = espacio_usuario; //puntero hacia el primer frame.
	procesos = malloc((TAM_MEMORIA / TAM_PAGINA) * sizeof(t_pcb_memoria));
    for(int i=0; i<(TAM_MEMORIA/TAM_PAGINA); i++)
        procesos[i].pid = -1; // Inicializo mi array de procesos con procesos con PID = -1 para indicar que las celdas del array están vacías
}