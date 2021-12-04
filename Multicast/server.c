// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>

#define PORT 8080

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in6 address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[192] = {0};
    static const char *msg = "You've found me!\n";
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET6, SOCK_DGRAM, 0)) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons( PORT );
    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // JOIN MEMBERSHIP
    struct ipv6_mreq group;
    group.ipv6mr_interface = 0;
    inet_pton(AF_INET6, "ff02::1234", &group.ipv6mr_multiaddr);
    setsockopt(server_fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &group, sizeof group);

    printf("Server Ready!\n");
    char astring[INET6_ADDRSTRLEN];
    struct sockaddr_in6 caddr;
    socklen_t clen = 0;
    while(1){
        printf("Reading...\t");
        recvfrom(server_fd, buffer, 192, 0, (struct sockaddr *)&caddr, &clen);
        printf("%s\n",buffer );
        //Send a message back
        if(sendto(server_fd, msg, strlen(msg), 0, (struct sockaddr*)&caddr, clen) == -1)
            perror("sendto()");

        inet_ntop(AF_INET6, &(caddr.sin6_addr), astring, INET6_ADDRSTRLEN);
	    printf("Peer was: %s\n", astring);
    }
    
    
    /*char astring[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(address.sin6_addr), astring, INET6_ADDRSTRLEN);
	printf("IPv6 Addr chosen: %s\n", astring);*/
    return 0;
}
