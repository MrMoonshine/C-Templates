#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//List Template
typedef struct __T__{
    char word[16];
    struct __T__ *next;
}__T__;

//Insert at the first position
__T__ * insert__T__(__T__ **head, const char *data){
    __T__ *elem = (__T__*) malloc(sizeof(__T__));
    if(elem == NULL)
        return elem;
    //Assign data
    memset(elem->word, 0, 16);
    strcpy(elem->word, data);
    //Next node is head
    elem->next = *head;
    // this is the new First node
    *head = elem;
    //Just to get a reference...
    return elem;
}
//Delete a given element
void delete__T__(__T__ **head, __T__ *elem){
    __T__ *tmp = *head;
    __T__ *prev = NULL;
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
void deleteLast__T__(__T__ **head){
    __T__ *tmp = *head;
    __T__ *prev = NULL;
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
void deleteAll__T__(__T__ **head){
    while(*head){
        __T__ *tmp = *head;
        *head = (*head)->next;
        free(tmp);
    }
    *head = NULL;
}
//Dump it
void print__T__(__T__ *head){
    __T__ *elem = head;
    while(elem){
        printf("%s ", elem->word);
        elem = elem->next;
    }
    printf("\n");
}

int main(void) {
    __T__ *head = NULL;
    //Example: an arbitrary elemnt needs to be deleted:
    insert__T__(&head, "aus!");
    insert__T__(&head, "augen");
    insert__T__(&head, "die");
    insert__T__(&head, "dir");
    insert__T__(&head, "ich");
    insert__T__(&head, "kratz");
    insert__T__(&head, "sonst");
    insert__T__(&head, "heraus");
    insert__T__(&head, "komm");
    __T__ * removeme = insert__T__(&head, "Maus");
    //insert__T__(&head, "Maus");
    //Dump
    print__T__(head);
    //Delete
    //delete__T__(&head, removeme);
    deleteLast__T__(&head);
    //Dump
    print__T__(head);
    //Remove all elements
    deleteAll__T__(&head);
}
