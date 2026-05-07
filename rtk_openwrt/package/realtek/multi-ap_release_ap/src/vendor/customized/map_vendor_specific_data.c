#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/file.h>

#include "map_vendor_specific_data.h"

#include "cJSON.h"

#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#ifdef RTK_ETHERNET_LANNETINFO
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#else
#include <sys/msg.h>
#include <sys/syscall.h>
#endif

#ifdef CONFIG_USER_LANNETINFO_JSON_SHOW
#include <ctype.h>
#endif

#if defined(CONFIG_RTK_PON_XDSL) || !defined(__UCLIBC__)
size_t strlcpy(char *dst, const char *src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';            /* NUL-terminate dst */
		while (*src++)
			;
	}

	return(src - osrc - 1); /* count does not include NUL */
}
#endif

#if 1//def RTK_ETHERNET_ADAPTER

int rtk_set_lock(int fd, int lock_type)
{
	int ret;

	if(lock_type==F_WRLCK)
		lock_type=LOCK_EX;
	else if(lock_type==F_UNLCK)
		lock_type=LOCK_UN;
	else if(lock_type==F_RDLCK)
		lock_type=LOCK_SH;	

	if(lock_type!=LOCK_EX && lock_type!=LOCK_UN && lock_type!=LOCK_SH)
	{
		printf("\n%s:%d Invalid argument!!!\n",__FUNCTION__,__LINE__);
		return RTK_FAILED;
	}

try_again:

	ret=flock(fd, lock_type);

 	if(ret == -1)
	{
		//printf("\n%s:%d errno:%d\n",__FUNCTION__,__LINE__,errno);
		if (errno == EINTR)
		{
			usleep(1000);
			//printf("\n%s:%d try again\n",__FUNCTION__,__LINE__);
			goto try_again;
		}
	}

	return RTK_SUCCESS;
}

#ifdef RTK_ETHERNET_LANNETINFO
int flock_set(int fd, int type)
{
	struct flock lock;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	lock.l_type = type;
	lock.l_pid = getpid();

	if((fcntl(fd, F_SETLKW, &lock)) < 0)
	{
		printf("Lock failed:type = %d\n", lock.l_type);
		return 1;
	}
	return 0;
}

void *read_data_from_file(int fd, unsigned char *file, unsigned int *size)
{
	struct stat status;
	int read_size, read_count;
	void *dataBuf = NULL;

	if(fd < 0 || file == NULL)
	{
		return NULL;
	}

	if ( stat((char *)file, &status) < 0 )
	{
		return NULL;
	}
	read_size = (unsigned long)(status.st_size);

	dataBuf = (void *) malloc(read_size);
	if(dataBuf == NULL)
	{
		return NULL;
	}
	memset(dataBuf, 0, read_size);

	read_count = read(fd, (void*)dataBuf, read_size);
	if (read_count != read_size)
	{
		printf("(%s:%d),Read file data failed!\n",__FUNCTION__, __LINE__);
	}

	*size = read_count;

	return dataBuf;
}

#if !defined( RTK_RETRY_GET_FLOCK)
int read_pid(const char *filename)
{
	FILE *fp;
	int pid = 0;

	if ((fp = fopen(filename, "r")) == NULL)
		return -1;
	if(fscanf(fp, "%d", &pid) != 1 || kill(pid, 0) != 0){
		pid = 0;
	}
	fclose(fp);

	return pid;
}
#endif

