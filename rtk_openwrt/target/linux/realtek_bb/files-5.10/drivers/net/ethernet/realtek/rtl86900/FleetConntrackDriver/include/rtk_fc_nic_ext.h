#ifndef __RTK_FC_NIC_EXT_H__
#define __RTK_FC_NIC_EXT_H__

#if defined(CONFIG_RTK_L34_G3_PLATFORM)

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RTK_FC_WIFI_FF_DEVID(nh_priv) (nh_priv->wifi_sw_id)
#define RTK_FC_IS_WIFI_FF_UC(nh_priv) (nh_priv->isWifiFF)
#define RTK_FC_WIFI_FF_COS(nh_priv) (nh_priv->cos)

#else
#define RTK_FC_WIFI_FF_DEVID(nh_priv) (nh_priv->hdr_cpu->mdata_raw.mdata_l&0xffff)
#define RTK_FC_IS_WIFI_FF_UC(nh_priv) ((nh_priv->hdr_cpu->mdata_raw.mdata_l&0xffff) != 0)
#define RTK_FC_WIFI_FF_COS(nh_priv) (nh_priv->hdr_a->bits.cos)
#endif

#endif

#endif
