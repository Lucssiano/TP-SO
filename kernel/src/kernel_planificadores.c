#include "../include/kernel_planificadores.h"

// VARIABLES GLOBALES
// Motivos finalizacion
#define FINALIZACION_SUCCESS "SUCCESS"
#define FINALIZACION_INVALID_RESOURCE "INVALID_RESOURCE"
#define FINALIZACION_INVALID_INTERFACE "INVALID_INTERFACE"
#define FINALIZACION_OUT_OF_MEMORY "OUT_OF_MEMORY"
#define FINALIZACION_INTERRUPTED_BY_USER "INTERRUPTED_BY_USER"
#define FINALIZACION_ERROR "ERROR" // lo agregue yo para los casos q no caen en ninguno de los anteriores

// Planificacion
t_estado_planificacion estado_planificacion;

// Estados
t_estado *estado_new;
t_estado *estado_ready;
t_estado *estado_ready_plus;
t_estado *estado_exec;
t_estado *estado_blocked;
t_estado *estado_exit;

// PID
int pid_actual;
pthread_mutex_t mutex_pid;

// Semaforos
pthread_mutex_t mutex_grado_multiprogramacion;
sem_t sem_grado_multiprogramacion;
pthread_mutex_t mutex_socket_memoria;
// CREAR UN MUTEX PARA CADA DICCIONARIO

// Recursos
t_dictionary *diccionario_recursos;

// Interfaces
t_dictionary *diccionario_interfaces;

// INICIALIZACION PLANIFICADORES ------------------------------------------
void iniciar_planificadores(){
    // Estructuras
    inicializar_estructuras();

    // Planificador largo plazo
    pthread_t hilo_planificador_largo_plazo;
    pthread_create(&hilo_planificador_largo_plazo, NULL, (void*)planificador_largo_plazo, NULL);
    pthread_detach(hilo_planificador_largo_plazo);

    // Planificador corto plazo
    pthread_t hilo_planificador_corto_plazo;
    pthread_create(&hilo_planificador_corto_plazo, NULL, (void*)planificador_corto_plazo, NULL);
    pthread_detach(hilo_planificador_corto_plazo);

    iniciar_planificacion();
}

void inicializar_estructuras(){
    inicializar_estructuras_estados();
    inicializar_estructuras_pid();
    inicializar_semaforos();
    inicializar_diccionarios();
}

void inicializar_estructuras_estados(){
    estado_new = crear_estado(NEW);
    estado_ready = crear_estado(READY);
    estado_ready_plus = crear_estado(READY_PLUS);
    estado_exec = crear_estado(EXEC);
    estado_blocked = crear_estado(BLOCKED);
    estado_exit = crear_estado(EXIT);
}

void inicializar_estructuras_pid(void){
    pid_actual = 0;
    pthread_mutex_init(&mutex_pid, NULL);
}

void inicializar_semaforos(){
    pthread_mutex_init(&mutex_grado_multiprogramacion, NULL);
    sem_init(&sem_grado_multiprogramacion, 0, GRADO_MULTIPROGRAMACION);
    pthread_mutex_init(&mutex_socket_memoria, NULL);
}

void inicializar_diccionarios(){
    diccionario_recursos = crear_diccionario_recursos(RECURSOS, INSTANCIAS_RECURSOS);
    diccionario_interfaces = dictionary_create();
}

void iniciar_planificacion(){
    estado_planificacion = ACTIVA;
}

void detener_planificacion(){
    estado_planificacion = PAUSADA;
}

void cambiar_grado_multiprogramacion_a(int nuevo_grado_multiprogramacion){
    pthread_mutex_lock(&mutex_grado_multiprogramacion);
    
    int diferencia = nuevo_grado_multiprogramacion - GRADO_MULTIPROGRAMACION;
    GRADO_MULTIPROGRAMACION = nuevo_grado_multiprogramacion;
    
    if( diferencia > 0 ){
        for(int i = 0; i < diferencia; i++){
            sem_post(&sem_grado_multiprogramacion);
        }
    }
    else if( diferencia < 0 ){
        for(int i = 0; i < -diferencia; i++){
            sem_wait(&sem_grado_multiprogramacion);
        }
    }

    pthread_mutex_unlock(&mutex_grado_multiprogramacion);
}