#ifndef CONFIG_USER_LANNETINFO_JSON_SHOW
int get_lan_net_info(lanHostInfo_t **ppLANNetInfoData, unsigned int *pCount)
{
	int fd, read_count;
	void *p = NULL;
#ifdef RTK_RETRY_GET_FLOCK
	unsigned long long msecond = 0;
	struct timeval tv;
	struct timeval initial_tv;
#else
	int pid;
#endif

	if( (ppLANNetInfoData==NULL)||(pCount==NULL) )
	{
		return -1;
	}

	*ppLANNetInfoData=NULL;
	*pCount=0;

#if !defined( RTK_RETRY_GET_FLOCK)
	pid = read_pid(LANNETINFO_RUNFILE);
	if( pid>0 )
		kill(pid, SIGUSR2);

	usleep(60000);//60ms
#endif

	fd = open(LANNETINFOFILE, O_RDONLY);
	if(fd < 0)
		return -3;

#ifdef RTK_RETRY_GET_FLOCK
	gettimeofday (&tv, NULL);
	initial_tv.tv_sec = tv.tv_sec;
	initial_tv.tv_usec = tv.tv_usec;

	do {
		if (!flock_set(fd, F_RDLCK))
		{
			if ((p = (void *)read_data_from_file(fd, (unsigned char *)LANNETINFOFILE, (unsigned int *)&read_count)) == NULL)
			{
				printf("(%s)%d read file error!\n", __FUNCTION__, __LINE__);
				flock_set(fd, F_UNLCK);
				close(fd);
				return -3;
			}

			*ppLANNetInfoData = (lanHostInfo_t *)p;
			*pCount = read_count / sizeof(lanHostInfo_t);

			flock_set(fd, F_UNLCK);
			break;
		}

		gettimeofday(&tv, NULL);
		msecond = ((unsigned long long) tv.tv_sec * 1000 + tv.tv_usec / 1000) - ((unsigned long long) initial_tv.tv_sec * 1000 + initial_tv.tv_usec / 1000);
	} while(msecond < MAX_READLANNETINFO_INTERVAL);
#else
	if(!flock_set(fd, F_RDLCK))
	{
		if ((p = (void *)read_data_from_file(fd, (unsigned char *)LANNETINFOFILE, (unsigned int *)&read_count)) == NULL)
		{
			printf("(%s)%d read file error!\n", __FUNCTION__, __LINE__);
			flock_set(fd, F_UNLCK);
			close(fd);
			return -3;
		}

		*ppLANNetInfoData = (lanHostInfo_t *)p;
		*pCount = read_count / sizeof(lanHostInfo_t);
		flock_set(fd, F_UNLCK);
	}
#endif

	close(fd);
	return 0;
}
#else
void convertMacFormat(char *str, unsigned char *mac)
{
	char tmpBuf[4];
	int i, j = 0;

	for (i = 0; i < 17; i += 3)
	{
		tmpBuf[0] = str[i];
		tmpBuf[1] = str[i+1];
		tmpBuf[2] = 0;

		if (!isxdigit(tmpBuf[0]) || !isxdigit(tmpBuf[1]))
			return;

		mac[j++] = (unsigned char)strtol(tmpBuf, (char**)NULL, 16);
	}
	//printf("%x %x %x %x %x %x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int get_lan_net_info(lanHostInfo_t **ppLANNetInfoData, unsigned int *pCount)
{
	int ret = RTK_FAILED;
	cJSON *data = NULL, *devices = NULL, *client = NULL, *array = NULL, *arrayObj = NULL;
	int device_number = 0, i, j;
	cJSON* statistics;
	int fd = -1;
	FILE *fp = NULL;

	int alloc_sz = 0;
	struct stat st;
	char *buf = NULL;

	lanHostInfo_t *tmpInfo = NULL;

	memset(&st, 0, sizeof(struct stat));
	//prepare buffer and fp
	if(stat(LANNETINFOFILE, &st)==-1) {
		goto exit;
	}
	alloc_sz = st.st_size + 1;

	buf = (char *)malloc(alloc_sz);
	if(buf==NULL) {
		goto exit;
	}
	memset(buf, 0x0, alloc_sz);

	fp = fopen(LANNETINFOFILE, "r");
	if(fp==NULL) {
		goto exit;
	}
	fd = fileno(fp);

	//read file and parse
	flock_set(fd, F_RDLCK);

	int read_nr = fread(buf, 1, alloc_sz, fp);
	if(read_nr == 0){
		flock_set(fd, F_UNLCK);
		goto exit;
	}

	flock_set(fd, F_UNLCK);

	data = cJSON_Parse(buf);
	if(data==NULL) {
		goto exit;
	}

	devices = cJSON_GetObjectItem(data, "clients");
	if(devices == NULL){
		goto exit;
	}

	device_number = cJSON_GetArraySize(devices);
	tmpInfo = (lanHostInfo_t *)malloc(sizeof(lanHostInfo_t)*device_number);
	if(tmpInfo==NULL) {
		goto exit;
	}
	memset(tmpInfo, 0, sizeof(lanHostInfo_t)*device_number);

	for(i=0; i<device_number; i++){
		lanHostInfo_t* lanInfo = &tmpInfo[i];

		client = cJSON_GetArrayItem(devices, i);
		if(client == NULL){
			goto exit;
		}
		//macAddress
		cJSON* cur = cJSON_GetObjectItem(client, "macAddress");
		if(cur == NULL){
			goto exit;
		}
		convertMacFormat(cur->valuestring, lanInfo->mac);

		//agent macAddress
		cur = cJSON_GetObjectItem(client, "agent_macAddress");
		if(cur == NULL){
			goto exit;
		}
		convertMacFormat(cur->valuestring, lanInfo->agent_mac);

		//connect time
		cur = cJSON_GetObjectItem(client, "connect time");
		if(cur == NULL){
			goto exit;
		}
		lanInfo->onLineTime = cur->valueint;

		//device name
		cur = cJSON_GetObjectItem(client, "device name");
		if(cur == NULL){
			goto exit;
		}
		strncpy(lanInfo->devName, cur->valuestring, sizeof(lanInfo->devName));

		//ipAddress
		cur = cJSON_GetObjectItem(client, "ipAddress");
		if(cur == NULL){
			goto exit;
		}

		struct in_addr ipaddr;
		memset(&ipaddr, 0 , sizeof(struct in_addr));
		inet_aton(cur->valuestring, &ipaddr);
		lanInfo->ip = ipaddr.s_addr;

		//ipv6Addr
		cur = cJSON_GetObjectItem(client, "ip6Address");
		if(cur == NULL){
			goto exit;
		}
		snprintf((char *)lanInfo->ipv6Addr, sizeof(lanInfo->ipv6Addr), cur->valuestring);

		//port
		cur = cJSON_GetObjectItem(client, "port");
		if(cur == NULL){
			goto exit;
		}
		lanInfo->port = cur->valueint;

		//phy_port
		cur = cJSON_GetObjectItem(client, "phy_port");
		if(cur == NULL){
			goto exit;
		}
		lanInfo->phy_port = cur->valueint;

		//connectionType
		cur = cJSON_GetObjectItem(client, "interface");
		if(cur == NULL){
			goto exit;
		}
		strncpy(lanInfo->ifName, cur->valuestring, sizeof(lanInfo->ifName));

		//lanhost_ipv6s
		array = cJSON_GetObjectItem(client, "lanhost_ipv6s");
		if(array == NULL){
			goto exit;
		}
		for (j = 0; j < MAX_IPV6_CNT; j++) {
			arrayObj = cJSON_GetArrayItem(array, j);
			statistics = cJSON_GetObjectItem(arrayObj, "ipv6Addr");
			if(statistics == NULL){
				goto exit;
			}
			snprintf(lanInfo->lanhost_ipv6s[j].ipv6Addr, sizeof(lanInfo->lanhost_ipv6s[j].ipv6Addr), statistics->valuestring);

			statistics = cJSON_GetObjectItem(arrayObj, "type");
			if(statistics == NULL){
				goto exit;
			}
			lanInfo->lanhost_ipv6s[j].type = statistics->valueint;

			statistics = cJSON_GetObjectItem(arrayObj, "use");
			if(statistics == NULL){
				goto exit;
			}
			lanInfo->lanhost_ipv6s[j].use = statistics->valueint;
		}
		//statistics
		cur = cJSON_GetObjectItem(client, "statistics");
		if(cur == NULL){
			goto exit;
		}

		//rx rate_rt
		statistics = cJSON_GetObjectItem(cur, "rx rate_rt");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->downRate = statistics->valueint;

		//tx rate_rt
		statistics = cJSON_GetObjectItem(cur, "tx rate_rt");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->upRate = statistics->valueint;

		//rx bytes
		statistics = cJSON_GetObjectItem(cur, "rx bytes");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->rxBytes = statistics->valueint;

		//tx bytes
		statistics = cJSON_GetObjectItem(cur, "tx bytes");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->txBytes = statistics->valueint;

		//rx packets
		statistics = cJSON_GetObjectItem(cur, "rx packets");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->rx_packets = statistics->valueint;

		//tx packets
		statistics = cJSON_GetObjectItem(cur, "tx packets");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->tx_packets = statistics->valueint;

		//rx errors
		statistics = cJSON_GetObjectItem(cur, "rx errors");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->rx_errors = statistics->valueint;

		//rx discards
		statistics = cJSON_GetObjectItem(cur, "rx discards");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->rx_dropped = statistics->valueint;

		//onLineTime
		statistics = cJSON_GetObjectItem(client, "onLineTime");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->onLineTime = statistics->valueint;

		//firstConnect
		statistics = cJSON_GetObjectItem(client, "firstConnect");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->firstConnect = statistics->valueint;

		//disConnect
		statistics = cJSON_GetObjectItem(client, "disConnect");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->disConnect = statistics->valueint;

		//latestActiveTime
		statistics = cJSON_GetObjectItem(client, "latestActiveTime");
		if(statistics == NULL){
			goto exit;
		}
		snprintf(lanInfo->latestActiveTime, sizeof(lanInfo->latestActiveTime), statistics->valuestring);

		//latestInactiveTime
		statistics = cJSON_GetObjectItem(client, "latestInactiveTime");
		if(statistics == NULL){
			goto exit;
		}
		snprintf(lanInfo->latestInactiveTime, sizeof(lanInfo->latestInactiveTime), statistics->valuestring);

		//controlStatus
		statistics = cJSON_GetObjectItem(client, "controlStatus");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->controlStatus = statistics->valueint;

		//internetAccess
		statistics = cJSON_GetObjectItem(client, "internetAccess");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->internetAccess = statistics->valueint;

		//storageAccess
		statistics = cJSON_GetObjectItem(client, "storageAccess");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->storageAccess = statistics->valueint;

		//devType
		statistics = cJSON_GetObjectItem(client, "devType");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->devType = statistics->valueint;

		//connectionType
		statistics = cJSON_GetObjectItem(client, "connectionType");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->connectionType = statistics->valueint;

		//brand
		statistics = cJSON_GetObjectItem(client, "brand");
		if(statistics == NULL){
			goto exit;
		}
		snprintf(lanInfo->brand, sizeof(lanInfo->brand), statistics->valuestring);

		//model
		statistics = cJSON_GetObjectItem(client, "model");
		if(statistics == NULL){
			goto exit;
		}
		snprintf(lanInfo->model, sizeof(lanInfo->model), statistics->valuestring);

		//os
		statistics = cJSON_GetObjectItem(client, "os");
		if(statistics == NULL){
			goto exit;
		}
		snprintf(lanInfo->os, sizeof(lanInfo->os), statistics->valuestring);

		//ntpFlag
		statistics = cJSON_GetObjectItem(client, "ntpFlag");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->ntpFlag= statistics->valueint;

		//firstStatistic
		statistics = cJSON_GetObjectItem(client, "firstStatistic");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->firstStatistic = statistics->valueint;

		//offline
		statistics = cJSON_GetObjectItem(client, "offline");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->offline = statistics->valueint;

		//offline_expired_time
		statistics = cJSON_GetObjectItem(client, "offline_expired_time");
		if(statistics == NULL){
			goto exit;
		}
		lanInfo->offline_expired_time = statistics->valueint;
	}


	*ppLANNetInfoData = tmpInfo;
	*pCount = device_number;

	ret = RTK_SUCCESS;

exit:
	if(fp)
		fclose(fp);
	if(buf)
		free(buf);
	if(data)
		cJSON_Delete(data);
	if(ret==RTK_FAILED && tmpInfo)
		free(tmpInfo);

	return ret;

}
#endif

int rtk_get_pid_from_file(char *filename)
{
	char buff[100];
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp) {
        //fprintf(stderr, "Read pid file error!\n");
		return RTK_FAILED;
   	}
	if (NULL == fgets(buff, 100, fp)) {
		fclose(fp);
		return RTK_FAILED;
	}
	fclose(fp);

	return (atoi(buff));
}

