#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tftp_server.h"


static const char* server_path;
static uint16_t server_port;

static tftp_t tftp;

static int do_recv_file(tftp_req_t* req)
{
    tftp_t* tftp = &req->tftp;
    char path_buffer[128];
    if (server_path)
    {
        snprintf(path_buffer, sizeof(path_buffer), "%s/%s", server_path, req->filename);
    }
    else
    {
        snprintf(path_buffer, sizeof(path_buffer), "%s", req->filename);
    }

    FILE* file = fopen(path_buffer, "wb");
    if (file == NULL)
    {
        printf("tftpd: create %s failed\n", path_buffer);
        tftp_send_error(tftp, TFTP_ERROR_DISK_FULL);
        return -1;
    }

    printf("tftpd: recv file %s ...\n", path_buffer);

    int error = req->option ? tftp_send_oack(tftp) : tftp_send_ack(tftp, 0);
    if (error < 0)
    {
        printf("tftpd: send ack failed\n");
        goto recv_failed;
    }

    uint16_t curr_blk = 1;
    int total_size = 0;
    int total_block = 0;
    while (1)
    {
        size_t pkt_size;
        error = tftp_wait_packet(tftp, TFTP_PACKET_DATA, curr_blk, &pkt_size);
        if (error < 0)
        {
            printf("tftpd: wait %d ack failed\n", curr_blk - 1);
            goto recv_failed;
        }
        size_t block_size = pkt_size - 4;
        size_t size = fwrite(tftp->rx_packet.data.data, 1, block_size, file);
        if (size < block_size)
        {
            printf("tftpd: write file %s failed\n", path_buffer);
            tftp_send_error(tftp, TFTP_ERROR_ACCESS_AIOLATION);
            goto recv_failed;
        }

        int error = tftp_send_ack(tftp, curr_blk++);
        if (error < 0)
        {
            printf("tftpd: send ack block failed\n");
            goto recv_failed;
        }

        total_size += (int)size;
        total_block++;
        if (size < tftp->block_size)
        {
            break;
        }
    }

    printf("tftpd: recv %s %dbytes %dblocks\n", path_buffer, total_size, total_block);
    fclose(file);
    return 0;
recv_failed:
    printf("tftpd: recv failed.\n");
    fclose(file);
    return -1;

}

static int do_send_file(tftp_req_t* req)
{
    tftp_t* tftp = &req->tftp;
    char path_buffer[128];
    if (server_path)
    {
        snprintf(path_buffer, sizeof(path_buffer), "%s/%s", server_path, req->filename);
    }
    else
    {
        snprintf(path_buffer, sizeof(path_buffer), "%s", req->filename);
    }

    FILE* file = fopen(path_buffer, "rb");
    if (file == NULL)
    {
        printf("tftpd: file %s does not exist\n", path_buffer);
        tftp_send_error(tftp, TFTP_ERROR_NO_FILE);
        return -1;
    }

    printf("tftpd: sending file %s....\n", path_buffer);

    fseek(file, 0, SEEK_END);
    tftp->file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (req->option)
    {
        int error = tftp_send_oack(tftp);
        if (error < 0)
        {
            printf("tftpd: send oack failed\n");
            goto send_failed;
        }

        size_t pkt_size;
        error = tftp_wait_packet(tftp, TFTP_PACKET_ACK, 0, &pkt_size);
        if (error < 0)
        {
            printf("tftpd: wait ack failed\n");
            goto send_failed;
        }

    }

    uint16_t curr_blk = 1;
    int total_size = 0;
    int total_block = 0;
    while (1)
    {
        size_t size = fread(tftp->tx_packet.data.data, 1, tftp->block_size, file);
        if (size < 0)
        {
            printf("tftpd: read file %s failed\n", path_buffer);
            tftp_send_error(tftp, TFTP_ERROR_ACCESS_AIOLATION);
            goto send_failed;
        }

        int error = tftp_send_data(tftp, curr_blk, size);
        if (error < 0)
        {
            printf("tftpd: send data block failed\n");
            goto send_failed;
        }

        size_t pkt_size;
        error = tftp_wait_packet(tftp, TFTP_PACKET_ACK, curr_blk++, &pkt_size);
        if (error < 0)
        {
            printf("tftpd: wait %d ack failed\n", curr_blk - 1);
            goto send_failed;
        }

        total_size += (int)size;
        total_block++;
        if (size < tftp->block_size)
        {
            break;
        }
    }

    printf("tftpd: send %s %dbytes %dblocks\n", path_buffer, total_size, total_block);
    fclose(file);
    return 0;
send_failed:
    printf("tftpd: send failed.\n");
    fclose(file);
    return -1;
}