// PLANIFICADOR CORTO PLAZO ---------------------------------------------------

void planificador_corto_plazo(){    
    if( strcmp(ALGORITMO_PLANIFICACION, "FIFO") == 0 ){
        planificador_corto_plazo_fifo();
    }
    else if( strcmp(ALGORITMO_PLANIFICACION, "RR") == 0 ){
        planificador_corto_plazo_rr();
    }
    else if( strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0 ){
        planificador_corto_plazo_vrr();
    }
    else{
        log_error(kernel_logger, "Algoritmo de planificacion invalido!");
    }
}

// FIFO
void planificador_corto_plazo_fifo(){
    while( estado_planificacion ){
        t_pcb *pcb = elegir_proceso_segun_fifo();
        proceso_a_exec(pcb);
        enviar_contexto_de_ejecucion(pcb);
        recibir_contexto_de_ejecucion_actualizado(); // Modifica directo al pcb con lo que recibe
    }
}

t_pcb *elegir_proceso_segun_fifo(){
    return estado_desencolar_primer_pcb(estado_ready);
}

// RR
void planificador_corto_plazo_rr(){ //TERMINAR
    while( estado_planificacion ){
        t_pcb *pcb = elegir_proceso_segun_rr();
        proceso_a_exec(pcb);
        enviar_contexto_de_ejecucion(pcb);
        recibir_contexto_de_ejecucion_actualizado(); // Modifica directo al pcb con lo que recibe
    }
}

t_pcb *elegir_proceso_segun_rr(){
    return estado_desencolar_primer_pcb(estado_ready); 
}

// VRR
void planificador_corto_plazo_vrr(){ //TERMINAR
    while( estado_planificacion ){
        t_pcb *pcb = elegir_proceso_segun_rr();
        proceso_a_exec(pcb);
        enviar_contexto_de_ejecucion(pcb);
        recibir_contexto_de_ejecucion_actualizado(); // Modifica directo al pcb con lo que recibe
        // estado_ordenar_por_quantum_restante(estado_ready_plus); lo comente para q pueda compilar nomas
    }
}

t_pcb *elegir_proceso_segun_vrr(){
    if( estado_contiene_pcbs(estado_ready_plus) ){
        return estado_desencolar_primer_pcb(estado_ready_plus);
    }
    else{
        return estado_desencolar_primer_pcb(estado_ready);
    }
}

// Manda a CPU el contexto de la ejecucion (pid y registros) por el Dispatch
void enviar_contexto_de_ejecucion(t_pcb *pcb){
    t_paquete *paquete_contexto_de_ejecucion = crear_paquete(CONTEXTO_DE_EJECUCION);
    agregar_contexto_ejecucion_a_paquete(paquete_contexto_de_ejecucion, pcb);
    enviar_paquete(fd_cpu_dispatch, paquete_contexto_de_ejecucion);
    eliminar_paquete(paquete_contexto_de_ejecucion);
}