int rtk_lan_get_dhcp_client_list(LAN_DHCP_CLIENT_INFO *pclient, int array_len, int *real_num)
{
	FILE *fp=NULL;
	int i=0;
	char *buf=NULL;
	int dhcp_client_num=0;
	int fd;

	char *ptr;

	//DHCP_OFFER_ADDR_INFO *pDhcpEntry = NULL;
	struct rtk_dhcpOfferedAddr pDhcpEntry;
	struct stat status;
	int pid;
	unsigned int fileSize=0;

	if(pclient==NULL || array_len<1 || real_num==NULL)
		return RTK_FAILED;

	memset(pclient, 0, sizeof(LAN_DHCP_CLIENT_INFO)*array_len);
	// siganl DHCP server to update lease file
	pid = rtk_get_pid_from_file("/var/run/udhcpd.pid");
	if(pid<=0)
		goto err;

	kill(pid, SIGUSR1);
	usleep(1000);

	// open before stat to prevent race
	fp = fopen("/var/udhcpd/udhcpd.leases", "r");
	if ( fp == NULL )
		goto err;

	if ( stat("/var/udhcpd/udhcpd.leases", &status) < 0 )
		goto err;

	fileSize=status.st_size;

	dhcp_client_num=fileSize / sizeof(pDhcpEntry);

	if(dhcp_client_num<1)
		goto err;

	buf = malloc(fileSize);
	if ( buf == NULL )
		goto err;

	fd = fileno(fp);
	rtk_set_lock(fd, F_WRLCK);

	if (0 == fread(buf, 1, fileSize, fp)) {
		printf("No bytes are read to the buffer!\n");
	}

	rtk_set_lock(fd, F_UNLCK);
	fclose(fp);
	fp = NULL;
	ptr=buf;

	for(i=0;i<dhcp_client_num && i<array_len; i++)
	{
		if ( fileSize < sizeof(pDhcpEntry) )
			goto err;

		pDhcpEntry = *((struct rtk_dhcpOfferedAddr *)ptr);
		ptr = ptr + sizeof(pDhcpEntry);
		fileSize = fileSize - sizeof(pDhcpEntry);

		if (pDhcpEntry.expires == 0)
			continue;
		//star: conflict ip addr will not be displayed on web
		if(pDhcpEntry.chaddr[0]==0&&pDhcpEntry.chaddr[1]==0&&pDhcpEntry.chaddr[2]==0&&pDhcpEntry.chaddr[3]==0&&pDhcpEntry.chaddr[4]==0&&pDhcpEntry.chaddr[5]==0)
			continue;

		pclient[i].ip=pDhcpEntry.yiaddr;
		memcpy(pclient[i].mac, pDhcpEntry.chaddr, MAC_ADDR_LEN);
		pclient[i].expires=ntohl(pDhcpEntry.expires);
		pDhcpEntry.hostName[sizeof(pDhcpEntry.hostName) - 1] = L'\0';   // coverity check: pDhcpEntry.hostName non-null-terminated string
		strlcpy((char *)pclient[i].hostname, (char *)pDhcpEntry.hostName, sizeof(pclient[i].hostname));
		//ISP_API_DEBUG("\nipAddr:%x  %s\nmacAddr:%02x:%02x:%02x:%02x:%02x:%02x  expires:%u\n",pclient[i].ip,inet_ntoa(pclient[i].ip),pclient[i].mac[0],pclient[i].mac[1],pclient[i].mac[2],pclient[i].mac[3],pclient[i].mac[4],pclient[i].mac[5],pclient[i].expires);
	}

	*real_num = i;
	if(buf)
		free(buf);

	return RTK_SUCCESS;

err:
	*real_num=i;
	if(buf)
		free(buf);
	if(fp)
		fclose(fp);

	return RTK_FAILED;
}