static void* tftp_worikng_thread(void* arg)
{
    tftp_req_t* req = (tftp_req_t*)arg;
    tftp_t* tftp = &req->tftp;

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        printf("tftpd: create working socket failed\n");
    }

    tftp->socket = sockfd;

    struct timeval tmo;
    tmo.tv_sec = tftp->tmo_sec;
    tmo.tv_usec = 0;
    setsockopt(tftp->socket, SOL_SOCKET, SO_RCVTIMEO, (const void*)&tmo, sizeof(tmo));

    tftp->tmo_retry = TFTP_MAX_RETYR;
    tftp->tmo_sec = TFTP_TMO_SEC;
    tftp->tx_size = req->filesize;
    tftp->block_size = req->block_size;

    if (req->opcode == TFTP_PACKET_WRQ)
    {
        do_recv_file(req);
    }
    else
    {
        do_send_file(req);
    }

    // close(sockfd); 和下面一样，删掉就行了
    // free(req);
init_error:
    if (sockfd >= 0)
    {
        close(sockfd);
    }
    free(req);
    return NULL;
}

static int wait_req(tftp_t* tftp, tftp_req_t* req)
{
    tftp_packet_t* pkt = &tftp->rx_packet;
    size_t pkt_size;
    int error = tftp_wait_packet(tftp, TFTP_PACKET_REQ, 0, &pkt_size);

    req->opcode = ntohs(pkt->opcode);
    req->option = 0;
    req->block_size = TFTP_DEFAULT_BLOCK_SIZE;
    req->filesize = 0;
    memset(req->filename, 0, sizeof(req->filename));
    memset(&req->tftp, 0, sizeof(req->tftp));
    memcpy(&req->tftp.remote, &tftp->remote, sizeof(tftp->remote));

    struct sockaddr_in* addr = (struct sockaddr_in*)&tftp->remote;
    printf("tftpd: recv req %s from %s %d\n",
        req->opcode == TFTP_PACKET_RRQ ? "get" : "put",
        inet_ntoa(addr->sin_addr), ntohs(addr->sin_port)
    );

    char* buffer = (char*)pkt->req.args;
    char* end = (char*)pkt + pkt_size;

    strncpy(req->filename, buffer, sizeof(req->filename));
    buffer += strlen(req->filename) + 1;

    if (strcmp(buffer, "octet") != 0)
    {
        printf("tftpd: unknow transfer mode %s\n", buffer);
        tftp_send_error(tftp, TFTP_ERROR_OP);
        return -1;
    }
    buffer += strlen("octet") + 1;

    while ((buffer < end) && (*buffer))
    {
        req->option = 1;
        if (strcmp(buffer, "blksize") == 0)
        {
            buffer += strlen("blksize") + 1;
            int size = atoi(buffer);
            if (size < 0)
            {
                tftp_send_error(tftp, TFTP_ERROR_OP);
                return -1;
            }
            else if (size > TFTP_BLOCK_SIZE)
            {
                req->block_size = TFTP_BLOCK_SIZE;
            }
            else
            {
                req->block_size = size;
            }

            buffer += strlen(buffer) + 1;
        }
        else if (strcmp(buffer, "tsize") == 0)
        {
            buffer += strlen(buffer) + 1;
            req->filesize = atoi(buffer);

            buffer += strlen(buffer) + 1;
        }
        else
        {
            buffer += strlen(buffer) + 1;
        }
    }

    return 0;
}

static void* tftp_server_thread(void*)
{
    printf("tftp server is running...\n");

    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0)
    {
        printf("tftpd: create server socket failed!\n");
        return NULL;
    }
    struct sockaddr_in sockaddr;
    memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(server_port);
    if (bind(sockfd, (const struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0)
    {
        printf("tftpd: bind error, port: %d\n", server_port);
        close(sockfd);
        return NULL;
    }

    tftp.socket = sockfd;


    while (1)
    {
        tftp_req_t* req = (tftp_req_t*)malloc(sizeof(tftp_req_t));
        if (req == NULL)
        {
            continue;
        }

        // 一个单独的线程去读取客户端发来的请求，进行请求的分发，分发到不同的线程去处理
        int error = wait_req(&tftp, req);
        if (error < 0)
        {
            free(req);
            continue;
        }

        pthread_t thread;
        error = pthread_create(&thread, NULL, tftp_worikng_thread, (void*)req);
        if (error != 0)
        {
            printf("tftpd: create working thread failed.\n");
            free(req);
            continue;
        }
    }
    close(sockfd);
    return NULL;
}

int tftpd_start(const char* dir, uint16_t port)
{
    server_path = dir;
    server_port = port ? port : TFTP_DEFAULT_PORT;

    pthread_t server_thread;
    int error = pthread_create(&server_thread, NULL, tftp_server_thread, (void*)NULL);
    if (error != 0)
    {
        printf("tftpd: create server thread failed.\n");
        return -1;
    }

    return 0;
}
