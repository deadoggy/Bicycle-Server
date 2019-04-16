#!/bin/bash

cp -r resource/. target/

if [ ! -d "target/" ]; then 
    mkdir target/
fi

if [ ! -d "target/cgi-bin" ]; then 
    mkdir target/cgi-bin
fi

gcc -g src/rio.c src/bicyclewebserv.c src/main.c -o target/bicyclewebserv
gcc -g src/cgi/testcgi.c -o target/cgi-bin/testcgi