int rtk_get_dhcp_info_by_mac(char *macAddress, LAN_DHCP_CLIENT_INFO *info)
{
	int i, real_num = 0;
	// LAN_DHCP_CLIENT_INFO dhcpClient[128];
	LAN_DHCP_CLIENT_INFO *dhcpClient = (LAN_DHCP_CLIENT_INFO *)malloc(sizeof(LAN_DHCP_CLIENT_INFO) * 128);

	if (!dhcpClient) {
		printf("Fail to allocate buffer for dhcpClient, get dhcp info by mac fail. \n");
		return RTK_FAILED;
	}

	if(macAddress == NULL || info ==NULL)
	{
		free(dhcpClient);
		return RTK_FAILED;
	}

	memset(dhcpClient, 0, sizeof(LAN_DHCP_CLIENT_INFO) * 128);
	if(rtk_lan_get_dhcp_client_list(dhcpClient, 128, &real_num) != RTK_SUCCESS)
	{
		free(dhcpClient);
		return RTK_FAILED;
	}

	for(i = 0 ; i < real_num ; i++)
	{
		if(!memcmp(macAddress, dhcpClient[i].mac, 6))
		{
			info->expires = dhcpClient[i].expires;
			snprintf((char *)info->hostname, sizeof(info->hostname), "%s", dhcpClient[i].hostname);
			info->ip = dhcpClient[i].ip;
			free(dhcpClient);
			return RTK_SUCCESS;
		}
	}

	free(dhcpClient);
	return RTK_FAILED;
}

int rtk_lan_get_lan_dev_info(LAN_DEV_BASE_INFO *clientInfo, unsigned int array_size, unsigned int *real_num)
{
	int i = 0;
	lanHostInfo_t *pLanNetInfo=NULL;
	unsigned int idx = 0,  count;
	//char ssid[MAX_SSID_LEN] = {0}, wlan_ifname[64] = {0};
	//unsigned char rssi = 0;
	//rtk_Negotiation_rate_T negoia_rate;
	LAN_DHCP_CLIENT_INFO dhcpInfo;

	if (clientInfo == NULL || real_num == NULL || array_size < 1)
		return RTK_FAILED;

	*real_num = 0;

	memset(clientInfo, 0, sizeof(LAN_DEV_BASE_INFO)*array_size);

	if(get_lan_net_info(&pLanNetInfo, &count) != 0)
	{
		printf("(%s)%d  get lan net info faild!\n", __FUNCTION__, __LINE__);

		if(pLanNetInfo)
			free(pLanNetInfo);

		return RTK_FAILED;
	}

	if(count > 0)
	{
		for(i=0; (i<(int)count) && (idx<array_size); i++)
		{
			clientInfo[i].ip = pLanNetInfo[i].ip;
			memcpy(clientInfo[i].mac, pLanNetInfo[i].mac, MAC_ADDR_LEN);

			snprintf(clientInfo[i].dev_name, sizeof(clientInfo[i].dev_name), "%s", pLanNetInfo[i].devName);

			clientInfo[i].devType = pLanNetInfo[i].devType; /*CMCC [0-other, 1-phone, 2-pc, 3-pad, 4-stb, 5-ap]; ELSE[0-phone 1-pad 2-PC 3-STB 4-other  0xff-unknown] */
			clientInfo[i].conn_port = pLanNetInfo[i].phy_port;
			clientInfo[i].link_time = pLanNetInfo[i].onLineTime;

			if (pLanNetInfo[i].disConnect == 0)
				clientInfo[i].is_alive = 1;
			else
				clientInfo[i].is_alive = 0;

			snprintf(clientInfo[i].brand, sizeof(clientInfo[i].brand), "%s", pLanNetInfo[i].brand);
			snprintf(clientInfo[i].model, sizeof(clientInfo[i].model), "%s", pLanNetInfo[i].model);
			snprintf(clientInfo[i].os, sizeof(clientInfo[i].os), "%s", pLanNetInfo[i].os);
#ifdef RTK_ETHERNET_LANNETINFO
			snprintf(clientInfo[i].access_time, sizeof(clientInfo[i].access_time), "%s", pLanNetInfo[i].latestActiveTime);
#endif

			clientInfo[i].up_speed = pLanNetInfo[i].upRate;
			clientInfo[i].down_speed = pLanNetInfo[i].downRate;
			clientInfo[i].tx_bytes= pLanNetInfo[i].txBytes;
			clientInfo[i].rx_bytes = pLanNetInfo[i].rxBytes;
			clientInfo[i].tx_packets = pLanNetInfo[i].tx_packets;
			clientInfo[i].rx_packets = pLanNetInfo[i].rx_packets;
			clientInfo[i].rx_errors = pLanNetInfo[i].rx_errors;
			clientInfo[i].rx_dropped = pLanNetInfo[i].rx_dropped;

			clientInfo[i].internetAccess = pLanNetInfo[i].internetAccess;
			clientInfo[i].storageAccess = pLanNetInfo[i].storageAccess;

			snprintf(clientInfo[i].conn_ifname, sizeof(clientInfo[i].os), "%s", pLanNetInfo[i].ifName);

			if(pLanNetInfo[i].connectionType == 0)//0:Host_Ethernet, 1:Host_802_11
				clientInfo[i].conn_type = LAN_DEV_CONN_WIRED;
/*
			if(pLanNetInfo[i].connectionType == 1)
			{
				snprintf(wlan_ifname, sizeof(wlan_ifname), "%s", clientInfo[i].conn_ifname);

				if(rtk_wlan_get_rf_band(wlan_ifname, &band) == RTK_SUCCESS)
				{
					if(band == PHYBAND_2G)
						clientInfo[i].conn_type = LAN_DEV_CONN_WLAN_2G;
					else
						clientInfo[i].conn_type = LAN_DEV_CONN_WLAN_5G;
				}else{
					printf("(%s)%d rtk wlan get ssid failed!\n", __FUNCTION__, __LINE__);
				}

				memset(ssid, 0, sizeof(ssid));
	            if(rtk_wlan_get_ssid(clientInfo[i].conn_ifname, ssid) == RTK_SUCCESS)
					snprintf(clientInfo[i].ssid, sizeof(clientInfo[i].ssid), "%s", ssid);
				else
					printf("(%s)%d rtk wlan get ssid failed!\n", __FUNCTION__, __LINE__);

				memset(&negoia_rate, 0, sizeof(rtk_Negotiation_rate_T));

				if(rtk_wlan_get_Rssi_and_Negotiation_rate(wlan_ifname, clientInfo[i].mac, &negoia_rate, &rssi) == RTK_SUCCESS)
				{
					clientInfo[i].rssi = rssi;
					snprintf(clientInfo[i].rxrate, sizeof(clientInfo[i].rxrate), "%s", negoia_rate.rxRate);
					snprintf(clientInfo[i].txrate, sizeof(clientInfo[i].txrate), "%s", negoia_rate.txRate);
				}
			}
*/
			memset(&dhcpInfo, 0, sizeof(LAN_DHCP_CLIENT_INFO));

			if(rtk_get_dhcp_info_by_mac((char *)clientInfo[idx].mac, &dhcpInfo) == RTK_SUCCESS)
				clientInfo[idx].expires = dhcpInfo.expires;

			if(clientInfo[idx].dev_name[0]==0 || strcmp(clientInfo[idx].dev_name, "unknown") ==0)
			{
				snprintf(clientInfo[idx].dev_name, sizeof(clientInfo[idx].dev_name),  "%02X-%02X-%02X-%02X-%02X-%02X",\
							clientInfo[i].mac[0], clientInfo[i].mac[1], clientInfo[i].mac[2], clientInfo[i].mac[3], clientInfo[i].mac[4], clientInfo[i].mac[5]);
			}

			idx++;
		}

	}

	if(pLanNetInfo)
		free(pLanNetInfo);

	*real_num = idx;
/*
	dump_lan_net_info(clientInfo, *real_num);
*/
	return RTK_SUCCESS;
}

