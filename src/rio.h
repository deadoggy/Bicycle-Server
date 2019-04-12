#include<unistd.h>
#include<sys/errno.h>
#include<string.h>

#ifndef RIO_H_HEADER
#define RIO_H_HEADER   

#define RIO_BUFSIZE 8192

typedef struct {
    int rio_fd;                     //rio descriptor
    int rio_cnt;                    //data bytes in buf
    char *rio_bufptr;               //next read position
    char rio_buf[RIO_BUFSIZE];      //buffer area
} rio_t;

/*no buffer read & write, transfer data between files and memory directly*/
ssize_t rio_readn(int fd, void *usrbuf, size_t n); // read by using read() of unistd.h
ssize_t rio_writen(int fd, void *usrbuf, size_t n); // write by using write() of unistd.h

/*init rio buffer*/
void rio_initb(rio_t *rp, int fd);

/*read a line from buffer to usr buffer, maxlen is the max length of line*/
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);

/*read n bytes fromrio buffer to usr buffer*/
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n);

#endif