#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

int connect_inet(char *host, char *service)
{
    struct addrinfo hints, *info_list, *info;
    int sock, error;
    char addr_buf[64];

    // look up remote host
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;  // in practice, this means give us IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // indicate we want a streaming socket

    error = getaddrinfo(host, service, &hints, &info_list);
    if (error) {
        fprintf(stderr, "error looking up %s:%s: %s\n", host, service, gai_strerror(error));
        return -1;
    }

    for (info = info_list; info != NULL; info = info->ai_next) {
	    if (info->ai_family != AF_INET)
		    continue;
	    inet_ntop(AF_INET, &((struct sockaddr_in *)info->ai_addr)->sin_addr, addr_buf, sizeof(addr_buf));
	    printf("IP: %s\n", addr_buf);
	    sock = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	    if (sock < 0) continue;

	    error = connect(sock, info->ai_addr, info->ai_addrlen);
	    if (error) {
		    close(sock);
		    continue;
	    }

	    break;
    }
    freeaddrinfo(info_list);

    if (info == NULL) {
        fprintf(stderr, "Unable to connect to %s:%s\n", host, service);
        return -1;
    }

    return sock;
}



#define BUFLEN 256

int main(int argc, char **argv)
{
    int sock, bytes;
    char buf[BUFLEN];
    char message[BUFLEN];
    int batch = 4;

    if (argc != 3) {
        printf("Specify host and service\n");
        exit(EXIT_FAILURE);
    }

    sock = connect_inet(argv[1], argv[2]);
    if (sock < 0) exit(EXIT_FAILURE);

    setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &batch, sizeof(batch));
    while (1) {
        int fd = read(sock, buf, BUFLEN);
        fprintf(stdout, "%s\n", buf);
	fflush(stdout);
        fgets(message, BUFLEN, stdin);

        bytes = strlen(message);

        write(sock, message, bytes);

        if (strcmp(message, "exit\n") == 0) {
             break;
        }
    }
    close(sock);

    return EXIT_SUCCESS;
}
