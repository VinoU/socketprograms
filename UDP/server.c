/* 
 * Author: Chi Zhang (czhang2@scu.edu)
 * File name: server.c
 * Description: The file builds the server side of UDP (User Datagram Protocol). The server creates
 * a new text file using the file name and data received from the client. The server receives the
 * file data in chunks of 10 bytes and then writes the data into the new file in chunks of 5 bytes.
 * For every chunk of data received, the server sends back an acknowledgement.
 * 
 * Random functions are used to create the scenarios where acknowledgements are lost or duplicated, 
 * in order to test the program's ability to resolve these situations and create a correct file.
 *
 * Referencer:
 * Socket Programming in C
 * http://stackoverflow.com/questions/3060950/how-to-get-ip-address-from-sock-structure-in-c
 * http://stackoverflow.com/questions/5850000/how-to-split-array-into-two-arrays-in-c
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
#include <unistd.h>



#define BACKLOG 10 // how many pending connections queue will hold

#define CHUNK 10 /* read 10 bytes at a time */

typedef struct udp_pack {
	int seq_num;
	short checksum;
	char data[CHUNK];
} udp_pack;


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

	int port_num;
	char ip_addr[INET_ADDRSTRLEN];
	
	int new_sock; 
	struct sockaddr_in sock_addr, recv_addr;
	socklen_t size; 

	FILE *newfile;

	udp_pack packet;
	udp_pack packet_ack;


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
	new_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (new_sock < 0) {
		printf("ERROR: failed to create a client socket\n");
		exit(1);
	}

	printf("\nSocket created...");


	// set up server_addr values
	inet_ntop(AF_INET, &sock_addr, ip_addr, INET_ADDRSTRLEN); // get the current ip address
	bzero(&sock_addr, sizeof sock_addr);
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	sock_addr.sin_port = htons(port_num);
	memset(sock_addr.sin_zero, '\0', sizeof sock_addr.sin_zero);  



	// bind
	if (bind(new_sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0) {
		printf("ERROR: failed binding\n");
		exit(1);
	}

	printf("\nBinding success\n");


	socklen_t addrlen = sizeof(recv_addr);

	if (recvfrom(new_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&recv_addr, &addrlen) > 0) {
		printf("received message: \"%s\"\n", packet.data);
		printf("New file name received!\n");
	}
	else {
		printf("ERROR: failed sending file name\n");
		exit(1);
	}

	packet_ack.seq_num = 0;


	// create a new file

	newfile = fopen(packet.data, "wb+");
	if (newfile == NULL) {
		newfile = fopen(packet.data, "wb+");
	}



	// receive info from the client and write to the new file

	char buf_wri[5];

	while(recvfrom(new_sock, &packet, sizeof(packet), 0, (struct sockaddr *)&recv_addr, &addrlen) > 0) {

		// check when the client finishes sending data
		char *end;
		end = "***End***";

		if (strcmp(end, packet.data) == 0){
			printf("END\n");
			break;
		}


		// check the data receive & make the ack
		short new_checksum = checksum(packet.data, sizeof packet.data);
		int new_seq = packet_ack.seq_num ^ 1;

		printf("CHECKSUM: %d, %d\n", packet.checksum, new_checksum);
		printf("SEQ_NUM: %d, %d\n", packet.seq_num, new_seq);

		if (new_checksum == packet.checksum && new_seq == packet.seq_num) {

			// make new ack
			packet_ack.seq_num ^= 1;
			packet_ack.checksum = new_checksum;
			bzero(packet_ack.data, sizeof packet_ack.data);
			memcpy(packet_ack.data, "ACK", sizeof(packet_ack.data));

			// write the data into the file
			// write the file in 5 byte chunks
			int i, d, c;
			for (i = 0; i < 2; i++) {
				bzero(buf_wri, sizeof buf_wri);
				d = i * 5;
				memcpy(buf_wri, &packet.data[d], 5 * sizeof(char));
				fprintf(newfile, "%s", buf_wri);
			}
			
			printf("DATA: %s\n", packet.data);
			bzero(packet.data, sizeof packet.data);

		}
		else {
			printf("Wrong data\n");
		}

		// random function to decide whether to send the ack
		int rand_1 = rand() % 10;
		if (rand_1 < 3) {
			printf("NO ACK SENT\n");
			continue;
		}


		// sent the ack
		if (sendto(new_sock, &packet_ack, sizeof(packet_ack), 0, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) > 0) {
			printf("**************************************\n");
			printf("ACK: %d, %d\n", packet_ack.seq_num, packet_ack.checksum);
			printf("**************************************\n");
    	}

    	// random function to decide whether to falsely duplicate the ack
    	int rand_2 = rand() % 10;
		if (rand_2 < 3) {
			if (sendto(new_sock, &packet_ack, sizeof(packet_ack), 0, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) > 0) {
				printf("DUPLICATE ACK\n");
    		}
		}

	}
	


	fclose(newfile); 

	close(new_sock);

	return 0;

}