int rtk_lan_get_wired_client_info(LAN_DEV_BASE_INFO *pWiredClient, int array_len, int *real_num)
{
	int count, i, idx;

	// LAN_DEV_BASE_INFO clientInfo[MAX_STA_NUM];
	LAN_DEV_BASE_INFO *clientInfo = (LAN_DEV_BASE_INFO *)malloc(sizeof(LAN_DEV_BASE_INFO) * MAX_STA_NUM);
	if (!clientInfo) {
		printf("Fail to allocate buffer for clientInfo, get wired client info failed. \n");
		return RTK_FAILED;
	}

	if(pWiredClient == NULL || array_len <=0 || real_num == NULL)
	{
		free(clientInfo);
		return RTK_FAILED;
	}

	memset(clientInfo, 0, sizeof(LAN_DEV_BASE_INFO) * MAX_STA_NUM);

	if(rtk_lan_get_lan_dev_info(clientInfo, MAX_STA_NUM, (unsigned int *)&count) == RTK_FAILED)
	{
		printf("(%s)%d get lan net info failed!\n", __FUNCTION__, __LINE__);
		free(clientInfo);
		return RTK_FAILED;
	}

	idx = 0;

	if(count >0)
	{
		for(i=0; i<count && idx < array_len; i++)
		{
			if(clientInfo[i].conn_type == LAN_DEV_CONN_WIRED)
			{
				memcpy(&pWiredClient[idx], &clientInfo[i], sizeof(LAN_DEV_BASE_INFO));

				idx++;
			}
		}
	}
/*
	for(i=0; i < idx; i++)
	{
		ISP_API_DEBUG("pWiredClient[%d].mac : %02x:%02x:%02x:%02x:%02x:%02x\n", i, pWiredClient[i].mac[0], pWiredClient[i].mac[1], pWiredClient[i].mac[2], pWiredClient[i].mac[3], pWiredClient[i].mac[4], pWiredClient[i].mac[5]);
		ISP_API_DEBUG("pWiredClient[%d].ip  : %d\n", i, pWiredClient[i].ip);
	}
*/
	*real_num = idx;
	free(clientInfo);
	return RTK_SUCCESS;
}

int rtk_ethernet_get_l2_list(RTK_L2_TAB_INFO *pL2List, int array_len, int *real_num, int flag)//flag 0:l2_all, 1:l2_wired, 2:l2_wlan
{
	int i, idx;
	unsigned int count = 0;
	lanHostInfo_t *pLanNetInfo=NULL;

	if (pL2List == NULL || real_num == NULL || array_len < 1)
		return RTK_FAILED;

	memset(pL2List, 0, sizeof(RTK_L2_TAB_INFO)*array_len);

	if(get_lan_net_info(&pLanNetInfo, &count) != 0)
	{
		printf("(%s)%d  rtk get lan net info failed!\n", __FUNCTION__, __LINE__);
		printf("[%s](%d) flag is %d\n",__FUNCTION__,__LINE__,flag);
		if(pLanNetInfo)
			free(pLanNetInfo);

		return RTK_FAILED;
	}

	idx = 0;

	if(count > 0)
	{
		for(i=0; ((unsigned int)i<count) && (idx<array_len); i++)
		{
/*
			if( flag == 1 && pLanNetInfo[i].connectionType == 0) {//filter wlan
				continue;
			}else if (flag ==2, pLanNetInfo[i].connectionType == 1){//filter wired
				continue;
			}
*/
			memcpy(pL2List[idx].mac, pLanNetInfo[i].mac, MAC_ADDR_LEN);
			memcpy(pL2List[idx].ifName, pLanNetInfo[i].ifName, strlen(pLanNetInfo[i].ifName));
			pL2List[idx].port_num = pLanNetInfo[i].phy_port;

			idx++;
		}
	}


/*
	ISP_API_DEBUG("hw_l2 real_num=%d\n", idx);

	for(i=0; i < idx; i++)
	{
		ISP_API_DEBUG("\n\tpL2List[%d].port_num:%d\n\tpL2List[%d].mac:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",i,pL2List[i].port_num,i,pL2List[i].mac[0],pL2List[i].mac[1],pL2List[i].mac[2],pL2List[i].mac[3],pL2List[i].mac[4],pL2List[i].mac[5]);
	}
*/
	if(pLanNetInfo)
		free(pLanNetInfo);

	*real_num = idx;

	return RTK_SUCCESS;
}

