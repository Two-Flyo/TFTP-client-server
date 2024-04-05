#ifndef TFTP_BASE_H
#define TFTP_BASE_H

#include <stdio.h>
#include <stdint.h>

#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#define TFTP_BLOCK_SIZE 8192
#define TFTP_DEFAULT_BLOCK_SIZE 512
#define TFTP_DEFAULT_PORT 69
#define TFTP_MAX_RETYR 10
#define TFTP_TMO_SEC 10

typedef enum _tftp_error_t
{
    TFTP_ERROR_OK = 0,
    TFTP_ERROR_NO_FILE,
    TFTP_ERROR_ACCESS_AIOLATION,
    TFTP_ERROR_DISK_FULL,
    TFTP_ERROR_OP,
    TFTP_ERROR_UNKNOWN_TID,
    TFTP_ERROR_FILE_EXIST,
    TFTP_ERROR_USER,

    TFTP_ERROR_END,
}tftp_error_t;

typedef enum _tftp_op_t
{
    TFTP_PACKET_RRQ = 1,
    TFTP_PACKET_WRQ,
    TFTP_PACKET_DATA,
    TFTP_PACKET_ACK,
    TFTP_PACKET_ERROR,
    TFTP_PACKET_OACK,

    TFTP_PACKET_REQ,
}tftp_op_t;

#pragma pack(1)
typedef  struct _tftp_packet_t
{
    uint16_t opcode; // 操作码

    union
    {
        struct
        {
            uint8_t args[1];
        }req;
        struct
        {
            uint16_t block_num; // 块编号
            uint8_t data[TFTP_BLOCK_SIZE];
        }data;
        struct
        {
            uint16_t block_num; // 块编号
        }ack;

        struct
        {
            char option[1];
        }oack;
        struct
        {
            uint16_t error_code;
            char error_msg[1];
        }error;
    };

}tftp_packet_t;
#pragma pack(1)



typedef struct _tftp_t
{
    int socket;
    struct sockaddr remote;

    int tmo_sec; // 最长等待数据包的时间
    int tmo_retry; // 重传次数

    int tx_size; // 数据包的有效空间
    int block_size;
    int file_size;
    tftp_packet_t rx_packet; // 接收
    tftp_packet_t tx_packet; // 发送
}tftp_t;

#define TFTP_NAME_SIZE 128

typedef struct _tftp_req_t
{
    tftp_t tftp;
    tftp_op_t opcode;
    int option;
    int block_size;
    int filesize;
    char filename[TFTP_NAME_SIZE];
}tftp_req_t;

int tftp_send_request(tftp_t* tftp, int is_read, const char* filename, uint32_t file_size, int option);
int tftp_send_ack(tftp_t* tftp, uint16_t block_num);
int tftp_send_data(tftp_t* tftp, uint16_t block_num, size_t size);
int tftp_send_error(tftp_t* tftp, uint16_t error_code);

int tftp_wait_packet(tftp_t* tftp, tftp_op_t, uint16_t block_num, size_t* pkt_size);
int tftp_parse_oack(tftp_t* tftp);
int tftp_send_oack(tftp_t* tftp);




#endif // !TFTP_BASE_H
