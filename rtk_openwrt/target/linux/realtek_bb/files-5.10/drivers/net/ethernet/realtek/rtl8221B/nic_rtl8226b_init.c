
/* SW_SDK: include files */
#include <common/rt_type.h>
#include <common/debug/rt_log.h>
#include <osal/time.h>
//#include <osal/phy_osal.h>
#include "nic_rtl8226b.h"

static const MMD_REG Rtl8226b_n0_ramcode[] =
{
    { 31, 0xa436, 0xA016, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa436, 0xA012, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa436, 0xA014, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8010, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x801a, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x803f, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8045, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8067, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x806d, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8079, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x807e, },
    { 31, 0xa438, 0xd093, },
    { 31, 0xa438, 0xd1c4, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x135c, },
    { 31, 0xa438, 0xd704, },
    { 31, 0xa438, 0x5fbc, },
    { 31, 0xa438, 0xd504, },
    { 31, 0xa438, 0xc9f1, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x0fc9, },
    { 31, 0xa438, 0xbb50, },
    { 31, 0xa438, 0xd505, },
    { 31, 0xa438, 0xa202, },
    { 31, 0xa438, 0xd504, },
    { 31, 0xa438, 0x8c0f, },
    { 31, 0xa438, 0xd500, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x1519, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x135c, },
    { 31, 0xa438, 0xd75e, },
    { 31, 0xa438, 0x5fae, },
    { 31, 0xa438, 0x9b50, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x135c, },
    { 31, 0xa438, 0xd75e, },
    { 31, 0xa438, 0x7fae, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x135c, },
    { 31, 0xa438, 0xd707, },
    { 31, 0xa438, 0x40a7, },
    { 31, 0xa438, 0xd719, },
    { 31, 0xa438, 0x4071, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x1557, },
    { 31, 0xa438, 0xd719, },
    { 31, 0xa438, 0x2f70, },
    { 31, 0xa438, 0x803b, },
    { 31, 0xa438, 0x2f73, },
    { 31, 0xa438, 0x156a, },
    { 31, 0xa438, 0x5e70, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x155d, },
    { 31, 0xa438, 0xd505, },
    { 31, 0xa438, 0xa202, },
    { 31, 0xa438, 0xd500, },
    { 31, 0xa438, 0xffed, },
    { 31, 0xa438, 0xd709, },
    { 31, 0xa438, 0x4054, },
    { 31, 0xa438, 0xa788, },
    { 31, 0xa438, 0xd70b, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x172a, },
    { 31, 0xa438, 0xc0c1, },
    { 31, 0xa438, 0xc0c0, },
    { 31, 0xa438, 0xd05a, },
    { 31, 0xa438, 0xd1ba, },
    { 31, 0xa438, 0xd701, },
    { 31, 0xa438, 0x2529, },
    { 31, 0xa438, 0x022a, },
    { 31, 0xa438, 0xd0a7, },
    { 31, 0xa438, 0xd1b9, },
    { 31, 0xa438, 0xa208, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x080e, },
    { 31, 0xa438, 0xd701, },
    { 31, 0xa438, 0x408b, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x0a65, },
    { 31, 0xa438, 0xf003, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x0a6b, },
    { 31, 0xa438, 0xd701, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x0920, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x0915, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x0909, },
    { 31, 0xa438, 0x228f, },
    { 31, 0xa438, 0x804e, },
    { 31, 0xa438, 0x9801, },
    { 31, 0xa438, 0xd71e, },
    { 31, 0xa438, 0x5d61, },
    { 31, 0xa438, 0xd701, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x022a, },
    { 31, 0xa438, 0x2005, },
    { 31, 0xa438, 0x091a, },
    { 31, 0xa438, 0x3bd9, },
    { 31, 0xa438, 0x0919, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x0916, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x14c5, },
    { 31, 0xa438, 0xd703, },
    { 31, 0xa438, 0x3181, },
    { 31, 0xa438, 0x8077, },
    { 31, 0xa438, 0x60ad, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x135c, },
    { 31, 0xa438, 0xd703, },
    { 31, 0xa438, 0x5fba, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x0cc7, },
    { 31, 0xa438, 0xd096, },
    { 31, 0xa438, 0xd1a9, },
    { 31, 0xa438, 0xd503, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x0c94, },
    { 31, 0xa438, 0xa802, },
    { 31, 0xa438, 0xa301, },
    { 31, 0xa438, 0xa801, },
    { 31, 0xa438, 0xc004, },
    { 31, 0xa438, 0xd710, },
    { 31, 0xa438, 0x4000, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x1e79, },
    { 31, 0xa436, 0xA026, },
    { 31, 0xa438, 0x1e78, },
    { 31, 0xa436, 0xA024, },
    { 31, 0xa438, 0x0c93, },
    { 31, 0xa436, 0xA022, },
    { 31, 0xa438, 0x0cc5, },
    { 31, 0xa436, 0xA020, },
    { 31, 0xa438, 0x0915, },
    { 31, 0xa436, 0xA006, },
    { 31, 0xa438, 0x020a, },
    { 31, 0xa436, 0xA004, },
    { 31, 0xa438, 0x1726, },
    { 31, 0xa436, 0xA002, },
    { 31, 0xa438, 0x1542, },
    { 31, 0xa436, 0xA000, },
    { 31, 0xa438, 0x0fc7, },
    { 31, 0xa436, 0xA008, },
    { 31, 0xa438, 0xff00, },
};


