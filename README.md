# Bicycle Server

A web server implemented during the class "Linux Kernal Analysis".

## Dictionary:


    .
    ├── compile.sh
    ├── README.md
    ├── resource
    ├── src
    └── target

1. compile.sh: shell script for compiling
2. resource: dictionary which stores static resources, all files/dict in this folder will be copied to target
3. src: source code
4. target: program and static resources

## Usage:

### Compile:

Run following command under the root of project
    
```shell
    $ ./compile.sh
```

Programs and static resources will be in the ./target

### Run:

```shell
    cd target
    ./bicyclewebser <port>
```

### Functions:

1. Support GET method
2. Support html/pdf/gif/jpg/png
3. Use SIGCHLD and signal to process cgi children process
4. Use malloc, fread and rio_writen to upload static resources
5. Use random access and divide big files into slices during responding static requests in order to save main memory

## Confuse:

When I read a 4M pdf file using rio_readn, it can not read the entire file; But it is ok with fread, I don't know why