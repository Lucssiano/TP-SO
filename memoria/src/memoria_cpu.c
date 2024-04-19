#include "../include/memoria_cpu.h"

void atender_memoria_cpu(){
    int continuar = 1;
	while( continuar ){
		int cod_op = recibir_operacion(fd_cpu);
		switch(cod_op){
			case MENSAJE:
				// ...
				break;
			case PAQUETE:
				// ...
				break;
			case -1:
				log_error(memoria_logger, "Se perdio la conexion con CPU!");
				continuar = 0;
				break;
			default:
				log_warning(memoria_logger, "MEMORIA: Operacion desconocida recibida de CPU");
				break;
		}
	}
}