/* 
 * Author: Chi Zhang (czhang2@scu.edu)
 * File name: client.c
 * Description: The file builds the client side of TCP (Transmission Control Protocol). The client reads a text
 * file and sends the file content to the server in chunks of 10 bytes. 
 *
 * Referencer:
 * Socket Programming in C
 * https://docs.oracle.com/cd/E19455-01/806-1017/6jab5di2e/index.html
 * http://stackoverflow.com/questions/10527187/reading-and-writing-in-chunks-on-linux-using-c
 * http://stackoverflow.com/questions/13837868/getting-or-symbol-when-reading-from-text-file-with-fread
 * 
 *
 */



#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>

#define CHUNK 10 /* read 10 bytes at a time */


int main(int argc, char *argv[]) {

	// set up variables

	// input variables
	char *oldfile_name;
	char *newfile_name;
	char *ip_addr;
	int port_num;

	// socket variable
	int des_sock; 
	struct sockaddr_in sock_addr;
	socklen_t size;

	// file reading variables
	FILE *oldfile;
	char buf[CHUNK+1];


	// examine the use input

	if (argc < 2) {
		printf("ERROR: no input\n");
		exit(1);
	}
	else if (argc != 5) {
		printf("ERROR: wrong input\n");
		exit(1);
	}
	else {
		printf("Input received\n");
		oldfile_name = argv[1];
		newfile_name = argv[2];
		ip_addr = argv[3];
		port_num = atoi(argv[4]);
	}


	// create a socket that connects to the server

	des_sock = socket(AF_INET, SOCK_STREAM, 0);


	if (des_sock < 0) {
		printf("ERROR: failed creating the socket\n");
		close(des_sock);
		exit(1);
	}


	printf("Client socket created\n");


	bzero(&sock_addr, sizeof sock_addr);
	sock_addr.sin_family = AF_INET; 
	sock_addr.sin_port = htons(port_num);
	sock_addr.sin_addr.s_addr = inet_addr(ip_addr);
	memset(sock_addr.sin_zero, '\0', sizeof sock_addr.sin_zero);  

	//inet_pton(AF_INET, ip, &server_addr.sin_addr);

	if (connect(des_sock, (struct sockaddr *)&sock_addr, sizeof sock_addr) != 0) {
		printf("\nERROR: failed connecting to the server\n");
		close(des_sock);
		exit(1);
	} else {
		printf("\n************************");
		printf("\nConnecting to the server");
		printf("\n************************");
	}


	// send over the new file name

	printf("%s\n", newfile_name);
	if (send(des_sock, newfile_name, 20, 0) < 0) {
		printf("Error in sending the new file name\n");
    	exit(1);
	}

	printf("New file name sent\n");



	printf("\nRead file...\n");

	oldfile = fopen(oldfile_name, "rb");

	if (oldfile == NULL) {
		printf("Error in opening the file\n");
		close(des_sock);
		exit(1);
	}


	// reading the file in chunks and send it over to the server

	bzero(buf, sizeof buf);
	while (fread(buf, 1, sizeof buf - 1, oldfile) > 0) {
		//printf("%s\n", buf);
    	if (send(des_sock, buf, sizeof(buf) - 1, 0) < 0) {
    		printf("Error in sending the file\n");
    		exit(1);
    	}
    	bzero(buf, sizeof buf);
	}


	printf("Finish reading file, close the socket\n");

	fclose(oldfile);

	close(des_sock);

	return 0;

}