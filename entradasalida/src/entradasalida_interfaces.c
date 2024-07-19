#include "../include/entradasalida_interfaces.h"

// El valor de la cant_unidades_trabajo viene de kernel y se multiplica por el valor que tiene el config de tiempo_unidad_trabajo
void interfaz_generica(int cant_unidades_trabajo) {
    int tiempo_total = (cant_unidades_trabajo * TIEMPO_UNIDAD_TRABAJO) / 1000;
    log_info(entradasalida_logger, "%d", tiempo_total);

    // Simular el tiempo de espera
    log_info(entradasalida_logger, "E/S: haciendo un sleep");
    sleep(tiempo_total);
    log_info(entradasalida_logger, "E/S: deje de hacer un sleep");

    /* Agregar log obligatorio: Operación: "PID: <PID> - Operacion: <OPERACION_A_REALIZAR>" */
}

// SOPORTE: lista de direcciones fisicas - tamaño total (preguntar)
void interfaz_stdin(char* registro_direccion, char* registro_tamanio){
    char *texto;
	texto = readline("> ");

    if (strlen(texto) > atoi(registro_tamanio)) {
        texto[atoi(registro_tamanio)] = '\0';
    }

    enviar_texto_a_memoria(registro_direccion, texto, atoi(registro_tamanio));

    /* Agregar que se confirma la escritura del dato en memoria */
    /* Agregar log obligatorio: Operación: "PID: <PID> - Operacion: <OPERACION_A_REALIZAR>" */

	free(texto);
}

void enviar_texto_a_memoria(char* direccion_fisica, char* texto, int tamanio) {
    t_paquete* paquete = crear_paquete(SOLICITUD_ESCRITURA); // Creo que se usa ese codigo de operacion (PREGUNTAR A MATI)
    agregar_a_paquete(paquete, direccion_fisica, sizeof(char*));
    agregar_a_paquete(paquete, texto, tamanio);

    enviar_paquete(fd_memoria, paquete);

    eliminar_paquete(paquete);
}

void interfaz_stdout(char* registro_direccion, char* registro_tamanio) {
    solicitar_datos_a_memoria(registro_direccion, atoi(registro_tamanio));

    char* texto = recibir_datos_de_memoria(atoi(registro_tamanio));
    
    if (texto != NULL) {
        log_info(entradasalida_logger, "TEXTO LEIDO: %s", texto);
        /* Agregar log obligatorio: Operación: "PID: <PID> - Operacion: <OPERACION_A_REALIZAR>" */
        free(texto);
    } else {
        log_error(entradasalida_logger, "Error al recibir datos de memoria.\n");
    }
}

void solicitar_datos_a_memoria(char* direccion_fisica, int tamanio){
    t_paquete* paquete = crear_paquete(SOLICITUD_LECTURA);

    agregar_a_paquete(paquete, direccion_fisica, sizeof(char*));
    agregar_a_paquete(paquete, tamanio, sizeof(int));

    enviar_paquete(fd_memoria, paquete);

    eliminar_paquete(paquete);
}

char* recibir_datos_de_memoria(int tamanio) {
    t_buffer* buffer = crear_buffer();
    t_codigo_operacion cod_operacion;
    recibir_paquete(fd_memoria, &cod_operacion, buffer);

    /* Poner logica para leer cod operacion */
    char* texto = malloc(tamanio + 1);
    if (texto != NULL) {
        buffer_desempaquetar(buffer, texto);
        /* Poner desempaquetar string */
        texto[tamanio] = '\0';
    }

    eliminar_buffer(buffer);

    return texto;
}

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
        /* Ver si tendria que fijarme si hay lugar pero no puedo guardarlo por fragmentacion externa (en ese caso deberia hacer compactacion) */
        return;
    }

    // Creo el archivo de metadata
    FILE* metadata_file = fopen(filename, "w");
    if (metadata_file == NULL) {
        perror("Error al crear el archivo de metadata");
        return;
    }
    fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=0\n", block_index);
    fclose(metadata_file);
}

void interfaz_fs_delete(char* filename) {
    // Abro el archivo de metadata
    FILE* metadata_file = fopen(filename, "r");
    if (metadata_file == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }

    // Lee el bloque inicial del archivo
    int block_index;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\n", &block_index);
    fclose(metadata_file);

    // Libera el bloque en el bitmap
    bitarray_clean_bit(bitarray, block_index);

    // Elimina el archivo de metadata
    if (remove(filename) != 0) {
        perror("Error al eliminar el archivo de metadata");
    }
}

/* void IO_FS_TRUNCATE(char* filename, int new_size) {
    // Abre el archivo de metadata
    FILE* metadata_file = fopen(filename, "r+");
    if (metadata_file == NULL) {
        perror("Error al abrir el archivo de metadata");
        return;
    }

    // Lee el bloque inicial y el tamaño actual del archivo
    int block_index, current_size;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &block_index, &current_size);

    // Actualiza el tamaño del archivo en el archivo de metadata
    fseek(metadata_file, 0, SEEK_SET);
    fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", block_index, new_size);
    fclose(metadata_file);

    // Maneja la lógica de asignación y liberación de bloques según el nuevo tamaño
    // Aquí podrías necesitar agregar lógica para asignar o liberar bloques adicionales
} */

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