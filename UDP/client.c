/* 
 * Author: Chi Zhang (czhang2@scu.edu)
 * File name: client.c
 * Description: The file builds the client side of UDP (User Datagram Protocol). The client reads a text
 * file and sends the file content to the server in chunks of 10 bytes. After every chunk is sent, the client
 * waits for an acknowledgement from the server. If the ackowledge received is not correct, the client repeats
 * sending the chunk until it's correct.
 *
 * Random functions are used to create the scenarios where the data is lost, erred, or duplicated
 * in order to test the program's ability to resolve these situations and sends a correct file.
 *
 * Referencer:
 * Socket Programming in C
 * https://docs.oracle.com/cd/E19455-01/806-1017/6jab5di2e/index.html
 * http://stackoverflow.com/questions/10527187/reading-and-writing-in-chunks-on-linux-using-c
 * http://stackoverflow.com/questions/13837868/getting-or-symbol-when-reading-from-text-file-with-fread
 * http://stackoverflow.com/questions/13547721/udp-socket-set-timeout
 *
 */



#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>


#define CHUNK 10 /* read 10 bytes at a time */

typedef struct udp_pack
{
	int seq_num;
	short checksum;
	char data[CHUNK];
}udp_pack;


// https://locklessinc.com/articles/tcp_checksum/
unsigned short checksum(const char *buf, unsigned size)
{
	unsigned sum = 0;
	int i;

	/* Accumulate checksum */
	for (i = 0; i < size - 1; i += 2)
	{
		unsigned short word16 = *(unsigned short *) &buf[i];
		sum += word16;
	}

	/* Handle odd-sized case */
	if (size & 1)
	{
		unsigned short word16 = (unsigned char) buf[i];
		sum += word16;
	}

	/* Fold to get the ones-complement result */
	while (sum >> 16) sum = (sum & 0xFFFF)+(sum >> 16);

	/* Invert to get the negative in ones-complement arithmetic */
	return ~sum;
}