static const MMD_REG Rtl8226b_n1_ramcode[] =
{
    { 31, 0xa436, 0xA016, },
    { 31, 0xa438, 0x0010, },
    { 31, 0xa436, 0xA012, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa436, 0xA014, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8010, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x801d, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x802c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x802c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x802c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x802c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x802c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x802c, },
    { 31, 0xa438, 0xd700, },
    { 31, 0xa438, 0x6090, },
    { 31, 0xa438, 0x60d1, },
    { 31, 0xa438, 0xc95c, },
    { 31, 0xa438, 0xf007, },
    { 31, 0xa438, 0x60b1, },
    { 31, 0xa438, 0xc95a, },
    { 31, 0xa438, 0xf004, },
    { 31, 0xa438, 0xc956, },
    { 31, 0xa438, 0xf002, },
    { 31, 0xa438, 0xc94e, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x00cd, },
    { 31, 0xa438, 0xd700, },
    { 31, 0xa438, 0x6090, },
    { 31, 0xa438, 0x60d1, },
    { 31, 0xa438, 0xc95c, },
    { 31, 0xa438, 0xf007, },
    { 31, 0xa438, 0x60b1, },
    { 31, 0xa438, 0xc95a, },
    { 31, 0xa438, 0xf004, },
    { 31, 0xa438, 0xc956, },
    { 31, 0xa438, 0xf002, },
    { 31, 0xa438, 0xc94e, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x022a, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x0132, },
    { 31, 0xa436, 0xA08E, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA08C, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA08A, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA088, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA086, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA084, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA082, },
    { 31, 0xa438, 0x012f, },
    { 31, 0xa436, 0xA080, },
    { 31, 0xa438, 0x00cc, },
    { 31, 0xa436, 0xA090, },
    { 31, 0xa438, 0x0103, },
};


static const MMD_REG Rtl8226b_n2_ramcode[] =
{
    { 31, 0xa436, 0xA016, },
    { 31, 0xa438, 0x0020, },
    { 31, 0xa436, 0xA012, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa436, 0xA014, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8010, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8020, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x802a, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x8035, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x803c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x803c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x803c, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x803c, },
    { 31, 0xa438, 0xd107, },
    { 31, 0xa438, 0xd042, },
    { 31, 0xa438, 0xa404, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x09df, },
    { 31, 0xa438, 0xd700, },
    { 31, 0xa438, 0x5fb4, },
    { 31, 0xa438, 0x8280, },
    { 31, 0xa438, 0xd700, },
    { 31, 0xa438, 0x6065, },
    { 31, 0xa438, 0xd125, },
    { 31, 0xa438, 0xf002, },
    { 31, 0xa438, 0xd12b, },
    { 31, 0xa438, 0xd040, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x077f, },
    { 31, 0xa438, 0x0cf0, },
    { 31, 0xa438, 0x0c50, },
    { 31, 0xa438, 0xd104, },
    { 31, 0xa438, 0xd040, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x0aa8, },
    { 31, 0xa438, 0xd700, },
    { 31, 0xa438, 0x5fb4, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x0a2e, },
    { 31, 0xa438, 0xcb9b, },
    { 31, 0xa438, 0xd110, },
    { 31, 0xa438, 0xd040, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x0b7b, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x09df, },
    { 31, 0xa438, 0xd700, },
    { 31, 0xa438, 0x5fb4, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x081b, },
    { 31, 0xa438, 0x1000, },
    { 31, 0xa438, 0x09df, },
    { 31, 0xa438, 0xd704, },
    { 31, 0xa438, 0x7fb8, },
    { 31, 0xa438, 0xa718, },
    { 31, 0xa438, 0x1800, },
    { 31, 0xa438, 0x074e, },
    { 31, 0xa436, 0xA10E, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA10C, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA10A, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA108, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xA106, },
    { 31, 0xa438, 0x074d, },
    { 31, 0xa436, 0xA104, },
    { 31, 0xa438, 0x0818, },
    { 31, 0xa436, 0xA102, },
    { 31, 0xa438, 0x0a2c, },
    { 31, 0xa436, 0xA100, },
    { 31, 0xa438, 0x077e, },
    { 31, 0xa436, 0xA110, },
    { 31, 0xa438, 0x000f, },

};

