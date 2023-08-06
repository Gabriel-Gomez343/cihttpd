#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#define PORT 80
#define MAX_BUF 1024
#define BACKLOG 5

void parse_request(int client_socket);
void handle_get_request(char *path, int client_socket);
void handle_head_request(char *path, int client_socket);
void handle_404(int client_socket);
void handle_501(int client_socket);
char format_time_string(char* time_str, size_t size, time_t* time);




