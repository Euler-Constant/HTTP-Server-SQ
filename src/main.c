#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 4221
#define BUFFER 1024 * 4

int main() {

  // Disable output buffering:

  setbuf(stdout, NULL);
  int server_fd, client_addr_len;
  struct sockaddr_in client_addr;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    printf("Socket creation failed: %s...\n", strerror(errno));
    return 1;
  }

  // Since the tester restarts the program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors

  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
      0) {
    printf("SO_REUSEPORT failed: %s \n", strerror(errno));
    return 1;
  }

  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(PORT),
      .sin_addr = {htonl(INADDR_ANY)},
  };

  // Binding server address and port to socket:

  if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) {
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
  int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                         (socklen_t *)&client_addr_len);

  printf("Client connected\n");

  char *read_buffer[BUFFER];
  ssize_t status_read = recv(client_fd, read_buffer, BUFFER, 0);
  if (status_read == -1) {
    printf("Reciving data from client failed: %s \n", strerror(errno));
    return 1;
  }

  printf("Receieved %ld bytes from client\n", status_read);
  char *write_buffer = "HTTP/1.1 200 OK\r\n\r\n";
  if (strstr(read_buffer, "GET / ") == NULL)
    write_buffer = "HTTP/1.1 404 Not Found\r\n\r\n";
  ssize_t status_write = send(client_fd, write_buffer, strlen(write_buffer), 0);
  if (status_write == -1) {
    printf("Sending data to client failed: %s \n", strerror(errno));
    return 1;
  }

  printf("Sent %ld bytes to client\n", status_write);
  close(server_fd);

  return 0;
  
}