static const MMD_REG Rtl8226b_uc2_ramcode[] =
{
    { 31, 0xa436, 0xb87c, },
    { 31, 0xa438, 0x8625, },
    { 31, 0xa436, 0xb87e, },
    { 31, 0xa438, 0xaf86, },
    { 31, 0xa438, 0x3daf, },
    { 31, 0xa438, 0x8689, },
    { 31, 0xa438, 0xaf88, },
    { 31, 0xa438, 0x69af, },
    { 31, 0xa438, 0x8887, },
    { 31, 0xa438, 0xaf88, },
    { 31, 0xa438, 0x9caf, },
    { 31, 0xa438, 0x889c, },
    { 31, 0xa438, 0xaf88, },
    { 31, 0xa438, 0x9caf, },
    { 31, 0xa438, 0x889c, },
    { 31, 0xa438, 0xbf86, },
    { 31, 0xa438, 0x49d7, },
    { 31, 0xa438, 0x0040, },
    { 31, 0xa438, 0x0277, },
    { 31, 0xa438, 0x7daf, },
    { 31, 0xa438, 0x2727, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7205, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7208, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x71f3, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x71f6, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7229, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x722c, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7217, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x721a, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x721d, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7211, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7220, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7214, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x722f, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7223, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7232, },
    { 31, 0xa438, 0x0000, },
    { 31, 0xa438, 0x7226, },
    { 31, 0xa438, 0xf8f9, },
    { 31, 0xa438, 0xfae0, },
    { 31, 0xa438, 0x85b3, },
    { 31, 0xa438, 0x3802, },
    { 31, 0xa438, 0xad27, },
    { 31, 0xa438, 0x02ae, },
    { 31, 0xa438, 0x03af, },
    { 31, 0xa438, 0x8830, },
    { 31, 0xa438, 0x1f66, },
    { 31, 0xa438, 0xef65, },
    { 31, 0xa438, 0xbfc2, },
    { 31, 0xa438, 0x1f1a, },
    { 31, 0xa438, 0x96f7, },
    { 31, 0xa438, 0x05ee, },
    { 31, 0xa438, 0xffd2, },
    { 31, 0xa438, 0x00da, },
    { 31, 0xa438, 0xf605, },
    { 31, 0xa438, 0xbfc2, },
    { 31, 0xa438, 0x2f1a, },
    { 31, 0xa438, 0x96f7, },
    { 31, 0xa438, 0x05ee, },
    { 31, 0xa438, 0xffd2, },
    { 31, 0xa438, 0x00db, },
    { 31, 0xa438, 0xf605, },
    { 31, 0xa438, 0xef02, },
    { 31, 0xa438, 0x1f11, },
    { 31, 0xa438, 0x0d42, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x4202, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xef02, },
    { 31, 0xa438, 0x1b03, },
    { 31, 0xa438, 0x1f11, },
    { 31, 0xa438, 0x0d42, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x4502, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xef02, },
    { 31, 0xa438, 0x1a03, },
    { 31, 0xa438, 0x1f11, },
    { 31, 0xa438, 0x0d42, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x4802, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xbfc2, },
    { 31, 0xa438, 0x3f1a, },
    { 31, 0xa438, 0x96f7, },
    { 31, 0xa438, 0x05ee, },
    { 31, 0xa438, 0xffd2, },
    { 31, 0xa438, 0x00da, },
    { 31, 0xa438, 0xf605, },
    { 31, 0xa438, 0xbfc2, },
    { 31, 0xa438, 0x4f1a, },
    { 31, 0xa438, 0x96f7, },
    { 31, 0xa438, 0x05ee, },
    { 31, 0xa438, 0xffd2, },
    { 31, 0xa438, 0x00db, },
    { 31, 0xa438, 0xf605, },
    { 31, 0xa438, 0xef02, },
    { 31, 0xa438, 0x1f11, },
    { 31, 0xa438, 0x0d42, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x4b02, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xef02, },
    { 31, 0xa438, 0x1b03, },
    { 31, 0xa438, 0x1f11, },
    { 31, 0xa438, 0x0d42, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x4e02, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xef02, },
    { 31, 0xa438, 0x1a03, },
    { 31, 0xa438, 0x1f11, },
    { 31, 0xa438, 0x0d42, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x5102, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xef56, },
    { 31, 0xa438, 0xd020, },
    { 31, 0xa438, 0x1f11, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x5402, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x5702, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x5a02, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0xa0ef, },
    { 31, 0xa438, 0x0348, },
    { 31, 0xa438, 0x0a28, },
    { 31, 0xa438, 0x05ef, },
    { 31, 0xa438, 0x201b, },
    { 31, 0xa438, 0x01ad, },
    { 31, 0xa438, 0x2735, },
    { 31, 0xa438, 0x1f44, },
    { 31, 0xa438, 0xe085, },
    { 31, 0xa438, 0x88e1, },
    { 31, 0xa438, 0x8589, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x5d02, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xe085, },
    { 31, 0xa438, 0x8ee1, },
    { 31, 0xa438, 0x858f, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x6002, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xe085, },
    { 31, 0xa438, 0x94e1, },
    { 31, 0xa438, 0x8595, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x6302, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xe085, },
    { 31, 0xa438, 0x9ae1, },
    { 31, 0xa438, 0x859b, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x6602, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xaf88, },
    { 31, 0xa438, 0x3cbf, },
    { 31, 0xa438, 0x883f, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x9cad, },
    { 31, 0xa438, 0x2835, },
    { 31, 0xa438, 0x1f44, },
    { 31, 0xa438, 0xe08f, },
    { 31, 0xa438, 0xf8e1, },
    { 31, 0xa438, 0x8ff9, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x5d02, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xe08f, },
    { 31, 0xa438, 0xfae1, },
    { 31, 0xa438, 0x8ffb, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x6002, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xe08f, },
    { 31, 0xa438, 0xfce1, },
    { 31, 0xa438, 0x8ffd, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x6302, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xe08f, },
    { 31, 0xa438, 0xfee1, },
    { 31, 0xa438, 0x8fff, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x6602, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xaf88, },
    { 31, 0xa438, 0x3ce1, },
    { 31, 0xa438, 0x85a1, },
    { 31, 0xa438, 0x1b21, },
    { 31, 0xa438, 0xad37, },
    { 31, 0xa438, 0x341f, },
    { 31, 0xa438, 0x44e0, },
    { 31, 0xa438, 0x858a, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x8bbf, },
    { 31, 0xa438, 0x885d, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7de0, },
    { 31, 0xa438, 0x8590, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x91bf, },
    { 31, 0xa438, 0x8860, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7de0, },
    { 31, 0xa438, 0x8596, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x97bf, },
    { 31, 0xa438, 0x8863, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7de0, },
    { 31, 0xa438, 0x859c, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x9dbf, },
    { 31, 0xa438, 0x8866, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7dae, },
    { 31, 0xa438, 0x401f, },
    { 31, 0xa438, 0x44e0, },
    { 31, 0xa438, 0x858c, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x8dbf, },
    { 31, 0xa438, 0x885d, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7de0, },
    { 31, 0xa438, 0x8592, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x93bf, },
    { 31, 0xa438, 0x8860, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7de0, },
    { 31, 0xa438, 0x8598, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x99bf, },
    { 31, 0xa438, 0x8863, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7de0, },
    { 31, 0xa438, 0x859e, },
    { 31, 0xa438, 0xe185, },
    { 31, 0xa438, 0x9fbf, },
    { 31, 0xa438, 0x8866, },
    { 31, 0xa438, 0x026e, },
    { 31, 0xa438, 0x7dae, },
    { 31, 0xa438, 0x0ce1, },
    { 31, 0xa438, 0x85b3, },
    { 31, 0xa438, 0x3904, },
    { 31, 0xa438, 0xac2f, },
    { 31, 0xa438, 0x04ee, },
    { 31, 0xa438, 0x85b3, },
    { 31, 0xa438, 0x00af, },
    { 31, 0xa438, 0x39d9, },
    { 31, 0xa438, 0x22ac, },
    { 31, 0xa438, 0xeaf0, },
    { 31, 0xa438, 0xacf6, },
    { 31, 0xa438, 0xf0ac, },
    { 31, 0xa438, 0xfaf0, },
    { 31, 0xa438, 0xacf8, },
    { 31, 0xa438, 0xf0ac, },
    { 31, 0xa438, 0xfcf0, },
    { 31, 0xa438, 0xad00, },
    { 31, 0xa438, 0xf0ac, },
    { 31, 0xa438, 0xfef0, },
    { 31, 0xa438, 0xacf0, },
    { 31, 0xa438, 0xf0ac, },
    { 31, 0xa438, 0xf4f0, },
    { 31, 0xa438, 0xacf2, },
    { 31, 0xa438, 0xf0ac, },
    { 31, 0xa438, 0xb0f0, },
    { 31, 0xa438, 0xacae, },
    { 31, 0xa438, 0xf0ac, },
    { 31, 0xa438, 0xacf0, },
    { 31, 0xa438, 0xacaa, },
    { 31, 0xa438, 0xa100, },
    { 31, 0xa438, 0x0ce1, },
    { 31, 0xa438, 0x8ff7, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x8402, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xaf26, },
    { 31, 0xa438, 0xe9e1, },
    { 31, 0xa438, 0x8ff6, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x8402, },
    { 31, 0xa438, 0x6e7d, },
    { 31, 0xa438, 0xaf26, },
    { 31, 0xa438, 0xf520, },
    { 31, 0xa438, 0xac86, },
    { 31, 0xa438, 0xbf88, },
    { 31, 0xa438, 0x3f02, },
    { 31, 0xa438, 0x6e9c, },
    { 31, 0xa438, 0xad28, },
    { 31, 0xa438, 0x03af, },
    { 31, 0xa438, 0x3324, },
    { 31, 0xa438, 0xad38, },
    { 31, 0xa438, 0x03af, },
    { 31, 0xa438, 0x32e6, },
    { 31, 0xa438, 0xaf32, },
    { 31, 0xa438, 0xfb00, },
    { 31, 0xa436, 0xb87c, },
    { 31, 0xa438, 0x8ff6, },
    { 31, 0xa436, 0xb87e, },
    { 31, 0xa438, 0x0705, },
    { 31, 0xa436, 0xb87c, },
    { 31, 0xa438, 0x8ff8, },
    { 31, 0xa436, 0xb87e, },
    { 31, 0xa438, 0x19cc, },
    { 31, 0xa436, 0xb87c, },
    { 31, 0xa438, 0x8ffa, },
    { 31, 0xa436, 0xb87e, },
    { 31, 0xa438, 0x28e3, },
    { 31, 0xa436, 0xb87c, },
    { 31, 0xa438, 0x8ffc, },
    { 31, 0xa436, 0xb87e, },
    { 31, 0xa438, 0x1047, },
    { 31, 0xa436, 0xb87c, },
    { 31, 0xa438, 0x8ffe, },
    { 31, 0xa436, 0xb87e, },
    { 31, 0xa438, 0x0a45, },
    { 31, 0xa436, 0xb85e, },
    { 31, 0xa438, 0x271E, },
    { 31, 0xa436, 0xb860, },
    { 31, 0xa438, 0x3846, },
    { 31, 0xa436, 0xb862, },
    { 31, 0xa438, 0x26E6, },
    { 31, 0xa436, 0xb864, },
    { 31, 0xa438, 0x32E3, },
    { 31, 0xa436, 0xb886, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb888, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb88a, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb88c, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb838, },
    { 31, 0xa438, 0x000f, },

};

