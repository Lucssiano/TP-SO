#include "../include/inicializar_interfaces.h"

void inicializar_interfaz(char* nombre_interfaz, char* archivo_configuracion){
    iniciar_logger(nombre_interfaz);
    iniciar_config(nombre_interfaz, archivo_configuracion);

    if (strcmp(nombre_interfaz,"dialfs") == 0) {
        inicializar_fs();
        // dialfs_inicializado = true;
    }

    /* Chequear si hubo previamente creación de archivos */
    /* dirent.h -- Revisar libreria */
    /* Quedarme con los archivos metadata - me sirven para la compactación */
    /* T bit array para el bitmap , debe coincidir con el tamaño del archivo */
    /* Investigar sobre la función mmap */
    /* cuando inicio el bitarray tengo que hacer un ftrunc para darle al archivo el espacio que necesita */
}

void inicializar_fs(){
    /* El 512 es para asegurar que sea lo suficientemente grande como para contener cualquier ruta de archivo que necesite construir */
    char BLOQUES_PATH[512];
    char BITMAP_PATH[512];

    /* Armo la ruta de ambos archivos */
    sprintf(BLOQUES_PATH, "%sbloques.dat", PATH_BASE_DIALFS);
    sprintf(BITMAP_PATH, "%sbitmap.dat", PATH_BASE_DIALFS);

    FILE* bloques_file = fopen(BLOQUES_PATH, "r");
    if (bloques_file == NULL) {
        bloques_file = fopen(BLOQUES_PATH, "wb");
        if (bloques_file == NULL) {
            perror("Error al crear bloques.dat");
            exit(1);
        }
        // Con ftruncate establezco el tamaño del archivo recién creado a BLOCK_SIZE * BLOCK_COUNT
        // fileno obtiene el file descriptor del archivo que necesita ftruncate
        ftruncate(fileno(bloques_file), BLOCK_SIZE * BLOCK_COUNT);
        fclose(bloques_file);
    } else {
        // Si el archivo ya existia lo cierro
        fclose(bloques_file);
    }

    // Manejo del archivo de bitmap usando mmap
    // Me permite mapear el archivo de bitmap directamente en la memoria
    // Una vez mapeado el archivo se puede acceder al contenido como si fuera una porción de memoria, lo que puede hacer que las operaciones de lectura y escritura sean más rápidas y eficientes
    
    // Puntero que va a apuntar al área de memoria donde el archivo del bitmap va a ser mapeado
    void* bitmap_data = NULL;
    // File descriptor que se va a usar para manejar el archivo bitmap.dat
    int bitmap_fd = -1;
    
    // open se usa para abrir bitmap.dat con permisos de lectura y escritura (O_RDWR) , o crear el archivo si no existe (O_CREAT). Los permisos de usuario se establecen en lectura y escritura (S_IRUSR | S_IWUSR).
    bitmap_fd = open(BITMAP_PATH, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (bitmap_fd == -1) {
        perror("Error al abrir bitmap.dat");
        exit(1);
    }

    // (BLOCK_COUNT / 8) + (BLOCK_COUNT % 8 != 0) convierte el número total de bloques en el número necesario de bytes, 
    // teniendo en cuenta que pueden ser necesarios bits adicionales si el número de bloques no es un múltiplo exacto de 8 (por eso el + si no da un multiplo exactao)
    size_t bitmap_size = (BLOCK_COUNT / 8) + (BLOCK_COUNT % 8 != 0);

    // fstat se usa para obtener información sobre el archivo de bitmap
    struct stat st;
    if (fstat(bitmap_fd, &st) == -1) {
        perror("Error al obtener información del bitmap");
        exit(1);
    }

    // Si el tamaño del archivo es 0, se inicializa el tamaño del archivo a bitmap_size usando ftruncate
    if (st.st_size == 0) {
        if (ftruncate(bitmap_fd, bitmap_size) == -1) {
            perror("Error al inicializar el tamaño del bitmap");
            exit(1);
        }
    }

    // Mapear el archivo del bitmap a memoria
    //   - NULL: El sistema decide la dirección de inicio del mapeo
    //   - bitmap_size: El tamaño del mapeo
    //   - PROT_READ | PROT_WRITE: Permisos de lectura y escritura
    //   - MAP_SHARED: Los cambios en el mapeo se reflejan en el archivo 
    //   - bitmap_fd: Descriptor de archivo del bitmap
    //   - 0: Desplazamiento desde el inicio del archivo
    bitmap_data = mmap(NULL, bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, bitmap_fd, 0);
    if (bitmap_data == MAP_FAILED) {
        perror("Error al mapear el archivo del bitmap");
        close(bitmap_fd);
        exit(1);
    }

    // Crear el bitarray
    // se usa para crear el bitarray utilizando los datos mapeados en memoria. MSB_FIRST indica que el bit más significativo es el primero
    /* Ver si usar MSB O LSB */
    bitarray = bitarray_create_with_mode(bitmap_data, bitmap_size, MSB_FIRST);
    if (bitarray == NULL) {
        perror("Error al crear el bitarray");
        // munmap se usa para liberar la región de memoria mapeada
        //  - bitmap_data es el puntero a la región de memoria mapeada.
        //  - bitmap_size es el tamaño de la región de memoria a desmapear.
        munmap(bitmap_data, bitmap_size);

        // close se usa para cerrar el descriptor de archivo bitmap_fd una vez que ya no se necesita
        close(bitmap_fd);
        exit(1);
    }

    // Inicializar el bitarray si es un nuevo archivo
    /* Se repite el if, ver si puedo hacerlo de otra manera */
    if (st.st_size == 0) {
        // memset se usa para establecer todos los bits del bitarray a 0, marca todos los bloques del sistema de archivos como libres
        //  - bitarray->bitarray es el puntero a la región de memoria que representa el bitarray
        //  - 0 es el valor con el que se inicializa cada byte (todos los bits en 0)
        //  - bitmap_size es el tamaño total de la región de memoria a inicializar
        memset(bitarray->bitarray, 0, bitmap_size);

        // msync se usa para asegurarse de que los cambios realizados en la región de memoria mapeada se escriban en el archivo en disco
        //  - bitmap_data es el puntero a la región de memoria mapeada
        //  - bitmap_size es el tamaño de la región de memoria a sincronizar
        //  - MS_SYNC es una bandera que indica que la función debe bloquearse hasta que los cambios estén escritos en el disco
        msync(bitmap_data, bitmap_size, MS_SYNC);
    }

    close(bitmap_fd);

    log_info(entradasalida_logger, "Filesystem inicializado correctamente");
}

/* El de soporte me dijo que conviene tener un solo logger pero algo me dijo de los logs distintos - ver grabacion */
/* Capaz también me conviene poner un parametro como en el config */
void iniciar_logger(char* nombre_interfaz){

   /*  if (strcmp(nombre_interfaz, "generica") == 0) {
        generica_logger = log_create("generica.log", "INTERFAZ GENERICA", 1, LOG_LEVEL_INFO);
        // Ver de validar en cada uno que si no se pudo crear el log que tire un error
    } else if (strcmp(nombre_interfaz, "stdin") == 0) {
        stdin_logger = log_create("stdin.log", "INTERFAZ STDIN", 1, LOG_LEVEL_INFO);
    } else if (strcmp(nombre_interfaz, "stdout") == 0) {
        stdout_logger = log_create("stdout.log", "INTERFAZ STDOUT", 1, LOG_LEVEL_INFO);
    } else if (strcmp(nombre_interfaz, "dialfs") == 0) {
        dialfs_logger = log_create("dialfs.log", "INTERFAZ DIALFS", 1, LOG_LEVEL_INFO);
    } else {
        perror("Tipo de interfaz desconocida: %s\n", nombre_interfaz);
        exit(1);
    } */

    entradasalida_logger = log_create("entradasalida_logger.log", "ENTRADASALIDA", 1, LOG_LEVEL_INFO);
	if(entradasalida_logger == NULL){
		perror("No se pudo crear el logger.");
		exit(1);
	}

    entradasalida_logger_min_y_obl = log_create("entradasalida_min_y_obl.log", "ENTRADASALIDA", 1, LOG_LEVEL_INFO);
	if(entradasalida_logger_min_y_obl == NULL){
		perror("No se pudo crear el logger.");
		exit(1);
	}
}

void iniciar_config(char* nombre_interfaz, char* archivo_configuracion){
    char config_path[256]; // Le pongo ese tamaño para asegurarme que es lo suficientemente grande como para guardar la ruta del archivo
    sprintf(config_path, "/home/utnso/tp-2024-1c-GSN/entradasalida/%s", archivo_configuracion);
    entradasalida_config = config_create(config_path);
	if(entradasalida_config == NULL){
		perror("No se pudo crear el config.");
        perror(config_path);
		exit(2); // El exit(2) se utiliza para indicar que el programa terminó debido a un error relacionado con la creación del archivo de configuración (ver de ponerlo en los otros módulos también)
	}

    TIPO_INTERFAZ = config_get_string_value(entradasalida_config, "TIPO_INTERFAZ");
	TIEMPO_UNIDAD_TRABAJO = config_get_int_value(entradasalida_config, "TIEMPO_UNIDAD_TRABAJO");
	IP_KERNEL = config_get_string_value(entradasalida_config, "IP_KERNEL");
    PUERTO_KERNEL = config_get_string_value(entradasalida_config, "PUERTO_KERNEL");
    IP_MEMORIA = config_get_string_value(entradasalida_config, "IP_MEMORIA");
    PUERTO_MEMORIA = config_get_string_value(entradasalida_config, "PUERTO_MEMORIA");
    PATH_BASE_DIALFS = config_get_string_value(entradasalida_config, "PATH_BASE_DIALFS");
    BLOCK_SIZE = config_get_int_value(entradasalida_config, "BLOCK_SIZE");
    BLOCK_COUNT = config_get_int_value(entradasalida_config, "BLOCK_COUNT");
    RETRASO_COMPACTACION = config_get_int_value(entradasalida_config, "RETRASO_COMPACTACION");
}

// Logs obligatorios
/* Todos - Operación: "PID: <PID> - Operacion: <OPERACION_A_REALIZAR>"
DialFS - Crear Archivo: "PID: <PID> - Crear Archivo: <NOMBRE_ARCHIVO>"
DialFS - Eliminar Archivo: "PID: <PID> - Eliminar Archivo: <NOMBRE_ARCHIVO>"
DialFS - Truncar Archivo: "PID: <PID> - Truncar Archivo: <NOMBRE_ARCHIVO> - Tamaño: <TAMAÑO>"
DialFS - Leer Archivo: "PID: <PID> - Leer Archivo: <NOMBRE_ARCHIVO> - Tamaño a Leer: <TAMAÑO> - Puntero Archivo: <PUNTERO_ARCHIVO>"
DialFS - Escribir Archivo: "PID: <PID> - Escribir Archivo: <NOMBRE_ARCHIVO> - Tamaño a Escribir: <TAMAÑO> - Puntero Archivo: <PUNTERO_ARCHIVO>"
DialFS - Inicio Compactación: "PID: <PID> - Inicio Compactación."
DialFS - Fin Compactación: "PID: <PID> - Fin Compactación."
 */