#include "tftp_base.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char* tftp_error_message(uint16_t error_code)
{
    static const char* msg[] =
    {
        "unknow error",
        "file not found",
        "Access violation",
        "Disk full or allocation exceeded",
        "Illegal TFTP operation",
        "Unknown transfer ID",
        "File already exists",
        "No such user"
    };

    if (error_code >= TFTP_ERROR_END)
    {
        error_code = 0;
    }

    return msg[error_code];
}

static char* write_information(tftp_t* tftp, char* buffer, const char* information, int value)
{
    // 计算包末端的地址
    char* buffer_end = ((char*)&tftp->tx_packet) + sizeof(tftp_packet_t);
    size_t len = strlen(information) + 1; // +1是因为算了'\0'
    if (buffer + len > buffer_end)
    {
        printf("tftp: send buffer too small\n");
        return NULL;
    }
    strcpy(buffer, information);
    buffer += len;

    if (value >= 0)
    {
        if (buffer + 16 >= buffer_end)
        {
            printf("tftp: send buffer too small\n");
            return NULL;
        }

        sprintf(buffer, "%d", value);
        buffer += strlen(buffer) + 1;
    }

    return buffer;
}

int tftp_send_packet(tftp_t* tftp, tftp_packet_t* pkt, int size)
{
    ssize_t send_size = sendto(tftp->socket, (const void*)pkt, size, 0, &tftp->remote, sizeof(tftp->remote));
    if (send_size < 0)
    {
        printf("tftp: send error\n");
        return -1;
    }

    tftp->tx_size = size;
    return 0;
}

int tftp_send_request(tftp_t* tftp, int is_read, const char* filename, uint32_t file_size, int option)
{
    tftp_packet_t* pkt = &tftp->tx_packet;

    pkt->opcode = htons(is_read ? TFTP_PACKET_RRQ : TFTP_PACKET_WRQ);
    char* buffer = (char*)pkt->req.args;
    buffer = write_information(tftp, buffer, filename, -1);
    if (buffer == NULL)
    {
        printf("tftp: filename too long: %s\n", filename);
        return -1;
    }

    buffer = write_information(tftp, buffer, "octet", -1);
    if (buffer == NULL)
    {
        // 文件名太长导致选项没有空间了
        printf("tftp: filename too long: %s\n", filename);
        return -1;
    }

    if (option)
    {
        buffer = write_information(tftp, buffer, "blksize", tftp->block_size);
        if (buffer == NULL)
        {
            return -1;
        }

        buffer = write_information(tftp, buffer, "tsize", file_size);
        if (buffer == NULL)
        {
            return -1;
        }
    }

    int size = (int)(buffer - (char*)pkt->req.args) + 2;
    int error = tftp_send_packet(tftp, pkt, size);
    if (error < 0)
    {
        printf("tftp: send req failed.\n");
        return -1;
    }
    return 0;
}


int tftp_send_ack(tftp_t* tftp, uint16_t block_num)
{
    tftp_packet_t* pkt = &tftp->tx_packet;
    pkt->opcode = htons(TFTP_PACKET_ACK);
    pkt->ack.block_num = htons(block_num);

    int error = tftp_send_packet(tftp, pkt, 4);
    if (error < 0)
    {
        printf("tftp: send ack failed. block_num = %d\n", block_num);
        return -1;
    }

    return 0;
}

int tftp_send_data(tftp_t* tftp, uint16_t block_num, size_t size)
{
    tftp_packet_t* pkt = &tftp->tx_packet;

    pkt->opcode = htons(TFTP_PACKET_DATA);
    pkt->data.block_num = htons(block_num);
    int error = tftp_send_packet(tftp, pkt, 4 + (int)size);
    if (error < 0)
    {
        printf("tftp: send data failed. block_num = %d\n", block_num);
        return -1;
    }

    return 0;
}

int tftp_send_error(tftp_t* tftp, uint16_t error_code)
{
    tftp_packet_t* pkt = &tftp->tx_packet;
    pkt->opcode = htons(TFTP_PACKET_ERROR);
    pkt->error.error_code = htons(error_code);

    const char* msg = tftp_error_message(error_code);
    strcpy(pkt->error.error_msg, msg);

    int error = tftp_send_packet(tftp, pkt, 4 + (int)strlen(msg) + 1);
    if (error < 0)
    {
        printf("tftp: send data failed. error_code = %d", error_code);
        return -1;
    }

    return 0;
}

