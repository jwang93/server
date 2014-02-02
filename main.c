#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>

int server(uint16_t port);
int client(const char * addr, uint16_t port);

#define MAX_MSG_LENGTH (1300)
#define MAX_BACK_LOG (5)
#define MAX_CLIENTS (1) 


int main(int argc, char ** argv)
{
	if (argc < 3) {
		printf("usage: myprog c <port> <address> or myprog s <port>\n");
		return 0;
	}

	uint16_t port = atoi(argv[2]);
	if (port < 1024) {
		fprintf(stderr, "port number should be equal to or larger than 1024\n");
		return 0;
	}
	if (argv[1][0] == 'c') {
		return client(argv[3], port);
	} else if (argv[1][0] == 's') {
		return server(port);
	} else {
		fprintf(stderr, "unkonwn command type %s\n", argv[1]);
		return 0;
	}
	return 0;
}

int client(const char * addr, uint16_t port)
{
	int sock;
	struct sockaddr_in server_addr;
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];

	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, IPPROTO_TCP)) < 0) {
		perror("Create socket error (client):");
		return 1;
	}

	printf("Client Socket created\n");
	server_addr.sin_addr.s_addr = inet_addr(addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
		perror("Connect error:");
		return 1;
	}

	printf("Connected to server %s:%d\n", addr, port);

	while (1) {
		printf("Enter message: \n");
		scanf("%s", msg);

		if (send(sock, msg, strnlen(msg, MAX_MSG_LENGTH), 0) < 0) {
			perror("Send error:");
			return 1;
		}
		int recv_len = 0;
		if ((recv_len = recv(sock, reply, MAX_MSG_LENGTH, 0)) < 0) {
			perror("Recv error:");
			return 1;
		}
		reply[recv_len] = 0;
		printf("Server reply:\n%s\n", reply);
	}
	return 0;
}

int server(uint16_t port) 
{
	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr; 
	char msg[MAX_MSG_LENGTH], reply[MAX_MSG_LENGTH];
	int sock, s;
	int len;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;


	// not sure how to set the client address? 

	if ((sock = socket(AF_INET, SOCK_STREAM/* use tcp */, IPPROTO_TCP)) < 0) {
		perror("Create socket error (server):");
		return 1;
	}
	printf("Socket created!\n");

	if ((bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
		perror("Error binding socket to client address in server");
		exit(1);
	}
	printf("Bind successful!\n");
	
	if (listen(sock, MAX_CLIENTS) < 0) {
		perror("Error setting up listen");	
	} //does this only allow one client? 

	if ((s = accept(sock, (struct sockaddr *)&client_addr, &len)) < 0) {
		perror("Error w/ server accepting connection");
		exit(1);				
	}

	while (1) {
		len = sizeof(client_addr);
		int recv_len = recv(s, reply, MAX_MSG_LENGTH, 0);

		if (recv_len <= 0) {
			if (recv_len == 0) {
				printf("Client disconnected\n");
				if ((s = accept(sock, (struct sockaddr *)&client_addr, &len)) < 0) {
					perror("Error w/ server accepting connection");
					exit(1);				
				}
			} else {
				perror("Recv error:");
				return 1;				
			}
		} else {
			printf("Received message from client: %s\n", reply);
		}


		if (send(s, reply, strnlen(reply, MAX_MSG_LENGTH), 0) < 0) {
			perror("Send error:");
			return 1;
		}
		memset(&reply[0], 0, sizeof(reply));
	}

	return 0;
}