void recibir_contexto_de_ejecucion_actualizado(){ // TERMINAR
    // Espera por el Dispatch la llegada del contexto actualizado tras la ejecucion del proceso (pid y registros)
    // Junto con el contexto debe llegar el motivo por el cual finalizo la ejecucion (motivo de desalojo)
    // En cualquier caso se lo debe desencolar de EXEC -> NOTA: no necesariamente, si el proceso quiere hacer un signal deberia seguir ejecutando el mismo proceso
    // Si puede seguir ejecutando se lo encola en READY
    // Si no se bloqueo entonces se lo encola en BLOCKED
    t_codigo_operacion motivo_desalojo;
    t_buffer *buffer = crear_buffer();
    recibir_paquete(fd_cpu_dispatch, &motivo_desalojo, buffer);
    t_pcb *pcb = estado_desencolar_primer_pcb(estado_exec); // No se si lo correcto seria desencolarlo por lo del signal por ejemplo -> se podria desencolar, pero al hacer el signal se deberia meter primero en la cola de ready de maxima prioridad
    buffer_desempaquetar_contexto_ejecucion(buffer, pcb); // Modifica al pcb con lo que recibe
    
    switch(motivo_desalojo){
        case SUCCESS:
            break;
        case INTERRUPT_QUANTUM:
            break;
        case INTERRUPT_USER:
            break;
        case OUT_OF_MEMORY:
            break;
        case WAIT:
            ejecutar_instruccion_wait(pcb, buffer_desempaquetar_string(buffer));
            break;
        case SIGNAL:
            ejecutar_instruccion_signal(pcb, buffer_desempaquetar_string(buffer));
            break;
        case IO:
            char *nombre_interfaz = buffer_desempaquetar_string(buffer);
            // sem_wait diccionario
            if( diccionario_interfaces_existe_interfaz(nombre_interfaz) ){ // la corroboracion de la conexion se hace al momento de mandar la operacion a la interfaz (si existe, es muy probable q siga conectada (salvo q se desconecte justo dsp de la corroboracion))
                // Obtengo la interfaz
                t_interfaz *interfaz = diccionario_interfaces_get_interfaz(diccionario_interfaces, nombre_interfaz);
                // sem_post diccionario
                
                // Desempaqueto la operacion a realizar y corroboro que sea valida
                t_codigo_operacion operacion;
                buffer_desempaquetar(buffer, operacion);


                
                
                
                
                
                
                
                
                
                // Obtengo la interfaz y compruebo que este conectada
                t_interfaz *interfaz = diccionario_interfaces_get_interfaz(diccionario_interfaces, nombre_interfaz);
                if( interfaz_esta_conectada(interfaz) ){
                    // Desempaqueto la operacion a realizar y corroboro que sea valida
                    t_codigo_operacion operacion;
                    buffer_desempaquetar(buffer, operacion);
                    switch( operacion ){
                        case IO_GEN_SLEEP:
                            // Desempaqueto los parametros de la operacion
                            char *parametros = buffer_desempaquetar_string(buffer);

                            // Creo el paquete con la operacion a realizar y sus parametros, y se lo envio a la interfaz
                            t_paquete paquete_operacion_interfaz = crear_paquete(operacion);
                            agregar_string_a_paquete(paquete_operacion_interfaz, parametros);
                            enviar_paquete(interfaz_get_fd(interfaz), paquete_operacion_interfaz);
                            eliminar_paquete(paquete_operacion_interfaz);

                            // Bloqueo al proceso
                            interfaz_encolar_proceso(interfaz, pcb);
                            proceso_a_blocked(pcb, nombre_interfaz);
                            break;
                        default:
                            proceso_a_exit(pcb, FINALIZACION_INVALID_INTERFACE);
                            // sem_post(&sem_grado_multiprogramacion);
                    }
                }
                else{
                    proceso_a_exit(pcb, FINALIZACION_INVALID_INTERFACE);
                    // sem_post(&sem_grado_multiprogramacion);
                }
            }
            else{
                proceso_a_exit(pcb, FINALIZACION_INVALID_INTERFACE);
                // sem_post(&sem_grado_multiprogramacion);
            }
            break;
        default:
            log_error(kernel_logger, "Motivo de desalojo desconocido");
    }

    eliminar_buffer(buffer);
}

// PLANIFICADOR LARGO PLAZO ---------------------------------------------------

// Crea el pcb y lo encola en new
void iniciar_proceso(char *path){
    int pid = generar_pid();
    t_pcb *pcb = crear_pcb(pid, path);
    estado_encolar_pcb(estado_new, pcb);
    log_creacion_proceso(pcb);
}