static const MMD_REG Rtl8226b_uc_ramcode[] =
{
    { 31, 0xa436, 0x846e, },
    { 31, 0xa438, 0xaf84, },
    { 31, 0xa438, 0x86af, },
    { 31, 0xa438, 0x8690, },
    { 31, 0xa438, 0xaf86, },
    { 31, 0xa438, 0xa4af, },
    { 31, 0xa438, 0x86a4, },
    { 31, 0xa438, 0xaf86, },
    { 31, 0xa438, 0xa4af, },
    { 31, 0xa438, 0x86a4, },
    { 31, 0xa438, 0xaf86, },
    { 31, 0xa438, 0xa4af, },
    { 31, 0xa438, 0x86a4, },
    { 31, 0xa438, 0xee82, },
    { 31, 0xa438, 0x5f00, },
    { 31, 0xa438, 0x0284, },
    { 31, 0xa438, 0x90af, },
    { 31, 0xa438, 0x0441, },
    { 31, 0xa438, 0xf8e0, },
    { 31, 0xa438, 0x8ff3, },
    { 31, 0xa438, 0xa000, },
    { 31, 0xa438, 0x0502, },
    { 31, 0xa438, 0x84a4, },
    { 31, 0xa438, 0xae06, },
    { 31, 0xa438, 0xa001, },
    { 31, 0xa438, 0x0302, },
    { 31, 0xa438, 0x84c8, },
    { 31, 0xa438, 0xfc04, },
    { 31, 0xa438, 0xf8f9, },
    { 31, 0xa438, 0xef59, },
    { 31, 0xa438, 0xe080, },
    { 31, 0xa438, 0x15ad, },
    { 31, 0xa438, 0x2702, },
    { 31, 0xa438, 0xae03, },
    { 31, 0xa438, 0xaf84, },
    { 31, 0xa438, 0xc3bf, },
    { 31, 0xa438, 0x53ca, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8ad, },
    { 31, 0xa438, 0x2807, },
    { 31, 0xa438, 0x0285, },
    { 31, 0xa438, 0x2cee, },
    { 31, 0xa438, 0x8ff3, },
    { 31, 0xa438, 0x01ef, },
    { 31, 0xa438, 0x95fd, },
    { 31, 0xa438, 0xfc04, },
    { 31, 0xa438, 0xf8f9, },
    { 31, 0xa438, 0xfaef, },
    { 31, 0xa438, 0x69bf, },
    { 31, 0xa438, 0x53ca, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8ac, },
    { 31, 0xa438, 0x2822, },
    { 31, 0xa438, 0xd480, },
    { 31, 0xa438, 0x00bf, },
    { 31, 0xa438, 0x8684, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9bf, },
    { 31, 0xa438, 0x8687, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9bf, },
    { 31, 0xa438, 0x868a, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9bf, },
    { 31, 0xa438, 0x868d, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9ee, },
    { 31, 0xa438, 0x8ff3, },
    { 31, 0xa438, 0x00af, },
    { 31, 0xa438, 0x8526, },
    { 31, 0xa438, 0xe08f, },
    { 31, 0xa438, 0xf4e1, },
    { 31, 0xa438, 0x8ff5, },
    { 31, 0xa438, 0xe28f, },
    { 31, 0xa438, 0xf6e3, },
    { 31, 0xa438, 0x8ff7, },
    { 31, 0xa438, 0x1b45, },
    { 31, 0xa438, 0xac27, },
    { 31, 0xa438, 0x0eee, },
    { 31, 0xa438, 0x8ff4, },
    { 31, 0xa438, 0x00ee, },
    { 31, 0xa438, 0x8ff5, },
    { 31, 0xa438, 0x0002, },
    { 31, 0xa438, 0x852c, },
    { 31, 0xa438, 0xaf85, },
    { 31, 0xa438, 0x26e0, },
    { 31, 0xa438, 0x8ff4, },
    { 31, 0xa438, 0xe18f, },
    { 31, 0xa438, 0xf52c, },
    { 31, 0xa438, 0x0001, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xf4e5, },
    { 31, 0xa438, 0x8ff5, },
    { 31, 0xa438, 0xef96, },
    { 31, 0xa438, 0xfefd, },
    { 31, 0xa438, 0xfc04, },
    { 31, 0xa438, 0xf8f9, },
    { 31, 0xa438, 0xef59, },
    { 31, 0xa438, 0xbf53, },
    { 31, 0xa438, 0x2202, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xa18b, },
    { 31, 0xa438, 0x02ae, },
    { 31, 0xa438, 0x03af, },
    { 31, 0xa438, 0x85da, },
    { 31, 0xa438, 0xbf57, },
    { 31, 0xa438, 0x7202, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xf8e5, },
    { 31, 0xa438, 0x8ff9, },
    { 31, 0xa438, 0xbf57, },
    { 31, 0xa438, 0x7502, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xfae5, },
    { 31, 0xa438, 0x8ffb, },
    { 31, 0xa438, 0xbf57, },
    { 31, 0xa438, 0x7802, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xfce5, },
    { 31, 0xa438, 0x8ffd, },
    { 31, 0xa438, 0xbf57, },
    { 31, 0xa438, 0x7b02, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xfee5, },
    { 31, 0xa438, 0x8fff, },
    { 31, 0xa438, 0xbf57, },
    { 31, 0xa438, 0x6c02, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xa102, },
    { 31, 0xa438, 0x13ee, },
    { 31, 0xa438, 0x8ffc, },
    { 31, 0xa438, 0x80ee, },
    { 31, 0xa438, 0x8ffd, },
    { 31, 0xa438, 0x00ee, },
    { 31, 0xa438, 0x8ffe, },
    { 31, 0xa438, 0x80ee, },
    { 31, 0xa438, 0x8fff, },
    { 31, 0xa438, 0x00af, },
    { 31, 0xa438, 0x8599, },
    { 31, 0xa438, 0xa101, },
    { 31, 0xa438, 0x0cbf, },
    { 31, 0xa438, 0x534c, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8a1, },
    { 31, 0xa438, 0x0303, },
    { 31, 0xa438, 0xaf85, },
    { 31, 0xa438, 0x77bf, },
    { 31, 0xa438, 0x5322, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8a1, },
    { 31, 0xa438, 0x8b02, },
    { 31, 0xa438, 0xae03, },
    { 31, 0xa438, 0xaf86, },
    { 31, 0xa438, 0x64e0, },
    { 31, 0xa438, 0x8ff8, },
    { 31, 0xa438, 0xe18f, },
    { 31, 0xa438, 0xf9bf, },
    { 31, 0xa438, 0x8684, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9e0, },
    { 31, 0xa438, 0x8ffa, },
    { 31, 0xa438, 0xe18f, },
    { 31, 0xa438, 0xfbbf, },
    { 31, 0xa438, 0x8687, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9e0, },
    { 31, 0xa438, 0x8ffc, },
    { 31, 0xa438, 0xe18f, },
    { 31, 0xa438, 0xfdbf, },
    { 31, 0xa438, 0x868a, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9e0, },
    { 31, 0xa438, 0x8ffe, },
    { 31, 0xa438, 0xe18f, },
    { 31, 0xa438, 0xffbf, },
    { 31, 0xa438, 0x868d, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9af, },
    { 31, 0xa438, 0x867f, },
    { 31, 0xa438, 0xbf53, },
    { 31, 0xa438, 0x2202, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xa144, },
    { 31, 0xa438, 0x3cbf, },
    { 31, 0xa438, 0x547b, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8e4, },
    { 31, 0xa438, 0x8ff8, },
    { 31, 0xa438, 0xe58f, },
    { 31, 0xa438, 0xf9bf, },
    { 31, 0xa438, 0x547e, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8e4, },
    { 31, 0xa438, 0x8ffa, },
    { 31, 0xa438, 0xe58f, },
    { 31, 0xa438, 0xfbbf, },
    { 31, 0xa438, 0x5481, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8e4, },
    { 31, 0xa438, 0x8ffc, },
    { 31, 0xa438, 0xe58f, },
    { 31, 0xa438, 0xfdbf, },
    { 31, 0xa438, 0x5484, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8e4, },
    { 31, 0xa438, 0x8ffe, },
    { 31, 0xa438, 0xe58f, },
    { 31, 0xa438, 0xffbf, },
    { 31, 0xa438, 0x5322, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8a1, },
    { 31, 0xa438, 0x4448, },
    { 31, 0xa438, 0xaf85, },
    { 31, 0xa438, 0xa7bf, },
    { 31, 0xa438, 0x5322, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xc8a1, },
    { 31, 0xa438, 0x313c, },
    { 31, 0xa438, 0xbf54, },
    { 31, 0xa438, 0x7b02, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xf8e5, },
    { 31, 0xa438, 0x8ff9, },
    { 31, 0xa438, 0xbf54, },
    { 31, 0xa438, 0x7e02, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xfae5, },
    { 31, 0xa438, 0x8ffb, },
    { 31, 0xa438, 0xbf54, },
    { 31, 0xa438, 0x8102, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xfce5, },
    { 31, 0xa438, 0x8ffd, },
    { 31, 0xa438, 0xbf54, },
    { 31, 0xa438, 0x8402, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xe48f, },
    { 31, 0xa438, 0xfee5, },
    { 31, 0xa438, 0x8fff, },
    { 31, 0xa438, 0xbf53, },
    { 31, 0xa438, 0x2202, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xa131, },
    { 31, 0xa438, 0x03af, },
    { 31, 0xa438, 0x85a7, },
    { 31, 0xa438, 0xd480, },
    { 31, 0xa438, 0x00bf, },
    { 31, 0xa438, 0x8684, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9bf, },
    { 31, 0xa438, 0x8687, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9bf, },
    { 31, 0xa438, 0x868a, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9bf, },
    { 31, 0xa438, 0x868d, },
    { 31, 0xa438, 0x0252, },
    { 31, 0xa438, 0xa9ef, },
    { 31, 0xa438, 0x95fd, },
    { 31, 0xa438, 0xfc04, },
    { 31, 0xa438, 0xf0d1, },
    { 31, 0xa438, 0x2af0, },
    { 31, 0xa438, 0xd12c, },
    { 31, 0xa438, 0xf0d1, },
    { 31, 0xa438, 0x44f0, },
    { 31, 0xa438, 0xd146, },
    { 31, 0xa438, 0xbf86, },
    { 31, 0xa438, 0xa102, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xbf86, },
    { 31, 0xa438, 0xa102, },
    { 31, 0xa438, 0x52c8, },
    { 31, 0xa438, 0xd101, },
    { 31, 0xa438, 0xaf06, },
    { 31, 0xa438, 0xa570, },
    { 31, 0xa438, 0xce42, },
    { 31, 0xa436, 0xb818, },
    { 31, 0xa438, 0x043d, },
    { 31, 0xa436, 0xb81a, },
    { 31, 0xa438, 0x06a3, },
    { 31, 0xa436, 0xb81c, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb81e, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb850, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb852, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb878, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb884, },
    { 31, 0xa438, 0xffff, },
    { 31, 0xa436, 0xb832, },
    { 31, 0xa438, 0x0003, },

};

