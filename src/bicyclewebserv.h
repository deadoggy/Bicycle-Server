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
#include <sys/time.h>
#include<sys/select.h>
#include<signal.h>
#include<sys/wait.h>
#include<stdio.h>
#include "rio.h"

#define DEFAULT_READ_TIMEOUT_SEC 1
#define DEFAULT_READ_TIMEOUT_MCSEC 0
#define DEFAULT_BACKLOG 1024
#define DEFAULT_FILE_SLICE_SIZE 512
#define MAXLINE 1024
#define MAXBUF 4072

typedef struct {
    FILE* fp;
    int fd;
    size_t file_size;
    size_t beg;
    size_t cnt;
    unsigned char* slice_ptr;
} file_slice;

volatile __sig_atomic_t cgi_pid;

int open_clientfd(char *hostname, char *port);                 // open a client, encapsulate getaddrinfo
int open_listenfd(char* port);                                 // open a listen socket, encapsulate getaddrinfo

void process_http(int fd);                                     // process a http transaction
size_t read_requesthdrs(rio_t *rp);                            // process request headers
int parse_uri(char *uri, char *filename, char *cgiargs);       // parse a uri
void get_filetype(char *filename, char *filetype);             // parse filetype from filename
void serve_static(int fd, char *filename, size_t filesize);     // supply static content
void serve_dynamic(int fd, char *filename, char * cgiargs);    // supply dynamic content
void clienterror(int fd, char *cause, char *errnum,         
                            char *shortmsg, char *longmsg);    // process errors

void init_file_slice(char* filename, file_slice* fs);          // init a file slice
void destroy_file_slice(file_slice* fs);                       // destory a file_slice
int next_slice(file_slice *fs);                                // get a file slice, return 1 if get success, 0 for next slice doesn't exists, error return -1

int timeout_check(rio_t *rp, int timeout_sec, 
                            int timeout_msec);                 // 1 for ready to read, 0 for timeout and -1 for error

void cgi_chdpro_handler(int sig);                              // a child process handler function
void sigpipe_handler(int sig);                                 // process sigpipe

void ride_bicycle(int argc, char** argv);                      // main logic of bicycle server

#endif