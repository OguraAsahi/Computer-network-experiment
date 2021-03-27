/* TFTP 客户端代码 */
#include "Client.h"
using namespace std;

int main(void) {
  char host_name[20];
  char file_name[50];
  char port4addr[20] = "69";
  int type;
  tftp_c *tc = NULL;
  //    printf("%d %d\n",sizeof(uint16_t),sizeof(ushort));
  log_fp = fopen("log.txt", "w+");
  //    fclose(log_fp);
  cout << "\nplease enter TFTP-Server host:";
  cin >> host_name;
  cout << "\ndownload or upload(download -> 0 || upload -> 1)?:";
  cin >> type;
  cout << "\nplease enter file name:";
  cin >> file_name;

  /* ========================DEBUG================================= */
  /*cout<<"========================DEBUG=================================\n"<<endl;
  cout<<"hostname = "<<host_name<<"\nfilename = "<<file_name<<"\ntype
  ="<<type<<endl;
  cout<<"========================DEBUG=================================\n"<<endl;*/

  /* Connect to TFTP Server */
  tc = tftp_connect(host_name, port4addr, (char *)MODE_NETASCII, type,
                    file_name);
  if (!tc) {
    now_time();
    cout << "ERROR:fail to connect to server" << endl;
    fprintf(log_fp, "ERROR:fail to connect to server\n");
    return -1;
  }

  /* Download or Upload */
  if (!type) {  // 0 -> Download
    tftp_recv(tc);
  } else {  // 1 -> Upload
    tftp_put(tc);
  }
  return 0;
}

