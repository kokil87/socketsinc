#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BACKLOG 5
#define MAXIMUM_SIZE 100
#define FLAG 0
#define PACKET_SIZE 2048

size_t len, error_len;
char msg_to_be_sent[] = "HTTP/1.0 200 OK\nContent-Type: text/html; charset=UTF-8\n\n";
char error_msg[] = "HTTP/1.0 404 Not Found\n";

char* getIp(struct sockaddr_in *client_addr) {
	char ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client_addr->sin_addr), ip, INET_ADDRSTRLEN);
	char *ip4 = (char *)malloc(strlen(ip)+1);
    strcpy(ip4,ip);
    return ip4;
}

void sendError(char *p) {
	printf("The following error occurred during %s:\t", p);
	perror("");
}

int getSocketForBinding(const char *PORT){
	int status, socketfd, yes = 1;

	struct addrinfo hints;
	struct addrinfo *servres;
	struct addrinfo *serverInfo;
	
	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo( NULL, PORT, &hints, &servres );

	if(status) {
		perror("No IP found");
		exit(1);
	}

	serverInfo = servres;

	for (; serverInfo != NULL; serverInfo = serverInfo->ai_next)
	{
		socketfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
		if(socketfd == -1) {
			sendError("Getting new socket server");
			continue;
		}
		if(setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
			sendError("Clearing of socket"); continue;
		}
		if(bind(socketfd, (struct sockaddr *)serverInfo->ai_addr, sizeof *(serverInfo->ai_addr)) == -1) {
			sendError("binding"); close(socketfd); 
		}
		else{
			break;
		}
	}
	if(serverInfo == NULL) {
		printf("Sorry No server available\n");
		exit(1);
	}
	freeaddrinfo(servres);
	if(listen(socketfd, BACKLOG) == -1) {sendError("listening"); exit(1); }
	return socketfd;	
}

void handleClient(int new_fd) {
	int bytes, total_msg_sent = 0;
	int fildes;
	char packet[PACKET_SIZE];
	char *message;
	message = malloc(10000);
	int r = recv(new_fd, message, 9999,0);
	if(r>0){
		char* token;
		token = strtok(message, " ");
		token = strtok(NULL, " ");
		char* file;
		file = malloc(1000);
		if(strcmp("/", token) == 0){
			strcpy(file ,"public/index.html");
		}
		else if(strcmp("/sample", token) == 0){
			strcpy(file , "public/sample.html");
		}
		else {
			strcpy(file , "0");
		}
		fildes = open(file, O_RDONLY);
		if(fildes == -1) {
			perror("Error due to :");
			write(new_fd, error_msg, error_len);
			return;
		}
		send(new_fd, msg_to_be_sent, len, FLAG);
		while((bytes = read(fildes, packet, PACKET_SIZE)) > 0){
			write(new_fd, packet, bytes);
		}
	}
	printf("closing connection\n");
}

int main(int argc, char const *argv[])
{
	int listener, bytes_recv, clientfd;
	socklen_t addr_size;
	struct sockaddr_storage client_addr;

	char *msg;

	len = sizeof msg_to_be_sent;
	error_len = sizeof error_msg;

	fd_set all;
	fd_set currently_all;

	if(argc <= 1){
		printf("Usage : ./testserver PORT_NUMBER\n");
		exit(1);
	}

	listener = getSocketForBinding(argv[1]);

	FD_SET(listener, &(all));
	int fdmax = listener;

	while(1) {

		currently_all = all;

		int res = select(fdmax + 1, &currently_all, NULL, NULL, NULL);
		for (int i = fdmax; i >= 0; --i)
		{
			if(FD_ISSET(i, &currently_all)) {
				if(i == listener) {
					int s = (int)len;
					addr_size = sizeof client_addr;
					int new_fd = accept(listener, (struct sockaddr *)&client_addr, &addr_size);
					if(new_fd == -1) { sendError("new_fd"); continue; }
					else {
						printf("The following connection has been established:\t%s\n", getIp((struct sockaddr_in*)&client_addr));
						handleClient(new_fd);
					}
					close(new_fd);
				}
			}
		}
	}
}
