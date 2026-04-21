#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <hiredis/hiredis.h>
#include <hiredis/hiredis_ssl.h>
#include <time.h>

typedef struct {
    char flag;
    char oa_addr[32];
    char da_addr[32];
    int qid;
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

typedef struct {
    char name[32];
    char value[128];
} update_info;

typedef struct {
    char name[32];
    int length;
    char type[10];
    char function[2];
    char value[128];
} migration_info;

int fileOpen(char *fileName, FILE **fp);
int redisHget(redisContext *c, char *key, char *field, stdb_field *da);
int redisHgetBuf(redisContext *c, struct_info *info, char *key, int count, char *buffer);
int structBuffer(struct_info *structInfo);
void getProfile(char *section, redis_info *info);
int redisHset(redisContext *c, char *key, char *fieldBuffer, int dataLen);
int getAlign(char *type);
