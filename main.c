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

/* Jay Wang, Harris Osserman */ 
/* Friday, Feb 2014 */
/* CS 356 Computer Network Architecture */ 

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
	int server_socket;
	int new_socket;
	int len;

	/* Configuring the server_addr */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	/* Creating the server socket using SOCK_STREAM and TCP PROTOCOL */
	if ((server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Create socket error (server):");
		return 1;
	}
	printf("Server's Socket created!\n");

	/* Binding the socket to the server's address */
	if ((bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0) {
		perror("Error binding socket to client address in server");
		exit(1);
	}
	printf("Server's Bind successful!\n");
	
	/* Allowing the server to listen to a max of MAX_CLIENTS = 1 */
	if (listen(server_socket, MAX_CLIENTS) < 0) {
		perror("Error setting up listen");
	}

	memset(&client_addr.sin_addr, 0, sizeof(client_addr.sin_addr));
	len = sizeof(client_addr);

	/* Listening for a connection from a client */
	if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &len)) < 0) {
		perror("Error w/ server accepting connection");
		exit(1);				
	} else {
		printf("Accepted connection from: %s\n", inet_ntoa(client_addr.sin_addr));
	}

	while (1) {
		len = sizeof(client_addr);

		/* recv() is the method that gets the input from the client, we store return value so we know the status */
		int recv_len = recv(new_socket, reply, MAX_MSG_LENGTH, 0);

		if (recv_len <= 0) { /* Only called when client receives error or client disconnected */
			if (recv_len == 0) {
				printf("Client disconnected\n");
				/* Upon client disconnecting, server waits to accept the next client */
				if ((new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &len)) < 0) {
					perror("Error w/ server accepting connection");
					exit(1);				
				} else {
					printf("Accepted connection from: %s\n", inet_ntoa(client_addr.sin_addr));
				}
			} else {
				perror("Recv error:");
				return 1;				
			}
		} else { 
			/* This is the case when recv_len > 0, which signals the server received a valid reply from client */
			printf("Received message from client: %s\n", reply);
		}

		/* This is part of the "echo" server where server returns the client's message */
		if (send(new_socket, reply, strnlen(reply, MAX_MSG_LENGTH), 0) < 0) {
			perror("Send error:");
			return 1;
		}

		/* Clearing the reply buffer with 0's for the next reply */
		memset(&reply[0], 0, sizeof(reply));
	}

	return 0;
}
