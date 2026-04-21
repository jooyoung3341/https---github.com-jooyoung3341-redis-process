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
    printf("processName :: %s\n ", processName);
    //========== 버퍼로 레디스 읽기 test start ===========
    if(strcmp(processName, "test_bufGet") == 0){
        printf("test_bufGet\n");

        struct_info bufInfo[100];
        int structResult = structBuffer(bufInfo);
        char strBuf[1028];
        int bufResult = redisHgetBuf(ctx, bufInfo, fileName, structResult, strBuf);

        for (int i = 0; i < structResult; i++){
            char temp[256] = {0};
            memcpy(temp, strBuf + bufInfo[i].offset, bufInfo[i].length);

            if(strcmp(bufInfo[i].type, "char") == 0){
                printf("%s: %s\n", bufInfo[i].name, temp);
            }else{
                int val = 0;
                memcpy(&val, temp, sizeof(int));
                printf("%s: %d\n", bufInfo[i].name, val);
            }

        }
        exit(0);
    }
    //========== 버퍼로 레디스 읽기 test end ===========
    
    //========== 구조체로 레디스 읽기 test start ===========
    stdb_field data;
    if(strcmp(processName, "test_redisGet") == 0){
        printf("test_redisGet\n");
        int a = redisHget(ctx, fileName, "stat", &data);        
        printf("flag   : %c\n", data.flag);
        printf("oa_addr: %s\n", data.oa_addr);
        printf("da_addr: %s\n", data.da_addr);
        printf("QID    : %d\n", data.qid);
        exit(0);
    }   
    //========== 레디스 읽기 test end ===========
    
    //========== 버퍼로 레디스 삽입 start ====================
    if(strcmp(processName, "test_bufInsert") == 0){
        printf("test_bufInsert\n");
        int offset = 0;
        int count = 0;

        char type[32], name[32];
        int length;
        struct_info bufInfo[100];
        int structResult = structBuffer(bufInfo);

        char buffer[1028];
        memset(buffer, 0, sizeof(buffer));
        int total = 0;
        for (int i = 0; i < structResult; i++){
            if(strcmp(bufInfo[i].name, "flag") == 0){
                printf("test_bufInsert name : %s\n", bufInfo[i].name);
                memcpy(buffer+bufInfo[i].offset, "A", bufInfo[i].length);
            }else if(strcmp(bufInfo[i].name, "oa_addr") == 0 || strcmp(bufInfo[i].name, "da_addr") == 0){
                printf("test_bufInsert name : %s\n", bufInfo[i].name);
                memset(buffer + bufInfo[i].offset, 0, bufInfo[i].length);
                memcpy(buffer+bufInfo[i].offset, "01011112222", strlen("01011112222"));
            }else if(strcmp(bufInfo[i].name, "qid") == 0){
                printf("test_bufInsert name : %s\n", bufInfo[i].name);
                int val = 3;
                memcpy(buffer+bufInfo[i].offset, &val, bufInfo[i].length);
            }
            total = bufInfo[structResult - 1].offset + bufInfo[structResult - 1].length;
        }
        //데이터 테스트 출력
        /*for (int i = 0; i < total; i++) {
            printf("%02X ", (unsigned char)buffer112[i]);
        }*/
        //데이터 테스트 출력
        //printf("\n");

        int a = redisHset(ctx, fileName, buffer, total);
        exit(0);
    }
    //========== 레디스 버퍼 삽입 end ====================

    //=========== 레디스 구조체 데이터 삽입 test start===========
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
                        mt.flag = value[0];
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
                    /*else if(strcmp(field, "drtime") == 0){
                        mt.drtime = atoll(value);
                    }*/
                }
            }
            
            testCount++;
        }
        printf("==============test data insert end ==============\n");
        char redis_key[128];
        sprintf(redis_key, "%s_%ld", key, time(NULL));
        redisReply *reply = redisCommand(ctx, "HSET %s stat %b", redis_key, &mt, sizeof(mt));
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
            printf("search start \n");
            unsigned long long cursor = 0;
            int scanCount = 0;

            //구조체 컬럼 갯수
            int structCount = structBuffer(structInfo);

            do{
                //해당하는 키 모두 조회
                //scan 나눠서 테스트 진행 해야 함
                redisReply *reply = redisCommand(ctx, "SCAN %llu MATCH testin*", cursor);
                printf("cursor : %ld\n", cursor);
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
                    int bufResult = redisHgetBuf(ctx, structInfo, key, structCount, searchBuf);
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
                            printf("dat w name : %s \n", structInfo[k].name);
                            printf("  : %.*s \n", searchBuf+structInfo[k].offset);
                            memcpy(temp, searchBuf+structInfo[k].offset, structInfo[k].length);
                            temp[structInfo[k].length] = '\0';
                            printf("structInfo[k].type : %s\n", structInfo[k].type);
                            if(strcmp(structInfo[k].type, "char") == 0){
                                fprintf(searchFile, "%s : %.*s\n", structInfo[k].name, structInfo[k].length,
                                                                    searchBuf+structInfo[k].offset);
                            }else if(strcmp(structInfo[k].type, "int") == 0){
                                int val;
                                memcpy(&val, searchBuf + structInfo[k].offset,  structInfo[k].length);

                                fprintf(searchFile, "%s : %d\n", structInfo[k].name, val);
                            }else if(strcmp(structInfo[k].type, "long") == 0){
                                long val;
                                memcpy(&val, searchBuf + structInfo[k].offset,  structInfo[k].length);

                                fprintf(searchFile, "%s : %lld\n", structInfo[k].name, val);
                            }

                            //  데이터,     크기,   개수,   fp
                           // fwrite(lineStr, strlen(lineStr), 1, searchFile);
                        }
                        fclose(fp);
                    }
                }
                freeReplyObject(reply);
            }while(cursor != 0);

        }
        exit(0);
    }else if(strcmp(processName, "update") == 0){
        printf("update IN\n");

        char keyVal[128] = {0};
        char *temp = strchr(fileName, '.');
        if(temp){
            int len = temp - fileName;
            strncpy(keyVal, fileName, len);
            keyVal[len] = '\0';
        }

        int updateCount = 0;
        char fileBuf[256];

        update_info updateInfo[100];
        char buffer[256];
        //update.dat 파일 읽어서 구조체에 저장
        while(fgets(fileBuf, sizeof(fileBuf), fp) != NULL){
            if(updateCount != 0){
                fileBuf[strcspn(fileBuf, "\n")] = '\0';
                
                char *updateData;
                updateData = strtok(fileBuf, " ");
                strcpy(updateInfo[updateCount-1].name, updateData);

                updateData = strtok(NULL, " ");
                strcpy(updateInfo[updateCount-1].value, updateData);

            }
            printf("update.dat info ==========\n");
            printf("name : %s ", updateInfo[updateCount-1].name);
            printf(" / value : %s \n", updateInfo[updateCount-1].value);
            updateCount++;
        }
        
        //exit(0);
        int structCount = structBuffer(structInfo);

        char updateBuf[1024];
        int hgetResult = redisHgetBuf(ctx, structInfo, keyVal, structCount, updateBuf);

        if(hgetResult < 0){
            //값 못가져옴
            exit(0);
        }
        //예시
/*        int qid = 123;
        memcpy(buffer + offset, &qid, sizeof(int));

        long long drtime = 202604020910;
        memcpy(buffer + offset, &drtime, sizeof(long long));
*/

        //updateBuf에는 파일 이름의 key값을 레디스에서 조회한 데이터가 들어가져있음,
        //update.dat파일에 있는 컬럼을 가지고 반복문을 돌아 컬럼을 찾으면 
        //struct_info에 type에 맞게 넣는 방식 다르게 해야함
        //

        //offset~length 까지 수정하는 내용을 덮어씌움
        //그 다음 최종 로그를 찍고 redis에 set, 사용한 파일은 done경로로 이동
        for (int i = 0; i < updateCount-1; i++){
            for (int j = 0; j < structCount; j++){
                if(strcmp(updateInfo[i].name,structInfo[j].name) == 0){
                    printf("=========%d========", j);
                    printf("updateInfo name : %s \n",updateInfo[i].name);
                    printf("updateInfo value : %s \n",updateInfo[i].value);
                    printf("structInfo name : %s \n",structInfo[j].name);

                    char *temp[256];
                    memcpy(temp, updateBuf + structInfo[j].offset, structInfo[j].length);
        
        
                    printf("structInfo value : %.*s\n", structInfo[j].length, temp);
                    printf("=========%d========", j);
                    //exit(0);
                    //수정할 컬럼이랑 구조체 이름이랑 같을경우
                    //buffer를 잘라 수정된 내용으로 덮어씌움
                    //타입별로 덮어씌우는 방식이 다름
                    char *strType = structInfo[j].type;
                    printf("str type : %s \n", strType);
                    int offset = structInfo[j].offset;
                    memset(updateBuf+offset, 0, structInfo[j].length);
                    if(strcmp(strType, "char") == 0){
                        printf("%d update %s char %.*s -> %s\n", j, updateInfo[i].name,structInfo[j].length, temp, updateInfo[i].value);
                        memcpy(updateBuf+offset, updateInfo[i].value, strlen(updateInfo[i].value));
                    }else if(strcmp(strType, "int") == 0){
                        printf("%d update %s int %.*s -> %s\n", j, updateInfo[i].name,structInfo[j].length, temp, updateInfo[i].value);
                        int val = atoi(updateInfo[i].value);
                        memcpy(updateBuf+offset, &val, sizeof(val));
                    }else if(strcmp(strType, "long") == 0){
                        printf("%d update long %.**s -> %s\n", j, structInfo[j], updateInfo[i].value);
                        long val = atoll(updateInfo[i].value);
                        memcpy(updateBuf+offset, &val, sizeof(val));
                    }
                }
            }
            
        }
        int hsetResult = redisHset(ctx, keyVal, updateBuf, sizeof(updateBuf));
        if(hsetResult < 0){
            printf("hset error\n");
            //오류로그
            exit(0);
        }
        printf("success\n");
        exit(0);
        if(rename("지금파일위치경로/key.dat", "이동할경로/key.dat") != 0){
            //redis에 set은 성공했지만 파일이동은 실패함 로그 추가
            exit(0);
        }
        //업데이트 성공
        exit(0);
    }else if(strcmp(processName, "migration") == 0){
        //1. structure.dat파일을 읽어 migration 구조체에 function 값을 포함한 데이터 저장
        //2. 기존 구조체파일기준으로 레디스 데이터를 조회(키값만 조회?) -> scan으로 키값만 조회
        //3. 변경된 구조체 기준으로 데이터 삽입 후 레디스에 저장 -> 모든 키값에 대한 데이터가 동일하게 삽입이 되는데 문제되는지?
        //      => 해당 키로 hget으로 데이터를 가져온 후 변경된 구조체에 데이터 삽입,  
        // update인 경우 컬럼명이 변경 되면 어떻게 처리?
        // function D, I, U 에 따른 로직 나눠서 
        // D : struct_info에서 해당하는 name을 찾아서 offset으로 데이터 가공? -> 너무 일이 커진다
        // I : 
        // U : 
        // 
        
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
    printf("hset key : %s\n", key);
    //printf("hset buffer : %s\n", fieldBuffer);
    printf("hset dataLen : %d\n", dataLen);

    char *buf = malloc(dataLen);
    if(buf == NULL){
        printf("hset malloc fail (OOM)\n");
        return -1;
    }
    memcpy(buf, fieldBuffer, dataLen);

    redisReply *reply = redisCommand(c, "HSET %s stat %b", key, buf, (size_t)dataLen);

    free(buf);

    if(!reply){
        printf("hset fail : %s\n", c->errstr);
        return -1;
    }
    if(reply->type == REDIS_REPLY_ERROR){
        printf("hset redis error : %s\n", reply->str);
        freeReplyObject(reply);
        return -1;
    }
    printf("hset success\n");
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

int getAlign(char *type){
    if(strcmp(type, "char") == 0) return 1;
    if(strcmp(type, "int") == 0) return sizeof(int);
    if(strcmp(type, "long") == 0) return sizeof(long);
    return 1;
}
int structBuffer(struct_info *structInfo){
    FILE *structFile;
    int fileResult = fileOpen("msg_structure.dat", &structFile);
    if(fileResult < 0){
        return -1;
    }
    int offset = 0;
    int count = 0;
    int align = 1;
    int maxAlign = 1;  
    int length = 1;
    char name[32], type[32];

    //name, length, align 값 읽기
    while(fscanf(structFile, "%31s %31s %d", type, name, &length) == 3){
        /*if(count >= 100){
            printf("배열 넘김");
            return -1;
        }*/
        //패딩 적용: offset을 align 배수로 올림
        if(strcmp(type, "char") != 0 && strcmp(type, "int") != 0 && strcmp(type, "long") != 0){
            FILE *subFp;
            char subFileName[64];
            sprintf(subFileName, "%s.dat", type);
            int subFile = fileOpen(subFileName, &subFp);
            char subName[32], subType[32];
            int subLength = 1;
            while(fscanf(subFp, "%31s %31s %d", subType, subName, &subLength) == 3){
               /* if(count >= 100){
                    printf("배열 넘김 (sub)\n");
                    fclose(subFp);
                    return -1;
                }*/
                int align = getAlign(subType);
                if(align > maxAlign) maxAlign = align;
                if(align > 1){
                    offset = (offset + align - 1) / align * align;
                }
                structInfo[count].length = subLength;
                structInfo[count].offset = offset;
                strncpy(structInfo[count].type, subType, sizeof(structInfo[count].type)-1);
                structInfo[count].type[sizeof(structInfo[count].type)-1] = '\0';

                snprintf(structInfo[count].name, sizeof(structInfo[count].name), "%s.%s", name, subName);
                
                offset += structInfo[count].length;
                count++;   
            }
            fclose(subFp);
        }else{
            int align = getAlign(type);
            if(align > maxAlign) maxAlign = align;

            if(align > 1){
                offset = (offset + align - 1) / align * align;
            }

            strncpy(structInfo[count].type, type, sizeof(structInfo[count].type)-1);
            structInfo[count].type[sizeof(structInfo[count].type)-1] = '\0';

            strncpy(structInfo[count].name, name, sizeof(structInfo[count].name)-1);
            structInfo[count].name[sizeof(structInfo[count].name)-1] = '\0';
            structInfo[count].length = length;
            //bufInfo에 구조체.dat 데이터를 넣음
            structInfo[count].offset = offset;
            offset += structInfo[count].length;
            count++;
        }
    }
    fclose(structFile);    

    return count;
}

//                레디스 접속정보   구조받을 구조체                             
int redisHgetBuf(redisContext *c, struct_info *info, char *key, int count, char *buffer){
    redisReply *reply;

    reply = redisCommand((c), "HGET %s stat", key);
    //reply = redisCommand((c), "HGET %s stat", key);

    if (!reply) {
        printf("Redis data null\n");
        return -1;
    }
    memcpy(buffer, reply->str, reply->len);
    for (int i = 0; i < count; i++) {
        char temp[256] = {0};
        memcpy(temp, buffer + info[i].offset, info[i].length);
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
        reply = redisCommand(c, "SCAN %llu MATCH %s", cursor, searchStr);
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

void convertDateTime(time_t time, char *timeBuf){
    time += 9 * 60 * 60;
    struct tm *lt = gmtime(&time);

    sprintf(timeBuf, "%04d%02d%02d%02d%02d",
            lt->tm_year + 1900,
            lt->tm_mon + 1,
            lt->tm_mday,
            lt->tm_hour,
            lt->tm_min);
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


