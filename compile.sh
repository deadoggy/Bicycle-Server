#!/bin/bash


if [ ! -d "target/" ]; then 
    mkdir target/
fi

if [ ! -d "target/cgi-bin" ]; then 
    mkdir target/cgi-bin
fi

cp -r resource/. target/
gcc -g src/rio.c src/bicyclewebserv.c src/main.c -o target/bicyclewebserv
gcc -g src/cgi/testcgi.c -o target/cgi-bin/testcgi