void finalizar_proceso(int pid){ // TERMINAR
    detener_planificacion();
    // Me gusta mas la idea de la lista negra
    // pthread_mutex_lock(mutex_lista_procesos_pendientes_a_finalizar);
    // list_add(procesos_pendientes_a_finalizar, pid);
    // pthread_mutex_unlock(mutex_lista_procesos_pendientes_a_finalizar);
    // FORMA 1
    // t_pcb *pcb = estado_rastrear_y_desencolar_pcb_por_pid(pid);
    // FORMA 2
    t_estado *estado = estado_rastrear_pcb_por_pid(pid);
    if( estado == NULL ){
        log_error(kernel_logger, "El PCB solicitado no se encuentra en el sistema");
    }
    switch(estado_get_nombre_estado(estado)){
        case BLOCKED:
            // Deberia mandarlo a exit cuando la io avise q termino
            break;
        case EXEC:
            // Mandar interrupcion a CPU
            break;
        case EXIT:
            log_error(kernel_logger, "El PCB solicitado ya esta siendo eliminado del sistema");
            break;
        default:
            t_pcb *pcb = estado_desencolar_pcb_por_pid(estado, pid); // revisar el quilombo de kernel_estado.c
            sem_post(&sem_grado_multiprogramacion);
    }
    
    // Se deberia liberar la memoria asignada al proceso
    // list_remove_by_condition();
    // proceso_a_exit();
    // sem_post(&sem_grado_multiprogramacion);

    // Pseudocodigo
    // detener_planificacion(); -> tiene que frenar las transiciones?
}

void planificador_largo_plazo(){
    // Manejar ESTADO -> EXIT
    pthread_t hilo_liberar_procesos_exit;
    pthread_create(&hilo_liberar_procesos_exit, NULL, (void *)liberar_procesos_exit, NULL);
    pthread_detach(hilo_liberar_procesos_exit);

    // Manejar NEW -> READY
    while( estado_planificacion ){
        sem_wait(&sem_grado_multiprogramacion);
        t_pcb *pcb = estado_desencolar_primer_pcb(estado_new);
        t_codigo_operacion respuesta_memoria = pedir_a_memoria_iniciar_proceso(pcb_get_pid(pcb), pcb_get_path(pcb));
        if( respuesta_memoria == CONFIRMACION_PROCESO_INICIADO ){
            proceso_a_ready(pcb);
        }
        else{
            proceso_a_exit(pcb, FINALIZACION_ERROR);
            // sem_post(&sem_grado_multiprogramacion);
        } 
    }

    
    // // Estas funciones se podrían unir en una sola y hacer un for, pero capaz queda mas simple hacer estas 4 funciones y listo
    // manejador_new_exit();
    // manejador_ready_exit();
    // manejador_exec_exit();
    // manejador_blocked_exit();
}

t_codigo_operacion pedir_a_memoria_iniciar_proceso(int pid, char *path){
    t_codigo_operacion respuesta_memoria;
    t_paquete *paquete_solicitud_iniciar_proceso = crear_paquete(SOLICITUD_INICIAR_PROCESO);
    agregar_pid_a_paquete(paquete_solicitud_iniciar_proceso, pid);
    agregar_string_a_paquete(paquete_solicitud_iniciar_proceso, path);
    
    pthread_mutex_lock(&mutex_socket_memoria);
    // Envio la solicitud
    enviar_paquete(fd_memoria, paquete_solicitud_iniciar_proceso);
    // Recibo la respuesta
    recibir_codigo_operacion(fd_memoria, &respuesta_memoria);
    pthread_mutex_unlock(&mutex_socket_memoria);

    eliminar_paquete(paquete_solicitud_iniciar_proceso);

    return respuesta_memoria;
}

void liberar_procesos_exit(){
    while(1){
        t_pcb *pcb = estado_desencolar_primer_pcb(estado_exit);
        t_codigo_operacion respuesta_memoria = pedir_a_memoria_finalizar_proceso(pcb_get_pid(pcb));
        if( respuesta_memoria == CONFIRMACION_PROCESO_FINALIZADO ){
            if( !dictionary_is_empty(pcb_get_diccionario_recursos_usados(pcb)) ){
                liberar_recursos_usados(pcb);
            }
            eliminar_pcb(pcb);
            // sem_post(&sem_grado_multiprogramacion);
        }
        else{
            log_error(kernel_logger, "MEMORIA no finalizo correctamente el proceso!");
        }
    }
}

