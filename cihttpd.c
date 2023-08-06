#include "http.h"

int sockfd;

void closingTime(int signum)
{
	printf("\nReceived SIGINT, shutting down...\n");
	close(sockfd);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	// catch ctrl-C
	signal(SIGINT, closingTime);


	// create socket
	int client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    
    // Create server socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return -1;
    }
    
    // Bind server socket to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return -1;
    }
    
    // Listen for connections
    if (listen(sockfd, PORT) < 0) {
        perror("listen");
        close(sockfd);
        return -1;
    }
    
    printf("Server listening on port %d...\n", PORT);
    
    // Accept connections and handle clients
    for (;;) {
        client_addr_len = sizeof(client_addr);
        client_socket = accept(sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("accept");
            close(sockfd);
            return -1;
        }
        
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(client_socket);
            continue;
        } else if (pid == 0) {
            // child process
            close(sockfd);
            parse_request(client_socket);
            exit(EXIT_SUCCESS);
        } else {
            // parent process
            close(client_socket);
        }
    }
    
    // Close server socket
    close(sockfd);
    return 0;
}

void parse_request(int client_socket){
    char buffer[BUFSIZ];
    ssize_t bytes_read;

    //receive request
    bytes_read = recv(client_socket, buffer, BUFSIZ - 1, 0);
    if (bytes_read < 0)          
    {
        perror("recv");
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';
    char *request = strtok(buffer, " \r\n");
    char *path = strtok(NULL, " \r\n");
    char *http_version = strtok(NULL, " \r\n");

    if (path[0] == '/') {
    path++;
    }
   

    if (strcmp(request, "GET") == 0){
        handle_get_request(path, client_socket);
    }
    else if (strcmp(request, "HEAD") == 0)
    {
        handle_head_request(path, client_socket);
    }
    else {
        handle_501(client_socket);
    }

    close(client_socket);
}


void handle_get_request(char *path, int client_socket){
	//TODO
    //printf("GET request received\n");
    
    if (path == NULL) {
        path = "index.html";
    }
    handle_head_request(path, client_socket);
    FILE* file = fopen(path, "r");
    if(file == NULL){
        return;
    }
        
    char buffer[MAX_BUF];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, MAX_BUF, file)) > 0) {
        send(client_socket, buffer, bytes_read, 0);
    }
    fclose(file);
}
void handle_head_request(char *path, int client_socket){
    
    FILE* file = fopen(path, "r");
    if(file == NULL){
        handle_404(client_socket);
        return;
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    struct stat last_modified_stat;
    if (stat(path, &last_modified_stat) == -1){
        perror("stat");
        exit(1);
    }
    char modified_time_str[128];
    format_time_string(modified_time_str, sizeof(modified_time_str), &last_modified_stat.st_mtime);
    time_t t = time(NULL);
    struct tm tm = *gmtime(&t);

    char current_time[128];
    format_time_string(current_time, sizeof(current_time), &t);

    char header[MAX_BUF];
    sprintf(header, "HTTP/1.1 200 OK\r\n\
    Date: %s\r\n\
    Server: Cihttpd\r\n\
    Last-Modified: %s\r\n\
    Content-Length: %d\r\n\r\n", current_time, modified_time_str, file_size);

    send(client_socket, header, strlen(header), 0);
    fclose(file);
}
void handle_404(int client_socket){
	//TODO
    FILE* file = fopen("404.html", "r");
	if (file == NULL){
		perror("fopen");
		return;
	}

	// Read the contents of the file
	char buffer[MAX_BUF];
	size_t bytes_read;
	while ((bytes_read = fread(buffer, 1, MAX_BUF, file)) > 0) {
		send(client_socket, buffer, bytes_read, 0);
	}

	fclose(file);
}
void handle_501(int client_socket){
	//TODO
    FILE* file = fopen("501.html", "r");
	if (file == NULL){
		perror("fopen");
		return;
	}

	// Read the contents of the file
	char buffer[MAX_BUF];
	size_t bytes_read;
	while ((bytes_read = fread(buffer, 1, MAX_BUF, file)) > 0) {
		send(client_socket, buffer, bytes_read, 0);
	}

	fclose(file); 
}
char format_time_string(char* time_str, size_t size, time_t* time){
    struct tm tm = *gmtime(time);
    strftime(time_str, size, "%a, %d %b %Y %H:%M:%S GMT", &tm);
}