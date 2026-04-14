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
    
    if(redis_info.redis_ID == NULL || redis_info.redis_PASS == NULL || redis_info.redis_CRT == NULL){
        printf("cfg file error\n");
        //하나라도 없으면 설정파일 오류
        // 로그추가
        exit(0);
    }
    printf("%s %s %s \n", redis_info.redis_ID, redis_info.redis_PASS, redis_info.redis_CRT);

    // 레디스 로컬 connect 
    ctx = redisConnect("127.0.0.1", 6379);
    // 레디스 로컬 connect
    
    //레디스 서버 connect START
    /*char *env = getenv("REDIS");
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
    }*/
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
    struct_info structInfo[100];
    char *processName = argv[1];
    strcpy(fileName, argv[2]);
    //========== 버퍼로 레디스 읽기 test start ===========
    if(strcmp(processName, "test_bufGet") == 0){
        printf("test_bufGet\n");
        int ab = fileOpen(fileName, &fp);
        int offset = 0;
        int count = 0;
        struct_info info[100];

        while(fscanf(fp, "%s %d", info[count].name, &info[count].length) == 2){
            info[count].offset = offset;
            offset += info[count].length;
            printf("rscanf name : %s, length : %d, offset : %d \n", info[count].name, info[count].length, info[count].offset);
            count++;
        }
        fclose(fp);

        char buf[1024];
        int data_len = 0;
        int qq = redisHgetBuf(ctx, info, "test111", "stat", count, buf);
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
        int a = redisHget(ctx, "testhash1", "stat", &data);        
        printf("flag   : %s\n", data.flag);
        printf("oa_addr: %s\n", data.oa_addr);
        printf("da_addr: %s\n", data.da_addr);
        printf("QID    : %s\n", data.qid);
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
                    else if(strcmp(field, "qid") == 0){
                        mt.qid = atoi(value);
                    }
                    else if(strcmp(field, "drtime") == 0){
                        mt.drtime = atoll(value);
                    }
                }
            }
            
            testCount++;
        }
        printf("==============test data insert end ==============\n");
        redisReply *reply = redisCommand(ctx, "HSET %s stat %b", key, &mt, sizeof(mt));
        freeReplyObject(reply);
        exit(0);
    }

    int fileOpenResult = fileOpen(fileName, &fp);

    char buf[50];
    char *str[100];
    int count = 0;


    if(strcmp(processName, "search") == 0){
        printf("search IN\n");
        if(strcmp(fileName, "search.dat") != 0){
            printf("search file name error %s\n", fileName);
            exit(0);
        }
        search_info searchInfo[100];
        memset(&searchInfo, 0, sizeof(searchInfo));
        char searchBuf[256];
        int searchCount = 0;
        int isSearch = 0;
        int f = 0;
        while(fgets(searchBuf, sizeof(searchBuf), fp) != NULL){
            printf("f str %s \n", searchBuf);
            //첫 단어 스킵
            
            //search.dat 파일 읽어 구조체에 저장
            if(searchCount != 0){
                
                    // \n 제거 
                searchBuf[strcspn(searchBuf, "\n")] = '\0';
                    
                char *line = searchBuf;

                if(line[0] == '#'){
                    line++;
                }
                while(*line == ' '){
                    line++;
                }
                if(*line == '\0'){
                    continue;
                }

                //검색조건 여부
                char *searchData;
                searchData = strtok(line, " ");
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
            }
                        printf("search_info=======\n");
            printf("oa_addr : %s\n", searchInfo[searchCount-1].oa_addr);
            printf("da_addr : %s\n", searchInfo[searchCount-1].da_addr);
            printf("qid : %d\n", searchInfo[searchCount-1].qid);
            printf("start_time : %lld\n", searchInfo[searchCount-1].start_time);
            printf("end_time : %lld\n", searchInfo[searchCount-1].end_time);
            searchCount++;

        }
        if(isSearch == 1){
            printf("dat file error\n");
        }else{
            unsigned long long cursor = 0;
            int scanCount = 0;

            //구조체 컬럼 갯수
            int structCount = structBuffer(structInfo);

            do{
                //해당하는 키 모두 조회
                redisReply *reply = redisCommand(ctx, "SCAN %llu MATCH test_in COUNT 1000", cursor);
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
                    printf("===========key : %s================\n", key);
                    //searchBuf안에 redisHgetBuf 데이터로 차있음
                    char searchBuf[1024];

                    //key값 조회
                    //int bufResult = redisHgetBuf(ctx, structInfo, key, "stat", structCount, searchBuf);
                    int bufResult = redisHgetBuf(ctx, structInfo, key, "stat", structCount, searchBuf);
                    //stdb_field = st;
                    //int bufResult = redisHget(ctx, key, 'stat', &st){
                    if(bufResult < 0){
                        printf("hget으로 값 못가져옴");
                        continue;
                    }

                    char oa_addr[32] = {0};
                    char da_addr[32] = {0};
                    int qid = 0;
                    long drtime = 0;
                    //각 변수에 조회한 key값의 데이터를 저장
                    for (int j = 0; j < structCount; j++){
                        
                        char temp[256] = {0};
                        if(strcmp(structInfo[j].name, "oa_addr") == 0){
                            memcpy(oa_addr, searchBuf+structInfo[j].offset, structInfo[j].length);
                            //temp에 oa_addr값이 들어있음
                        }else if(strcmp(structInfo[j].name, "da_addr") == 0){
                            memcpy(da_addr, searchBuf+structInfo[j].offset, structInfo[j].length);
                        }else if(strcmp(structInfo[j].name, "qid") == 0){
                            memcpy(&qid, searchBuf + structInfo[j].offset, sizeof(int));
                        }else if(strcmp(structInfo[j].name, "drtime") == 0){
                            memcpy(&drtime, searchBuf + structInfo[j].offset, sizeof(long));
                        }
                    }
                    
                    printf("redis data oa_addr : %s\n", oa_addr);

                    printf("redis data da_addr : %s\n", da_addr);
                    printf("redis data qid : %d\n", qid);
                    printf("redis data drtime : %d\n", drtime);
                    printf("searchCount : %d\n", searchCount);
                    for (int j = 0; j < searchCount-1; j++)
                    {
                        printf("j : %d\n", j);
                        //-아니면서 검색조건이 맞지않으면 continue
                        if(strcmp(searchInfo[j].oa_addr, "-") != 0 &&
                                strcmp(searchInfo[j].oa_addr, oa_addr) != 0){
                                    printf("1con\n");
                            continue;
                        }
                        printf("searchInfo[j].da_addr : %s\n", searchInfo[j].da_addr);
                        if(strcmp(searchInfo[j].da_addr, "-") != 0 &&
                                strcmp(searchInfo[j].da_addr, da_addr) != 0){
                                    printf("2con\n");
                            continue;
                        }
                                
                        if(searchInfo[j].qid != 0 && searchInfo[j].qid != qid){
                            printf("3con");
                            continue;
                        }
                                
                        if(searchInfo[j].start_time != 0 &&
                                searchInfo[j].end_time != 0){
                            if(drtime < searchInfo[j].start_time || drtime > searchInfo[j].end_time){
                                printf("4con");
                                continue;
                            }
                        }
                        
                        //파일로 데이터 저장
                        char keyTitle[256];                  //경로 추가
                        snprintf(keyTitle, sizeof(keyTitle), "%s.dat", key);
                        FILE *searchFile = fopen(keyTitle, "w");
                        for (int k = 0; k < structCount; k++){
                            char temp[256] = {0};
                            printf("dat w name : %s ", structInfo[k].name);
                            printf("  : %.*s ", searchBuf+structInfo[k].offset);
                            memcpy(temp, searchBuf+structInfo[k].offset, structInfo[k].length);
                            temp[structInfo[k].length] = '\0';
                            
                            fprintf(searchFile, "%s : %.*s\n", structInfo[k].name, structInfo[k].length,
                                                                    searchBuf+structInfo[k].offset);
                            //  데이터,     크기,   개수,   fp
                           // fwrite(lineStr, strlen(lineStr), 1, searchFile);
                        }
                        fclose(fp);
                    }
                }
                freeReplyObject(reply);
            }while(cursor != 0);

        }
            //로직추가할것
        exit(0);
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

