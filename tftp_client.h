#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H

#include "tftp_base.h"

#define TFTP_CMD_BUFFER_SIZE 128

// gethostbyname :域名转换
int tftp_get(const char* ip, uint16_t port, int block_size, const char* filename, int option);
int tftp_put(const char* ip, uint16_t port, int block_size, const char* filename, int option);

int tftp_start(const char* ip, uint16_t port);
#endif // !TFTP_CLIENT_H