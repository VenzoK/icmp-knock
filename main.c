#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#define DEST_IP_BUFF 100
#define SRC_IP_BUFF 100
#define ICMP_BUFF 8 // ICMP header size
#define RCVD_MSG_BUFF 1024
#define FQDN_BUFF 100
int create_socket(struct sockaddr_in* dest_addr)
{
        // Socket creation
	int sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(sock_fd < 0)
	{
		perror("Error: Failed to initialize socket.");
		exit(1);
	};
	// Socket configuring
	dest_addr->sin_family = AF_INET;
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

void resolve_FQDN(char* FQDN, struct sockaddr_in* dest_addr, char* dest_addr_str, int dest_addr_str_size)
{
	// FQDN resolution
        struct addrinfo* result;
        int status = getaddrinfo(FQDN, NULL, NULL, &result);
        if(status != 0)
        {
                fprintf(stderr, "FQDN resolution error: %s\n", gai_strerror(status));
                exit(1);
        }

        struct addrinfo* p;
        struct sockaddr_in* ipv4;
        for(p = result; p != NULL; p = p->ai_next)
        {
                // The first entry with an IPv4 address for the provided FQDN is selected.
		if(p->ai_family == AF_INET) // IPv4
                {
                        ipv4 = (struct sockaddr_in*)p->ai_addr;
                        dest_addr->sin_addr = ipv4->sin_addr;
                        break;
                }
                else
                {
                        continue;
                }
        }
        inet_ntop(p->ai_family, &ipv4->sin_addr, dest_addr_str, dest_addr_str_size);
        
        freeaddrinfo(result);
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

	resolve_FQDN(argv[1], &dest_IP, dest_IP_str, sizeof(dest_IP_str));
        printf("Dest ip: %s\n", dest_IP_str);

        sock_fd = create_socket(&dest_IP);
        construct_packet(sock_fd, packet, seq_number, ttl);
        send_packet(sock_fd, packet, sizeof(packet), (struct sockaddr*)&dest_IP, dest_IP_size);
        recv_packet(sock_fd, rcvd_msg, sizeof(rcvd_msg), (struct sockaddr*)&node_IP, &node_IP_size);
        strcpy(src_IP_str, inet_ntoa(node_IP.sin_addr));
        printf("Reply from %s, TTL: %d\n", src_IP_str, ttl);
        
        close(sock_fd);


	return 0;
}
