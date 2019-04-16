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

void process_http(int fd){
    int is_static;
    size_t body_len;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;


    /*read request*/
    rio_initb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Headers: \n");
    printf("%s\n", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if(strcasecmp(method, "GET")){
        clienterror(fd, method, "501", "NOT IMPLEMENTED", "BICYCLE WEB SERVER ONLY SUPPORT GET METHOD");
        //TODO: implements posts requests
        return;
    }

    body_len = read_requesthdrs(&rio);
    /*if there is no headers, don't response*/
    if(0==body_len){                        
        return;
    }

    /*parse URI*/
    is_static = parse_uri(uri, filename, cgiargs);
    // printf("[Debug] filename:%s\n", filename);
    if(stat(filename, &sbuf) < 0){
        clienterror(fd, method, "404", "NOT FOUND", "BICYCLE WEB SERVER COULD NOT FIND THIS FILE");
        return;
    }

    if(is_static){
        if( !(S_ISREG(sbuf.st_mode)) || !(S_IRUSR&sbuf.st_mode)){ // S_ISREG: whether a regualr file; S_IRUSR: whether can be read by this user
            clienterror(fd, method, "403", "FORBIDDEN", "BICYCLE WEB SERVER COULD NOT READ THIS FILE");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }else {
        if( !(S_ISREG(sbuf.st_mode)) || !(S_IXUSR&sbuf.st_mode)){ // S_ISREG: whether a regualr file; S_IXUSR: whether can be executed by this user
            clienterror(fd, method, "403", "FORBIDDEN", "BICYCLE WEB SERVER COULD NOT EXECUTE THIS FILE");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

size_t read_requesthdrs(rio_t *rp){
    //TODO: read request headers
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE);
    int rcnt;
    size_t total_cnt = 0;
    rcnt = rio_readlineb(rp, buf, MAXLINE);
    while(strcmp("\r\n", buf) && rcnt!=0){
        total_cnt += rcnt;
        rcnt = rio_readlineb(rp, buf, MAXLINE);
        printf("%s", buf);
    }
    return total_cnt;
}

int parse_uri(char *uri, char *filename, char *cgiargs){
    //TODO: usr can config cgi path
    //TODO: check whether post method
    char *ptr;

    if(!strstr(uri, "cgi-bin")){
        strcpy(cgiargs, "");
        strcpy(filename, ".");
        strcat(filename, uri);
        if('/' == uri[strlen(uri)-1]){
            strcat(filename, "index.html");
        }
        return 1;
    }else{
        ptr = index(uri, "?");
        if(ptr){
            strcpy(cgiargs, ptr+1);
            *ptr = '\0';
        }else{
            strcpy(cgiargs, "");
        }
        strcpy(filename, ".");
        strcat(filename, uri);
        return 0;
    }
}

void get_filetype(char *filename, char *filetype){
    if(strstr(filename, ".html")){
        strcpy(filetype, "text/html");
    }else if(strstr(filename, ".gif")){
        strcpy(filetype, "image/gif");
    }else if(strstr(filename, ".png")){
        strcpy(filetype, "image/png");
    }else if(strstr(filename, ".jpg")){
        strcpy(filetype, "image/jpeg");
    }else{
        strcpy(filetype, "text/plain");
    }
}

void init_file_slice(char* filename, file_slice* fs){
    struct stat statbuf;
    
    fs->fp = fopen(filename, "rb");
    // get file size
    stat(filename, &statbuf);
    fs->file_size = statbuf.st_size;
    fs->beg = 0;
    fs->cnt = 0;
    fs->slice_ptr = (char*) malloc(DEFAULT_FILE_SLICE_SIZE);
}

void destroy_file_sizle(file_slice* fs){
    free(fs->slice_ptr);
    fs->beg = -1;
    fs->cnt = -1;
    fs->file_size = -1;
    close(fs->fp);
}

int next_slice(file_slice *fs){
    int rd_loc;
    
    fs->cnt = 0;
    memset(fs->slice_ptr, 0, DEFAULT_FILE_SLICE_SIZE);
    for(rd_loc=fs->beg;  rd_loc<fs->beg+DEFAULT_FILE_SLICE_SIZE; rd_loc++){
        if(rd_loc >= fs->file_size){
            return -1;
        }
        fseek(fs->fp, rd_loc, SEEK_SET);
        fs->slice_ptr[fs->cnt++] = getc(fs->fp);
    }
}

void serve_static(int fd, char *filename, size_t filesize){
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];

    /*send headers*/
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Bicycle Web Server", buf);
    // sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));

    printf("Response headers:");
    printf("%s", buf);

    /*send body using munmap*/
    // srcfd = open(filename, O_RDONLY, 0);
    // srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); //PORT_READ: can be read; MAP_PRIVATE: do not share this map with other processes
    // close(srcfd);
    // rio_writen(fd, srcp, filesize);
    // munmap(srcp, filesize);

    

}

void serve_dynamic(int fd, char *filename, char * cgiargs){
    char buf[MAXLINE], *emptylist[] = {NULL};

    /*return head*/
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Bicycle Web Server\r\n", buf);
    rio_writen(fd, buf, strlen(buf));

    /*fork a child process to execute the cgi program*/
    if(0==fork()){
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO);                // redirect stdout to client
        execve(filename, emptylist, NULL);
    }
    wait(NULL);
}

void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
    char buf[MAXLINE], body[MAXBUF];

    /*build body*/
    sprintf(body, "<html><title>Bicycle Web Server Error</title>");
    sprintf(body, "%s<body bgcolor=\"ffffff\">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>Bicycle Web Server</em>\r\n", body);

    /*build response*/
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    sprintf(buf, "%sContent-type: text/html\r\n", buf);
    sprintf(buf, "%sCOntent-length: %d\r\n\r\n", buf, (int)strlen(body));
    rio_writen(fd, buf, strlen(buf));
    rio_writen(fd, body, strlen(body));
}

void ride_bicycle(int argc, char** argv){

    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    //TODO: init config from config file
    if(argc!=2){
        fprintf(stderr, "usage: %s <port>", argv[0]);
        exit(1);
    }

    listenfd = open_listenfd(argv[1]);
    printf("listening port: %s\n", argv[1]);
    while(1){
        clientlen = sizeof(clientaddr);
        printf("Waiting for connection...\n");
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
        printf("Accept!\n");
        getnameinfo((struct sockaddr *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accepted connetion from (%s, %s)\n", hostname, port);
        process_http(connfd);
        close(connfd);
    }

}
