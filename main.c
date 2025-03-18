#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#define DEST_IP_BUFF 100
#define SRC_IP_BUFF 100
#define ICMP_BUFF 8 // ICMP header size
#define RCVD_MSG_BUFF 512
#define FQDN_BUFF 512
#define MAX_HOPS 30
#define PACKETS_PER_TTL 3
#define TIMEOUT_SEC 0
#define TIMEOUT_MICROSEC 500000
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

void construct_packet(int sock_fd, void* packet, int seq_number, int ttl, char* interface)
{
        struct icmphdr icmp_hdr;

        if(interface != NULL)
        {
                if(setsockopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, interface, strlen(interface)) != 0)
                {
                        perror("Error: binding socket to interface failed.");
                        exit(1);
                }
        }

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

void reverse_FQDN_resolve(struct sockaddr* node_IP, int node_IP_size, char* FQDN, int FQDN_size)
{
        int status = getnameinfo(node_IP, node_IP_size, FQDN, FQDN_size, NULL, 0, NI_NAMEREQD);
        if(status != 0)
        {
                strcpy(FQDN, "Unknown");
        }
}

int packet_timed_out(int sock_fd)
{
        fd_set fds;
        int status;
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT_SEC;
        timeout.tv_usec = TIMEOUT_MICROSEC;
        FD_ZERO(&fds);
        FD_SET(sock_fd, &fds);
        status = select(sock_fd+1, &fds, NULL, NULL, &timeout);
        if(status < 0)
        {
                perror("Error: timeout failed.");
        }
        return !status;
}

int main(int argc, char* argv[])
{
	char dest_IP_str[DEST_IP_BUFF];
        char src_IP_str[SRC_IP_BUFF]; 
        char packet[ICMP_BUFF]; // ICMP packet buffer(IPv4 payload)
        char rcvd_msg[RCVD_MSG_BUFF];
        char FQDN[FQDN_BUFF]; 
        char* interface = (argc > 2) ? argv[2] : NULL; 
        struct sockaddr_in dest_IP; 
        struct sockaddr_in node_IP;
        struct timeval time_start;
        struct timeval time_end;
        int node_IP_size = sizeof(node_IP);
        int dest_IP_size = sizeof(dest_IP);
        int sock_fd; 
        int ttl = 1; // IPv4 time to live
        int seq_number = 1; // ICMP sequence number
        int first_reply = 1; // To print FQDN and IP address of an intermediate node just once
        int line_overflow = 0; // To keep track of how many packets per TTL value is sent out
        long double rtt; // Round trip time

        if(argc > 3 || argc < 2)
        {
                fprintf(stderr, "Usage: %s <Destination FQDN> [interface]\n", argv[0]);
                exit(1);
        }

	resolve_FQDN(argv[1], &dest_IP, dest_IP_str, sizeof(dest_IP_str));
        printf("Destination ip: %s, %d hops max\n", dest_IP_str, MAX_HOPS);

        sock_fd = create_socket(&dest_IP);
        while(ttl <= MAX_HOPS)
        {
                printf("%d.\t Reply from ", ttl);
                for(line_overflow = 0; line_overflow < PACKETS_PER_TTL; line_overflow++)
                {
                        construct_packet(sock_fd, packet, seq_number, ttl, interface);
                        gettimeofday(&time_start, NULL);
                        send_packet(sock_fd, packet, sizeof(packet), (struct sockaddr*)&dest_IP, dest_IP_size);
                        if(packet_timed_out(sock_fd))
                        {
                                printf("* ");
                        }
                        else
                        {
                                recv_packet(sock_fd, rcvd_msg, sizeof(rcvd_msg), (struct sockaddr*)&node_IP, &node_IP_size);
                                gettimeofday(&time_end, NULL);
                                rtt = ((long double)time_end.tv_sec - (long double)time_start.tv_sec)*1000.0 + ((long double)time_end.tv_usec - (long double)time_start.tv_usec)/1000;
                                reverse_FQDN_resolve((struct sockaddr*)&node_IP, sizeof(node_IP), FQDN, sizeof(FQDN));
                                strcpy(src_IP_str, inet_ntoa(node_IP.sin_addr));
                                if(first_reply)
                                {
                                        printf("%s (%s) ", FQDN, src_IP_str);
                                        first_reply = 0;
                                }
                                printf("%.3Lf ms ", rtt);
                        }
                        seq_number++;
                }
                printf("\n");
                first_reply = 1;

                if(memcmp(&dest_IP.sin_addr, &node_IP.sin_addr, sizeof(dest_IP.sin_addr)) == 0)        
                {
                        break;
                }
                ttl++;
        }
        close(sock_fd);


	return 0;
}
