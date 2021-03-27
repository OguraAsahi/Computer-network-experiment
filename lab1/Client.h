#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <malloc.h>
#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <time.h>
/* TFTP 模式 */
#define MODE_NETASCII "netascii"
#define MODE_OCTET "octet"
#define MODE_MAIL "mail"
/* 上传或下载 */
#define TYPE_READ 0  
#define TYPE_WRITE 1
/*操作码 1-RRQ 2-WRQ 3-DATA 4-ACK 5-ERROR */
#define OPCODE_RRQ 1
#define OPCODE_WRQ 2
#define OPCODE_DATA 3
#define OPCODE_ACK 4
#define OPCODE_ERROR 5

#define DATA_SIZE 512

struct tftp_c { /* tftp 连接处理 */
    char *mode; /* netascii,octet,mail */
    char *file_name; /* file name */
    int sockfd; /* socketfd to server */
    int type; /* read=0 or write=1 */
    sockaddr_in addr_server; /* server address */
    socklen_t addr_len; /* addrlen 4 socket use */
};


#define TFTP_RRQ_LEN(f,m) (sizeof(tftp_rrq)+strlen(f)+strlen(m)+2)  //计算 rrq packet的长度
struct tftp_rrq {
    uint16_t opcode;
    char req[];
};
#define TFTP_WRQ_LEN(f,m) (sizeof(tftp_rrq)+strlen(f)+strlen(m)+2)  //计算 wrq packet的长度
struct tftp_wrq {
    uint16_t opcode;
    char req[];
};

/* 接收自服务器:DATA or ERROR*/
struct tftp_recv_pack
{
    uint16_t opcode;
    uint16_t bnum_ecode; //DATA:BlockNum   ERROR:ErrorCode
    char data[DATA_SIZE];
};

/* packet Data */
struct tftp_data {
    uint16_t opcode;
    uint16_t blocknum;
    char data[DATA_SIZE];
};
/* packet ACK */
struct tftp_ack {
    uint16_t opcode;
    uint16_t blocknum;
};

/* 日志文件: log.txt */
FILE *log_fp;

/* 获取时间 & 打印 */
time_t n_time;
tm* l_time;

/* 上传或下载的时间 */
clock_t start_c, end_c;
double time_all;

/* all data size for 1 download or upload */
double size_all;

/* 功能声明和描述 */
tftp_c *tftp_connect(char *host_name, char *port4addr,char *mode, int type, char *file_name);//和服务器建立联系
int tftp_recv(tftp_c *tc);//get file from server从服务器下载文件
int tftp_put(tftp_c *tc);//put file 2 server上传文件
void now_time(void);//get & print now time获取并打印现在的时间


