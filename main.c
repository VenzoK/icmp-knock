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

void send_packet(int sock_fd, const void* packet, int packet_size, struct sockaddr* dest, int dest_size)
{
        int data_sent = sendto(sock_fd, packet, packet_size, 0, (struct sockaddr *)dest, dest_size);
        if(data_sent < 0)
        {
                perror("Error: sending data failed.");
                exit(1);
        }
}

void recv_packet(int sock_fd, char* rcvd_msg, int rcvd_msg_size, struct sockaddr* node_IP, int* node_IP_size)
{
        int data_recvd = recvfrom(sock_fd, rcvd_msg, rcvd_msg_size, 0, node_IP, node_IP_size);
        if(data_recvd < 0)
        {
                perror("Error: receiving data failed.");
                exit(1);
        }
}

int main(int argc, char* argv[])
{
        // TODO: implement main programm logic.
	return 0;
}
