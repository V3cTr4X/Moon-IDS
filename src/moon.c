#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/socket.h>
#include <getopt.h>
#include <sys/statvfs.h>

#define TIMEOUT 1  // Tiempo de espera en segundos
#define BUFSIZE 1024

// Función para escanear un puerto
int scan_port(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return 0;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return 0;  // Puerto cerrado
    }

    close(sockfd);
    return 1;  // Puerto abierto
}

// Función para obtener el banner de un servicio HTTP
void get_http_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Enviar solicitud HTTP
    send(sockfd, request, strlen(request), 0);

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';

        // Eliminar saltos de línea y retorno de carro
        char *banner = buffer;
        for (int i = 0; i < len; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\r') {
                buffer[i] = ' ';  // Reemplazar los saltos de línea con un espacio en blanco
            }
        }

        // Imprimir la respuesta en una sola línea
        printf("%s\n", banner);

        // Aquí puedes enviarlo al receptor para que lo procese (a través de otro socket)
        // En este caso lo enviamos por la misma conexión, pero normalmente sería otro socket
        // Enviar el banner al receptor o guardarlo en la base de datos
        // Puedes agregar el código para enviar el banner al servidor receptor aquí
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define BUFSIZE 1024

// Función para obtener el banner de un servicio HTTPS
void get_https_banner(const char *ip, int port) {
    SSL_CTX *ctx;
    SSL *ssl;
    int sockfd;
    struct sockaddr_in server_addr;
    char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char buffer[BUFSIZE];

    // Inicializar OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Error al crear el contexto SSL");
        return;
    }

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        SSL_CTX_free(ctx);
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        SSL_CTX_free(ctx);
        return;
    }

    // Establecer la conexión SSL
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) != 1) {
        perror("Error en la conexión SSL");
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return;
    }

    // Enviar una solicitud HTTPS
    SSL_write(ssl, request, strlen(request));

    // Leer la respuesta del servidor
    int len = SSL_read(ssl, buffer, BUFSIZE - 1);
    if (len > 0) {
        buffer[len] = '\0';

        // Eliminar saltos de línea y retorno de carro
        for (int i = 0; i < len; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\r') {
                buffer[i] = ' ';  // Reemplazar los saltos de línea con un espacio en blanco
            }
        }

        // Imprimir la respuesta en una sola línea
        printf("%s\n", buffer);

        // Aquí puedes enviarlo al receptor para que lo procese (a través de otro socket)
        // En este caso lo enviamos por la misma conexión, pero normalmente sería otro socket
        // Enviar el banner al receptor o guardarlo en la base de datos
        // Puedes agregar el código para enviar el banner al servidor receptor aquí
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    // Limpiar
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
}


void get_ftp_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor (el banner)
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';

        // Aquí extraemos solo el banner relevante (ejemplo para FTP)
        // Asumimos que el banner está en la primera línea
        char *banner = strtok(buffer, "\n"); // Obtiene la primera línea (el banner)
        
        // Solo imprimimos el banner
        if (banner != NULL) {
            printf("%s\n", banner);
            
            // Aquí puedes enviarlo al receptor para que lo procese (a través de otro socket)
            // En este caso lo enviamos por la misma conexión, pero normalmente sería otro socket
            // Enviar el banner al receptor o guardarlo en la base de datos
            // Puedes agregar el código para enviar el banner al servidor receptor aquí
        }
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

void get_ssh_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Enviar un saludo SSH simple para obtener el banner
    send(sockfd, "SSH-1.99-OpenSSH_7.2p2 Ubuntu-4ubuntu2.8\r\n", 43, 0);

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';  // Asegurarse de que el buffer está correctamente terminado

        // Verificar que el banner realmente corresponde a un servicio SSH
        if (strstr(buffer, "SSH-") != NULL) {
            printf("Banner SSH recibido en puerto %d:\n%s\n", port, buffer);
        } else {
            printf("El banner recibido en puerto %d no es SSH, contenido:\n%s\n", port, buffer);
        }
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFSIZE 1024

