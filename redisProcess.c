#include "redisProcess.h"


int main(int argc, char **argv) {
    if(argc < 3){
        //인자값 부족
        printf("argc down");
        exit( 0 );
    } 

    redisContext *ctx;
    redisSSLContext *ssl;
    redisSSLContextError sslError = REDIS_SSL_CTX_NONE;
    redisReply *reply;

    
    redis_info redis_info;
    getProfile("REDIS", &redis_info);
    
    if(redis_info.redis_ID == NULL || redis_info.redis_PASS == NULL || redis_info.redisCRT == NULL){
        //하나라도 없으면 설정파일 오류 로그추가
        exit(0);
    }

    // 레디스 로컬 connect 
    //ctx = redisConnect("127.0.0.1", 6379);
    // 레디스 로컬 connect
    
    //레디스 서버 connect START
    char *env = getenv("REDIS");
    if(env == NULL){
        printf("REDIS env NULL \n");
        exit(0);
    }
    char cp[128];
    memset(cp, 0x00, sizeof(cp));
    sprintf(cp, "%s", env);
    struct timeval timeout = {1, 500000};
    ctx = redisConnectWithTimeout((char*)cp, 6379, timeout);
    if(ctx == NULL || ctx->err){
        //로그추가
        redisFree(ctx);
        exit(0);
    }


    ssl = redisCreateSSLContext(redis_info.redis_CRT, NULL, NULL, NULL, NULL, &ssl_error);
    if(!ssl || ssl_error != REDIS_SSL_CTX_NONE){
        //오류로그 추가
        exit(0);
    }

    redisOptions options = {0};
    REDIS_OPTIONS_SET_TCP(&options, env, 6379);
    ctx = redisConnectWithOptions(&options);
    if(ctx == NULL || ctx -> err){
        //오류로그
    }
    //레디스 서버 connect END



    reply = (redisReply *) redisCommand(ctx, "AUTH %s %s", redis_info.redis_ID, redis_info.redis_PASS);
    if(reply == NULL || ctx->err){
        //오류로그
        exit(0);
    }
    freeReplyObject(reply);

    printf("Redis Connected Success\n");

    char fileName[100]; 
    FILE *fp;
    field_info fieldInfo[100];
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
        int qq = redisHgetBuf(c, info, "test111", "stat", count, buf);
        if(qq < 0){
            printf("no data\n");
        }
        
        for (int i = 0; i < count; i++){
            char temp[256] = {0};
            memcpy(temp, buf + info[i].offset, info[i].length);

            printf("%s: ", info[i].name);
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

    int fileOpenResult = fileOpen(fileName, &fp);

    char buf[50];
    char *str[100];
    int count = 0;


    if(strcmp(processName, "search") == 0){
        printf("search IN\n");
        search_info searchInfo[100];
        memset(&searchInfo, 0, sizeof(searchInfo));
        char searchBuf[256];
        int searchCount = 0;
        int isSearch = 0;
        while(fgets(searchBuf, sizeof(searchBuf), fp) != NULL){
            //첫 단어 스킵
            int f = 0;
            //search.dat 파일 읽어 구조체에 저장
            if(searchCount != 0){
                if(f > 0){
                    // \n 제거 
                    searchBuf[strcspn(searchBuf, "\n")] = '\0';
                    
                    //검색조건 여부
                    char *searchData;
                    searchData = strtok(searchBuf, " ");
                    if(!searchData){
                        isSearch = 1;
                        break;
                    }
                    strcpy(searchInfo[searchCount-1].oa_addr, searchData);
                    
                    searchData = strtok(NULL, " ");
                    if(!searchData){
                        isSearch = 1;
                        break;
                    }
                    strcpy(searchInfo[searchCount-1].da_addr, searchData);
                    
                    searchData = strtok(NULL, " ");
                    if(!searchData){
                        isSearch = 1;
                        break;
                    }
                    searchInfo[searchCount-1].qid = atoi(searchData);
                    
                    searchData = strtok(NULL, " ");
                    if(!searchData){
                        isSearch = 1;
                        break;
                    }
                    //검색조건 search_info 구조체에 저장
                    if(strcmp(searchData, "-") != 0){
                        char *searchTime = strtok(searchData, "~");
                        searchInfo[searchCount-1].start_time = atoll(searchTime);
                        searchTime = strtok(NULL, "~");
                        searchInfo[searchCount-1].end_time = atoll(searchTime);
                    }else{
                        searchInfo[searchCount-1].start_time = 0;
                        searchInfo[searchCount-1].end_time = 0;
                    }
                }else{
                    f++;
                }
            }
            searchCount++;
        }
        if(isSearch == 1){
            printf("잘못된 dat파일");
        }else{
            unsigned long long cursor = 0;
            int scanCount = 0;

            int structCount = structBuffer(fieldInfo);

            do{
                //해당하는 키 모두 조회
                redisReply *reply = redisCommand(c, "SCAN %llu MATCH RECOVER_M COUNT 1000", cursor);
                if(!reply){
                    //에러로그
                    return -1;
                }
                cursor = strtoull(reply->element[0]->str, NULL, 10);
                //scan해서 key값만 가져옴
                redisReply *keys = reply->element[1];

                //가져온 key값만 하나씩 반복문
                for (int i = 0; i < keys->elements; i++) {
                    char *key = keys->element[i]->str;

                    //searchBuf안에 redisHgetBuf 데이터로 차있음
                    char searchBuf[1024];

                    //key값 조회
                    int bufResult = redisHgetBuf(c, fieldInfo, key, "stat", structCount, searchBuf);
                    if(bufResult < 0){
                        printf("hget으로 값 못가져옴");
                        continue;
                    }

                    char oa_addr[32] = {0};
                    char da_addr[32] = {0};
                    int qid = 0;
                    long drtime = 0;
                    //각 변수에 조회한 key값의 데이터를 저장
                    for (int j = 0; j < count; j++){
                        char temp[256] = {0};
                        if(strcmp(fieldInfo[j].name, "oa_addr") == 0){
                            memcpy(oa_addr, searchBuf+fieldInfo[j].offset, fieldInfo[j].length);
                            //temp에 oa_addr값이 들어있음
                        }else if(strcmp(fieldInfo[j].name, "da_addr") == 0){
                            memcpy(da_addr, searchBuf+fieldInfo[j].offset, fieldInfo[j].length);
                        }else if(strcmp(fieldInfo[j].name, "qid") == 0){
                            if(fieldInfo[j].length == sizeof(int)){
                                memcpy(&qid, searchBuf+fieldInfo[j].offset, sizeof(int));
                            }
                        }else if(strcmp(fieldInfo[j].name, "drtime") == 0){
                            //windows long 4, linux 8
                            if(fieldInfo[j].length == sizeof(long)){
                                memcpy(&drtime, searchBuf+fieldInfo[j].offset, sizeof(long));
                            }
                        }
                    }
                    
                    for (int j = 0; j < searchCount-1; j++)
                    {
                        //-아니면서 검색조건이 맞지않으면 continue
                        if(strcmp(searchInfo[j].oa_addr, "-") != 0 &&
                                strcmp(searchInfo[j].oa_addr, oa_addr) != 0){
                            continue;
                        }
                                
                        if(strcmp(searchInfo[j].da_addr, "-") != 0 &&
                                strcmp(searchInfo[j].da_addr, da_addr) != 0){
                            continue;
                        }
                                
                        if(searchInfo[j].qid != 0 && searchInfo[j].qid != qid){
                            continue;
                        }
                                
                        if(searchInfo[j].start_time != 0 &&
                                searchInfo[j].end_time != 0){
                            if(drtime < searchInfo[j].start_time || drtime > searchInfo[j].end_time){
                                continue;
                            }
                        }
                        
                        //파일로 데이터 저장
                        char keyTitle[256];                  //경로 추가
                        snprintf(keyTitle, sizeof(keyTitle), "%s.dat", key);
                        FILE *searchFile = fopen(keyTitle, "w");
                        for (int k = 0; k < count; k++){
                            char temp[256] = {0};
                            
                            memcpy(temp, searchBuf+fieldInfo[k].offset, fieldInfo[k].length);
                            temp[fieldInfo[k].length] = '\0';
                            
                            fprintf(searchFile, "%s : %.*s\n", fieldInfo[k].name, fieldInfo[k].length,
                                                                    searchBuf+fieldInfo[k].offset);
                            //  데이터,     크기,   개수,   fp
                           // fwrite(lineStr, strlen(lineStr), 1, searchFile);
                        }
                    }
                }

            }

        }
            //로직추가할것
        exit(0);
    }
}
    else if(strcmp(processName, "update") == 0){
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
        return -1;
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


int structBuffer(field_info *info){
    FILE *structFile;
    int fileResult = fileOpen("msg_structure.dat", &structFile);
    if(fileResult < 0){
        return -1;
    }
    int offset = 0;
    int count = 0;

    //name, length 값 넣고
    while(fscanf(structFile, "%s %d", info[count].name, &info[count].length) == 2){
        if(count >= 100){
            printf("배열 넘김");
            return -1;
        }
        //offset 값
        info[count].offset = offset;
        offset += info[count].length;
        
        //printf("rscanf name : %s, length : %d, offset : %d \n", info[count].name, info[count].length, info[count].offset);
        count++;
    }
    fclose(structFile);    
    return count;
}

//                레디스 접속정보   구조받을 구조체                             
int redisHgetBuf(redisContext *c, field_info *info, char *key, char *field, int count, char *buffer){
    redisReply *reply;

    //reply = redisCommand((c), "HGET %s %s", key, field);
    reply = redisCommand((c), "HGET %s stat", key);

    if (!reply) {
        printf("Redis data null\n");
        return -1;
    }
    memcpy(buffer, reply->str, reply->len);

    for (int i = 0; i < count; i++)
    {
        char temp[256] = {0};
        memcpy(temp, buffer + info[i].offset, info[i].length);
        
        printf("%s: ", info[i].name);
        printf("%.*s\n", info[i].length, temp);
    }
    freeReplyObject(reply);
    return 0;
}


void searchProcess(redisContext *c){

}

char **redisScan(redisContext *c, char *searchStr, int *keyCount) {

    unsigned long long cursor = 0;
    int count = 0;
    int listSize = 100;

    char **result = malloc(sizeof(char*) * listSize);
    
    do{
        redisReply *reply;
        reply = redisCommand(c, "SCAN %llu MATCH %s COUNT 1000", cursor, searchStr);
        if(!reply){
            printf("scan error \n");
            return NULL;
        } 
        cursor = strtoull(reply->element[0]->str, NULL, 10);

        //scan 결과값 중 key값만 가져옴
        redisReply *keys = reply->element[1];
        for (int i = 0; i < keys->elements; i++)
        {
            if(count >= listSize){
                listSize *= 2;
                char **tmp = realloc(result, sizeof(char*) * listSize);
                if (!tmp) {
                    freeReplyObject(reply);
                    return NULL;
                }
                result = tmp;
            
            }
            result[count] = strdup(keys->element[i]->str);
            count++;
        }
        freeReplyObject(reply);
    }while(cursor != 0);

    *keyCount = count;
    return result;
}

void getProfile(char *section, redis_info *info){
    FILE *fp_profile;
    int file_open = fileOpen("redisProcess.cfg", &fp_profile);
    if(file_open < 0){
        exit(0);
    }
    char line[512];

    while(fgets(line, sizeof(line), fp_profile)){
        line[strcspn(line, "\n")] = 0;

        if(line[0] == '#' || line[0] == '\0'){
            continue;
        }

        if(line[0] == '['){
            if(strcmp(line, section)){
                isSection = 1;
            }else{
                isSection = 0;
            }
            continue;
        }

        if(!isSection){
            continue;
        }

        char *eq = strchr(line, "=");
        if(!eq){
            continue;
        }

        *eq = '\0';

        char *key = line;
        char *value = eq + 1;
        if(strcmp(section, "REDIS")){
            if(strcmp(key, "redis_ID") == 0){
                strcpy(info->redis_ID, value, sizeof(info->redis_ID)-1);
            }else if(strcmp(key, "redis_PASS") == 0){
                strcpy(info->redis_PASS, value, sizeof(info->redis_PASS)-1);
            }else if(strcmp(key, "redis_CRT") == 0){
                strcpy(info->redis_CRT, value, sizeof(info->redis_CRT)-1);
            }
        }else if(strcmp(section, "STRUCT")){

        }

    }
    fclose(fp_profile);
}


