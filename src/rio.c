#include "rio.h"


ssize_t rio_readn(int fd, void *usrbuf, size_t n){
    size_t nleft = n;                               // size of unread bytes
    char *tgtptr = usrbuf;                          // target buf pointer
    ssize_t nread = 0;                              // size of read bytes returned by read()

    while(nleft > 0){
        if((nread = read(fd, tgtptr, nleft))<0){
            if(errno == EINTR){                     // interrupted by sig handler return
                nread = 0;                          // continue read
            }else{
                return -1;
            }
        }
        else if(0==nread){                          // n > size of input and read finished
            break;
        }

        nleft -= nread; 
        tgtptr += nread;                            // reloacte the pointer of target position
    }

    return n-nleft;                                 // return size of bytes which has been read
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n){
    size_t nleft = n;                               // size of unwritten bytes
    char *srcptr = usrbuf;                          // source buf pointer
    ssize_t nwritten = 0;                           // size of written bytes returned by write

    while(nleft > 0){
        if((nwritten=write(fd, srcptr, nleft))<0){  
            if(errno == EINTR){                     // interrupted by sig handler return
                nwritten = 0;
            }else{
                return -1;
            }
        }       
        /*  
            if return nwritten == 0, continue to write; 
            once there are errors or interuptions, they will be processed by above if-else 
        */

        nleft -= nwritten;
        srcptr += nwritten;
    }

    return n;                                       // return size of written bytes
}

void rio_initb(rio_t *rp, int fd){
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

/*read n bytes from buffer to usrbuf, this function can not be open to other files*/
static ssize_t rio_read(rio_t *rp, void *usrbuf, size_t n){
    
    int cnt;

    while(rp->rio_cnt <= 0){                        // buf empty, refille the buffer
        if((rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf)))<0){
            if(errno == EINTR){
                rp->rio_cnt = 0;
            }else{
                return -1;
            }
        }else if(rp->rio_cnt==0){                   // EOF
            return 0;
        }else{
            rp->rio_bufptr = rp->rio_buf;           // fill buffer success, set rio_bufptr to the beginning of buffer
        }
    }

    cnt = n;                                        
    if(cnt>rp->rio_cnt){                            
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;                                     // return the actual supplied bytes's size
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen){
    
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n){
    
}