tftp_c *tftp_connect(char *host_name, char *port4addr, char *mode, int type,
                     char *file_name) {
  tftp_c *tc = NULL;
  /* addrinfo结构主要在网络编程解析hostname时使用 */
  addrinfo hints;
  addrinfo *result = NULL;
  tc = (tftp_c *)malloc(sizeof(tftp_c));

  /*
      Create a socket:
      AF_INET:IPV4
      SOCK_DGREAM UDP
      0:auto choose protocal for type
  */
  /*SOCKET socket(int af, int type, int protocol);所有的同学在建立前都要创建一个socket*/
  /* 指定地址族（address family），一般填 AF_INET（使用 Internet 地址）*/
  /* 指定 SOCKET 的类型：SOCK_STREAM（流类型），SOCK_DGRAM（数据报类型） */
  /* 指定 af 参数指定的地址族所使用的具体一个协议。建议设为 0，那么它就会根据地址格式和 SOCKET 类型，
自动为你选择一个合适的协议 */
    /* 函数执行成功返回一个新的 SOCKET，失败则返回 INVALID_SOCKET。这时可以调用 WSAGetLastError 函数取得具
体的错误代码 */
  if ((tc->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    now_time();
    cout << "ERROR:fail to create socket" << endl;
    fprintf(log_fp, "ERROR:fail to create socket\n");
    free(tc);
    return NULL;
  }

  /* settings 4 getaddrinfo() */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  /* get server addr 4 sendto() */
  if (getaddrinfo(host_name, port4addr, &hints, &result) != 0) {
    now_time();
    cout << "ERROR:fail to get server address" << endl;
    fprintf(log_fp, "ERROR:fail to get server address\n");
    free(tc);
    return NULL;
  }
  memcpy(&tc->addr_server, result->ai_addr, result->ai_addrlen);
  tc->addr_len = sizeof(sockaddr_in);
  tc->mode = mode;
  tc->file_name = file_name;
  tc->type = type;

  /* ========================DEBUG================================= */
  /*cout<<"========================DEBUG=================================\n"<<endl;
  cout<<"tc->mode = "<<tc->mode<<"\ntc->filename = "<<tc->file_name<<endl;
  cout<<"========================DEBUG=================================\n"<<endl;*/

  return tc;
}


/* download from server */
int tftp_recv(tftp_c *tc) {
  tftp_rrq *snd = (tftp_rrq *)malloc(TFTP_RRQ_LEN(tc->file_name, tc->mode));
  tftp_recv_pack recv;
  tftp_ack ack;
  int re, bp_re, stop_flag = 0;
  uint16_t blocknum = 1;
  
  /* 请求的头部 request head */
  /* 将一个无符号短整型的主机数值转换为网络字节顺序，即大尾顺序(big-endian) */
  /* 参数u_short hostshort：16位无符号整数 */
  /* 返回值：TCP/IP网络字节顺序. */
  snd->opcode = htons(OPCODE_RRQ);
  /* 向rrq报文写入文件名和模式等信息 */
  sprintf(snd->req, "%s%c%s%c", tc->file_name, 0, tc->mode, 0);

  /* 向服务器发送读文件请求包 send request to server */
  /* sendto函数 该函数一般用于通过无连接的 SOCKET 发送数据报文，报文的接受者由 to 参数指定 */
  /*int sendto (SOCKET s, char * buf, int len ,int flags,
        struct sockaddr_in * to, int tolen);*/
  /* s:一个socket buf: 指向要传输数据的缓冲区的指针 len:buf的长度
     flags: 指定函数的调用方式, 一般为0  to:指向目标地址结构体的指针
     tolen: 目标地址结构体的长度 */
  re = sendto(tc->sockfd, snd, TFTP_RRQ_LEN(tc->file_name, tc->mode), 0,
              ((sockaddr *)&tc->addr_server), tc->addr_len);
  if (re == -1) {
    now_time();
    cout << "ERROR:fail to send request to server" << endl;
    fprintf(log_fp, "ERROR:fail to send request to server\n");
  }
  /* ========================DEBUG================================= */
  /* cout<<"========================DEBUG================================="<<endl;
   cout<<"snd.opcode = "<<ntohs(snd->opcode)<<"\nreq.head = "<<snd->req<<endl;
   cout<<"re = "<<re<<endl;
   cout<<"========================DEBUG================================="<<endl;*/
  now_time();
  cout << "Download Start" << endl;
  fprintf(log_fp, "Download Start");
  FILE *fp = fopen(tc->file_name, "w+");
  //   fprintf(fp, "%s", "test");

  /* begin recv data */
  start_c = clock();
  size_all = 0;
  while (1) {
    memset(recv.data, 0, sizeof(recv.data));
    for (timer = 0; timer < RECV_TIMEOUT; timer += 10000) {
      /* recvfrom函数 该函数一般用于通过无连接的 SOCKET 接收数据报文，
      报文的发送者由 from 参数指定 */
      /* int recvfrom (SOCKET s, char * buf, int len ,int flags,
            struct sockaddr_in * from, int * fromlen); */
      re = recvfrom(tc->sockfd, &recv, sizeof(tftp_recv_pack), MSG_DONTWAIT,
                    ((sockaddr *)&tc->addr_server), &tc->addr_len);
      /* recv success & send ACK */
      // TODO:Throughput
      if (recv.opcode == htons(OPCODE_DATA) &&
          recv.bnum_ecode == htons(blocknum)) {
        //    now_time();
        //    cout<<"DATA: BlockNum = "<<ntohs(recv.bnum_ecode)<<", DataSize =
        //    "<<(re - 4)<<endl; fprintf(log_fp,"DATA: BlockNum = %d, DataSize =
        //    %d\n", ntohs(recv.bnum_ecode), (re-4));
        fprintf(fp, "%s", recv.data);
        /* record re */
        size_all += re;
        /* 发送ACK报文 Send ACK */
        ack.opcode = htons(OPCODE_ACK);
        ack.blocknum = recv.bnum_ecode;
        sendto(tc->sockfd, &ack, sizeof(tftp_ack), 0,
               ((sockaddr *)&tc->addr_server), tc->addr_len);
        /* 判断是否传输至文件末尾 */
        if (re < DATA_SIZE + 4) {
          /* end of recv */
          end_c = clock();
          time_all = (double)(end_c - start_c) / CLOCKS_PER_SEC;
          now_time();
          cout << "Download Finish" << endl;
          fprintf(log_fp, "Download Finsish");

          /* 计算本次下载速度 */
          fprintf(log_fp, "\n=================================\n");
          //    fprintf(log_fp,"file size = %lf kB\n", (size_all)/1024);
          //    fprintf(log_fp,"download time = %lf s\n", time_all);
          fprintf(log_fp, "this download average throughout  = %.2lf kB/s",
                  size_all / (1024 * time_all));
          fprintf(log_fp, "\n=================================\n");
          stop_flag = 1;
          break;
        }
        break;
      }
      /* ========================DEBUG================================= */
      /*cout<<"========================DEBUG================================="<<endl;
      cout<<"re = "<<re<<endl;
      cout<<"recv.opcode = "<<ntohs(recv.opcode)<<endl;
      cout<<"recv.bnum_ecode = "<<ntohs(recv.bnum_ecode)<<endl;
      cout<<"blocknum = "<<blocknum<<endl;
      cout<<"========================DEBUG=z================================"<<endl;*/

      /* the last block size < 512, end recv */
      usleep(10000);  // recv delay
    }
    if (timer >= RECV_TIMEOUT) {
      if (blocknum == 1) {
        now_time();
        cout << "ERROR:fail to send request to server" << endl;
        fprintf(log_fp, "ERROR:fail to send request to server\n");
        return 0;
      }
      now_time();
      fprintf(log_fp, "ERROR:Data package blocknum = %d timeout\n", blocknum);
      return 0;
    }
    blocknum++;
    if (stop_flag) break;
  }
  fclose(fp);
  return 1;
}

int tftp_put(tftp_c *tc) {
  tftp_wrq *snd = (tftp_wrq *)malloc(TFTP_WRQ_LEN(tc->file_name, tc->mode));
  tftp_recv_pack recv;
  tftp_data snd_data;
  int re = 0;
  /* WRQ msg */
  snd->opcode = htons(OPCODE_WRQ);
  sprintf(snd->req, "%s%c%s%c", tc->file_name, 0, tc->mode, 0);
  /* send WRQ 2 server */
  re = sendto(tc->sockfd, snd, TFTP_WRQ_LEN(tc->file_name, tc->mode), 0,
              ((sockaddr *)&tc->addr_server), tc->addr_len);
  if (re == -1) {
    now_time();
    cout << "ERROR:fail to send request to server" << endl;
    fprintf(log_fp, "ERROR:fail to send request to server");
  }
  re = recvfrom(tc->sockfd, &recv, sizeof(tftp_recv_pack), 0,
                ((sockaddr *)&tc->addr_server), &tc->addr_len);
  if (re == -1) {
    now_time();
    cout << "ERROR:fail to recv from server" << endl;
    fprintf(log_fp, "ERROR:fail to recv from server\n");
    return 0;

  } /* recv blocknum=0 => start transfer */
  /*  cout<<"========================DEBUG================================="<<endl;
                cout<<"re = "<<re<<endl;
                cout<<"recv.opcode = "<<ntohs(recv.opcode)<<endl;
                cout<<"recv.bnum_ecode = "<<ntohs(recv.bnum_ecode)<<endl;
         //       cout<<"blocknum = "<<blocknum<<endl;
                cout<<"========================DEBUG=z================================"<<endl;*/
  if (recv.opcode == htons(OPCODE_ACK) && recv.bnum_ecode == htons(0)) {
    now_time();
    printf("Upload Start\n");
    fprintf(log_fp, "Upload Start\n");
    FILE *fp = fopen(tc->file_name, "r");
    size_all = 0;
    start_c = clock();
    //    printf("%s\n",tc->file_name);
    if (fp == NULL) {
      now_time();
      cout << "ERROR: non-exist file" << endl;
      fprintf(log_fp, "ERROR: non-exist file\n");
      return 0;
    }
    int blocknum = 1;
    int size_t;
    snd_data.opcode = htons(OPCODE_DATA);

    while (1) {
      /* init data packet */
      memset(snd_data.data, 0, sizeof(snd_data.data));
      snd_data.blocknum = htons(blocknum);
      size_t = fread(snd_data.data, 1, DATA_SIZE, fp);
      //    printf("%s\n", snd_data.data);
      /* send data 2 server */
      re = sendto(tc->sockfd, &snd_data, size_t + 4, 0,
                  ((sockaddr *)&tc->addr_server), tc->addr_len);
      if (re == -1) {
        now_time();
        cout << "ERROR: fail to send data to server" << endl;
        fprintf(log_fp, "ERROR: fail to send data to server\n");
        return 0;
      }
      size_all += re;
      /* recv from server (ACK|ERROR) */
      re = recvfrom(tc->sockfd, &recv, sizeof(tftp_recv_pack), 0,
                    ((sockaddr *)&tc->addr_server), &tc->addr_len);
      if (re == -1) {
        now_time();
        cout << "ERROR: fail to recv ACK from server" << endl;
        fprintf(log_fp, "ERROR: fail to recv ACK from server\n");
        return 0;
      }
      /* ack yes */
      if (recv.opcode == htons(OPCODE_ACK) &&
          recv.bnum_ecode == htons(blocknum)) {
        size_all += re;
        blocknum++;
      }
      /* ========================DEBUG================================= */
      /* cout<<"========================DEBUG================================="<<endl;
      cout<<"re = "<<re<<endl;
      cout<<"recv.opcode = "<<ntohs(recv.opcode)<<endl;
      cout<<"recv.bnum_ecode = "<<ntohs(recv.bnum_ecode)<<endl;
      cout<<"blocknum = "<<blocknum<<endl;
      cout<<"========================DEBUG================================"<<endl;
    */
      if (size_t != DATA_SIZE) {
        /* end of recv */
        end_c = clock();
        time_all = (double)(end_c - start_c) / CLOCKS_PER_SEC;

        now_time();
        printf("Upload Finish\n");
        fprintf(log_fp, "Upload Finish\n");

        /* statistic this upload */
        fprintf(log_fp, "\n======================================\n");
        //    fprintf(log_fp,"file size = %lf kB\n", (size_all)/1024);
        //    fprintf(log_fp,"upload time = %lf s\n", time_all);
        fprintf(log_fp, "this upload average throughout  = %.2lf kB/s",
                size_all / (1024 * time_all));
        fprintf(log_fp, "\n======================================\n");
        break;
      }
    }
    fclose(fp);
  }
  return 1;
}

void now_time(void) {
  n_time = time(NULL);
  l_time = localtime(&n_time);
  printf("\n%s    ", asctime(l_time));
  fprintf(log_fp, "\n%s    ", asctime(l_time));
  return;
}