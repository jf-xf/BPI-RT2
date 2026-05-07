#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <net/if.h>
#ifdef CONFIG_DEV_xDSL
#include <rtk/linux/rtk_smux.h>
#else
#include "rtk_smux.h"
#endif

extern int smux_fd;

#define MAC_SRC 0x00,0x11,0x22,0x33,0x44,0x55
#define MAC_DST 0x00,0x11,0x22,0x33,0x44,0x56
#define TPID_IP 0x08,0x00
#define DUMMY_PAYLOAD \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07, \
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07
#define DEFAULT_TPID_VLAN_TCI 0x81,0x00,0x00,0x01

static int test00_setup(void)
{

    system("/bin/smuxctl --if eth0.2 --rx --tags 0 --rule-remove-all");
    system("/bin/smuxctl --if eth0.2 --tx --tags 0 --rule-remove-all");

    system("/bin/smuxctl --if-create-name eth0.2 test01");
    system("/bin/smuxctl --if eth0.2 --rx --tags 0 --set-rxif test01 --rule-append");
    system("/bin/smuxctl --if eth0.2 --tx --tags 0 --filter-txif test01 --push-tag --set-vid 100 1 --rule-append");
    system("ifconfig test01 up");
    return 0;
}

static int test00_teardown(void)
{
    /* remove all rules */
    //system("/bin/smuxctl --if eth0.2 --rx --tags 0 --rule-remove-all");
    //system("/bin/smuxctl --if eth0.2 --tx --tags 0 --rule-remove-all");
    return 0;
}

static void dump_ioctl_test_s(struct smux_ioctl_test_s *t)
{
    printf("result:%d flags:%xh dir:%d plen:%d skb:%x,%x mark=%x pri=%x dev=%s\n",
           t->result, t->flags, t->dir, t->payload_len, t->skbinfo.in_skb, t->skbinfo.out_skb,t->skbinfo.mark,t->skbinfo.priority,t->skbinfo.devname);
}

static void dump_bytes(uint8_t *p, uint32_t len)
{
    int i;

    for (i=0; i<len; i++) {
        if ((i&0xf)==0x0)
            printf("\n%4x: ",i);

        printf("%02x ", p[i]);
    }
    printf("\n");
}

static int t_payload_eq(struct smux_ioctl_test_s *pt, uint8_t *payload, uint32_t payload_len)
{
    if (pt->payload_len != payload_len)  {

        goto error;
    }
    if (memcmp(pt->payload,payload,payload_len)) {
        goto error;
    }
    return 1;
error:
    printf("Expected payload(%d)", payload_len);
    dump_bytes(payload,payload_len);
    printf("Acutal payload(%d)", pt->payload_len);
    dump_bytes(pt->payload,payload_len);
    return 0;
}

static int t_ifname_eq(struct smux_ioctl_test_s *pt, const char *ifname)
{
    if (strcmp(pt->skbinfo.devname,ifname)) {
        printf("Expected if=%s, got %s\n", ifname, pt->skbinfo.devname);
        return 0;
    }
    return 1;
}

#define T_SET_IFNAME(pt,x) 	strncpy((pt)->skbinfo.devname, x, IFNAMSIZ)
#define T_SET_PAYLOAD(pt, x) do { \
	(pt)->payload_len = sizeof(x); \
	memcpy((pt)->payload, x, sizeof(x)); \
} while (0)

#define T_PAYLOAD_EQ(pt, x) t_payload_eq(pt, x, sizeof(x))
#define T_IFNAME_EQ(pt, x) t_ifname_eq(pt, x)

static int test00_run00(void)
{

    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    strcpy(test.skbinfo.devname, "eth0.2");
    test.flags = 1;
    test.dir = SMUX_DIR_RX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);
    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"test01") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}


static int test00_run01(void)
{

    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"eth0.3");
    test.flags = 1;
    test.dir = SMUX_DIR_RX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);
    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.3") || !T_PAYLOAD_EQ(&test, payload)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static int test00_run02(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test01_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 0 --filter-txif test01 --push-tag --rule-append");
}

static int test01_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test02_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 1 --filter-txif test01 --pop-tag --rule-append");
}

static int test02_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test03_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 0 --filter-txif test01 --push-tag --set-cfi 1 1 --rule-append");
}

static int test03_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x10, 0x01, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test04_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 0 --filter-txif test01 --push-tag --set-priority 7 1 --rule-append");
}

