// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <poll.h>

#define PORT 42069
#define CLIENT_COUNT_MAX 10

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in6 address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    //Setup Poller
    int fdCount = 0;
    int fdSize = CLIENT_COUNT_MAX;
    struct pollfd *pfds = malloc(sizeof *pfds * fdSize);
       
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET6, SOCK_STREAM, 0)) == 0)
    {
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
    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, 
                                 sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    pfds[0].fd = server_fd;
    pfds[0].events = POLLIN;
    fdCount = 1;

    while(true){
        int pollCount = poll(pfds, fdCount, -1);
        if (pollCount == -1) {
            perror("poll");
            exit(1);
        }

        for(int i = 0; i < fdCount; i++){
            // is one ready?
            if (pfds[i].revents & POLLIN){
                // is the listener ready to read?
                if(pfds[i].fd == server_fd){
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
                        perror("accept");
                    }else{
                        //If there is not enough space, the buffer for the other descriptors gets doubled in size
                        if(fdCount == fdSize){
                            fdSize *= 2;
                            pfds = realloc(pfds, sizeof(*pfds) * fdSize);
                            if(!pfds)
                                exit(EXIT_FAILURE);
                        }

                        pfds[fdCount].fd = new_socket;
                        pfds[fdCount].events = POLLIN;
                        fdCount++;
                        //Print some infos
                        char astring[INET6_ADDRSTRLEN];
                        inet_ntop(AF_INET6, &(address.sin6_addr), astring, INET6_ADDRSTRLEN);
                        printf("Added %s to the Pollserver\n", astring);           
                    }
                }else{
                    //Standard client procedure...
                    int byteCount = recv(pfds[i].fd, buffer, sizeof buffer, 0);
                    if(byteCount <= 0){
                        if(byteCount == 0)
                            /*
                                Test with telnet:
                                ctrl + ]
                                telnet> quit
                            */
                            printf("Socket hung up: %d\n", pfds[i].fd);
                        else
                            perror("recv");

                        close(pfds[i].fd);

                        pfds[i] = pfds[fdCount-1];
                        fdCount--;
                    }else{
                        //Got some Data!
                        for(int j = 0; j < fdCount; j++){
                            int dest_fd = pfds[j].fd;
                            //sed to everybody except the sender and the server:
                            if (dest_fd != server_fd && dest_fd != pfds[i].fd){
                                if(send(dest_fd, buffer, byteCount, 0) == -1)
                                    perror("send");
                            }
                        }
                    }
                }
            }
        }
    }
    free(pfds);
    return 0;
}
