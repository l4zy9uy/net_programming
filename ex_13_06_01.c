#include<stdio.h>
#include <unistd.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include <arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>
#include <dirent.h>
#include <ctype.h>

int checkFileExists(const char* filename) {
    DIR* dir;
    struct dirent* entry;

    // Open the directory
    dir = opendir("path/to/directory");
    if (dir == NULL) {
        printf("Unable to open the directory.\n");
        return -1;
    }

    // Check if the file exists
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, filename) == 0) {
            closedir(dir);
            return 1;  // File exists
        }
    }

    closedir(dir);
    return 0;  // File does not exist
}

int sendFileContents(int client_socket, const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen failed");
        return -1;
    }

    fseek(file, 0L, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    char msg_file_size[16];

    sprintf(msg_file_size, "OK %d\r\n", file_size);
    // Send file size to the client
    send(client_socket, msg_file_size, strlen(msg_file_size), 0);

    char buffer[256];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), file)) > 0) {
        if (send(client_socket, buffer, bytesRead, 0) != bytesRead) {
            perror("send failed");
            fclose(file);
            return -1;
        }
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[])
{
	int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(listener == -1){
		perror("connect() failed");
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9000);

	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr))){
		perror("bind () failed");
		return 1;
	}
	if(listen(listener, 5)){
		perror("listen() failed");
		return 1;
	}

	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(client_addr);

	int client = accept(listener, (struct sockaddr *)&client_addr, &client_addr_len);

	printf("Accepted socket %d from IP: %s:%d\n", 
        client,
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port));

    DIR *dir;
    struct dirent *entry;

    // Open the directory
    dir = opendir("/home/l4zy9uy/Desktop/net_programming/");

    if (dir == NULL) {
        printf("Unable to open the directory.\n");
        return 1;
    }

    int ret, count = 0;
    char buf[256], counting_files[16], file_names[64][256];

    while ((entry = readdir(dir)) != NULL) {
        // Ignore directories and special entries
        if (entry->d_type == DT_REG) {
            // Copy file name to the array
            snprintf(file_names[count], sizeof(file_names[count]), "%s\r\n", entry->d_name);
            count++;
        }
    }
    
    if(count == 0){
        char *msg = "ERROR NO files to download\n";
        send(client, msg, strlen(msg), 0);
    }
    else{
        sprintf(counting_files, "OK %d\r\n", count);
        send(client, counting_files, strlen(counting_files), 0);

        for (int i = 0; i < count; i++) {
            send(client, file_names[i], strlen(file_names[i]), 0);
        }
    }

    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        char* trimmedFilename = buf + strlen(buf) - 1;
        while (trimmedFilename >= buf && isspace((unsigned char)*trimmedFilename)) {
            *trimmedFilename-- = '\0';
        }
        int fileExists = checkFileExists(buf);
        if(fileExists){
            sendFileContents(client, buf);
            close(client);
            exit(0);
        }
        else{
            char *msg = "Khong ton tai file.\n";
            send(client, msg, strlen(msg), 0);
        }
    }

    closedir(dir);
    close(client);
    close(listener);


}