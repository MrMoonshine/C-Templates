// Server side C/C++ program to demonstrate Socket programming
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <poll.h>
#include "client.h"

#define PORT 42069
#define CLIENT_COUNT_MAX 10

static const char* PROMPT = "\033[1;33m(chathub) > \033[0m";
static const char* TAG = "[Chathub]: ";
static const char* CMD_DELIMETER = " ";

static const char* msgsent = "Hugachaga Huga Huga Hugachaga!\r\n";

/*
    @brief Builds pollfd array
    @param
    @return count of clients. -1 on failure
*/
int buildPollers(struct pollfd* out, Client *head){
    int counter = 0;
    Client *elem = head;
    while(elem){
        counter++;
        // Newly assign all pointers within the array & set FDs
        elem->poller = out;
        (out++)->fd = elem->fd;
        elem = elem->next;
    }

    if(counter > CLIENT_COUNT_MAX)
        return -1;
    else
        return counter;
}

void handleCommand(char* buffer, size_t size, Client* sender, Client *list){
    char* token = strtok(buffer, CMD_DELIMETER);
    char bufferCmd[64] = {0};
    strcpy(bufferCmd, token);

    if(strstr(bufferCmd, "show")){
        char* token = strtok(NULL, CMD_DELIMETER);
        strcpy(bufferCmd, token);
        if(strstr(bufferCmd, "users")){
            sprintf(buffer, "Name\t\tIPv6\t\tFD\r\n");
            char astring[INET6_ADDRSTRLEN];
            Client *elem = list;
            while(elem){
                if(elem->type == TYPE_CLIENT){
                    //Print some infos
                    inet_ntop(AF_INET6, &(elem->address.sin6_addr), astring, INET6_ADDRSTRLEN);
                    sprintf(buffer,"%s%s\t%s\t%d\r\n", buffer, elem->name, astring, elem->fd);
                }
                elem = elem->next;
            }
        }else{
            sprintf(buffer, "Unknown show command: %s\r\n", bufferCmd);
        }
    }else if(strstr(bufferCmd, "send")){
        /*
            S E N D
        */
        char message[512];
        snprintf(message, sizeof message, "\r\nNew message from %s:\r\n%s%s", sender->name, buffer + strlen("send") + 1,PROMPT);
        Client *elem = list;
        while(elem){
            if(elem->fd != sender->fd && elem->type == TYPE_CLIENT){
                //printf("Sending \"%s\" to %d\n", buffer, elem->fd);
                if(send(elem->fd, message, strlen(message), 0) == -1)
                    perror("send");
            }
            elem = elem->next;
        }
        strcpy(buffer, "");
    }else if(strlen(bufferCmd) > 2){
        sprintf(buffer, "Unknown command: %s\r\n", bufferCmd);
    }else{
        strcpy(buffer, "");
    }
    strcat(buffer, PROMPT);
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    int server_fd, new_socket;
    struct sockaddr_in6 address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    //Poller counter
    int fdCount = 0;
    struct pollfd pfds[CLIENT_COUNT_MAX];
       
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

    // Setup client List
    Client *fdList = NULL;
    insertClient(&fdList, server_fd, pfds, TYPE_SERVER, address);
    printClient(fdList);
    // Count sockets & recompute poller array
    fdCount = buildPollers(pfds, fdList);

    while(true){
        printf("%sPoll\n",TAG);
        int pollCount = poll(pfds, fdCount, -1);
        if (pollCount == -1) {
            perror("poll");
            exit(1);
        }
        printf("%sPoll finished\n", TAG);

        Client *elem = fdList;
        while(elem){
            // Nothing has been polled
            if(!(elem->poller->revents & POLLIN)){
                elem = elem->next;
                continue;
            }

            switch (elem->type){
            case TYPE_SERVER:{
                    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
                        elem = elem->next;
                        perror("accept");
                        continue;
                    }
                    // don't add if maxed out
                    if(fdCount < CLIENT_COUNT_MAX){
                        Client* newclient = insertClient(&fdList, new_socket, pfds + fdCount, TYPE_CLIENT, address);
                        fdCount = countClient(fdList);
                        printClient(fdList);
                        // send prompt
                        send(new_socket, PROMPT, strlen(PROMPT), 0);
                    }else{
                        sprintf(buffer, "%sMaximum capacity reached!\r\n", TAG);
                        printf(buffer);
                        send(new_socket, buffer, strlen(buffer), 0);
                        close(new_socket);
                    }
                }
                break;
            case TYPE_CLIENT:{
                    // Clear buffer
                    memset(buffer, 0, sizeof buffer);
                    // Receive from socket
                    int byteCount = recv(elem->fd, buffer, sizeof buffer, 0);
                    if(byteCount <= 0){
                        if(byteCount == 0)
                            /*
                                Test with telnet:
                                ctrl + ]
                                telnet> quit
                            */
                            printf("Socket hung up: %d\n", elem->fd);
                        else
                            perror("recv");

                        close(elem->fd);
                        deleteClient(&fdList, elem);
                        fdCount = buildPollers(pfds, fdList);
                        elem = fdList;
                    }else{
                        //Got some Data!
                        printf("%sGot data from %d:\t%s\n", TAG, elem->fd, buffer);

                        if(strstr(buffer, "exit\r\n")){
                            close(elem->fd);
                            deleteClient(&fdList, elem);
                            fdCount = buildPollers(pfds, fdList);
                            //return to beginning
                            elem = fdList;
                        }else{
                            handleCommand(buffer, sizeof buffer, elem, fdList);
                            if(send(elem->fd, buffer, strlen(buffer), 0) == -1){
                                elem = elem->next;
                                perror("send");
                                continue;
                            }
                        }
                    }
                }
                break;
            
            default:
                break;
            }
            elem = elem->next;
        }
    }

    deleteAllClient(&fdList);
    return 0;
}
