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
#define BACKLOG 5
#define MAXIMUM_SIZE 100
#define FLAG 0

int max(int a, int b){
	return (a>b)?a:b;
}

char* getIp(struct sockaddr_storage client_addr) {
    if(client_addr.ss_family == AF_INET) {
		struct sockaddr_in *inet = (struct sockaddr_in *)&client_addr;
		char ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(inet->sin_addr), ip, INET_ADDRSTRLEN);
		char *ip4 = (char *)malloc(strlen(ip)+1);
	    strcpy(ip4,ip);
	    return ip4;
	}
	else {
		struct sockaddr_in6 *inet6 = (struct sockaddr_in6 *)&client_addr;
		char ip[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, &(inet6->sin6_addr), ip, INET_ADDRSTRLEN);
		char *ip6 = (char *)malloc(strlen(ip)+1);
	    strcpy(ip6,ip);
	    return ip6;
	}
}

void fixmsg(char *msg) {
	msg = "HTTP....";
}

void sendError(char *p) {
	printf("The following error occurred during %s:\t", p);
	perror("");
}

int main(int argc, char const *argv[])
{
	int status, socketfd, total_msg_sent, bytes_recv, bytes_sent, yes = 1, len;
	
	socklen_t addr_size;
	
	char *msg;
	char *msg_to_be_sent;
	fixmsg(msg_to_be_sent);
	len = sizeof msg_to_be_sent;

	struct addrinfo hints;
	struct sockaddr_storage client_addr;
	struct addrinfo *servres;
	struct fd_set *all;
	struct fd_set *currently_all;
	struct timeval timeIsPrecious;
	timeIsPrecious.tv_usec = 1000;

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo( NULL, "8080", &hints, &servres );

	if(status) {
		perror("No IP found");
		exit(1);
	}

	struct addrinfo *serverInfo = servres;

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
	}
	if(serverInfo == NULL) {
		printf("Sorry No server available\n");
		exit(1);
	}
	if(listen(socketfd, BACKLOG) == -1) {sendError("listening"); exit(1); }
	FD_SET(socketfd, all);
	int fdmax = socketfd;
	while(true) {
		*currently_all = *all;
		//SERVER WILL SEND DATA WHEN IT RECV SOMETHING OR IT FINDS A NEW CONNECTION
		select(fdmax + 1, currently_all, 0, 0, timeIsPrecious);
		for (int i = 0; i <= fdmax; ++i)
		{
			if(FD_ISSET(i, currently_all)) {
				if(i == socketfd) {
					addr_size = sizeof client_addr;
					int new_fd = accept(socketfd, (struct sockaddr *)&client_addr, &addr_size);
					if(new_fd == -1) { sendError("new_fd"); continue; }
					FD_SET(new_fd, all);
					fdmax = max(fdmax, new_fd);
					printf("The following connection has been established:\t%s\n", getIp(client_addr));
					total_msg_sent = 0;
					while (total_msg_sent < len) {
						total_msg_sent += send(i, msg, len, FLAG);
					}
				}
				else {
					if((bytes_recv = recv(i, msg, MAXIMUM_SIZE, FLAG)) == 0) {
						if(i == fdmax) fdmax--;
						FD_CLR(i, all);
						printf("One Connection Lost\n");
						close(i);
					}
					else if(bytes_recv == -1) {
						sendError("Receiving");
					}
					else {
						printf("%s\n", msg);
						total_msg_sent = 0;
						while (total_msg_sent < len) {
							total_msg_sent += send(i, msg, len, FLAG);
						}
					}
				}
			}
		}
	}
}