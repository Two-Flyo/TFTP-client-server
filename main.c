#include <stdio.h>

#include "tftp_client.h"
#include "tftp_server.h"

// get
// put 
// block
// quit

int main(void)
{
    printf("(to) ");
    char ip[16] = { 0 };
    scanf("%15s", ip);
    getchar();

    tftpd_start(".", 10000);
    // tftpd_start(".", TFTP_DEFAULT_PORT);
    tftp_start(ip, TFTP_DEFAULT_PORT);
    printf("test");
    // tftp_get(ip, TFTP_DEFAULT_PORT, 1024, "1.png", 1);
    // tftp_put(ip, TFTP_DEFAULT_PORT, 1024, "2.jpg", 1);
    return 0;
}