static int test04_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0xE0, 0x01, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test05_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 0 --filter-txif test01 --push-tag --set-vid 4095 1 --rule-append");
}

static int test05_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x0F, 0xFF, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test06_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

	system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
	system("/bin/smuxctl --if test01 --tx --tags 0 --filter-txif test01 --push-tag --set-vid 150 1 --set-vid 100 1 --set-vid 50 1 --rule-append");
}

static int test06_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x32, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test07_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 1 --filter-txif test01 --push-tag --set-vid 100 2 --rule-append");
}

static int test07_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test08_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 2 --filter-txif test01 --push-tag --set-vid 100 3 --rule-append");
}

static int test08_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test09_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --push-tag --set-vid 100 4 --rule-append");
}

static int test09_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test10_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --pop-tag --rule-append");
}

static int test10_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test11_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 2 --filter-txif test01 --pop-tag --rule-append");
}

static int test11_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test12_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --copy-vid 3 2 --rule-append");
}

static int test12_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x10, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x10, 0x64, 0x81, 0x00, 0x00, 0x64, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test13_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --copy-cfi 3 2 --rule-append");
}

static int test13_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x11, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x11, 0x64, 0x81, 0x00, 0x10, 0x01, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test14_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --copy-priority 3 2 --rule-append");
}

static int test14_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x60, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x60, 0x64, 0x81, 0x00, 0x60, 0x01, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test15_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --copy-tpid 3 2 --rule-append");
}

static int test15_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x88, 0xA8, 0x60, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x88, 0xA8, 0x60, 0x64, 0x88, 0xA8, 0x00, 0x01, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test16_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --copy-8021q 3 2 --rule-append");
}

static int test16_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x70, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x70, 0x64, 0x81, 0x00, 0x70, 0x64, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test17_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --copy-vid 3 2 --copy-8021q 2 1 --rule-append");
}

static int test17_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x70, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x70, 0x64, 0x81, 0x00, 0x00, 0x64, 0x81, 0x00, 0x00, 0x64, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test18_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 3 --filter-txif test01 --copy-vid 3 2 --copy-8021q 2 1 --copy-priority 1 3 --rule-append");
}

static int test18_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x70, 0x64, DEFAULT_TPID_VLAN_TCI, DEFAULT_TPID_VLAN_TCI, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x10, 0x64, 0x81, 0x00, 0x00, 0x64, 0x81, 0x00, 0x00, 0x64, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test19_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 1 --filter-vid 100 1 --push-tag --set-vid 50 2 --rule-append");
}

static int test19_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x64, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x00, 0x32, 0x81, 0x00, 0x00, 0x64, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test20_setup(void)
{
        system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
	system("/bin/smuxctl --if test01 --tx --tags 1 --filter-priority 6 1 --pop-tag --rule-append");
}

static int test20_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0xC0, 0x64, TPID_IP, DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, TPID_IP, DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test21_setup(void)
{
	system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 1 --dscp-mapping-enable 1 1");
	system("/bin/smuxctl --if test01 --tx --tags 1 --dscp-mapping-set 0x10 2");
	system("/bin/smuxctl --if test01 --tx --tags 1 --filter-txif test01 --map-dscp-to-pbit 1 --rule-append");
}

static int test21_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0xA0, 0x64, TPID_IP, 0x45, 0x40,0x00,0x2A,0x04,0xD2,0x00,0x00,0x7F,0x00,0x36,0xF3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x40, 0x64, TPID_IP, 0x45, 0x40,0x00,0x2A,0x04,0xD2,0x00,0x00,0x7F,0x00,0x36,0xF3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test22_setup(void)
{
        system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 1 --filter-txif test01 --set-dscp 0x10 --rule-append");
}

static int test22_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, TPID_IP, 0x45, 0x20,0x00,0x2A,0x04,0xD2,0x00,0x00,0x7F,0x00,0x36,0xF3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, TPID_IP, 0x45, 0x40,0x00,0x2A,0x04,0xD2,0x00,0x00,0x7F,0x00,0x36,0xF3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test23_setup(void)
{
        system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 1 --dscp-mapping-enable 1 1");
        system("/bin/smuxctl --if test01 --tx --tags 1 --dscp-mapping-set 0x10 2");
        system("/bin/smuxctl --if test01 --tx --tags 1 --filter-txif test01 --map-dscp-to-pbit 1 --rule-append");
}