//구조체로 get
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


int structBuffer(struct_info *info){
    FILE *structFile;
    int fileResult = fileOpen("msg_structure.dat", &structFile);
    if(fileResult < 0){
        return -1;
    }
    int offset = 0;
    int count = 0;
    int align = 1;

    //name, length, align 값 읽기
    while(fscanf(structFile, "%s %d %d", info[count].name, &info[count].length, &align) == 3){
        if(count >= 100){
            printf("배열 넘김");
            return -1;
        }
        //패딩 적용: offset을 align 배수로 올림
        if(align > 1){
            offset = (offset + align - 1) / align * align;
        }
        info[count].offset = offset;
        offset += info[count].length;

        //printf("rscanf name : %s, length : %d, offset : %d \n", info[count].name, info[count].length, info[count].offset);
        count++;
    }
    fclose(structFile);    
    return count;
}

//                레디스 접속정보   구조받을 구조체                             
int redisHgetBuf(redisContext *c, struct_info *info, char *key, char *field, int count, char *buffer){
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
        
        printf("redisHgetBuf %s: ", info[i].name);
        printf(" / %.*s\n", info[i].length, temp);
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
        //로그추가
        exit(0);
    }

    char line[512];
    int isSection = 0;
    while(fgets(line, sizeof(line), fp_profile)){
        
        line[strcspn(line, "\n")] = 0;

        if(line[0] == '#' || line[0] == '\0'){
            continue;
        }

        //특정 섹션만 찾기
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

        char *eq = strchr(line, '=');
        if(!eq){
            continue;
        }

        *eq = '\0';

        char *key = line;
        char *value = eq + 1;
        //strcpy == 그대로 복사
        //strncpy == 길이 제한 복사
        if(strcmp(section, "REDIS") == 0){
            if(strcmp(key, "redis_ID") == 0){
                strncpy(info->redis_ID, value, sizeof(info->redis_ID)-1);
            }else if(strcmp(key, "redis_PASS") == 0){
                strncpy(info->redis_PASS, value, sizeof(info->redis_PASS)-1);
            }else if(strcmp(key, "redis_CRT") == 0){
                strncpy(info->redis_CRT, value, sizeof(info->redis_CRT)-1);
            }
        }else if(strcmp(section, "STRUCT") == 0){

        }

    }
    fclose(fp_profile);
}


