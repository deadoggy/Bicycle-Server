#include "rio.h"


ssize_t rio_readn(int fd, void *usrbuf, size_t n){

}

ssize_t rio_writen(int fd, void *usrbuf, size_t n){

}

void rio_initb(rio_t *rp, int fd){

}

/*read n bytes from buffer to usrbuf, this function can not be open to other files*/
static ssize_t rio_read(rio_t *rp, void *usrbuf, size_t n){

}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen){

}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n){
    
}