t_codigo_operacion pedir_a_memoria_finalizar_proceso(int pid){
    t_codigo_operacion respuesta_memoria;
    t_paquete *paquete_solicitud_finalizar_proceso = crear_paquete(SOLICITUD_FINALIZAR_PROCESO);
    agregar_pid_a_paquete(paquete_solicitud_finalizar_proceso, pid);
    
    pthread_mutex_lock(&mutex_socket_memoria);
    // Envio la solicitud
    enviar_paquete(fd_memoria, paquete_solicitud_finalizar_proceso);
    // Recibo la respuesta
    recibir_codigo_operacion(fd_memoria, &respuesta_memoria);
    pthread_mutex_unlock(&mutex_socket_memoria);

    eliminar_paquete(paquete_solicitud_finalizar_proceso);

    return respuesta_memoria;
}

void liberar_recursos_usados(t_pcb *pcb){
    t_dictionary *diccionario_recursos_usados = pcb_get_diccionario_recursos_usados(pcb);
    int size_dict = dictionary_size(diccionario_recursos_usados);
    t_list *recursos = dictionary_keys(diccionario_recursos_usados);
    for(int i = 0; i < size_dict; i++){
        char *nombre_recurso = list_get(recursos, i);
        liberar_recurso(pcb, nombre_recurso);
    }

    list_destroy_and_destroy_elements(recursos, free);
}

void liberar_recurso(t_pcb *pcb, char *nombre_recurso){
    t_dictionary *diccionario_recursos_usados = pcb_get_diccionario_recursos_usados(pcb);
    int instancias_usadas = *(int *) dictionary_get(diccionario_recursos_usados, nombre_recurso);
    for(int i = 0; i < instancias_usadas; i++){
        ejecutar_instruccion_signal(pcb, nombre_recurso);
    }
}

void ejecutar_instruccion_signal(t_pcb *pcb, char *nombre_recurso){
    // Verifico que el recurso exista
    if( !diccionario_recursos_existe_recurso(diccionario_recursos, nombre_recurso) ){
        proceso_a_exit(pcb, FINALIZACION_INVALID_RESOURCE);
        // sem_post(&sem_grado_multiprogramacion); -> Tengo q definir si lo hago en proceso_a_exit, cuando libero los recursos o lo hago siempre seguido al proceso_a_exit
    }
    else{
        // Impacto el signal en el recurso (aumento en 1 la cantidad de instancias)
        t_recurso *recurso = diccionario_recursos_get_recurso(diccionario_recursos, nombre_recurso);
        recurso_signal(recurso);

        // Impacto el signal en el pcb (disminuyo en 1 las instancias tomadas). Esto lo realizo solo si el proceso tiene tomada una instancia del recurso. Si el proceso no usa el recurso (tiene tomadas 0 instancias), entonces el mismo esta creando una nueva instancia del recurso con el signal -> no hay nada que impactar en el pcb (la instancia del recurso ya fue creada arriba con recurso_signal).
        if( pcb_usa_recurso(pcb, nombre_recurso) ){
            t_dictionary *diccionario_recursos_usados = pcb_get_diccionario_recursos_usados(pcb);
            int *instancias_usadas = (int *) dictionary_get(diccionario_recursos_usados, nombre_recurso);
            if( *instancias_usadas > 1 ){
                // Si el proceso tomo mas de una instancia => le resto una instancia (ya que realizo un signal y libero una) y actualizo su diccionario
                (*instancias_usadas)--;
                dictionary_put(diccionario_recursos_usados, nombre_recurso, instancias_usadas); 
            }
            else{
                // Si el proceso solo tiene tomada una instancia => directamente elimino al recurso de su diccionario, ya que al hacer el signal no tendra ninguna instancia
                dictionary_remove_and_destroy(diccionario_recursos_usados, nombre_recurso, free);
            }
        }
        
        // Compruebo si tras el signal se debe desbloquear algun proceso
        if( recurso_debe_desbloquear_proceso(recurso) ){
            // Desbloqueo al primero proceso
            t_pcb *pcb = recurso_desencolar_primer_proceso(recurso);
            estado_desencolar_pcb_por_pid(estado_blocked, pcb_get_pid(pcb)); // REVISAR SI ESTO ESTA BIEN, LA IDEA ES DESBLOQUEARLO DEL RECURSO Y TMB DE LA LISTA DE BLOQUEADOS
            // ACA PROBABLEMENTE TENGA Q HACER UN IF( LE_QUEDA_QUANTUM(PCB) ){ PROCESO_A_READY_PLUS } ELSE{ PROCESO_A_READY }
            proceso_a_ready(pcb);
        }
    }
}