// Función para obtener el banner de CUPS
void get_cups_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Enviar solicitud HTTP
    send(sockfd, request, strlen(request), 0);

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';

        // Eliminar saltos de línea y retorno de carro (convertirlos a espacios)
        for (int i = 0; i < len; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\r') {
                buffer[i] = ' ';  // Reemplazar los saltos de línea con un espacio en blanco
            }
        }

        // Imprimir la respuesta del banner en una sola línea
        printf("Banner CUPS recibido en puerto %d: %s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}


// Función para obtener el banner de LDAP
void get_ldap_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor (usualmente el banner de LDAP se encuentra en el inicio)
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner LDAP recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de TOR
void get_tor_banner(const char *ip, int port) {
    // Aquí, solo imprimimos el mensaje, ya que no estamos haciendo conexión.
    printf("TOR SOCKSv5 encontrado\n");
}

// Función para obtener el banner de Minecraft
void get_minecraft_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner Minecraft recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}


// Función para obtener el banner de MySQL
void get_mysql_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // MySQL responde con un "saludo" que contiene información sobre la versión
    // Enviar el paquete de saludo, siguiendo el protocolo binario de MySQL.
    char handshake[] = {0x0a, 0x00, 0x00, 0x00}; // Esto es una secuencia de bytes que MySQL espera
    send(sockfd, handshake, sizeof(handshake), 0);

    // Leer la respuesta del servidor MySQL (el banner)
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';  // Asegurarse de que el buffer es una cadena válida

        // Agregar "yes" al final del banner si se recibe alguna respuesta
        if (len > 0) {
            // Convertir el banner recibido a una cadena en formato legible
            printf("MySQL %d: %s yes\n", port, buffer);
        } else {
            printf("No se recibió un banner legible desde MySQL en el puerto %d\n", port);
        }
    } else {
        printf("No se pudo obtener el banner de MySQL en el puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de PostgreSQL
void get_postgresql_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Enviar el mensaje de saludo (StartupMessage) con protocolo 3.0
    unsigned char startup_message[] = {
        0x00, 0x00, 0x00, 0x10,  // Longitud del mensaje (16 bytes)
        0x00, 0x00, 0x00, 0x03,  // Tipo de mensaje: StartupMessage (0x03)
        0x00, 0x00, 0x00, 0x00,  // Versión del protocolo (0x00030000 -> 3.0)
        0x00, 0x00, 0x00, 0x00   // Base de datos vacío (si es necesario, puedes agregarla)
    };

    // Enviar el paquete de saludo de PostgreSQL (StartupMessage)
    send(sockfd, startup_message, sizeof(startup_message), 0);

    // Leer la respuesta del servidor PostgreSQL (el banner)
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';  // Asegurarse de que el buffer es una cadena válida
        printf("Banner PostgreSQL recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner de PostgreSQL en el puerto %d\n", port);
    }

    close(sockfd);
}

float get_cpu_usage() {
    FILE *fp;
    char buffer[128];
    long double a[4], b[4], load;

    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    fscanf(fp, "cpu %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
    fclose(fp);
    sleep(1); // Esperar un segundo para calcular diferencia

    fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error al abrir /proc/stat");
        return -1.0;
    }

    fscanf(fp, "cpu %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
    fclose(fp);

    load = ((b[0] + b[1] + b[2]) - (a[0] + a[1] + a[2])) /
           ((b[0] + b[1] + b[2] + b[3]) - (a[0] + a[1] + a[2] + a[3]));
    return load * 100.0;
}

float get_ram_usage() {
    FILE *fp;
    char buffer[128];
    int total_mem = 0, used_mem = 0;

    // Ejecutar el comando free -m y leer su salida
    fp = popen("free -m", "r");
    if (fp == NULL) {
        perror("Error al ejecutar el comando free");
        return -1.0;
    }

    // Leer y analizar la salida
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (strstr(buffer, "Mem:") != NULL) {
            sscanf(buffer, "Mem: %d %d", &total_mem, &used_mem);
            break;
        }
    }
    pclose(fp);

    if (total_mem > 0) {
        float ram_usage = ((float)used_mem / total_mem) * 100.0;
        printf("[DEBUG] Memoria total: %d MB, Memoria usada: %d MB, Uso de RAM: %.2f%%\n",
               total_mem, used_mem, ram_usage);
        return ram_usage;
    } else {
        fprintf(stderr, "[ERROR] No se pudo calcular el uso de RAM. Memoria total: %d, Memoria usada: %d\n",
                total_mem, used_mem);
        return -1.0;
    }
}