static const MMD_REG Rtl8226b_data_ramcode[] =
{

};

static const MMD_REG Rtl8226b_isram_patch[] =
{

};

static BOOL
Rtl8226b_wait_for_bit(
    IN HANDLE hDevice,
    IN UINT16 dev,
    IN UINT16 addr,
    IN UINT16 mask,
    IN BOOL   set,
    IN UINT16 timeoutms)
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    while (--timeoutms)
    {
        status = MmdPhyRead(hDevice, MMD_VEND2, addr, &phydata);
        if (!status)
            goto exit;

        if (!set)
            phydata = ~phydata;

        if ((phydata & mask) == mask)
            return SUCCESS;

        Sleep(1);
    }

    osal_printf("Timeout (dev=%02x addr=0x%02x mask=0x%02x timeout=%d)\n",
                dev, addr, mask, timeoutms);

exit:
    return FAILURE;
}

/*
pin  |  link speed														  | active(Tx/Rx)
     |  10Mbps		100Mbps		1000Mbps	2.5Gbps		1Gbps(2.5G lite)  |
-----+--------------------------------------------------------------------+--------------
LED0 |  31.0xD032.0	31.0xD032.1	31.0xD032.2	31.0xD032.5	31.0xD032.7       | 31.0xD040.0
LED1 |  31.0xD034.0	31.0xD034.1	31.0xD034.2	31.0xD034.5	31.0xD034.7       | 31.0xD040.1
LED2 |  31.0xD036.0	31.0xD036.1	31.0xD036.2	31.0xD036.5	31.0xD036.7       | 31.0xD040.2
 */
