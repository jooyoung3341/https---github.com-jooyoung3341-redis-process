#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <hiredis/hiredis.h>
#include <hiredis/hiredis_ssl.h>
#include <time.h>


typedef struct time_buf{
    long tv_sec;
    long tv_usec;
} time_buf;

typedef	struct 	
{
	unsigned char	flag;			/* using free or alloc flag	*/
	unsigned char	c_stat;			/* msg status 			*/
	char		dummy[2];
	unsigned int  	 msgid;                 		 /* internal message id number 	*/
	unsigned int 	cp_serial;		/* CP message id number 	*/
	unsigned int	smc_serial;		/* SMSC message id number	*/
	int 		tid;			/* teleservices id		*/
	char		oa_addr[32];			
	char		da_addr[32];
	char		callback[32];
	char		special_no[12];	/* SMSC로 전송되는 Source 특번	
	019151xxxx : 일반 MT, 019152xxxx : PUSH */
	char		cdr_spc_addr[32];	/* CDR생성시 과금 CTN 혹은 특번 */ 
	unsigned char	text[160];
	struct 		time_buf	dr_time;		/* data receive time 		*/
	struct 		time_buf	ds_time;		/* data send time 		*/
	struct 		time_buf	ar_time;		/* ack receive time 		*/
	struct 		time_buf	as_time;		/* ack send time		*/
	struct 		time_buf	dd_time;		/* Message done time		*/
	struct 		time_buf	ss_time;		/* Internal schdule time 	*/
	char		msg_class;		/* T:MT, O,MO, P:POLY, C:CDR	*/
	char		adjust_flag;		/* Adjustment flag, Y/N		*/
	char		cdr_flag;		/* CDR 생성 여부 Y/N	add by thyoon 20080109 */
	char		report_flag;		/* Report flag, Y/N		*/
	char		lgt_flag;			/* LGT CTN flag, Y/N		*/
	char		book_flag;		/* Booking Message flag, Y/N	*/
	char		binary_flag;		/* Binary Message flag, Y/N	*/
	char    		ktf_roaming; 		/* MO in KTF roaming area, Y/N 	*/
	char  		m_result;			/* message final status, S/F	*/
	unsigned int	m_code;			/* fail message error code 	*/
	int		cdr_type;			/* CDR Format, 0,1,2,3...	*/
	int    		cdr_money;		/* 정보이용료 과금방식에서 이용	*/
	char   		destcode[12]; 	/* MNP-010 착신망 코드 		*/

	/* GEN_MT의 경우에만 쓰이는 필드 */
	char   		call_npi[4];		/* CallingNPIi, CP 구분자	*/

	/* 국제 Roaming -mscid, baseid , chargeInfo 변수 추가 2007.03.05 PS.Cho */
	unsigned char			mscid[4];
	unsigned char			baseid[4];
	unsigned char			chargeInfo;

	/* SMSC로 부터 받은 리포트 메시지 상태,error code 저장변수  2007.10.02 PS.Cho */
	char    messageState[8];
	unsigned char    errorCode[4];
	/* SMPP34 음성,영상 VMS 메시지 개수 2007.11.22 PS.Cho */
	unsigned char			number_of_messages;

	/* report qid 저장 변수 추가 20080801 THYOON */
	int		qid;

	/* LONG MSG field 저장 변수 추가 20080903 thyoon */
	unsigned char	lms_field[3];
	/* cp code 저장 변수 추가 20090403 thyoon */
	char       cp_code[16];

	/* 실시간 G/W 연동 관련 필드 추가 thyoon 20090723 */
	unsigned char   hall_flag;                         /* hall flag */
	char        soc_code[10];
	int out_qid;                 /* SMSC,EXPORT 전송 qid 필드 추가 thyoon 20090817*/
	int mgw_stat;

	unsigned char mgw_immediate_charge_result[5];	/* mgw IMMEDIATE_CHARGE_LIMIT result thyoon 20090824*/

	int submit_retry_cnt;				/* immediate_charge retry count thyoon 20090903 */
	int msg_repeat_cnt;			/* mgw 통화료 정보이용료 count 20090824 */

	struct 		time_buf	ms_time;		/* immediate_charge send time thyoon 20090903	*/
	struct 		time_buf	mr_time;		/* immediate_charge recv time thyoon 20090903		*/
	struct 		time_buf	cs_time;		/* cas send time thyoon 20090903	*/
	struct 		time_buf	cr_time;		/* cas recv time thyoon 20090903*/

	int mgw_bill_type;		/* mgw 요금 타입   1: 통화료, 2: 정보료, 3 : 통화료 + 정보이용료 thyoon 20090903*/
	char	    bill_agent;		/* 과금 주체   'S' : 발신자   'R' : 수신자    thyoon 20090903 */
	int	cas_result;	/* nCAS 조회 결과값 thyoon 20090914 */

	char	receiver_type;  /* 수신자 LGT 가입 여부  L : LGT E : 타사   */

	int hp_errorCode; /* === 2009-10-29 KJW : 단말 Pass code */

	char	spam_flag;	/* 스팸 차단 여부 thyoon 20091105 */
	char	br_id[25];	/* SMSGW 구조개선 추가 : 후보정 추가 ydjeon 20101021 */
	char oa_debug_name[20]; /* SMS 구조개선 유입 프로세스 added by tigerlee at 20100805 */
	char da_debug_name[20]; /* SMS 구조개선 발신 프로세스 added by tigerlee at 20100805 */
	unsigned char filename[36]; /* SMS 구조개선 msg filename added by tigerlee at 20100824 */

	int		nps_cnt;	/* npdb 전송 횟수 - SMS 구조개선 PS.Cho 20100820 */
	int		np_result;	/* npdb 조회 - SMS 구조개선 PS.Cho 20100820 */
	struct 		time_buf	nps_time;	/* npdb 요청 시간 - SMS 구조개선 PS.Cho 20100820 */
	struct 		time_buf	npr_time;	/* npdb 수신 시간 - SMS 구조개선 PS.Cho 20100820 */

	unsigned char	fwd_cnt; /* 착신전환 횟수 - SMS 구조개선 PS.Cho 20100820 */
	char			fwd_da_addr[32]; /* 착신전환 번호 - SMS 구조개선 PS.Cho 20100820 */
	char			fwd_contents_flag; 	/* 착신전환 PLUS "FW>" 추가 여부 flag, Y/N	*/
/* VoLTE ANI 추가 20150422 START */
	char utrancellid[20];

/* VoLTE ANI 추가 20150422 END */
	char cp_name[21];

	int             a_index;
	char            report_id[64];
	char		args_nickname[30]; 
	char aa_FAIL;
} stdb_field1;