float get_disk_usage(const char *path) {
    struct statvfs stat;

    if (statvfs(path, &stat) != 0) {
        perror("Error al obtener estadísticas del sistema de archivos");
        return -1.0;
    }

    long total = stat.f_blocks * stat.f_frsize;
    long available = stat.f_bfree * stat.f_frsize;

    return ((float)(total - available) / total) * 100.0;
}

void get_logged_users() {
    FILE *fp;
    char buffer[256];

    fp = popen("who", "r");
    if (fp == NULL) {
        perror("Error al ejecutar who");
        return;
    }

    printf("Usuarios conectados:\n");
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    pclose(fp);
}

void get_system_banner() {
    printf("==== Banner del Sistema ====\n");

    // CPU Usage
    float cpu_usage = get_cpu_usage();
    if (cpu_usage >= 0) {
        printf("Uso del procesador: %.2f%%\n", cpu_usage);
    }

    // RAM Usage
    float ram_usage = get_ram_usage();
    if (ram_usage >= 0) {
        printf("Uso de RAM: %.2f%%\n", ram_usage);
    } else {
        printf("[ERROR] No se pudo calcular el uso de RAM.\n");
    }

    // Disk Usage
    float disk_usage = get_disk_usage("/");
    if (disk_usage >= 0) {
        printf("Uso de disco: %.2f%%\n", disk_usage);
    }

    // Logged users
    get_logged_users();
    printf("============================\n");
}

void scan_ports(const char *ip, int start_port, int end_port) {
    for (int port = start_port; port <= end_port; ++port) {
        if (scan_port(ip, port)) {
            printf("Puerto %d abierto en %s\n", port, ip);

            // Variable para controlar si ya se encontró un banner
            int banner_found = 0;

            // Intentar obtener el banner para puertos conocidos
            if (port == 80 && !banner_found) {
                get_http_banner(ip, port);  // HTTP
                banner_found = 1;  // Marcamos que ya se encontró un banner
            } else if (port == 443 && !banner_found) {
                get_https_banner(ip, port);  // HTTPS
                banner_found = 1;
            } else if (port == 21 && !banner_found) {
                get_ftp_banner(ip, port);  // FTP
                banner_found = 1;
            } else if (port == 22 && !banner_found) {
                get_ssh_banner(ip, port);  // SSH
                banner_found = 1;
            } else if (port == 631 && !banner_found) {
                get_cups_banner(ip, port);  // CUPS
                banner_found = 1;
            } else if (port == 389 && !banner_found) {
                get_ldap_banner(ip, port);  // LDAP
                banner_found = 1;
            } else if (port == 3306 && !banner_found) {
                get_mysql_banner(ip, port);  // MySQL
                banner_found = 1;
            } else if (port == 5432 && !banner_found) {
                get_postgresql_banner(ip, port);  // PostgreSQL
                banner_found = 1;
            } else if ((port == 9050 || port == 9001) && !banner_found) {
                get_tor_banner(ip, port);  // TOR
                banner_found = 1;
            } else if (port == 25565 && !banner_found) {
                get_minecraft_banner(ip, port);  // Minecraft
                banner_found = 1;
            } else {
                // Si no es un puerto conocido, intentamos con banners genéricos
                

                // Intentar obtener un banner HTTP (puede ser cualquier servicio web)
                if (!banner_found) {
                    get_http_banner(ip, port);
                    banner_found = 1;  // Marcamos que se ha encontrado un banner
                }

                // Intentar obtener un banner SSH (por si es un servidor SSH no estándar)
                if (!banner_found) {
                    get_ssh_banner(ip, port);
                    banner_found = 1;
                }
                
                // Intentar obtener un banner HTTPS (por si es un servidor HTTPS no estándar) 
                if (!banner_found) {
                    get_https_banner(ip, port);
                    banner_found = 1;
                }

                // Intentar obtener un banner MySQL (por si es un servidor MySQL no estándar)
                if (!banner_found) {
                    get_mysql_banner(ip, port);
                    banner_found = 1;
                }

                // Intentar obtener un banner PostgreSQL (por si es un servidor PostgreSQL no estándar)
                if (!banner_found) {
                    get_postgresql_banner(ip, port);
                    banner_found = 1;
                }

                // Intentar obtener un banner FTP (por si es un servidor FTP no estándar)
                if (!banner_found) {
                    get_ftp_banner(ip, port);
                    banner_found = 1;
                }
            }
        }
    }
}