void led_init(HANDLE hDevice)
{
    // TODO: set RTL8221B port's LED here depending on hardware configuration
    //       and disable 8221B LED setting in board_rtl8198d.c

    // example code: if the board use LED1 (orange) for 10/100Mbps and LED2 (green) for 1000Mbps
    //MmdPhyWrite(hDevice, MMD_VEND2, 0xD032, 0);
    //MmdPhyWrite(hDevice, MMD_VEND2, 0xD034, 3);
    //MmdPhyWrite(hDevice, MMD_VEND2, 0xD036, 0x84);
    MmdPhyWrite(hDevice, MMD_VEND2, 0xD032, 0xA7);	/* Set LED 0 for 10/100/1g/2.5g/2.5glite */
    //MmdPhyWrite(hDevice, MMD_VEND2, 0xD040, 0x1);	/* active(Tx/Rx) LED0 */
    return;
}

BOOLEAN
Rtl8226b_phy_init(
    IN HANDLE hDevice,
    IN PHY_LINK_ABILITY *pphylinkability,
    IN BOOL singlephy
)
{
    BOOL status = FAILURE;
    UINT16 i = 0;   /* SW_SDK: use UINT16 instead of UINT8, for MMD_REG array may over 255 entries */
    UINT16 phydata = 0,rev_num,mod_num;
    const UINT16 patchver = 0x0020, patchaddr = 0x8024;

    // Polling PHY Status
    status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xA420, 0x3, 1, 100);
    if (status != SUCCESS)
        goto exit;

    // Check phy id, no need to init 8221B_VB/VM
    status = MmdPhyRead(hDevice, MMD_PMAPMD, 0x03, &phydata);
    if (status != SUCCESS)
        goto exit;
    rev_num = phydata&0x000f;
    mod_num = (phydata>>4)&0x003f;

    if ((rev_num == 8) &&(mod_num == 4))
    {
        osal_printf("PHY chip: RTL8226B/RTL8221B (id= 0x%x), API version: 0x%x\n",phydata,patchver);
    }
    else
    {
        osal_printf("PHY chip: RTL8221B-VB (id= 0x%x), API version: 0x%x\n",phydata,patchver);

        status = MmdPhyRead(hDevice, MMD_VEND1, 0x75B5, &phydata);
        if (status == SUCCESS) {
            phydata = (phydata & ~0xF) | 0x4; // update IPG from 6+2 to 4+2
            MmdPhyWrite(hDevice, MMD_VEND1, 0x75B5, phydata);
        }
        // set standard phy reg 4 bit 10 (PAUSE) and bit 11 (ASM_DIR) to 1
        status = MmdPhyRead(hDevice, MMD_AN, 0x10, &phydata);
        if (status == SUCCESS) {
            MmdPhyWrite(hDevice, MMD_AN, 0x10, phydata | 0xC00);
        }
		
        goto exit;
    }

    // MMD 31.0xA436[15:0] = 0x801E
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x801E);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    // Already patched.
    if (phydata == patchver)
    {
        status = 1;
        goto exit;
    }
    else
    {
        // Patch request & wait patch_rdy (for normal patch flow - Driver Initialize)
        // MMD 31.0xB820[4] = 1'b1     //(patch request)
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_4;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        //wait for patch ready = 1 (MMD 31.0xB800[6])
        status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xB800, BIT_6, 1, 100);
        if (status != SUCCESS)
            goto exit;

        //Set patch_key & patch_lock
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, patchaddr);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x3701
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, 0x3701);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA436[15:0] = 0xB82E
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0xB82E);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0001
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, 0x0001);
        if (status != SUCCESS)
            goto exit;

        // NC & UC patch
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_7;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch nc0
        for(i=0; i<sizeof(Rtl8226b_n0_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n0_ramcode[i].dev, Rtl8226b_n0_ramcode[i].addr, Rtl8226b_n0_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // patch nc1
        for(i=0; i<sizeof(Rtl8226b_n1_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n1_ramcode[i].dev, Rtl8226b_n1_ramcode[i].addr, Rtl8226b_n1_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }


        // patch nc2
        for(i=0; i<sizeof(Rtl8226b_n2_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n2_ramcode[i].dev, Rtl8226b_n2_ramcode[i].addr, Rtl8226b_n2_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // patch uc2
        for(i=0; i<sizeof(Rtl8226b_uc2_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_uc2_ramcode[i].dev, Rtl8226b_uc2_ramcode[i].addr, Rtl8226b_uc2_ramcode[i].value);

            if (status != SUCCESS)
                goto exit;
        }

        // MMD 31.0xB820[7] = 1'b0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_7);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch uc
        for(i=0; i<sizeof(Rtl8226b_uc_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_uc_ramcode[i].dev, Rtl8226b_uc_ramcode[i].addr, Rtl8226b_uc_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // GPHY OCP 0xB896 bit[0] = 0x0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB896, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_0);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB896, phydata);
        if (status != SUCCESS)
            goto exit;

        // GPHY OCP 0xB892 bit[15:8] = 0x0
        phydata = 0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB892, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch ram code
        for(i=0; i<sizeof(Rtl8226b_data_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_data_ramcode[i].dev, Rtl8226b_data_ramcode[i].addr, Rtl8226b_data_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }

        // GPHY OCP 0xB896 bit[0] = 0x1
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB896, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_0;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB896, phydata);
        if (status != SUCCESS)
            goto exit;

        // Clear patch_key & patch_lock
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0000
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xB82E[0] = 1'b0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB82E, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_0);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB82E, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA436[15:0] = patch_key_addr
        phydata = patchaddr;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0000
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;

        // Release patch request
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_4);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xB800, BIT_6, 0, 100);
        if (status != SUCCESS)
            goto exit;

        // Lock Main
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa46A, 0x0302);
        if (status != SUCCESS)
            goto exit;

// GPHY REG
        // Patch XG INRX parameters
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xac46, 0xB794);
        if (status != SUCCESS)
            goto exit;

        // Patch Fnet/ Giga CHNEST
        // normal patch
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa412, 0x0200);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa5d4, 0x0081);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xad30, 0x0A57);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xad30, 0x0A55);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87c, 0x80F5);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87e, 0x760E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87c, 0x8107);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87e, 0x360E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87c, 0x8551);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87e, 0x80E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xbf00, 0xB202);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xbf46, 0x0300);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8044);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x804A);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8050);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8056);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x805C);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8062);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8068);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x806E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8074);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x807A);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8045);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x804B);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8051);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8057);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x805D);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8063);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8069);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x806F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8075);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x807B);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA4CA, 0x6A50);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FF4);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x0000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FF6);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x03E8);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FF8);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FFA);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FFC);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FFE);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8015);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0xF9BF);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd12A, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd12C, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd144, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd146, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xbf84, 0xAC00);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8170);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0xd8a0);
        if (status != SUCCESS)
            goto exit;

        // Release Lock Main
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa46A, 0x0300);
        if (status != SUCCESS)
            goto exit;
