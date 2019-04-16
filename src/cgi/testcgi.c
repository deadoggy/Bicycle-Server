#include<stdio.h>
#include <stdlib.h>
#include<string.h>

#define MAX_ARGC 100

int main(int argc, char** argv){

    char* qry_str = getenv("QUERY_STRING");
    
    printf("QUERY_STRING = <%s>\r\n", qry_str);

    return 0;
}