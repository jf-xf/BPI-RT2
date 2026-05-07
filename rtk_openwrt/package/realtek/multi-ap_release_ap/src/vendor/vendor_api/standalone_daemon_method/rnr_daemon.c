#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include "sysconfig.h"

#include "map_tlvs.h"
#include "map_utils.h"
#include "map_commands.h"

#include "map_vendor_api.h"
#include "vendor_header.h"

#define NETLINK_RNR 25
#define MAX_PAYLOAD 1024
#define CONTROLLER_MODE "1"
#define AGENT_MODE "2"

#define ERR_EXIT(m) \
do\
{\
    perror(m);\
    exit(EXIT_FAILURE);\
}\
while (0);\

struct efd_subtype
{
	uint16_t sport;
	uint16_t dport;
	uint32_t sip;
	uint32_t dip;
	char protocol;
	char direction;
	char is_ef;
	char dummy3;
};

struct mac_subtype
{
	unsigned char mac[6];
};

struct msg_format
{
	uint8_t id;
	char sub_type;
	uint16_t num;
	char payload[];
};

void creat_daemon(void);

void recv_from_multiap()
{
	char buff[1024];
	int list_num;
	int i;
	char cmd[100] = {0};
	ssize_t bytes_read;
	unsigned char sip_bytes[4];
	unsigned char dip_bytes[4];
	struct mac_subtype *black_mac_list;
	struct mac_subtype *white_mac_list;
	struct efd_subtype *tuple_list;

	mqd_t mq;
	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 2;
	attr.mq_msgsize = 1024;
	attr.mq_curmsgs = 0;

	mq = mq_open("/map_efd", O_CREAT | O_RDONLY, 0644, &attr);
	CHECK((mqd_t)-1 != mq);

	while(1)
	{
		/* receive the message */
		printf("wait for message\n");
		memset(buff, 0, 1024);
		bytes_read = mq_receive(mq, buff, 1024, NULL);
		CHECK(bytes_read >= 0);
		/*read(fd, buff, sizeof(buff));*/

		struct msg_format *msg = (struct msg_format *)buff;

		msg->num = ntohs(msg->num);

		switch(msg->sub_type)
		{
		case 'n':
			if (msg->payload[0] == 1)
			{
				printf("RNR turn on\n");
				system("echo set on > /proc/realtek/rnr");
			}
			else if (msg->payload[0] == 0)
			{
				printf("RNR turn off\n");
				system("echo set off > /proc/realtek/rnr");
			}
			else
				printf("wrong format!");
			break;

		case 'b':

			list_num = msg->num;
			if(list_num == 0)
			{
				memset(cmd, 0, 100);
				system("echo set black_list 0 FF:00:00:00:00:00 > /proc/realtek/rnr");
				break;
			}

			black_mac_list = (struct mac_subtype *)msg->payload;

			for(i=0; i<list_num; i++)
			{
				memset(cmd, 0, 100);
				sprintf(cmd, "echo set black_list %d %02x:%02x:%02x:%02x:%02x:%02x > /proc/realtek/rnr", i, black_mac_list[i].mac[0], black_mac_list[i].mac[1], black_mac_list[i].mac[2], black_mac_list[i].mac[3], black_mac_list[i].mac[4], black_mac_list[i].mac[5]);

				printf("%s\n",cmd);
				system(cmd);
			}

			memset(cmd, 0, 100);
			sprintf(cmd, "echo set black_list %d FF:00:00:00:00:00 > /proc/realtek/rnr", i);
			printf("%s\n",cmd);
			system(cmd);

			break;

		case 'w':

			list_num = msg->num;
			if(list_num == 0)
			{
				memset(cmd, 0, 100);
				system("echo set white_list 0 FF:00:00:00:00:00 > /proc/realtek/rnr");
				break;
			}

			white_mac_list = (struct mac_subtype *)msg->payload;

			for(i=0; i<list_num; i++)
			{
				memset(cmd, 0, 100);
				sprintf(cmd, "echo set white_list %d %02x:%02x:%02x:%02x:%02x:%02x > /proc/realtek/rnr", i, white_mac_list[i].mac[0], white_mac_list[i].mac[1], white_mac_list[i].mac[2], white_mac_list[i].mac[3], white_mac_list[i].mac[4], white_mac_list[i].mac[5]);
				printf("%s\n",cmd);
				system(cmd);
			}

			memset(cmd, 0, 100);
			sprintf(cmd, "echo set white_list %d FF:00:00:00:00:00 > /proc/realtek/rnr", i);
			printf("%s\n",cmd);
			system(cmd);

			break;

		case 'e':

			tuple_list = (struct efd_subtype *)msg->payload;
			list_num = msg->num;

			for(i=0; i<list_num; i++)
			{
				sip_bytes[0] = ntohl(tuple_list[i].sip) & 0xFF;
				sip_bytes[1] = (ntohl(tuple_list[i].sip) >> 8) & 0xFF;
				sip_bytes[2] = (ntohl(tuple_list[i].sip) >> 16) & 0xFF;
				sip_bytes[3] = (ntohl(tuple_list[i].sip) >> 24) & 0xFF;

				dip_bytes[0] = ntohl(tuple_list[i].dip) & 0xFF;
				dip_bytes[1] = (ntohl(tuple_list[i].dip) >> 8) & 0xFF;
				dip_bytes[2] = (ntohl(tuple_list[i].dip) >> 16) & 0xFF;
				dip_bytes[3] = (ntohl(tuple_list[i].dip) >> 24) & 0xFF;

				memset(cmd, 0, 100);
				sprintf(cmd, "echo set flow %u %u %d.%d.%d.%d %d.%d.%d.%d %x %x %x > /proc/realtek/rnr",ntohs(tuple_list[i].sport), ntohs(tuple_list[i].dport),sip_bytes[3], sip_bytes[2], sip_bytes[1], sip_bytes[0], dip_bytes[3], dip_bytes[2], dip_bytes[1], dip_bytes[0], tuple_list[i].protocol, tuple_list[i].direction, tuple_list[i].is_ef );
				printf("%s\n",cmd);
				system(cmd);

			}

			break;

		default:
			printf("wrong format!\n");
			break;
		}


	}

	/* cleanup */
	CHECK((mqd_t)-1 != mq_close(mq));
	CHECK((mqd_t)-1 != mq_unlink("/map_efd"));

}