int main(int argc, char *argv[]) {

	// set up variables

	// input variables
	char *oldfile_name;
	char *newfile_name;
	char *ip_addr;
	int port_num;

	// socket variable
	int des_sock; 
	struct sockaddr_in sock_addr, des_addr;
	socklen_t size;

	// file reading variables
	FILE *oldfile;
	char buf[CHUNK+1];
	char buf_ack[CHUNK+1];

	// udp packet for sending and acknowledgment
	udp_pack packet;
	udp_pack packet_ack;


	// time out
	fd_set select_fds;
	struct timeval timeout;
	timeout.tv_sec = 1;
	timeout.tv_usec = 500;


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

	des_sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (des_sock < 0) {
		printf("ERROR: failed creating the socket\n");
		close(des_sock);
		exit(1);
	}


	printf("Client socket created\n");


	/*
	// set local sock address // U
	bzero(&sock_addr, sizeof sock_addr);
	sock_addr.sin_family = AF_INET; 
	sock_addr.sin_port = htons(port_num);
	sock_addr.sin_addr.s_addr = inet_addr(ip_addr);
	memset(sock_addr.sin_zero, '\0', sizeof sock_addr.sin_zero);  
	*/


	// set destination sock address
	bzero(&des_addr, sizeof des_addr);
	des_addr.sin_family = AF_INET; 
	des_addr.sin_port = htons(port_num);
	des_addr.sin_addr.s_addr = inet_addr(ip_addr);
	memset(des_addr.sin_zero, '\0', sizeof des_addr.sin_zero);  


	socklen_t addrlen = sizeof(sock_addr);


	// send the newfile name
	printf("\n", newfile_name);

	// packet the newfile name packet
	packet.seq_num = 0;
	bzero(packet.data, sizeof packet.data);
	strncpy(packet.data, newfile_name, sizeof newfile_name + 1); //assume the new file name has a length less than 10

	if (sendto(des_sock, &packet, sizeof(packet)+1, 0, (struct sockaddr *)&des_addr, sizeof(des_addr)) == -1) {
		printf("\nERROR: failed sending the newfile name\n");
		close(des_sock);
		exit(1);
	}

	printf("New file name sent\n");



	// reading file
	printf("\nRead file...\n");

	oldfile = fopen(oldfile_name, "rb");
	if (oldfile == NULL) {
		printf("Error in opening the file\n");
		close(des_sock);
		exit(1);
	}


	// set time out
	if (setsockopt(des_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
    	perror("Error in setting time out");
	}


	// reading the file in chunks and send it over to the server

	bzero(packet.data, sizeof packet.data);

	while (fread(&packet.data, 1, sizeof packet.data - 1, oldfile) > 0) {

		// set up packet header information
		int real_seq_num = packet.seq_num ^ 1;
		short real_checksum = checksum(packet.data, sizeof packet.data);

		// start sending this parcket repeatly until receiving the desired ACK
		while (1) {

			// absorb duplicated ACK from the previous
			while (1) {
				if (recvfrom(des_sock, &packet_ack, sizeof(packet_ack), 0, (struct sockaddr *)&sock_addr, &addrlen) > 0)  {
					continue;
				}
				break;
			}

			printf("sending packages!\n");


			// random functions that cause "accidents" if the transfer
			// in order to test the program's ability to solve data loss & error
			int rand_1 = rand() % 10;
			int rand_2 = rand() % 10;
			int rand_3 = rand() % 10;
			int rand_4 = rand() % 10;

			// random function to decide whether to send the right sequence number
			if (rand_1 < 8) {
				packet.seq_num = real_seq_num;
			}
			else {
				packet.seq_num = 0;
			}

			// random function to decide whether to send the right checksum
			if (rand_2 < 8) {
				packet.checksum = real_checksum;
			}
			else {
				packet.checksum = 0;
			}


			// random function to decide whether to skip this message
			if (rand_3 < 3) {
				printf("**************************************\n");
    			printf("SKIP ... resend later ...\n");
    			printf("**************************************\n");
				continue;
			}


			// send the file
			if (sendto(des_sock, &packet, sizeof(packet)+1, 0, (struct sockaddr *)&des_addr, sizeof(sock_addr)) < 0) {
    			printf("Error in sending the file\n");
    			exit(1);
    		}

    		// random function to decide whether to send a false duplicate
    		if (rand_4 < 3) {
    			if (sendto(des_sock, &packet, sizeof(packet)+1, 0, (struct sockaddr *)&des_addr, sizeof(sock_addr)) < 0) {
    				printf("Error in sending the file\n");
    				exit(1);
    			}
    			printf("DUPLICATE: %s\n", packet.data);
			}



    		printf("Package %d sent %s \n", packet.seq_num, packet.data);


    		// prepare to receive an ack
    		bzero(packet_ack.data, sizeof packet_ack.data);

			// try to receive an ACK
    		if (recvfrom(des_sock, &packet_ack, sizeof(packet_ack), 0, (struct sockaddr *)&sock_addr, &addrlen) > 0) {

    			printf("SEQ: %d, %d\n", real_seq_num, packet_ack.seq_num);

    			// no need to resend if correct ACK is received
    			if (packet_ack.seq_num == real_seq_num && packet_ack.checksum == real_checksum) {
    				break;
    			}

    			printf("**************************************\n");
    			printf("WRONG ACK ... resend ...\n");
    			printf("**************************************\n");
    			
    		} 

    		// if failed to receive an ack
    		else {
    			printf("**************************************\n");
    			printf("NO ACK RECEIVED ... resend ...\n");
    			printf("**************************************\n");
    			continue;
    		}

		}

		// clean up to read the next chunk
		bzero(packet.data, sizeof packet.data);

	}

	
	char *end;
	end = "***End***";

	bzero(packet.data, sizeof packet.data);
	strncpy(packet.data, end, sizeof end + 1);

	if (sendto(des_sock, &packet, sizeof packet + 1, 0, (struct sockaddr *)&des_addr, sizeof(sock_addr)) == -1) {
		printf("\nERROR: failed sending the end\n");
		close(des_sock);
		exit(1);
	}


	printf("Finish reading file, close the socket\n");

	fclose(oldfile);

	close(des_sock);

	return 0;

}