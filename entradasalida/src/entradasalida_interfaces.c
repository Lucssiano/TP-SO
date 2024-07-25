#include "../include/entradasalida_interfaces.h"

// El valor de la cant_unidades_trabajo viene de kernel y se multiplica por el valor que tiene el config de tiempo_unidad_trabajo
void interfaz_generica(int cant_unidades_trabajo) {
    int tiempo_total = (cant_unidades_trabajo * TIEMPO_UNIDAD_TRABAJO) * 1000;
    log_info(entradasalida_logger, "%d", tiempo_total);

    // Simular el tiempo de espera
    log_info(entradasalida_logger, "E/S: haciendo un sleep");
    usleep(tiempo_total);
    log_info(entradasalida_logger, "E/S: deje de hacer un sleep");

}

// SOPORTE: lista de direcciones fisicas - tamaño total (preguntar)
void interfaz_stdin(t_list *lista_direcciones, int pid){
    char *texto;
	texto = readline("Ingrese el texto a escribir en MEMORIA: ");

    if(texto == NULL) {
        log_error(entradasalida_logger,"Error al leer el texto");
        exit(1);
    }

    int bytesTexto = strlen(texto);
    int bytesLeidos = 0;
    int indice = 0;

    while(bytesLeidos < bytesTexto){
        t_paquete* paquete = crear_paquete(SOLICITUD_ESCRITURA);
        agregar_int_a_paquete(paquete, pid);

        t_direccion* t_direccion = list_get(lista_direcciones, indice);
        int df_a_enviar  = t_direccion->direccion_fisica;
        int bytes_a_enviar = t_direccion->bytes;

        agregar_int_a_paquete(paquete, df_a_enviar);
        agregar_int_a_paquete(paquete, bytes_a_enviar);

        // Calculo el tamaño del texto a enviar
        int bytes_restantes = bytesTexto - bytesLeidos;
        log_info(entradasalida_logger, "FALTAN ESCRIBIR %d BYTES!", bytes_restantes);
        if (bytes_restantes < bytes_a_enviar) {
            bytes_a_enviar = bytes_restantes;
        }
        log_info(entradasalida_logger, "VOY A ENVIAR %d BYTES!", bytes_a_enviar);

        // Considerando el caso donde bytesLeidos es 10 y bytesTexto es 16:
        // bytesTexto - bytesLeidos = 6 bytes restantes.
        // Si el próximo bytes_a_enviar es 8, se ajusta a 6 para no exceder los bytes restantes

        // Creo el texto a enviar
        void *textoCortado = malloc(bytes_a_enviar);
        memmove(textoCortado, texto + bytesLeidos, bytes_a_enviar);
        //textoCortado[bytes_a_enviar] = '\0';

        char *cadena_a_enviar = malloc(bytes_a_enviar+1);
        memcpy(cadena_a_enviar, textoCortado, bytes_a_enviar);
        cadena_a_enviar[bytes_a_enviar] = '\0';
        log_info(entradasalida_logger, "VOY A ENVIAR LA CADENA: %s", cadena_a_enviar);
        free(cadena_a_enviar);
        
        agregar_a_paquete(paquete, textoCortado, bytes_a_enviar);
        //agregar_string_a_paquete(paquete, textoCortado);

        enviar_paquete(fd_memoria, paquete);

        eliminar_paquete(paquete);

        t_codigo_operacion op_code;
        /* DEFINIR CUAL VA A SER EL COD OP*/ /* VER SI ESTÁ BIEN COLOCADO ACÁ */ /* VALIDACIONES DE ERRORES */
        recibir_codigo_operacion(fd_memoria, &op_code);
        if(op_code!= CONFIRMACION_ESCRITURA){
            log_error(entradasalida_logger,"NO SE RECIBE CONFIRMACION ESCRITURA");
            break;
        }
        
	    free(textoCortado);
        
        bytesLeidos += bytes_a_enviar;
        indice++;
    }
       // Hola cómo andas/0 --- 16 BYTES
                // 2 bytes - 11
                // 8 bytes - 31
                // 6 bytes - 8

                // Ho
                // la cómo 
                // andas/0

	free(texto); // Es necesario?
}