#else
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

int rtk_string_to_hex(const char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii = 0;

	if (string == NULL || key == NULL)
		return 0;

	for (idx = 0; idx < len; idx += 2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx + 1];
		tmpBuf[2] = 0;

		if (!_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = strtoul(tmpBuf, NULL, 16);
	}

	return 1;
}

int rtk_change_mac_to_octe(char *str_mac, char *octe_mac)
{
	if(str_mac==NULL || octe_mac==NULL)
		return RTK_FAILED;

	int i, j;
	char tmp_str_mac[13]={0};

	for(i=0, j=0; i<17 && j<12; i++)
	{
		if((str_mac[i]!=':')  && (str_mac[i]!='-'))
			tmp_str_mac[j++]=str_mac[i];
	}
	tmp_str_mac[12]=0;

	if (strlen(tmp_str_mac)==12 && rtk_string_to_hex(tmp_str_mac, (unsigned char *)octe_mac, 12))
		return RTK_SUCCESS;

	return RTK_FAILED;
}

int rtk_lan_get_wired_client_info(LAN_DEV_BASE_INFO *pWiredClient, int array_len, int *real_num)
{
	if(pWiredClient==NULL || array_len<1 || real_num==NULL)
		return RTK_FAILED;

	int i, idx=0;
	char buffer[512]={0};
	struct in_addr ip_addr;
	char str_ip[16]={0}, str_mac[32]={0};
	char *token=NULL;
	char *savestr=NULL;

	FILE *fp=NULL;
	int fd;

	memset(pWiredClient, 0, sizeof(LAN_DEV_BASE_INFO)*array_len);

	if((fp = fopen(LAN_DEV_LINK_INFO,"r")) == NULL)
		return RTK_FAILED;

	fd = fileno(fp);
	rtk_set_lock(fd, F_WRLCK);

	while(fgets(buffer, sizeof(buffer), fp) && idx<array_len)
	{
		for(i=0;buffer[i]!='\0';i++)
		if(buffer[i]=='\n')
			buffer[i]='\0';

		token=strtok_r(buffer,",",&savestr);
		while(token != NULL)
		{
 			if(strncmp("ip=",token,strlen("ip="))==0)
			{
				memset(str_ip, 0, sizeof(str_ip));
				strlcpy(str_ip,(token+strlen("ip=")),sizeof(str_ip));
				inet_aton(str_ip, &ip_addr);
				pWiredClient[idx].ip=ip_addr.s_addr;
			}

			if(strncmp("mac=",token,strlen("mac="))==0)
			{
				memset(str_mac, 0, sizeof(str_mac));
				strlcpy(str_mac,(token+strlen("mac=")),sizeof(str_mac));
				rtk_change_mac_to_octe(str_mac, (char *)pWiredClient[idx].mac);
			}

			if(strncmp("dev_name=",token,strlen("dev_name="))==0)
			{
				strlcpy(pWiredClient[idx].dev_name,(token+strlen("dev_name=")),LAN_DEV_NAME_MAX_LEN);
			}

			if(strncmp("expires=",token,strlen("expires="))==0)
			{
				pWiredClient[idx].expires=atol(token+strlen("expires="));
			}

			if(strncmp("conn_type=",token,strlen("conn_type="))==0)
			{
				pWiredClient[idx].conn_type=atoi(token+strlen("conn_type="));
			}

			if(strncmp("conn_port=",token,strlen("conn_port="))==0)
			{
				pWiredClient[idx].conn_port=atoi(token+strlen("conn_port="));
			}

			if(strncmp("link_time=",token,strlen("link_time="))==0)
			{
				pWiredClient[idx].link_time=atol(token+strlen("link_time="));
			}

			if(strncmp("is_alive=",token,strlen("is_alive="))==0)
			{
				pWiredClient[idx].is_alive=atoi(token+strlen("is_alive="));
			}

			if(strncmp("up_speed=",token,strlen("up_speed="))==0)
			{
				pWiredClient[idx].up_speed=atol(token+strlen("up_speed="));
			}

			if(strncmp("down_speed=",token,strlen("down_speed="))==0)
			{
				pWiredClient[idx].down_speed=atol(token+strlen("down_speed="));
			}

			if(strncmp("rx_packets=",token,strlen("rx_packets="))==0)
			{
				pWiredClient[idx].rx_packets=atol(token+strlen("rx_packets="));
			}

			if(strncmp("tx_packets=",token,strlen("tx_packets="))==0)
			{
				pWiredClient[idx].tx_packets=atol(token+strlen("tx_packets="));
			}

			if(strncmp("rx_errors=",token,strlen("rx_errors="))==0)
			{
				pWiredClient[idx].rx_errors= atol(token+strlen("rx_errors="));
			}

			if(strncmp("rx_dropped=",token,strlen("rx_dropped="))==0)
			{
				pWiredClient[idx].rx_dropped= atol(token+strlen("rx_dropped="));
			}

			token=strtok_r(NULL,",",&savestr);
		}
		idx++;
	}

	rtk_set_lock(fd, F_UNLCK);
	fclose(fp);

	*real_num=idx;

	//rtk_dump_lan_client_info(pWiredClient, idx);
	return RTK_SUCCESS;
}

static int read_ethGetInfo_pid(const char *filename)
{
	FILE *fp;
	int pid = 0;

	if ((fp = fopen(filename, "r")) == NULL)
		return -1;
	if(fscanf(fp, "%d", &pid) != 1 || kill(pid, 0) != 0){
		pid = 0;
	}
	fclose(fp);

	return pid;
}

