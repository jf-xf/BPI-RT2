#include "./phl_types.h"
#include "./phl_test_mp_def.h"

#ifndef NEW_DAEMON_UDPSERVER_H
#define NEW_DAEMON_UDPSERVER_H

#define TEST_PROGRAM 0

#define CONNECT_PORT 9035

#if TEST_PROGRAM
// run on virtual machine
#define SERVER_IP "192.168.56.105"
#define CLIENT_IP "192.168.56.1"

#else
// run on 97G/98D platform
#define SERVER_IP "192.168.1.254"
#define CLIENT_IP "192.168.1.66"

#endif //end of TEST_PROGRAM

#define MAX_TEST_CMD_BUF 2000 //note: need need to sync with dll & core

#ifndef CONFIG_CPU_BIG_ENDIAN
#define endian_convert16(val) (((direction) == TO_DRIVER) ? be16toh(val) : htobe16(val))
#define endian_convert32(val) (((direction) == TO_DRIVER) ? be32toh(val) : htobe32(val))
#endif

struct argshell {
    u8 type;
    u8 buf[MAX_TEST_CMD_BUF];
    u16 len;
};

struct infoshell{
    s8 device_id[6];
    struct argshell arg_ctx;
};

enum TEST_SUB_MODULE {
    TEST_SUB_MODULE_MP = 0,
    TEST_SUB_MODULE_UNKNOWN,
};

enum {
	TO_DRIVER = 0,
	FROM_DRIVER = 1,
};

int rtw_phl_ioctl(int skfd, char *inifname, void *buffer, unsigned int buffer_size);
bool rtw_check_flash_op(struct argshell *src);
void sync_shadowmap(struct mp_flash_arg *arg, char *ifname);
#endif //NEW_DAEMON_UDPSERVER_H
