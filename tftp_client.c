#include <stdio.h>
#include <string.h>
#include "tftp_client.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>


static tftp_t tftp;

static int tftp_open(const char* ip, uint16_t port, int block_size)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        printf("error: create socket failed.\n");
        return -1;
    }

    tftp.socket = sockfd;
    tftp.block_size = block_size;
    tftp.file_size = 0;
    tftp.tmo_retry = TFTP_MAX_RETYR;
    tftp.tmo_sec = TFTP_TMO_SEC;

    struct sockaddr_in* sockaddr = (struct sockaddr_in*)&tftp.remote;
    memset(sockaddr, 0, sizeof(struct sockaddr_in));
    sockaddr->sin_family = AF_INET;
    sockaddr->sin_addr.s_addr = inet_addr(ip);
    sockaddr->sin_port = htons(port);

    struct timeval tmo;
    tmo.tv_sec = tftp.tmo_sec;
    tmo.tv_usec = 0;
    setsockopt(tftp.socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tmo, sizeof(tmo));

}
static void tftp_close(void)
{
    close(tftp.socket);
}
static int do_tftp_get(int block_size, const char* ip, uint16_t port, const char* filename, int option)
{
    if (tftp_open(ip, port, block_size) < 0)
    {
        printf("tftp connect failed.\n");
        return -1;
    }

    printf("tftp: try to get file: %s\n", filename);

    FILE* file = fopen(filename, "wb");
    if (file == NULL)
    {
        printf("tftp: create local file failed: %s\n", filename);
        goto get_error;
    }

    int error = tftp_send_request(&tftp, 1, filename, 0, option);
    if (error < 0)
    {
        printf("tftf: send tftf rrq failed.\n");
        goto get_error;
    }
    if (option)
    {
        size_t recv_size = 0;
        error = tftp_wait_packet(&tftp, TFTP_PACKET_OACK, 0, &recv_size);
        if (error < 0)
        {
            printf("tftp: wait oack error, file:%s\n", filename);
            goto get_error;
        }

        error = tftp_send_ack(&tftp, 0);
        if (error < 0)
        {
            printf("tftp: send ack failed. file: %s\n", filename);
            goto get_error;
        }

        printf("tftp: file size %d bytes\n", tftp.file_size);
    }

    uint16_t next_block = 1;
    uint32_t total_size = 0;
    uint32_t total_block = 0;
    while (1)
    {
        size_t recv_size = 0;

        error = tftp_wait_packet(&tftp, TFTP_PACKET_DATA, next_block, &recv_size);
        if (error < 0)
        {
            printf("tftp: wait error, block %d file: %s", 0, filename);
            goto get_error;
        }
        size_t block_size = recv_size - 4;
        if (block_size)
        {
            size_t size = fwrite(tftp.rx_packet.data.data, 1, block_size, file);
            if (size < block_size)
            {
                printf("tftp: write file failed %s\n", filename);
                goto get_error;
            }
        }

        error = tftp_send_ack(&tftp, next_block);
        if (error < 0)
        {
            printf("tftp: send ack failed! ack block = %d\n", next_block);
            goto get_error;
        }
        next_block++;
        total_block += (uint32_t)block_size;
        if (++total_block % 0x40 == 0)
        {
            printf(".");
            fflush(stdout);
        }
        if (block_size < tftp.block_size)
        {
            error = 0;
            break;
        }
    }

    printf("\n tftp: total recv: %d bytes, %d\n", total_size, total_block);
    fclose(file);
    tftp_close();
    return 0;

get_error:
    if (file)
        fclose(file);
    tftp_close();
    return error;
}
int tftp_get(const char* ip, uint16_t port, int block_size, const char* filename, int option)
{
    printf("Try to get file %s from %s\n", filename, ip);

    if (block_size > TFTP_BLOCK_SIZE)
    {
        block_size = TFTP_BLOCK_SIZE;
    }

    return do_tftp_get(block_size, ip, port, filename, option);
}