void get_banner(const char *ip, int port) {
    char buffer[BUFSIZE];
    int sockfd;
    struct sockaddr_in server_addr;

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner recibido en puerto %d:\n%s\n", port, buffer);
    }

    close(sockfd);
}

void print_usage(const char *prog_name) {
    printf("Uso: %s -i <IP> -s <puerto_inicial> -e <puerto_final> -r <ip-server> -p <puerto-ip-server>\n", prog_name);
}

int main(int argc, char *argv[]) {
    const char *target_ip = NULL;
    const char *receiver_ip = NULL;
    int start_port = 0, end_port = 0, receiver_port = 0;
    int opt;
    int send_banners = 0, send_metrics = 0;

    // Parsear los argumentos de la línea de comandos
    while ((opt = getopt(argc, argv, "i:s:e:r:p:bm")) != -1) {
        switch (opt) {
            case 'i':
                target_ip = optarg;
                break;
            case 's':
                start_port = atoi(optarg);
                break;
            case 'e':
                end_port = atoi(optarg);
                break;
            case 'r':
                receiver_ip = optarg;
                break;
            case 'p':
                receiver_port = atoi(optarg);
                break;
            case 'b':
                send_banners = 1;  // Enviar banners
                break;
            case 'm':
                send_metrics = 1;  // Enviar métricas
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Verificar que todos los argumentos necesarios fueron proporcionados
    if (target_ip == NULL || start_port <= 0 || end_port <= 0 || receiver_ip == NULL || receiver_port <= 0) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Asegurarse de que se ha especificado una opción válida (banners o métricas)
    if (!send_banners && !send_metrics) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    // Crear un socket para enviar los datos
    int send_sockfd;
    struct sockaddr_in send_addr;
    send_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (send_sockfd < 0) {
        perror("Error al crear el socket de envío");
        return EXIT_FAILURE;
    }

    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(receiver_port);
    send_addr.sin_addr.s_addr = inet_addr(receiver_ip);

    if (connect(send_sockfd, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0) {
        perror("Error al conectar al receptor");
        close(send_sockfd);
        return EXIT_FAILURE;
    }

    // Redirigir stdout al socket
    FILE *send_stream = fdopen(send_sockfd, "w");
    if (send_stream == NULL) {
        perror("Error al redirigir stdout");
        close(send_sockfd);
        return EXIT_FAILURE;
    }

    // Guardar el stdout original
    int stdout_fd = dup(STDOUT_FILENO);
    if (stdout_fd < 0) {
        perror("Error al duplicar stdout");
        fclose(send_stream);
        return EXIT_FAILURE;
    }

    // Redirigir stdout al socket
    if (dup2(fileno(send_stream), STDOUT_FILENO) < 0) {
        perror("Error al redirigir stdout al socket");
        fclose(send_stream);
        return EXIT_FAILURE;
    }

    // Enviar los datos según el tipo especificado
    if (send_banners) {
        scan_ports(target_ip, start_port, end_port);  // Enviar banners
    } else if (send_metrics) {
        get_system_banner();  // Enviar métricas del sistema
    }

    // Restaurar stdout original
    fflush(stdout);
    dup2(stdout_fd, STDOUT_FILENO);
    close(stdout_fd);
    fclose(send_stream);
    close(send_sockfd);

    return 0;
}