int rtk_ethernet_get_l2_list(RTK_L2_TAB_INFO *pL2List, int array_len, int *real_num, int flag)
{
	int idx=0,i,ret=RTK_FAILED,realNum=0;
	//for message
	ethReqInfoMsg_t *msgReqBuf=NULL;
	ethRespInfoMsg_t *msgRespBuf=NULL;
	rtk_l2TableEhtry_t *pL2table=NULL;
	int ethInfo_msgid;
	int req_length;
	int mtype;
	key_t key;
	//pid
	pid_t ethGetInfo_pid;
	pid_t ethl2list_pid;

	if(pL2List==NULL || array_len<1 || real_num==NULL)
		return RTK_FAILED;

	printf("[%s](%d) flag is %d\n",__FUNCTION__,__LINE__,flag);

	ethl2list_pid = (int)syscall(SYS_gettid);
	ethGetInfo_pid = read_ethGetInfo_pid(ETHGETINFO_RUNFILE);

	memset(pL2List, 0, sizeof(RTK_L2_TAB_INFO)*array_len);
	//msg init
	key = ftok("/bin/ethGetInfo", 0x01);
	ethInfo_msgid = msgget(key,0);
	if (ethInfo_msgid==RTK_FAILED)
	{
		printf("failed to create msgbox:errno\n");
		*real_num=realNum;
		return RTK_FAILED;
	}

	msgRespBuf=(ethRespInfoMsg_t *)malloc(sizeof(msgHeader_t));
	if(msgRespBuf==NULL){
		printf("malloc msgRespBuf failed!\n");
		ret=RTK_FAILED;
		goto exit;
	}
	memset(msgRespBuf,0x0,sizeof(msgHeader_t));//ethRespInfoMsg_t
	msgRespBuf->msg_hdr.mtype=ethGetInfo_pid;//lannetinfo_pid
	msgRespBuf->msg_hdr.request= ethl2list_pid;
	msgRespBuf->msg_hdr.cmd |= CMD_l2TABLE_INFO_REQ;
	msgRespBuf->msg_hdr.flag=GET_L2INFO_ALL;
resp_agein:
	if(msgsnd(ethInfo_msgid, (void*)msgRespBuf, (sizeof(msgHeader_t)-sizeof(long)), IPC_NOWAIT)==RTK_FAILED){
		printf("send  message error!\n");
		if( (errno==EAGAIN)||(errno == EINTR) )
		{
			printf("[%s %d]:send return message again!\n", __FUNCTION__, __LINE__);
			goto resp_agein;
		}else{
			printf("send  message failed!\n");
			goto exit;
		}
	}else
		printf("send  message successed, length:%d!\n", (sizeof(msgHeader_t)-sizeof(long)));

	//msg receive
	req_length=(MAXL2TBNUM*sizeof(rtk_l2TableEhtry_t)+sizeof(msgHeader_t));

	msgReqBuf=(ethReqInfoMsg_t *)malloc(req_length);
	if(msgReqBuf==NULL){
		printf("malloc msgReqBuf failed!\n");
		ret=RTK_FAILED;
		goto exit;
	}

	memset(msgReqBuf,0x0, req_length);

	mtype = ethl2list_pid;

rcv_agein:
	ret = msgrcv(ethInfo_msgid,(void *)msgReqBuf, req_length, mtype, IPC_NOWAIT);
	if(ret>0){
		realNum=msgReqBuf->msg_hdr.realnum;

		pL2table=(rtk_l2TableEhtry_t *)malloc(realNum*sizeof(rtk_l2TableEhtry_t));
		if(pL2table==NULL){
			printf("malloc pL2table failed!\n");
			ret=RTK_FAILED;
			goto exit;
		}

		memset(pL2table,0x0,sizeof(rtk_l2TableEhtry_t)*realNum);

		if(msgReqBuf->msg_hdr.cmd & CMD_l2TABLE_INFO_RESP){
			if(msgReqBuf->msg_hdr.flag==GET_L2INFO_ALL){
				memcpy(pL2table,msgReqBuf->payload,sizeof(rtk_l2TableEhtry_t)*realNum);
			}
		}
/*
		for(i=0;i<realNum;i++)
		{
			ISP_API_DEBUG("\n\tl2table[%d].port:%d\n\tl2table[%d].macaddr:%02X:%02X:%02X:%02X:%02X:%02X\n",i,pL2table[i].port,i,pL2table[i].macaddr[0],pL2table[i].macaddr[1],pL2table[i].macaddr[2],pL2table[i].macaddr[3],pL2table[i].macaddr[4],pL2table[i].macaddr[5]);
		}
*/
	}
	else if(ret==RTK_FAILED){
		if( (errno==EAGAIN)||(errno == EINTR)||(errno==ENOMSG) )
		{
                goto rcv_agein;
		}else{
			ret=RTK_FAILED;
			printf("receive  message failed!\n");
			goto exit;
		}
	}
	else{
		printf("[%s]%d Receive message failed!\n",__FUNCTION__,__LINE__);
		ret= RTK_FAILED;
		goto exit;
	}

	for(i=0;((i<array_len)&&(i<realNum));i++)
	{
		pL2List[i].port_num=pL2table[i].port;
		memcpy(pL2List[i].mac,pL2table[i].macaddr,MAC_ADDR_LEN);
		idx=i+1;
	}
/*
	//pl2list
	for(i=0;i<idx;i++)
	{
		ISP_API_DEBUG("\n\tpL2List[%d].port_num:%d\n\tpL2List[%d].mac:%.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n",i,pL2List[i].port_num,i,pL2List[i].mac[0],pL2List[i].mac[1],pL2List[i].mac[2],pL2List[i].mac[3],pL2List[i].mac[4],pL2List[i].mac[5]);
	}
*/
	*real_num=idx;
	ret =RTK_SUCCESS;

	exit:
		if(msgRespBuf){
			free(msgRespBuf);
			msgRespBuf=NULL;
		}

		if(msgReqBuf){
			free(msgReqBuf);
			msgReqBuf=NULL;
		}
		if(pL2table){
			free(pL2table);
			pL2table=NULL;
		}

	return ret;
}

