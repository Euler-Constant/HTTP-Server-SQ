#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

//Function to parse the request line and extract the URL (request target)
char* extract_url(const char* request) {
	char* request_copy = strdup(request);
	if (!request_copy) {
		fprintf(stderr, "MEMORY ALLOCATION FAILED: %s\n", strerror(errno));
		return NULL;
	}

	//Splitting request line:
	char* method = strtok(request_copy, " ");
	if (!method) {
		free(request_copy);
		return NULL;
	}

	//The URL is the second token, I believe:
	char* url = strtok(NULL, " ");
	if (!url) {
		free(request_copy);
		return NULL;
	}

	//Returning URL by duplication:
	char* result = strdup(url);
	free(request_copy);
	if(!result) {
		fprintf(stderr, "MEMORY ALLOCATION FAILED: %s\n", strerror(errno));
	}
	return result;
}

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

 	int server_fd, client_addr_len;
 	struct sockaddr_in client_addr;

 	server_fd = socket(AF_INET, SOCK_STREAM, 0);
 	if (server_fd == -1) {
 		printf("Socket creation failed: %s...\n", strerror(errno));
 		return 1;
 	}

 	// Since the tester restarts the program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors

	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(4221),
									 .sin_addr = { htonl(INADDR_ANY) },
									};

	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t*)&client_addr_len);
    if (client_fd < 0) {
        printf("Accept failed: %s \n", strerror(errno));
        close(server_fd);
        return 1;
    }

    printf("Client connected\n");

    // Buffer to hold the incoming request
    char buffer[1024] = {0};
    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read < 0) {
        fprintf(stderr, "Receive failed: %s\n", strerror(errno));
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // Null-terminate the buffer
    buffer[bytes_read] = '\0';

    // Extract the URL from the request
    char* url = extract_url(buffer);
    if (url) {
        printf("Extracted URL: %s\n", url);
        
        // Example: Respond based on the URL
        if (strcmp(url, "/") == 0) {
            const char* response = "HTTP/1.1 200 OK\r\n\r\n";
            send(client_fd, response, strlen(response), 0);
        } else {
            const char* response = "HTTP/1.1 404 Not Found\r\n\r\n";
            send(client_fd, response, strlen(response), 0);
        }
        free(url);
    } else {
        // Handle malformed request
        const char* response = "HTTP/1.1 400 Bad Request\r\n\r\n";
        send(client_fd, response, strlen(response), 0);
    }

    close(client_fd);
    close(server_fd);
    
    return 0;
}