static int test23_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0xA0, 0x64, 0x86,0xDD,0x64,0x00,0x00,0x00,0x00,0x00,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, 0x81, 0x00, 0x40, 0x64, 0x86,0xDD,0x64,0x00,0x00,0x00,0x00,0x00,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}

static void test24_setup(void)
{
        system("/bin/smuxctl --if-delete test01");

        system("/bin/smuxctl --if-create-name eth0.2 test01 --set-if-rsmux");
        system("/bin/smuxctl --if test01 --tx --tags 1 --filter-txif test01 --set-dscp 0x10 --rule-append");
}

static int test24_run(void)
{
    struct smux_ioctl_test_s test;
    int ret = 0, i;

    const uint8_t payload[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, 0x86,0xDD, 0x62,0x00,0x00,0x00,0x00,0x00,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };
    const uint8_t expected[] = { MAC_SRC, MAC_DST, DEFAULT_TPID_VLAN_TCI, 0x86,0xDD, 0x64,0x00,0x00,0x00,0x00,0x00,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,DUMMY_PAYLOAD };

    memset(&test, 0, sizeof(test));
    T_SET_IFNAME(&test,"test01");
    test.flags = 1;
    test.dir = SMUX_DIR_TX;
    T_SET_PAYLOAD(&test, payload);

    ret = ioctl(smux_fd, SMUXCTL_CMD_TEST, &test);

    dump_ioctl_test_s(&test);

    if (ret || !T_IFNAME_EQ(&test,"eth0.2") || !T_PAYLOAD_EQ(&test, expected)) {
        printf("%s(%d): ret=%d\n", __func__,__LINE__,ret);
        ret = -1;
    }

    return ret;
}


int run_test_cases(void)
{
    int ret = 0;
    unsigned int passed = 0, failed = 0;

    test00_setup();
    test00_run00() ? failed++ : passed++;
    test00_run01() ? failed++ : passed++;
    test00_run02() ? failed++ : passed++;
    test00_teardown();
    test01_setup();
    test01_run() ? failed++ : passed++;//test tx --push-tag without vid
    test02_setup();
    test02_run() ? failed++ : passed++;//test tx pop vlan tag
    test03_setup();
    test03_run() ? failed++ : passed++;//test tx set cfi(1)
    test04_setup();
    test04_run() ? failed++ : passed++;//test tx set priority(7)
    test05_setup();
    test05_run() ? failed++ : passed++;//test tx set maximum(4095) vid
    test06_setup();
    test06_run() ? failed++ : passed++;//test tx set three vid
    test07_setup();
    test07_run() ? failed++ : passed++;//test tx set 2 layer vlan tag
    test08_setup();
    test08_run() ? failed++ : passed++;//test tx set 3 layer vlan tag
    test09_setup();
    test09_run() ? failed++ : passed++;//test tx set 4 layer vlan tag
    test10_setup();
    test10_run() ? failed++ : passed++;//test tx pop 3 layer vlan tag
    test11_setup();
    test11_run() ? failed++ : passed++;//test tx pop 2 layer vlan tag
    test12_setup();
    test12_run() ? failed++ : passed++;//test tx copy vid
    test13_setup();
    test13_run() ? failed++ : passed++;//test tx copy cfi
    test14_setup();
    test14_run() ? failed++ : passed++;//test tx copy priority
    test15_setup();
    test15_run() ? failed++ : passed++;//test tx copy tpid
    test16_setup();
    test16_run() ? failed++ : passed++;//test tx copy entire vlan header
    test17_setup();
    test17_run() ? failed++ : passed++;//test tx two copy command
    test18_setup();
    test18_run() ? failed++ : passed++;//test tx three copy command
    test19_setup();
    test19_run() ? failed++ : passed++;//test tx filter vid and push tag
    test20_setup();
    test20_run() ? failed++ : passed++;//test tx filter priority and pop tag
    test21_setup();
    test21_run() ? failed++ : passed++;//test tx dscp mapping priority
    test22_setup();
    test22_run() ? failed++ : passed++;//test tx set dscp
    test23_setup();
    test23_run() ? failed++ : passed++;//test tx ipv6 dscp mapping priority
    test24_setup();
    test24_run() ? failed++ : passed++;//test tx ipv6 set dscp

    printf("Test Result: %d passed, %d failed\n", passed, failed);
    return ret;
};
