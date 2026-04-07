#include "redisProcess.h"


int main(int argc, char **argv) {
    if(argc < 3){
        //인자값 부족
        printf("argc down");
        exit( 0 );
    } 

    redisContext *c;
        // 레디스 로컬 connect
    c = redisConnect("127.0.0.1", 6379);
    if (c == NULL || c->err) {
        printf("Redis Connection error: %s\n", c->errstr);
        redisFree(c);
        return 0;
    }
    printf("Redis Connected Success\n");

    char fileName[100]; 
    FILE *fp;
    char *processName = argv[1];
    strcpy(fileName, argv[2]);
    //========== 버퍼로 레디스 읽기 test start ===========
    if(strcmp(processName, "test_bufGet") == 0){
        printf("test_bufGet\n");
        int ab = fileOpen(fileName, &fp);
        int offset = 0;
        int count = 0;
        field_info info[100];

        while(fscanf(fp, "%s %d", info[count].name, &info[count].length) == 2){
            info[count].offset = offset;
            offset += info[count].length;
            printf("rscanf name : %s, length : %d, offset : %d \n", info[count].name, info[count].length, info[count].offset);
            count++;
        }
        fclose(fp);

        char buf[1024];
        int data_len = 0;
        int qq = redisHgetBuf(c, "test111", "stat", buf, &data_len);
        if(qq < 0){
            printf("no data\n");
        }
        
        for (int i = 0; i < count; i++){
            char temp[256] = {0};
            memcpy(temp, buf + info[i].offset, info[i].length);

            printf("%s: ", info[i].name);
            printf("%.*s\n", info[i].length, temp);
        }
        exit(0);
    }
    //========== 버퍼로 레디스 읽기 test end ===========
    
    //========== 레디스 읽기 test start ===========
    stdb_field data;
    if(strcmp(processName, "test_redisGet") == 0){
        printf("test_redisGet\n");
        int a = redisHget(c, "testhash1", "stat", &data);        
        printf("flag   : %s\n", data.flag);
        printf("oa_addr: %s\n", data.oa_addr);
        printf("da_addr: %s\n", data.da_addr);
        //printf("QID    : %s\n", data.QID);
        printf("drtime : %s\n", data.drtime);
        exit(0);
    }   
    //========== 레디스 읽기 test end ===========
    
    //=========== 레디스 데이터 삽입 test start===========
    if(strcmp(processName, "test_redisInsert") == 0){
        printf("test_redisInsert\n");
        int testFile = fileOpen(fileName, &fp);
        char testBuf[50];
        char *testStr[100];
        int testCount = 0;
        stdb_field mt;
        memset(&mt, 0, sizeof(mt));
        char key[50];
        printf("==============test data insert start ==============\n");
        while(fgets(testBuf, sizeof(testBuf), fp) != NULL){
            testBuf[strcspn(testBuf, "\n")] = 0;
            
            testStr[testCount] = strdup(testBuf);
            if(testCount == 0){
                strcpy(key, testStr[testCount]);
            }else{
                char *field = strtok(testBuf, "|");
                char *value = strtok(NULL, "|");
            
                if(field && value){
                    if(strcmp(field, "flag") == 0){
                        strncpy(mt.flag, value, sizeof(mt.flag)-1);
                    }
                    else if(strcmp(field, "oa_addr") == 0){
                        strncpy(mt.oa_addr, value, sizeof(mt.oa_addr)-1);
                    }
                    else if(strcmp(field, "da_addr") == 0){
                        strncpy(mt.da_addr, value, sizeof(mt.da_addr)-1);
                    }
                    //else if(strcmp(field, "QID") == 0){
                    //    strncpy(mt.QID, value, sizeof(mt.QID)-1);
                    //}
                    else if(strcmp(field, "drtime") == 0){
                        strncpy(mt.drtime, value, sizeof(mt.drtime)-1);
                    }
                }
            }
            
            testCount++;
        }
        printf("==============test data insert end ==============\n");
        redisReply *reply = redisCommand(c, "HSET %s stat %b", key, &mt, sizeof(mt));
        freeReplyObject(reply);
        exit(0);
    }
    //=========== 레디스 데이터 삽입 test end ===========

    /*if(strcmp(processName, "test_redisInsert") == 0){
        printf("test_redisInsert\n");
        int testFile = fileOpen(fileName, &fp);
        char testBuf[50];
        char *testStr[100];
        int testCount = 0;
        
        
        char key[50];
        printf("==============test data insert start ==============\n");
        while(fgets(testBuf, sizeof(testBuf), fp) != NULL){
            testBuf[strcspn(testBuf, "\n")] = 0;
            
            testStr[testCount] = strdup(testBuf);
            if(testCount == 0){
                strcpy(key, testStr[testCount]);
            }else{
                char *field = strtok(testBuf, "|");
                char *value = strtok(NULL, "|");
            
                if(field && value){
                    if(strcmp(field, "flag") == 0){
                        strncpy(mt.flag, value, sizeof(mt.flag)-1);
                    }
                    else if(strcmp(field, "oa_addr") == 0){
                        strncpy(mt.oa_addr, value, sizeof(mt.oa_addr)-1);
                    }
                    else if(strcmp(field, "da_addr") == 0){
                        strncpy(mt.da_addr, value, sizeof(mt.da_addr)-1);
                    }
                    //else if(strcmp(field, "QID") == 0){
                    //    strncpy(mt.QID, value, sizeof(mt.QID)-1);
                    //}
                    else if(strcmp(field, "drtime") == 0){
                        strncpy(mt.drtime, value, sizeof(mt.drtime)-1);
                    }
                }
            }
            
            testCount++;
        }
        printf("==============test data insert end ==============\n");
        redisReply *reply = redisCommand(c, "HSET %s stat %b", key, &mt, sizeof(mt));
        freeReplyObject(reply);
        exit(0);
    }*/

    int fileOpenResult = fileOpen(fileName, &fp);

    char buf[50];
    char *str[100];
    int count = 0;


    if(strcmp(processName, "search") == 0){
        printf("search IN\n");
        exit(0);
    }else if(strcmp(processName, "update") == 0){
        exit(0);
    }else if(strcmp(processName, "migration") == 0){
        exit(0);
    }else{
        printf("processName error \n");
        exit(0);
    }

}

int fileOpen(char *fileName, FILE **fp){
    *fp = fopen(fileName, "r");
    if(*fp == NULL){
        printf("%s fileOpen fail", fileName);
        return 1;
    }
    return 0;
}


int redisHset(redisContext *c, char *key, char *fieldBuffer, int dataLen){
    redisReply *reply = redisCommand(c, "HSET %s stat %b", key, fieldBuffer, dataLen);
    if(!reply){
        return -1;
    }
    freeReplyObject(reply);
    return 0;
}

int redisHget(redisContext *c, char *key, char *field, stdb_field *data){
    redisReply *reply;

    reply = redisCommand((c), "HGET %s %s", key, field);

    if (reply && reply->type == REDIS_REPLY_STRING) {
        memcpy(data, reply->str, sizeof(stdb_field));

        fwrite(reply->str, 1, reply->len, stdout);
        printf("\n");
        
    }
    freeReplyObject(reply);
    return 0;
}

int redisHgetBuf(redisContext *c, char *key, char *field, char *buffer, int *dataLen){
    redisReply *reply;

    reply = redisCommand((c), "HGET %s %s", key, field);

    if (!reply) {
        printf("Redis data null\n");
        return -1;
    }
    memcpy(buffer, reply->str, reply->len);
    *dataLen = reply->len;

    freeReplyObject(reply);
    return 0;
}

