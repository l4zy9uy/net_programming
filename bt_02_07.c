#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <linux/limits.h>
#include <dirent.h>

#define BUFFER_SIZE 8192

void send_directory(int client, const char* directory) {
    char response[BUFFER_SIZE];
    snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Directory listing: %s</h1><ul>", directory);
    send(client, response, strlen(response), 0);

 

    // Open the directory
    DIR* dir = opendir(directory);
    if (dir == NULL) {
        snprintf(response, BUFFER_SIZE, "Could not open directory: %s", directory);
        send(client, response, strlen(response), 0);
        return;
    }

 

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", directory, entry->d_name);

 

        // Check if it's a directory
        if (entry->d_type == DT_DIR) {
            snprintf(response, BUFFER_SIZE, "<li><b><a href=\"%s\">%s</a></b></li>", path, entry->d_name);
        }
        // Check if it's a file
        else if (entry->d_type == DT_REG) {
            snprintf(response, BUFFER_SIZE, "<li><i><a href=\"%s\">%s</a></i></li>", path, entry->d_name);
        } else {
            continue;
        }

 

        send(client, response, strlen(response), 0);
    }

 

    snprintf(response, BUFFER_SIZE, "</ul></body></html>");
    send(client, response, strlen(response), 0);

 

    closedir(dir);
}

 

// Function to send the content of a file as an HTML response
/*void send_file(int client, const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "Could not open file: %s", file_path);
        send(client, response, strlen(response), 0);
        return;
    }

 

    char buffer[BUFFER_SIZE];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
        send(client, buffer, read_bytes, 0);
    }

 

    fclose(file);
}
*/
void send_file(int client, const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    
    if (file == NULL) {
        char response[BUFFER_SIZE];
        snprintf(response, BUFFER_SIZE, "Could not open file: %s", file_path);
        send(client, response, strlen(response), 0);
        return;
    }

 

    // Get the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

 

    // Allocate memory to read the file
    char* buffer = (char*)malloc(file_size);
    if (buffer == NULL) {
        fclose(file);
        perror("Memory allocation failed");
        return;
    }

 

    // Read the file into the buffer
    size_t read_bytes = fread(buffer, 1, file_size, file);
    if (read_bytes != file_size) {
        fclose(file);
        free(buffer);
        perror("Reading file failed");
        return;
    }

    fclose(file);
    // Send the HTTP response headers
    char response[BUFFER_SIZE];
    if(strstr(file_path, ".jpeg") || strstr(file_path, ".jpg") || strstr(file_path, ".png"))
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\n\r\n", file_size);
    else if(strstr(file_path, ".txt") || strstr(file_path, ".c") || strstr(file_path, ".cpp"))
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nContent-Length: %ld\r\n\r\n", file_size);
    else if(strstr(file_path, ".mp3"))
        snprintf(response, BUFFER_SIZE, "HTTP/1.1 200 OK\r\nContent-Type: audio/mpeg\r\n\r\n");
    send(client, response, strlen(response), 0);

 

    // Send the image file content
    send(client, buffer, file_size, 0);

 

    // Free the buffer
    free(buffer);
}

void *client_thread(void *);

int main() 
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) 
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5)) 
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        int client = accept(listener, NULL, NULL);
        if (client == -1)
        {
            perror("accept() failed");
            continue;
        }
        printf("New client connected: %d\n", client);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, client_thread, &client);
        pthread_detach(thread_id);
    }
    
    close(listener);    

    return 0;
}

void *client_thread(void *param)
{
    int client = *(int *)param;
    char buf[2048];

    int ret = recv(client, buf, sizeof(buf), 0);
    buf[ret] = 0;
    puts(buf);
    
    char method[16], path[PATH_MAX];
    sscanf(buf, "GET /%s", path);

    if (strcmp(path, ".") == 0)
    {
        send_directory(client, ".");
    }
    else if (opendir(path) != NULL)
    {
        send_directory(client, path);
    }
    if (access(path, F_OK) == 0)
    {/*
        if(strstr(path, ".jpeg") || strstr(path, ".jpg") || strstr(path, ".png")){
            send_image(client, path);
        }
        else*/
            send_file(client, path);
    }
    else
    {
        send_directory(client, ".");
        //char *response = "HTTP/1.1 404 File not found\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Khong tim thay noi dung yeu cau.</h1></body></html>";
        //send(client, response, strlen(response), 0);    
    }
    
    close(client);
}