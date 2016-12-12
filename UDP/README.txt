This socket program demonstrates an SFTP (Simple File Transfer Protocol) using UDP (User Datagram Protocol) where the file goes through an unreliable channel with bit errors with packet loss. The client sends a text file to the server, and the server sends back an acknowledgment after every data packet is sent. Random functions are implemented to create real unreliable data transfer scenarios such as data error and loss.

How to run the program:
Step 0: Make sure there’s a text file in the same directory with the client file
Step 1: compile both the server and the client programs: gcc -o server server.c gcc -o client client.c
Step2: Start off the server with ./server <port#>
Step3: Start off the client with ./client <input_filename> <output_filename> <server_ip_address> <server_port>
Step4: The both program terminates, a new file with he output_filename should appear in the same directory with the server program, and its content is same with that in the input file.