static int do_tftp_put(int block_size, const char* ip, uint16_t port, const char* filename, int option)
{
    if (!option)
    {
        block_size = TFTP_DEFAULT_BLOCK_SIZE;
    }



    if (tftp_open(ip, port, block_size) < 0)
    {
        printf("tftp: connect failed\n");
        return -1;
    }

    printf("tftp: Try to put file: %s\n", filename);

    FILE* file = fopen(filename, "rb");
    if (file == NULL)
    {
        printf("tftp: create local file failed: %s\n", filename);
        goto put_error;
    }

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    int error = tftp_send_request(&tftp, 0, filename, filesize, option);
    if (error < 0)
    {
        printf("tftp: send tftp wrq failed\n");
        goto put_error;
    }

    size_t recv_size;
    error = tftp_wait_packet(&tftp, option ? TFTP_PACKET_OACK : TFTP_PACKET_ACK, 0, &recv_size);
    if (error < 0)
    {
        printf("tftp: wait error, block %d file: %s\n", 0, filename);
        goto put_error;
    }

    uint16_t current_block = 1;
    uint16_t total_block = 0;
    uint16_t total_size = 0;
    while (1)
    {
        size_t block_size = fread(&tftp.tx_packet.data.data, 1, tftp.block_size, file);
        if (!feof(file) && (block_size != tftp.block_size))
        {
            error = -1;
            printf("tftp: read file failed %s\n", filename);
            goto put_error;
        }

        error = tftp_send_data(&tftp, current_block, block_size);
        if (error < 0)
        {
            printf("tftp: send data failed. block=%d\n", current_block);
            goto put_error;
        }

        tftp_wait_packet(&tftp, TFTP_PACKET_ACK, current_block, &recv_size);
        if (error < 0)
        {
            printf("tftp: wait error. block=%d file: %s\n", current_block, filename);
            goto put_error;
        }

        current_block++;

        total_size += (uint32_t)block_size;
        if (++total_block % 0x40 == 0)
        {
            printf(".");
            fflush(stdout);
        }

        if (block_size < tftp.block_size)
        {
            error = 0;
            break;
        }

    }

    printf("\n tftp: total send: %d bytes, %d block\n", total_size, total_block);
    fclose(file);
    tftp_close();
    return 0;

put_error:
    if (file)
        fclose(file);
    tftp_close();
    printf("\n tftp: send failed\n");
    return error;
}

int tftp_put(const char* ip, uint16_t port, int block_size, const char* filename, int option)
{
    printf("Try to put file %s from %s\n", filename, ip);
    if (block_size > TFTP_BLOCK_SIZE)
    {
        block_size = TFTP_BLOCK_SIZE;
    }

    return do_tftp_put(block_size, ip, port, filename, option);
}

void show_cmd_list(void)
{
    printf("usage: cmd arg0 arg1...\n");
    printf("    get filename               -- download file from server\n");
    printf("    gut filename               -- download file from server\n");
    printf("    block                      -- set block size\n");
    printf("    quit                       -- quit tftp client\n");
}

int tftp_start(const char* ip, uint16_t port)
{
    int block_size = TFTP_DEFAULT_BLOCK_SIZE;
    char buffer[TFTP_CMD_BUFFER_SIZE];
    if (port == 0)
        port = TFTP_DEFAULT_PORT;

    if (strcmp(ip, "") == 0)
        ip = "127.0.0.1";

    printf("tftp> Welcome to use tftp client\n");
    show_cmd_list();
    while (1)
    {
        printf("tftp> ");
        fflush(stdout);

        char* cmd_buf = fgets(buffer, sizeof(buffer), stdin);
        if (cmd_buf == NULL)
        {
            continue;
        }
        char* cr = strchr(cmd_buf, '\n');
        if (cr)
        {
            *cr = '\0';
        }

        cr = strchr(cmd_buf, '\r');
        if (cr)
        {
            *cr = '\0';
        }
        // get 1.png
        const char* split = "\t\n ";
        char* cmd = strtok(cmd_buf, split);
        if (cmd)
        {
            if (strcmp(cmd, "get") == 0)
            {
                char* filename = strtok(NULL, split);
                if (filename)
                {
                    do_tftp_get(block_size, ip, port, filename, 1);
                }
                else
                {
                    printf("error: no file\n");
                }
            }
            else if (strcmp(cmd, "put") == 0)
            {
                char* filename = strtok(NULL, split);
                if (filename)
                {
                    do_tftp_put(block_size, ip, port, filename, 1);
                }
                else
                {
                    printf("error: no file\n");
                }
            }
            else if (strcmp(cmd, "block") == 0)
            {
                char* blk = strtok(NULL, split);
                if (blk)
                {
                    int size = atoi(blk);
                    if (size <= 0)
                    {
                        printf("block size %d error, set to default\n", block_size);
                        size = TFTP_DEFAULT_BLOCK_SIZE;
                    }
                    else if (size > TFTP_BLOCK_SIZE)
                    {
                        printf("block size %d too long, set to default", block_size);
                        size = TFTP_DEFAULT_BLOCK_SIZE;
                    }
                    else
                    {
                        block_size = size;
                    }
                }
                else
                {
                    printf("error: no size\n");
                }
            }
            else if (strcmp(cmd, "quit") == 0)
            {
                printf("quit tftp client!\n");
                break;
            }
            else
            {
                printf("unknown cmd\n");
                show_cmd_list();
            }
        }
    }
    return 0;
}
