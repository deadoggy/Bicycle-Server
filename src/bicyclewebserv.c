#include "bicyclewebserv.h"



int open_clientfd(char *hostname, char *port){
    int clientfd;
    struct addrinfo hints, *listp, *p;

    /*get potencial socket list*/
    memset(&hints, 0, sizeof(struct addrinfo));             // set all bits to 0, because we can only set ai_family, ai_socktype,, ai_protocol and ai_flags
    hints.ai_socktype = SOCK_STREAM;                        // SOCK_STREAM: reliable connection, TCP
    hints.ai_flags |= AI_ADDRCONFIG | AI_NUMERICSERV;   
    getaddrinfo(hostname, port, &hints, &listp);

    /*traverse the listp and find a connectable socket*/
    for(p=listp; p; p=p->ai_next){
        if((clientfd = socket(p->ai_family, p->ai_socktype, 
        p->ai_protocol)) < 0){
            continue;                                       // socket failed, next socket
        }

        if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1){
            break;                                          // connection success
        }else{
            close(clientfd);
        }
    }

    /*clean up listp and return*/
    freeaddrinfo(listp);
    if(!p){
        return -1;
    }else{
        return clientfd;
    }
}

int open_listenfd(char *port){

    int listenfd, optval=1, backlog;
    struct addrinfo hints, *listp, *p;
    char *env_listenq;

    /*get potencial socket list*/
    memset(&hints, 0, sizeof(struct addrinfo));             
    hints.ai_flags |= AI_ADDRCONFIG | AI_PASSIVE | AI_NUMERICSERV;
    hints.ai_socktype = SOCK_STREAM;
    getaddrinfo(NULL, port, &hints, &listp);

    for(p=listp; p; p=p->ai_next){
        if((listenfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            continue;
        }
        
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                    (const void*)&optval, sizeof(int));      // make this port can be reused
        
        if(0==bind(listenfd, p->ai_addr, p->ai_addrlen)){
            break;                                           // bind succeeds
        }
        close(listenfd);                                     // bind fails, next
    }

    freeaddrinfo(listp);                                     // clean socket list
    if(!p){
        return -1;
    }

    if((env_listenq=getenv("LISTENQ")) != NULL){
        backlog = atoi(env_listenq);
    }else{
        backlog = DEFAULT_BACKLOG;
    }
    if(listen(listenfd, backlog) < 0){
        close(listenfd);
    }
    return listenfd;
}