int main(int argc, char *argv[])
{
	int state;
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	struct msghdr msg;
	int sock_fd, retval;
	int state_smg = 0;
	uint8_t target_al_mac[6] = MCAST_1905;
	uint8_t vendor_oui[3]    = { 0x00, 0xE0, 0x4C };
	uint8_t relay_indicator  = 1;
	uint16_t data_len;
	char payload[1024] = { 0 };
	int i;

	if(argc < 2)
	{
		printf("help:\n");
		printf("     Controller : map_rnr 1\n");
		printf("     Agent : map_rnr 2\n");

		return 0;
	}

	creat_daemon();
	if(!(strcmp(argv[1], AGENT_MODE)))
	{
		recv_from_multiap();
	}
	else if(!(strcmp(argv[1], CONTROLLER_MODE)))
	{
		sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_RNR);

		if(sock_fd == -1)
		{
			printf("error getting socket: %s", strerror(errno));
			return -1;
		}

		// To prepare binding
		memset(&src_addr, 0, sizeof(src_addr));
		src_addr.nl_family = AF_NETLINK;
		src_addr.nl_pid = getpid();
		src_addr.nl_groups = 0;

		retval = bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));
		if (retval < 0)
		{
			printf("bind failed: %s", strerror(errno));
			close(sock_fd);
			return -1;
		}

		nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
		if (!nlh)
		{
			printf("malloc nlmsghdr error!\n");
			close(sock_fd);
			return -1;
		}

		memset(&dest_addr,0,sizeof(dest_addr));
		dest_addr.nl_family = AF_NETLINK;
		dest_addr.nl_pid = 0;
		dest_addr.nl_groups = 0;
		nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
		nlh->nlmsg_pid = getpid();
		nlh->nlmsg_flags = 0;
		strcpy(NLMSG_DATA(nlh), "Hello from EFD daemon!");
		iov.iov_base = (void *)nlh;
		iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

		memset(&msg, 0, sizeof(msg));
		msg.msg_name = (void *)&dest_addr;
		msg.msg_namelen = sizeof(dest_addr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;

		//send message
		printf("state_smg\n");
		state_smg = sendmsg(sock_fd,&msg,0);
		if (state_smg == -1)
		{
			printf("get error sendmsg = %s\n",strerror(errno));
		}

		while(1)
		{
			printf("waiting received!\n");
			state = recvmsg(sock_fd, &msg, 0);
			if (state < 0)
			{
				printf("state<1");
			}

			struct msg_format *msg;
			char sub_type;
			int num;

			msg = (struct msg_format *)NLMSG_DATA(nlh);
			sub_type = msg->sub_type;
			num = msg->num;
			msg->num = htons(msg->num);

			memset(payload, 0, sizeof(payload));

			if(sub_type == 'e' )
			{
				for(i=0; i<num; i++)
				{
					((struct efd_subtype *)(msg->payload))[i].sport = htons(((struct efd_subtype *)(msg->payload))[i].sport);
					((struct efd_subtype *)(msg->payload))[i].dport = htons(((struct efd_subtype *)(msg->payload))[i].dport);
					((struct efd_subtype *)(msg->payload))[i].sip = htonl(((struct efd_subtype *)(msg->payload))[i].sip);
					((struct efd_subtype *)(msg->payload))[i].dip = htonl(((struct efd_subtype *)(msg->payload))[i].dip);
				}
				data_len = sizeof(struct msg_format) + sizeof(struct efd_subtype) * num;
				memcpy(payload, msg, data_len);
			}
			else if(sub_type == 'b' || sub_type == 'w')
			{
				data_len = sizeof(struct msg_format) + sizeof(struct mac_subtype) * num;
				memcpy(payload, msg, data_len);
			}
			else if(sub_type == 'n')
			{
				printf("in n case\n");
				data_len = sizeof(struct msg_format) + sizeof(char);
				memcpy(payload, msg, data_len);
			}
			else
				continue;

			send_vendor_specific_message(target_al_mac, relay_indicator, vendor_oui, data_len, (uint8_t *)payload);
		}
	}
	else
	{
		printf("not agnet or controller, exit\n");
	}

	return 0;
}

void creat_daemon(void)
{
	pid_t pid;
	pid = fork();
	if( pid == -1)
		ERR_EXIT("fork error");
	if(pid > 0 )
		exit(EXIT_SUCCESS);
	if(setsid() == -1)
		ERR_EXIT("SETSID ERROR");
	chdir("/");

	umask(0);
	return;
}

