#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int create_socket(char* dest_IP_str, struct sockaddr_in* dest_addr, int dest_addr_size)
{
        // Socket creation
	int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sock_fd < 0)
	{
		perror("Error: Failed to initialize socket.");
		exit(1);
	};
	// Socket configuring
	memset(dest_addr, 0, dest_addr_size);
	dest_addr->sin_family = AF_INET;
	dest_addr->sin_addr.s_addr = inet_addr(dest_IP_str);
	dest_addr->sin_port = 0;

        return sock_fd;
}	
int main(int argc, char* argv[])
{
        // TODO: implement main programm logic.
	return 0;
}
