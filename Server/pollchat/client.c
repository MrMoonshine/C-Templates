#include "client.h"
static const char *USER_NAMES[] = {
    "Pepe the Frog",
    "Rickroller",
    "Stickbug",
    "Thiccc boy",
    "Sad Cat"
};
//Insert at the first position
Client * insertClient(Client **head, int fd, struct pollfd *poller, enum SocketType type, struct sockaddr_in6 address){
    Client *elem = (Client*) malloc(sizeof(Client));
    if(elem == NULL)
        return elem;
    //Assign data
    switch (type){
        case TYPE_CLIENT:{
            strncpy(elem->name, USER_NAMES[rand() % 5], CLIENT_NAME_LEN_MAX);
        }break;
        case TYPE_SERVER: strncpy(elem->name, "Server", CLIENT_NAME_LEN_MAX); break;
        default: strcpy(elem->name, ""); break;
    }
    elem->type = type;
    elem->fd = fd;
    elem->poller = poller;
    elem->poller->fd = fd;
    elem->poller->events = POLLIN;
    elem->address = address;
    //Next node is head
    elem->next = *head;
    // this is the new First node
    *head = elem;
    //Just to get a reference...
    return elem;
}

int countClient(Client *head){
    if(!head)
        return -1;

    int counter = 0;
    Client *elem = head;
    while(elem){
        counter++;
        elem = elem->next;
    }
    return counter;
}

//Delete a given element
void deleteClient(Client **head, Client *elem){
    Client *tmp = *head;
    Client *prev = NULL;
    while(tmp){
        if(tmp == elem){
            if(prev)
                prev->next = tmp->next;
            else
                *head = tmp->next;
            free(tmp);
            return;
        }
        prev = tmp;
        tmp = tmp->next;
    }
}
// Delete Last Element
void deleteLastClient(Client **head){
    Client *tmp = *head;
    Client *prev = NULL;
    while(tmp){
        if(tmp->next == NULL){
            if(prev)
                prev->next = tmp->next;
            else
                *head = tmp->next;
            free(tmp);
            return;
        }
        prev = tmp;
        tmp = tmp->next;
    }
}
//Purge the entire list
void deleteAllClient(Client **head){
    while(*head){
        Client *tmp = *head;
        *head = (*head)->next;
        free(tmp);
    }
    *head = NULL;
}

void infoClient(Client *elem){
    //Print some infos
    char astring[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &(elem->address.sin6_addr), astring, INET6_ADDRSTRLEN);
    printf("Client Named: %s....\n\tIPv6:\t%s\nFD:\t%d\n", elem->name, astring, elem->fd);
}

//Dump it
void printClient(Client *head){
    Client *elem = head;
    while(elem){
        infoClient(elem);
        elem = elem->next;
    }
    printf("\n");
}