void interfaz_stdout(t_list* lista_direcciones, int cant_direcciones, int pid){
    char* texto = string_new();
    for (int i = 0; i < cant_direcciones; i++)
    {
        t_paquete* paquete = crear_paquete(SOLICITUD_LECTURA);
        agregar_int_a_paquete(paquete, pid);
        
        t_direccion* t_direccion = list_get(lista_direcciones, i);
        int df_a_enviar  = t_direccion->direccion_fisica;
        int bytes_a_enviar = t_direccion->bytes;

        agregar_int_a_paquete(paquete, df_a_enviar);
        agregar_int_a_paquete(paquete, bytes_a_enviar);

        enviar_paquete(fd_memoria, paquete);
        eliminar_paquete(paquete);

        /* DEFINIR COMO ME VA A TRAER LOS DATOS QUE LE MANDO ASÍ LOS PUEDO GUARDAR EN EL TEXTO */
        /* LECTURA_RESPUESTA DEBERIA SER CONFIRMACIÓN_LECTURA */
        t_codigo_operacion op_code;
        t_buffer *buffer = crear_buffer(); 
        recibir_paquete(fd_memoria, &op_code, buffer);
        if( op_code != DATO){
            log_error(entradasalida_logger,"NO SE RECIBE DATO");
            break;
        }
        char* textoTemporal = buffer_desempaquetar_string(buffer);
        string_append(&texto, textoTemporal);
        free(textoTemporal);
    }

    if (texto != NULL) {
        log_info(entradasalida_logger, "TEXTO LEIDO: %s", texto); // Preguntar si puede ser así o tiene que ser un printf u otra cosa
        free(texto);
    } else {
        log_error(entradasalida_logger, "Error al recibir datos de memoria.\n");
    }
    
}
/* 
void interfaz_fs_create(char* filename) {
    // Busca un bloque libre en el bitmap
    int block_index = -1;
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            block_index = i;
            bitarray_set_bit(bitarray, i);
            break;
        }
    }

    if (block_index == -1) {
        printf("No hay bloques libres disponibles.\n");
        return;
    }

    msync(bitmap_data, bitarray->size, MS_SYNC);

    // Creo el archivo de metadata
    FILE* metadata_file = fopen(filename, "w");
    if (metadata_file == NULL) {
        perror("Error al crear el archivo de metadata");
        return;
    }
    // USAR CONFIG_SET_VALUE 
    fprintf(metadata_file, "BLOQUE_INICIAL=%d \n TAMANIO_ARCHIVO=0 \n", block_index);
    fclose(metadata_file);
}
 */

