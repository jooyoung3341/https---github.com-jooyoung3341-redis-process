#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <hiredis/hiredis.h>
#include <hiredis/hiredis_ssl.h>
#include <time.h>

typedef struct {
    char flag[2];
    char oa_addr[32];
    char da_addr[32];
    int qid;
    long drtime;
} stdb_field;

typedef struct {
    char name[32];
    char type[10];
    int offset;
    int length;
} struct_info;
 
typedef struct {
    char oa_addr[32];
    char da_addr[32];
    int qid;
    long start_time;
    long end_time;
} search_info;

typedef struct {
    char redis_ID[256];
    char redis_PASS[256];
    char redis_CRT[256];
} redis_info;


int fileOpen(char *fileName, FILE **fp);
int redisHget(redisContext *c, char *key, char *field, stdb_field *da);
int redisHgetBuf(redisContext *c, field_info *info, char *key, char *field, int count, char *buffer);
int structBuffer(field_info *fieldInfo);
void getProfile(char *section, redis_info *info);

