/* 
 * Author: Chi Zhang (czhang2@scu.edu)
 * File name: client.c
 * Description: The file builds the server side of TCP (Transmission Control Protocol). As the client
 * sends over a txt file, The server receives data in chunk of 10 bytes and writes in chunk of 5 bytes.
 *
 * Referencer:
 * Socket Programming in C
 * http://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c
 * http://stackoverflow.com/questions/9840629/create-a-file-if-one-doesnt-exist-c
 * http://www.studytonight.com/c/file-input-output.php
 * http://stackoverflow.com/questions/5850000/how-to-split-array-into-two-arrays-in-c
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



#define BACKLOG 10 // how many pending connections queue will hold

#define CHUNK 10 /* read 10 bytes at a time */


int main(int argc, char *argv[]) {

	// set up variables

	int port_num;
	char newfile_name[20];
	char ip_addr[INET_ADDRSTRLEN];
	
	int accept_sock, new_sock; 
	struct sockaddr_in sock_addr;
	socklen_t size; 

	char buf[CHUNK + 1];
	FILE *newfile;


	// examine the user input (only need a port here)

	if (argc < 2) {
		printf("ERROR: no port number input\n");
		exit(1);
	}
	else if (argc != 2) {
		printf("ERROR: wrong input\n");
		exit(1);
	}
	else {
		port_num = atoi(argv[1]);
	}



	// create a new socket

	new_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (new_sock < 0) {
		printf("ERROR: failed to create a client socket\n");
		exit(1);
	}

	printf("\nConnection created...");


	// set up server_addr values

	inet_ntop(AF_INET, &sock_addr, ip_addr, INET_ADDRSTRLEN); // get the current ip address

	bzero(&sock_addr, sizeof sock_addr);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = inet_addr(ip_addr);
	sock_addr.sin_port = htons(port_num);
	memset(sock_addr.sin_zero, '\0', sizeof sock_addr.sin_zero);  



	// bind

	if (bind(new_sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
		printf("ERROR: failed binding\n");
		exit(1);
	}

	printf("\nBinding success\n");



	// listen

	if(listen(new_sock, BACKLOG) < 0 )
 	{  
 		printf("ERROR: failed listening\n");
       	exit(1);
    }

	printf("Listening success\n");



	// accept

	size = sizeof sock_addr;

	accept_sock = accept(new_sock, (struct sockaddr *)&sock_addr, &size);

	if (accept_sock < 0) {
		printf("\nERROR: failed accepting");
		exit(1);
	}
	else {
		printf("\nAccepting success\n");
	}



	// receive the file

	// receive the newfile name

	bzero(newfile_name, sizeof newfile_name);

	if (recv(accept_sock, newfile_name, sizeof(newfile_name), 0) > 0) {
		printf("New file name received!\n");
		//printf("%s\n", newfile_name);
	} 
	else {
		printf("File name receive error.\n");
		exit(1);
	}


	// create a new empty file ????????????

	newfile = fopen(newfile_name, "wb+");
	if (newfile == NULL) {
		newfile = fopen(newfile_name, "wb+");
	}
	

	// receive msg from the client and write to the new file

	char buf_wri[5];

	bzero(buf, sizeof buf);

	while (recv(accept_sock, buf, sizeof(buf)-1, 0) > 0) {

		// write the file in 5 byte chunks
		int i, d, c;
		for (i = 0; i < 2; i++) {
			bzero(buf_wri, sizeof buf_wri);
			d = i * 5;
			memcpy(buf_wri, &buf[d], 5 * sizeof(char));
			fwrite(buf_wri, sizeof(char), sizeof buf_wri, newfile);
		}

		// printf("%s\n", buf);
		bzero(buf, sizeof buf);
	}


	printf("File transfer finished, close the server.\n");


	fclose(newfile); 

	close(new_sock);

	return 0;

}