//--------------- SDS patch --------------n

        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x75B5, 0xE086);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x75B6, 0x0024);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x75B1, 0x0013);
        if (status != SUCCESS)
            goto exit;

//ISRAM PATCH
        if (singlephy)
        {
            for(i=0; i<sizeof(Rtl8226b_isram_patch)/sizeof(MMD_REG); i++)
            {
                status = MmdPhyWrite(hDevice, Rtl8226b_isram_patch[i].dev, Rtl8226b_isram_patch[i].addr, Rtl8226b_isram_patch[i].value);
                if (status != SUCCESS)
                    goto exit;
            }
        }

        // Update patch version
        phydata = 0x801E;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] =  driver_note_ver
        phydata = patchver;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;

        // PHYRST & Restart Nway
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, 0x9200);
        if (status != SUCCESS)
            goto exit;
    }
#if 0
	// if 8221B is connected to port 5,
	// disable phy 2.5G ability due to port 5 does not support 2.5G.
	MmdPhyRead(hDevice, MMD_AN, 0x20, &phydata);
	MmdPhyWrite(hDevice, MMD_AN, 0x20, (phydata & ~0x80));
#endif

exit:
#ifdef CONFIG_SWITCH_INIT_LINKDOWN
    Rtl8226b_enable_set(hDevice, DISABLED);
#endif

    led_init(hDevice);
    return status;
}