#endif
char * map_fill_topo_response_vendor_data(char *ip_addr, char *device_name)
{
	char *buffer=NULL;
	int m = 0;
	// LAN_DEV_BASE_INFO clientInfo[MAX_STA_NUM*2];
	// LAN_DEV_BASE_INFO lan_sta[MAX_STA_NUM];
	int connect_num1 = 0, connect_num2 = 0;
	int connect_num = 0;
	cJSON *XData = NULL;
	LAN_DEV_BASE_INFO *clientInfo = (LAN_DEV_BASE_INFO *)malloc(sizeof(LAN_DEV_BASE_INFO) * (MAX_STA_NUM * 2));
	if (!clientInfo) {
		printf("Fail to allocate buffer for clientInfo, fill topo response vendor data failed \n");
		return NULL;
	}
	LAN_DEV_BASE_INFO *lan_sta = (LAN_DEV_BASE_INFO *)malloc(sizeof(LAN_DEV_BASE_INFO) * MAX_STA_NUM);
	if (!lan_sta) {
		printf("Fail to allocate buffer for lan_sta, fill topo response vendor data failed \n");
		free(clientInfo);
		return NULL;
	}

	memset(clientInfo, 0, sizeof(LAN_DEV_BASE_INFO) * MAX_STA_NUM * 2);
	memset(lan_sta, 0, sizeof(LAN_DEV_BASE_INFO) * MAX_STA_NUM);
	// memset(&clientInfo, 0, sizeof(LAN_DEV_BASE_INFO)*MAX_STA_NUM*2);

	// get lan client info
	if(rtk_lan_get_wired_client_info(lan_sta, MAX_STA_NUM, &connect_num2)==-1){
		printf("[%s %d] get lan client info error!\n",__FUNCTION__,__LINE__);
		free(clientInfo);
		free(lan_sta);
		return NULL;
    }

	connect_num = connect_num1+connect_num2;
	for (m=connect_num1; m<connect_num; m++) {
		memcpy(&clientInfo[m], &lan_sta[m-connect_num1], sizeof(LAN_DEV_BASE_INFO));
	}

	//form wifi and lan client info as cjson data
	if (connect_num) {
		cJSON *Devices, *Node;

		XData = cJSON_CreateObject();
		cJSON_AddStringToObject(XData, "DeviceName", device_name);
		cJSON_AddStringToObject(XData, "IpAddr", ip_addr);

		Devices = cJSON_CreateArray();
		if (Devices == NULL) {
			printf("[error][map_fill_topo_response_vendor_data] Unable to create Devices!!");
			if(XData) {
				cJSON_Delete(XData);
			}
			free(clientInfo);
			free(lan_sta);
			return NULL;
		}
		cJSON_AddItemToObject(XData, "Devices", Devices);
		for(m=0; m<connect_num; m++){
			char tmp_ip[16] = {0}, mac[18] = {0};
			Node = cJSON_CreateObject();
			cJSON_AddItemToArray(Devices, Node);

			//DevName
			cJSON_AddStringToObject(Node, "DevName", clientInfo[m].dev_name);
			//IPAddress
			snprintf(tmp_ip,sizeof(tmp_ip),"%s",inet_ntoa(*((struct in_addr *)&clientInfo[m].ip)));
			cJSON_AddStringToObject(Node, "IPAddress", tmp_ip);
			//MacAddress
			snprintf(mac, sizeof(mac), "%02X%02X%02X%02X%02X%02X",
				clientInfo[m].mac[0], clientInfo[m].mac[1], clientInfo[m].mac[2], 
				clientInfo[m].mac[3], clientInfo[m].mac[4], clientInfo[m].mac[5]);
			cJSON_AddStringToObject(Node, "MacAddress", mac);
			//ConnType
			cJSON_AddNumberToObject(Node, "ConnType", clientInfo[m].conn_type);
			//ConnIfname
			cJSON_AddStringToObject(Node, "ConnIfname", clientInfo[m].conn_ifname);
			//LinkTime
			cJSON_AddNumberToObject(Node, "ConnTime", clientInfo[m].link_time);
			//CurRxRate
			cJSON_AddNumberToObject(Node, "CurRxRate", clientInfo[m].down_speed);
			//CurTxRate
			cJSON_AddNumberToObject(Node, "CurTxRate", clientInfo[m].up_speed);
			//IsBackhaulBSS
			cJSON_AddNumberToObject(Node, "IsBackhaulBSS", 0);
		}

		buffer = cJSON_PrintUnformatted(XData);
	}

    if(XData) {
        cJSON_Delete(XData);
	}
	free(clientInfo);
	free(lan_sta);
	return buffer;
}

int map_process_topo_response_vendor_data(unsigned char *m, unsigned short m_nr)
{
	char  *pt             = NULL;
	char  *device_name    = NULL;
	char  *buffer         = NULL;
	char   file_name[256] = { 0 };
	cJSON *json           = NULL;
	int    processed_len  = 0;

	// ip_addr
	if (NULL == m || 0 == m_nr)
		return 1;
	pt = (char *)m;
	processed_len += strlen(pt) + 1;

	// device_name
	if (processed_len >= m_nr)
		return 1;
	pt += strlen(pt) + 1;
	processed_len += strlen(pt) + 1;
	device_name = strdup(pt);

	// map_version
	if (processed_len >= m_nr) {
		if (device_name)
			free(device_name);
		return 1;
	}
	pt += strlen(pt) + 1;
	processed_len += strlen(pt) + 1;

	// buffer
	if (processed_len >= m_nr) {
		if (device_name)
			free(device_name);
		return 1;
	}
	pt += strlen(pt) + 1;
	processed_len += strlen(pt) + 1;
	buffer = strdup(pt);

	if (buffer) {
		snprintf(file_name, sizeof(file_name), MAP_NETWORK_STA_INFO, device_name);
		json = cJSON_Parse(buffer);
		if (json != NULL) {
			map_write_sta_info_to_file(file_name, buffer);
		} else {
			(void)unlink(file_name);
		}
	}

	if (json)
		cJSON_Delete(json);
	if (device_name)
		free(device_name);
	if (buffer)
		free(buffer);

	return 0;
}

int map_write_sta_info_to_file(char *file_name, char *buffer)
{
	FILE *fp = fopen(file_name, "w");
	int fh=0;

	if (fp == NULL) {
		printf("Error opening %s!\n", file_name);
		return -1;
	} else {
		if((fh=fileno(fp))<0){
			fclose(fp);
			return -1;
		}
		flock(fh, LOCK_EX);

		fprintf(fp, "%s\n", buffer);

		flock(fh, LOCK_UN);
		fclose(fp);
	}

	return 0;
}
#endif
