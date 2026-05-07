#ifndef _RTK_EX_GPHY_H_
#define _RTK_EX_GPHY_H_

#define RTK_EXTGPHY_IOCTL_NAME			"extGphy"

/* commands - up to 31 */
#define RTK_EXTGPHY_NODE_MAGIC			'G'
#define RTK_EXTGPHY_NONE_CMD			(_IO(RTK_EXTGPHY_NODE_MAGIC, 0))
#define RTK_EXTGPHY_INFO_CMD			(_IOWR(RTK_EXTGPHY_NODE_MAGIC, 1, rtk_extgphy_cmd_info_t))
#define RTK_EXTGPHY_REG_GET_CMD			(_IOWR(RTK_EXTGPHY_NODE_MAGIC, 2, rtk_extgphy_cmd_reg_get_t))
#define RTK_EXTGPHY_REG_SET_CMD			(_IOWR(RTK_EXTGPHY_NODE_MAGIC, 3, rtk_extgphy_cmd_reg_set_t))
#define RTK_EXTGPHY_AUTO_NEGO_ABILITY_CMD	(_IOWR(RTK_EXTGPHY_NODE_MAGIC, 4, rtk_extgphy_cmd_phy_ability_t))

#define bool int
typedef enum {
	EXT_PHY_SPEED_10	= 0,	/* 10M bps */
	EXT_PHY_SPEED_100	= 1,	/* 100M bps */
	EXT_PHY_SPEED_500	= 2,	/* 500M bps */
	EXT_PHY_SPEED_1000	= 3,	/* 1G bps */
	EXT_PHY_SPEED_10000	= 4,	/* 10G bps */
	EXT_PHY_SPEED_10000LITE	= 5,	/* 10G Lite bps */
	EXT_PHY_SPEED_2000	= 6,	/* 2G bps */
	EXT_PHY_SPEED_2500	= 7,	/* 2.5G bps */
	EXT_PHY_SPEED_2500LITE	= 8,	/* 2.5G Lite bps */
	EXT_PHY_SPEED_5000	= 9,	/* 5G bps */
	EXT_PHY_SPEED_5000LITE	= 10,	/* 5G Lite bps */
	EXT_PHY_SPEED_MAX
} ext_phy_speed_mode_t;

typedef enum {
	EXT_PHY_DUPLEX_HALF      = 0,
	EXT_PHY_DUPLEX_FULL      = 1,
	EXT_PHY_DUPLEX_INVALID   = 0xFF,
} ext_phy_duplex_t;

typedef struct {
	bool			link_up;
	ext_phy_duplex_t	duplex;
	ext_phy_speed_mode_t	speed;
} ext_phy_link_status_t;

typedef struct rtk_extgphy_info_s {
	unsigned char		phy_addr;
	unsigned char		st_code;
	ext_phy_link_status_t	link_status;
	unsigned int		oui;
	unsigned int 		model;
} rtk_extgphy_info_t;

typedef struct rtk_extgphy_cmd_info_s {
	int ret;
	unsigned int port;
	rtk_extgphy_info_t reg;
} rtk_extgphy_cmd_info_t;

typedef struct rtk_extgphy_reg_s {
	unsigned int page;
	unsigned int reg;
	unsigned int data;
} rtk_extgphy_reg_t;

typedef struct rtk_extgphy_cmd_reg_get_s {
	int ret;
	unsigned int port;
	rtk_extgphy_reg_t reg;
} rtk_extgphy_cmd_reg_get_t;

typedef struct rtk_extgphy_cmd_reg_set_s {
	int ret;
	unsigned int port;
	rtk_extgphy_reg_t reg;
} rtk_extgphy_cmd_reg_set_t;

typedef struct rtk_extgphy_ability_s
{
	unsigned int Half_10:1;
	unsigned int Full_10:1;
	unsigned int Half_100:1;
	unsigned int Full_100:1;
	unsigned int Half_1000:1;
	unsigned int Full_1000:1;
	unsigned int Full_2500:1;
	unsigned int Full_10000:1;
	unsigned int FC:1;
	unsigned int AsyFC:1;
} rtk_extgphy_ability_t;

typedef struct rtk_extgphy_cmd_phy_ability_s {
	int ret;
	unsigned int port;
	rtk_extgphy_ability_t ability;
} rtk_extgphy_cmd_phy_ability_t;

#define PRINTHEADER	"extGphy"
#ifdef __KERNEL__
/*******************************
 * Only for Kernel Space       *
 *******************************/
#define RTKEXTGPHY_PRINT(fmt,args...)	printk(KERN_INFO fmt"\n", ##args)
#define RTKEXTGPHY_MSG(fmt,args...)	printk(KERN_INFO PRINTHEADER": "fmt"\n", ##args)
#define RTKEXTGPHY_INFO(fmt,args...)	printk(KERN_INFO "\033[1;33;46m"PRINTHEADER": "fmt"\033[m""\n", ##args)
#define RTKEXTGPHY_ERROR(fmt,args...)	printk(KERN_INFO "\033[1;33;41m"PRINTHEADER": "fmt" [%s(line %d)]""\033[m""\n", ##args, __FUNCTION__, __LINE__)

//#define RTKEXTGPHY_PHY_DEBUG
#ifdef RTKEXTGPHY_PHY_DEBUG
#define RTKEXTGPHY_DEBUG		RTKEXTGPHY_INFO
#else
#define RTKEXTGPHY_DEBUG(fmt,args...)
#endif

#define BOOLEAN		bool
#define SUCCESS		TRUE
#define FAILURE		FALSE

#else
/*******************************
 * Only for User Space         *
 *******************************/
#define RTKEXTGPHY_PRINT(fmt,args...)	printf(fmt"\n", ##args)
#define RTKEXTGPHY_MSG(fmt,args...)	printf(PRINTHEADER": "fmt"\n", ##args)
#define RTKEXTGPHY_INFO(fmt,args...)	printf("\033[1;33;46m"PRINTHEADER": "fmt"\033[m""\n", ##args)
#define RTKEXTGPHY_ERROR(fmt,args...)	printf("\033[1;33;41m"PRINTHEADER": "fmt" [%s(line %d)]""\033[m""\n", ##args, __FUNCTION__, __LINE__)

//#define RTKEXTGPHY_PHY_DEBUG
#ifdef RTKEXTGPHY_PHY_DEBUG
#define RTKEXTGPHY_DEBUG		RTKEXTGPHY_INFO
#else
#define RTKEXTGPHY_DEBUG(fmt,args...)
#endif

#endif
#endif

