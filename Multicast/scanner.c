#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
 #include <poll.h>
#include <unistd.h>
#include <string.h>

#define PORT 53
#define RESULTS_MAX 12
#define ATTEMPTS 3

static const char* msg = "Is somebody there?\r\n";

int main(int argc, char const *argv[]){
	if(argc < 2){
		printf("Insufficient arguments!\n");
		return -1;
	}

    int sock = 0;
    struct sockaddr_in6 saddr;
	/* Create socket for communication with server */
	sock = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sock == -1) {
		perror("socket()");
		return EXIT_FAILURE;
	}
	
	struct timeval tv;
	tv.tv_sec = 1.5;
    tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

	int my_network_devices_scope_id = 0;
	struct if_nameindex *if_ni, *i;

   	if_ni = if_nameindex();
    if (if_ni == NULL) {
        perror("if_nameindex");
        exit(EXIT_FAILURE);
    }

	for (i = if_ni; ! (i->if_index == 0 && i->if_name == NULL); i++){
		if(strstr(i->if_name, "lo") == NULL){
			my_network_devices_scope_id = i->if_index;
			printf("Using %s for Multicast discovery. Index: %u\n", i->if_name, i->if_index);
			break;
		}
	}
    if_freenameindex(if_ni);   

	if(setsockopt(sock, IPPROTO_IPV6, IPV6_MULTICAST_IF, (char *)&my_network_devices_scope_id, sizeof(my_network_devices_scope_id)) < 0){
    	perror("Setting local interface error");
    	exit(1);
   	}
 
	saddr.sin6_family = AF_INET6;
	saddr.sin6_scope_id = my_network_devices_scope_id;
	saddr.sin6_port = htons(PORT);
	inet_pton(AF_INET6, argv[1], &saddr.sin6_addr);
 
	struct sockaddr_in6 caddr;
    socklen_t clen = 0;
	char astring[INET6_ADDRSTRLEN];
	char buffer[192] = {0};

	char hostbuffer[64] = {0};

	int fdCount = 0;
    const int fdSize = RESULTS_MAX;
    struct pollfd *pfds = malloc(sizeof *pfds * fdSize);

	pfds[0].fd = sock;
    pfds[0].events = POLLIN;
	fdCount = 1;

	for(int a = 0; a < ATTEMPTS; a++){
		printf("+---------------------------------------+\n");
		printf("|          Searching...                 |\n");
		printf("+---------------------------------------+\n");
		//send msg
        if(sendto(sock, msg, strlen(msg), 0, (struct sockaddr*)&saddr, sizeof(struct sockaddr_in6)) == -1)
            perror("sendto()");

		int counter = 0;
		while(counter++ < fdSize){
			switch(poll(pfds, fdCount, 1000)){
				case -1: {
					perror("poll()");
					counter = fdSize;
				} break;
				case 0: {
					printf("Timer Expired. Searching done!\n");
					counter = fdSize;
				} break;
				default:{
					for(int b = 0; b < fdCount; b++){
						if(pfds[b].revents & POLLIN){
							//printf("Got one!\n");
							if(recvfrom(sock, buffer, 192, 0, (struct sockaddr *)&caddr, &clen) != -1){
								inet_ntop(AF_INET6, &(caddr.sin6_addr), astring, INET6_ADDRSTRLEN);
								printf("%s replies: %s", astring, buffer);

								memset(hostbuffer, 0, 64);
								//This Program uses the link-local addresses, hence: useless
								/*printf("Host:\t");
								if(getnameinfo((struct sockaddr *)&caddr, clen, hostbuffer, 64, NULL, 0, NI_NAMEREQD))
									printf("couldn't resolve hostname\n");
								else
									printf("%s\n", hostbuffer);*/
							}else{
								perror("recvfrom()");
							}
						}
					}
				}break;
			}
		}
    }

	free(pfds);

	//sleep(20);
    return 0;
}