void ejecutar_instruccion_wait(t_pcb *pcb, char *nombre_recurso){
    // Verifico que el recurso exista
    if( !diccionario_recursos_existe_recurso(diccionario_recursos, nombre_recurso) ){
        proceso_a_exit(pcb, FINALIZACION_INVALID_RESOURCE);
        // sem_post(&sem_grado_multiprogramacion); -> Tengo q definir si lo hago en proceso_a_exit, cuando libero los recursos o lo hago siempre seguido al proceso_a_exit
    }
    else{
        // Impacto el wait en el recurso
        t_recurso *recurso = diccionario_recursos_get_recurso(diccionario_recursos, nombre_recurso);
        recurso_wait(recurso);

        // Impacto el wait en el pcb
        // Si el proceso ya tenia tomada una instancia le agrego una mas
        // Si el proceso no tenia tomada ninguna instancia le agrego el recurso a su diccionario con 1 instancia
        t_dictionary *diccionario_recursos_usados = pcb_get_diccionario_recursos_usados(pcb);
        if( pcb_usa_recurso(pcb, nombre_recurso) ){
            int *instancias_usadas = (int *) dictionary_get(diccionario_recursos_usados, nombre_recurso);
            (*instancias_usadas)++;
            dictionary_put(diccionario_recursos_usados, nombre_recurso, instancias_usadas);
        }
        else{
            int *instancias_usadas = malloc(sizeof(int));
            *instancias_usadas = 1; 
            dictionary_put(diccionario_recursos_usados, nombre_recurso, instancias_usadas);
        }
        
        // Compruebo si tras el wait se debe bloquear al proceso
        if( recurso_debe_bloquear_proceso(recurso) ){
            // Bloqueo al proceso (lo encolo en la cola del recurso)
            recurso_encolar_proceso(recurso, pcb);
            proceso_a_blocked(pcb, nombre_recurso);
        }
    }
}

// void manejador_new_exit() {
//     while (estado_planificacion) {
//         sem_wait(estado_get_sem(estado_new)); 
//         t_pcb *pcb = estado_desencolar_primer_pcb(estado_new);
//         proceso_a_exit(pcb);
//     }
// }

// void manejador_ready_exit() {
//     while (estado_planificacion) {
//         sem_wait(estado_get_sem(estado_ready));
//         t_pcb *pcb = estado_desencolar_primer_pcb(estado_ready);
//         proceso_a_exit(pcb);
//     }
// }

// void manejador_exec_exit() {
//     while (estado_planificacion) {
//         sem_wait(estado_get_sem(estado_exec));
//         t_pcb *pcb = estado_desencolar_primer_pcb(estado_exec);
//         proceso_a_exit(pcb);
//     }
// }

// void manejador_blocked_exit() {
//     while (estado_planificacion) {
//         sem_wait(estado_get_sem(estado_blocked));
//         t_pcb *pcb = estado_desencolar_primer_pcb(estado_blocked);
//         proceso_a_exit(pcb);
//     }
// }

// PROCESO A ... ---------------------------------------------------

void proceso_a_ready(t_pcb *pcb){
	pcb_cambiar_estado_a(pcb, READY);
	estado_encolar_pcb(estado_ready, pcb);
    log_ingreso_ready(estado_ready);
}

void proceso_a_exec(t_pcb *pcb){
    pcb_cambiar_estado_a(pcb, EXEC);
    estado_encolar_pcb(estado_exec, pcb);
}

void proceso_a_blocked(t_pcb *pcb, char *motivo_bloqueo){
    pcb_cambiar_estado_a(pcb, BLOCKED);
    estado_encolar_pcb(estado_blocked, pcb);
    log_motivo_bloqueo(pcb, motivo_bloqueo);
}

void proceso_a_exit(t_pcb *pcb, char *motivo_finalizacion){
    pcb_cambiar_estado_a(pcb, EXIT);
    estado_encolar_pcb(estado_exit, pcb);
    log_fin_proceso(pcb, motivo_finalizacion);
}