typedef	struct 	
{
	unsigned char	flag;			/* using free or alloc flag	*/
	unsigned char	c_stat;			/* msg status 			*/
	char		dummy[2];
	unsigned int  	 msgid;                 		 /* internal message id number 	*/
	unsigned int 	cp_serial;		/* CP message id number 	*/
	unsigned int	smc_serial;		/* SMSC message id number	*/
	int 		tid;			/* teleservices id		*/
	char		oa_addr[32];			
	char		da_addr[32];
	char		callback[32];
} stdb_field;

/*typedef struct {
    char flag;
    char oa_addr[32];
    char da_addr[32];
    time_buf dr_time;
    int qid;
} stdb_field;*/

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
	char redis_SSL[256];
} redis_data;

typedef struct{
	struct_PATH[256];
	struct_FILENAME[32];
} struct_data

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

//void convertDateTime(time_t time, char *timeBuf);
time_t convertDateTime(char *time);
int fileOpen(char *fileName, FILE **fp);
int redisHget(redisContext *c, char *key, char *field, stdb_field *da);
int redisHgetBuf(redisContext *c, struct_info *info, char *key, int count, char *buffer);
int structBuffer(struct_info *structInfo);
int editStructBuffer(migration_info *editStructInfo);
void getProfile(char *section, redis_info *info);
int redisHset(redisContext *c, char *key, char *fieldBuffer, int dataLen);
int getAlign(char *type);
char findOldField(struct_info *structInfo, int structCount, char *fieldName);

char rediskey[256] = "testBuf";