/*
void interfaz_fs_delete(char* filename) {
    // Abro el archivo de metadata
    FILE* metadata_file = fopen(filename, "r");
    if (metadata_file == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }

    // Lee el bloque inicial del archivo
    int block_index;
    // USAR CONFIG_GET_VALUE 
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\n", &block_index);
    fclose(metadata_file);

    // Libera el bloque en el bitarray
    bitarray_clean_bit(bitarray, block_index);

    // Sincronizar los cambios en el bitarray con el archivo bitmap
    msync(bitmap_data, bitarray->size, MS_SYNC);

    // Elimina el archivo de metadata
    if (remove(filename) != 0) {
        perror("Error al eliminar el archivo de metadata");
    }
}
/* 

void IO_FS_TRUNCATE(char* filename, int new_size) {
    // Abrir el archivo de metadata
    FILE* metadata_file = fopen(filename, "r+");
    if (metadata_file == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }

    int block_index, current_size;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &block_index, &current_size);

    int current_blocks = (current_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // Número actual de bloques
    int new_blocks = (new_size + BLOCK_SIZE - 1) / BLOCK_SIZE; // Nuevos bloques necesarios

    if (new_blocks == current_blocks) {
        // No se requiere cambio de tamaño
        fclose(metadata_file);
        return;
    } else if (new_blocks < current_blocks) {
        // Reducción del tamaño del archivo
        for (int i = current_blocks - 1; i >= new_blocks; i--) {
            bitarray_clean_bit(bitarray, block_index + i);
        }

        // Sincronizar cambios en el bitarray con el archivo bitmap
        msync(bitmap_data, bitarray->size, MS_SYNC);
    } else {
        // Ampliación del tamaño del archivo
        int additional_blocks_needed = new_blocks - current_blocks;
        int can_expand = 1;

        // Verificar si hay bloques contiguos disponibles
        for (int i = 0; i < additional_blocks_needed; i++) {
            if (bitarray_test_bit(bitarray, block_index + current_blocks + i)) {
                can_expand = 0;
                break;
            }
        }

        if (!can_expand) {
            // Realizar compactación
            compactar_fs();
            // Recalcular el bloque inicial ya que pudo haber cambiado después de la compactación
            block_index = recalcular_bloque_inicial(filename);
        }

        // Asignar nuevos bloques después de la compactación o si ya estaban contiguos
        for (int i = 0; i < additional_blocks_needed; i++) {
            bitarray_set_bit(bitarray, block_index + current_blocks + i);
        }

        // Sincronizar cambios en el bitarray con el archivo bitmap
        msync(bitmap_data, bitarray->size, MS_SYNC);
    }

    // Actualizar el tamaño del archivo en el archivo de metadata
    fseek(metadata_file, 0, SEEK_SET);
    fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", block_index, new_size);
    fclose(metadata_file);
}
 */

/* void IO_FS_WRITE(char* filename, int offset, char* data, int size) {
    // Abre el archivo de metadata
    FILE* metadata_file = fopen(filename, "r");
    if (metadata_file == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }

    // Lee el bloque inicial y el tamaño actual del archivo
    int block_index, current_size;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &block_index, &current_size);
    fclose(metadata_file);

    // Abre el archivo de bloques
    FILE* bloques_file = fopen(BLOQUES_PATH, "rb+");
    if (bloques_file == NULL) {
        perror("Error al abrir bloques.dat");
        return;
    }

    // Escribe los datos en el archivo de bloques en la posición adecuada
    fseek(bloques_file, block_index * BLOCK_SIZE + offset, SEEK_SET);
    fwrite(data, 1, size, bloques_file);
    fclose(bloques_file);

    // Actualiza el tamaño del archivo en el archivo de metadata si es necesario
    if (offset + size > current_size) {
        metadata_file = fopen(filename, "r+");
        if (metadata_file == NULL) {
            perror("Error al abrir el archivo de metadata");
            return;
        }
        fseek(metadata_file, 0, SEEK_SET);
        fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", block_index, offset + size);
        fclose(metadata_file);
    }
}
 */

/* void IO_FS_READ(char* filename, int offset, char* buffer, int size) {
    // Abre el archivo de metadata
    FILE* metadata_file = fopen(filename, "r");
    if (metadata_file == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }

    // Lee el bloque inicial y el tamaño actual del archivo
    int block_index, current_size;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &block_index, &current_size);
    fclose(metadata_file);

    // Asegúrate de no leer más allá del final del archivo
    if (offset + size > current_size) {
        size = current_size - offset;
    }

    // Abre el archivo de bloques
    FILE* bloques_file = fopen(BLOQUES_PATH, "rb");
    if (bloques_file == NULL) {
        perror("Error al abrir bloques.dat");
        return;
    }

    // Lee los datos del archivo de bloques en la posición adecuada
    fseek(bloques_file, block_index * BLOCK_SIZE + offset, SEEK_SET);
    fread(buffer, 1, size, bloques_file);
    fclose(bloques_file);
}
 */


/* int asignar_bloque() {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            bitarray_set_bit(bitarray, i);
            return i;
        }
    }
    return -1; // No hay bloques disponibles
}

void liberar_bloque(int block_index) {
    if (block_index >= 0 && block_index < BLOCK_COUNT) {
        bitarray_clean_bit(bitarray, block_index);
    }
} */