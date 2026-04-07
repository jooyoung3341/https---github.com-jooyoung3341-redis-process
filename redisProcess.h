#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <hiredis/hiredis.h>
typedef struct {
    char flag[2];        
    char oa_addr[20];
    char da_addr[20];
   // char QID[10];
    char drtime[20];
} stdb_field;

typedef struct {
    char name[32];
    int offset;
    int length;
    
} field_info;

int fileOpen(char *fileName, FILE **fp);
int redisHget(redisContext *c, char *key, char *field, stdb_field *da);
int redisHgetBuf(redisContext *c, char *key, char *field, char *buffer, int *out_len);