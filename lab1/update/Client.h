#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <malloc.h>
#include <stdlib.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <time.h>
/* 超时设置 3 SECOND */
#define RECV_TIMEOUT 3*1000*1000
#define SEND_TIMEOUT 12*1000*1000
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

struct tftp_c { /* tftp connect handle */
    char *mode; /* netascii,octet,mail */
    char *file_name; /* file name */
    int sockfd; /* socketfd to server */
    int type; /* read=0 or write=1 */
    sockaddr_in addr_server; /* 服务器地址结构体的指针server address */
    socklen_t addr_len; /* 服务器地址结构体指针的长度addrlen 4 socket use */
};


#define TFTP_RRQ_LEN(f,m) (sizeof(tftp_rrq)+strlen(f)+strlen(m)+2)  //计算 rrq packet 的长度
/* 读文件请求包 对应opcode为1 */
struct tftp_rrq {
    uint16_t opcode;
    char req[];
};

#define TFTP_WRQ_LEN(f,m) (sizeof(tftp_rrq)+strlen(f)+strlen(m)+2)  //计算 wrq packet 的长度
/* 写文件请求包 对应opcode为2 */
struct tftp_wrq {
    uint16_t opcode;
    char req[];
};

/* 从服务器收到的内容 ERROR包对应opcode为4 :DATA or ERROR*/
struct tftp_recv_pack
{
    uint16_t opcode;
    uint16_t bnum_ecode; //DATA:BlockNum   ERROR:ErrorCode
    char data[DATA_SIZE];
};

/* 数据包 对应opcode为3 packet Data */
struct tftp_data {
    uint16_t opcode;
    uint16_t blocknum;
    char data[DATA_SIZE];
};

/*ACK应答包 对应opcode为4 packet ACK */
struct tftp_ack {
    uint16_t opcode;
    uint16_t blocknum;
};

/* 定时器TIMER */
int timer;

/* 日志文件 log file: log.txt */
FILE *log_fp;

/* 获取时间并打印 get time & print */
time_t n_time;
tm* l_time;

/* 下载或上传的时间 time for download or upload */
clock_t start_c, end_c;
double time_all;

/* 一次下载或上传的所有数据长度 all data size for 1 download or upload */
double size_all;

/* 函数声明 Func Declaration */
/* 和服务器建立联系 */
tftp_c *tftp_connect(char *host_name, char *port4addr,char *mode, int type, char *file_name);
/* 从服务器获取文件 */
int tftp_recv(tftp_c *tc);
/* 向服务器上传文件 */
int tftp_put(tftp_c *tc);
/* 获取并打印时间 */
void now_time(void);