int tftp_resend(tftp_t* tftp)
{
    tftp_packet_t* pkt = &tftp->tx_packet;
    if (tftp_send_packet(tftp, pkt, tftp->tx_size))
    {
        printf("tftp: resend error\n");
        return -1;
    }

    return 0;
}

int tftp_wait_packet(tftp_t* tftp, tftp_op_t opcode, uint16_t block_num, size_t* pkt_size)
{
    tftp_packet_t* pkt = &tftp->rx_packet;

    tftp->tmo_retry = TFTP_MAX_RETYR;
    while (1)
    {
        socklen_t len = sizeof(struct sockaddr);
        ssize_t size = recvfrom(tftp->socket, (uint8_t*)pkt, sizeof(tftp_packet_t), 0, &tftp->remote, &len);
        if (size < 0)
        {
            if (--tftp->tmo_retry == 0)
            {
                printf("tftp: wait tmo\n");
                return -1;
            }
            else
            {
                tftp_resend(tftp);
                continue;
            }
        }

        *pkt_size = (size_t)size;

        uint16_t _opcode = htons(pkt->opcode);
        if (opcode == TFTP_PACKET_REQ)
        {
            if ((_opcode != TFTP_PACKET_RRQ) && (_opcode != TFTP_PACKET_WRQ))
            {
                continue;
            }
        }
        else if (_opcode != opcode)
        {
            tftp_resend(tftp);
            continue;
        }

        switch (_opcode)
        {
        case TFTP_PACKET_DATA:
        case TFTP_PACKET_ACK:
        {
            if (htons(pkt->data.block_num) != block_num)
            {
                tftp_resend(tftp);
                break;
            }
            return 0;
        }
        case TFTP_PACKET_RRQ:
        case TFTP_PACKET_WRQ:
        {
            return 0;
        }
        case TFTP_PACKET_ERROR:
        {
            pkt->error.error_msg[tftp->block_size - 1] = '\0';
            printf("tftp: recv error=%d, reason: %s\n", ntohs(pkt->error.error_code), pkt->error.error_msg);
            return -1;
        }
        case TFTP_PACKET_OACK:
        {
            tftp_parse_oack(tftp);
            return 0;
        }

        default:
        {
            tftp_resend(tftp);
            break;
        }
        }
    }
}

int tftp_parse_oack(tftp_t* tftp)
{
    char* buffer = (char*)&tftp->rx_packet.oack.option;
    char* end = (char*)&tftp->rx_packet + sizeof(tftp_packet_t);

    while ((buffer < end) && (*buffer))
    {
        if (strcmp(buffer, "blksize") == 0)
        {
            buffer += (strlen(buffer) + 1);

            int blksize = atoi(buffer);
            if (blksize == 0)
            {
                printf("tftp: unknown blksize \n");
                return -1;
            }
            else if (blksize < tftp->block_size)
            {
                tftp->block_size = blksize;
                printf("tftp: use new blksize %d\n", blksize);
            }
            else if (blksize > tftp->block_size)
            {
                printf("tftp: block size %d\n", blksize);
                return -1;
            }
            buffer += (strlen(buffer) + 1);
        }
        else if (strcmp(buffer, "tsize") == 0)
        {
            buffer += strlen(buffer) + 1;
            tftp->file_size = atoi(buffer);

            buffer += (strlen(buffer) + 1);
        }
        else
        {
            buffer += (strlen(buffer) + 1);
        }

    }

    return 0;
}

int tftp_send_oack(tftp_t* tftp)
{
    tftp_packet_t* pkt = &tftp->tx_packet;

    pkt->opcode = htons(TFTP_PACKET_OACK);
    char* buffer = pkt->oack.option;
    buffer = write_information(tftp, buffer, "blksize", tftp->block_size);
    if (buffer == NULL)
    {
        return -1;
    }

    buffer = write_information(tftp, buffer, "tsize", tftp->file_size);
    if (buffer == NULL)
    {
        return -1;
    }

    int error = tftp_send_packet(tftp, pkt, buffer - (char*)pkt);
    if (error < 0)
    {
        
        printf("tftp: send oack failed\n");
        return -1;
    }

    return 0;
}