#pragma once
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <poll.h>
//List Template
enum SocketType{
    TYPE_SERVER,
    TYPE_CLIENT
};
#define CLIENT_NAME_LEN_MAX 24
/*
    @brief Client
    @param name Name
    @param address IPv6 Addess of client
    @param fd File descriptor
    @param poller Poll FD
    @param next Next in list
*/
typedef struct Client{
    char name[CLIENT_NAME_LEN_MAX];
    struct sockaddr_in6 address;
    int fd;
    enum SocketType type;
    struct pollfd *poller;
    struct Client *next;
}Client;

// Print struct in human readable form
void infoClient(Client *elem);

// Insert new
Client * insertClient(Client **head, int fd, struct pollfd *poller, enum SocketType type, struct sockaddr_in6 address);
// Count elements
int countClient(Client *head);
//Delete a given element
void deleteClient(Client **head, Client *elem);
// Delete Last Element
void deleteLastClient(Client **head);
//Purge the entire list
void deleteAllClient(Client **head);
//Dump it
void printClient(Client *head);