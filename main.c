#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#define DEST_IP_BUFF 100
#define SRC_IP_BUFF 100
#define ICMP_BUFF 10 // ICMP header size
#define RCVD_MSG_BUFF 1024
#define FQDN_BUFF 100
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

uint16_t checksum(void* packet, int length)
{
        uint16_t* buff = (uint16_t*)packet;
        uint16_t sum = 0;
        uint16_t result;

        for(; length > 1; length -= 2)
        {
                sum+= *buff++;
        }
        if(length == 1)
        {
                sum+= *(unsigned char*)buff;
        }
        sum = (sum >> 16) + (sum & 0xFFFF);
        sum += (sum >> 16);
        result = ~sum;

        return result;
}

void construct_packet(int sock_fd, void* packet, int seq_number, int ttl)
{
        struct icmphdr icmp_hdr;

        setsockopt(sock_fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        // ICMP header initialization
        icmp_hdr.type = ICMP_ECHO;
        icmp_hdr.code = 0;
        icmp_hdr.un.echo.id = getpid();
        icmp_hdr.un.echo.sequence = seq_number;
        icmp_hdr.checksum = 0;
        icmp_hdr.checksum = checksum((unsigned short*)&icmp_hdr, sizeof(struct icmphdr));
        memcpy(packet, &icmp_hdr, sizeof(struct icmphdr));
}

int main(int argc, char* argv[])
{
	char dest_IP_str[DEST_IP_BUFF];
        char src_IP_str[SRC_IP_BUFF];
        char packet[ICMP_BUFF];
        char rcvd_msg[RCVD_MSG_BUFF];
        char FQDN[FQDN_BUFF]; // For further reverse FQDN resolution
        struct sockaddr_in dest_IP;
        struct sockaddr_in node_IP;
        int node_IP_size = sizeof(node_IP);
        int dest_IP_size = sizeof(dest_IP);
        int ttl = 1; // Different TTLs to track intermediate nodes
        int seq_number = 1; // For sending multiple messages
        int sock_fd;

        strcpy(dest_IP_str, argv[1]);
        sock_fd = create_socket(dest_IP_str, &dest_IP, dest_IP_size);
        construct_packet(sock_fd, packet, seq_number, ttl);
        send_packet(sock_fd, packet, sizeof(packet), (struct sockaddr*)&dest_IP, dest_IP_size);
        recv_packet(sock_fd, rcvd_msg, sizeof(rcvd_msg), (struct sockaddr*)&node_IP, &node_IP_size);
        strcpy(src_IP_str, inet_ntoa(node_IP.sin_addr));
        printf("Reply from %s, TTL: %d\n", src_IP_str, ttl);
        
        close(sock_fd);


	return 0;
}
