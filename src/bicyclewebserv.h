#ifndef BICYCLEWEBSERV_H_HEADER
#define BICYCLEWEBSERV_H_HEADER

#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/mman.h>
#include "rio.h"



#define DEFAULT_BACKLOG 1024
#define MAXLINE 1024
#define MAXBUF 4072


int open_clientfd(char *hostname, char *port);                 // open a client, encapsulate getaddrinfo
int open_listenfd(char* port);                                 // open a listen socket, encapsulate getaddrinfo

void process_http(int fd);                                     // process a http transaction
size_t read_requesthdrs(rio_t *rp);                              // process request headers
int parse_uri(char *uri, char *filename, char *cgiargs);       // parse a uri
void get_filetype(char *filename, char *filetype);             // parse filetype from filename
void serve_static(int fd, char *filename, size_t filesize);     // supply static content
void serve_dynamic(int fd, char *filename, char * cgiargs);    // supply dynamic content
void clienterror(int fd, char *cause, char *errnum,         
                            char *shortmsg, char *longmsg);    // process errors

void ride_bicycle(int argc, char** argv);                      // main logic of bicycle server

#endif