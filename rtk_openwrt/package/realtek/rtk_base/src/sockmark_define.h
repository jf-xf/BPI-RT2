/*
 * sockmark_define.h --- header file for socket mark value define
 */

/*QoS definitions, and pass these information to linux kernel

 * wan 802.1p mark 0-2 bit: 802.1p
 * We can not use 0 to as tc handle. So we use (i+1).
 *======================================================================
 *
 * For port based/protocol based/Source IP based ....etc VLAN function.
 *
 * Using Mark to let driver know how to insert 802.1Q header.
 * The rule is as following design.
 * Meter index         [31:27](5 bits): SW/HW meter index
 * Meter index enabled [26:26](1 bits): SW/HW meter enable
 * Reserved            [25:19](7 bits): Reserved
 * WAN Mark 	       [18:13](6 bits): WAN MARK used for policy route
 * Qos Entry	[12:7]( 6 bits): Qos Entry ID
 * SW QID		[6:4]( 4 bits): Queue ID (0~7)
 * 1p Enable	       [3:3](1 bit): 802.1p remarking enable
 * 802.1p		[2:0]( 3 bits): 802.1p value. (0~7)
 *
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * | Meter index [27:31] |Meter enable [26:26]|Reserved [25:19]|WAN Mark|QosEntryID|SWQID|e| p   |
 * |       (5bits)       |   	(1bits)       |     7bits      |  6bits |[12:7](6b)|[6:4]|1|[2:0]|
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *======================================================================*/


#ifndef INCLUDE_SOCKMARKDEFINE_H
#define INCLUDE_SOCKMARKDEFINE_H
/* 802.1p remarking: mark[0:2] */
#define SOCK_MARK_8021P_START                                       0
#define SOCK_MARK_8021P_END                                         2
#define SOCK_MARK_8021P_BIT_NUM                                     (SOCK_MARK_8021P_END-SOCK_MARK_8021P_START+1)
#define SOCK_MARK_8021P_BIT_MASK                                    (((1<<SOCK_MARK_8021P_BIT_NUM)-1) << SOCK_MARK_8021P_START)
#define SOCK_MARK_MARK_TO_8021P(MARK)                               ((MARK & SOCK_MARK_8021P_BIT_MASK) >> SOCK_MARK_8021P_START)
#define SOCK_MARK_8021P_TO_MARK(PRI_VAL)                            (PRI_VAL << SOCK_MARK_8021P_START)

/* 802.1p remarking enable: mark[3:3] */
#define SOCK_MARK_8021P_ENABLE_START                                3
#define SOCK_MARK_8021P_ENABLE_END                                  3
#define SOCK_MARK_8021P_ENABLE_BIT_NUM                              (SOCK_MARK_8021P_ENABLE_END-SOCK_MARK_8021P_ENABLE_START+1)
#define SOCK_MARK_8021P_ENABLE_BIT_MASK                             (((1<<SOCK_MARK_8021P_ENABLE_BIT_NUM)-1) << SOCK_MARK_8021P_ENABLE_START)
#define SOCK_MARK_MARK_TO_8021P_ENABLE(MARK)                        ((MARK & SOCK_MARK_8021P_ENABLE_BIT_MASK) >> SOCK_MARK_8021P_ENABLE_START)
#define SOCK_MARK_8021P_ENABLE_TO_MARK(PRI_ENABLE)                  (PRI_ENABLE << SOCK_MARK_8021P_ENABLE_START)

/* QoS queue ID: mark[4:6] */
#define SOCK_MARK_QOS_SWQID_START                                   4
#define SOCK_MARK_QOS_SWQID_END                                     6
#define SOCK_MARK_QOS_SWQID_BIT_NUM                                 (SOCK_MARK_QOS_SWQID_END-SOCK_MARK_QOS_SWQID_START+1)
#define SOCK_MARK_QOS_SWQID_BIT_MASK                                (((1<<SOCK_MARK_QOS_SWQID_BIT_NUM)-1) << SOCK_MARK_QOS_SWQID_START)
#define SOCK_MARK_MARK_TO_QOS_SWQID_ENABLE(MARK)                    ((MARK & SOCK_MARK_QOS_SWQID_BIT_MASK) >> SOCK_MARK_QOS_SWQID_START)
#define SOCK_MARK_QOS_SWQID_TO_MARK(QOS_SWQID)                      (QOS_SWQID << SOCK_MARK_QOS_SWQID_START)

/* WAN index for policy route: mark[18:13] */
#define SOCK_MARK_WAN_INDEX_START                                   13
#define SOCK_MARK_WAN_INDEX_END                                     18
#define SOCK_MARK_WAN_INDEX_BIT_NUM                                 (SOCK_MARK_WAN_INDEX_END-SOCK_MARK_WAN_INDEX_START+1)
#define SOCK_MARK_WAN_INDEX_BIT_MASK                                (((1<<SOCK_MARK_WAN_INDEX_BIT_NUM)-1) << SOCK_MARK_WAN_INDEX_START)
#define SOCK_MARK_MARK_TO_WAN_INDEX(MARK)                           ((MARK & SOCK_MARK_WAN_INDEX_BIT_MASK) >> SOCK_MARK_WAN_INDEX_START)
#define SOCK_MARK_WAN_INDEX_TO_MARK(WAN_INDEX)                      (WAN_INDEX << SOCK_MARK_WAN_INDEX_START)

#if 1//def CONFIG_RTK_SKB_MARK2
/* [Mark2] Forward by PS: mark2[0:0] */
#define SOCK_MARK2_FORWARD_BY_PS_START                              0
#define SOCK_MARK2_FORWARD_BY_PS_END                                0
#define SOCK_MARK2_FORWARD_BY_PS_BIT_NUM                            (SOCK_MARK2_FORWARD_BY_PS_END-SOCK_MARK2_FORWARD_BY_PS_START+1)
#define SOCK_MARK2_FORWARD_BY_PS_BIT_MASK                           (((1<<SOCK_MARK2_FORWARD_BY_PS_BIT_NUM)-1) << SOCK_MARK2_FORWARD_BY_PS_START)
#define SOCK_MARK2_MARK_TO_FORWARD_BY_PS(MARK)                      ((MARK & SOCK_MARK2_FORWARD_BY_PS_BIT_MASK) >> SOCK_MARK2_FORWARD_BY_PS_START)
#define SOCK_MARK2_FORWARD_BY_PS_TO_MARK(MARK)                      (MARK << SOCK_MARK2_FORWARD_BY_PS_START)
#define SOCK_MARK2_FORWARD_BY_PS_GET(MARK)                          ((MARK & SOCK_MARK2_FORWARD_BY_PS_BIT_MASK) >> SOCK_MARK2_FORWARD_BY_PS_START)
#define SOCK_MARK2_FORWARD_BY_PS_SET(MARK, V)                       ((MARK & ~SOCK_MARK2_FORWARD_BY_PS_BIT_MASK) | (V << SOCK_MARK2_FORWARD_BY_PS_START))

/* HW meter enable: mark2[1:1] */
#define SOCK_MARK2_METER_HW_INDEX_ENABLE_START                      1
#define SOCK_MARK2_METER_HW_INDEX_ENABLE_END                        1
#define SOCK_MARK2_METER_HW_INDEX_ENABLE_BIT_NUM					(SOCK_MARK2_METER_HW_INDEX_ENABLE_END-SOCK_MARK2_METER_HW_INDEX_ENABLE_START+1)
#define SOCK_MARK2_METER_HW_INDEX_ENABLE_BIT_MASK					(unsigned long long)(((1<<SOCK_MARK2_METER_HW_INDEX_ENABLE_BIT_NUM)-1) << SOCK_MARK2_METER_HW_INDEX_ENABLE_START)
#define SOCK_MARK2_MARK_TO_METER_HW_INDEX_ENABLE(MARK)     			(unsigned long long)((MARK & SOCK_MARK2_METER_HW_INDEX_ENABLE_BIT_MASK) >> SOCK_MARK2_METER_HW_INDEX_ENABLE_START)
#define SOCK_MARK2_METER_HW_INDEX_ENABLE_TO_MARK(MARK)              (unsigned long long)(MARK << SOCK_MARK2_METER_HW_INDEX_ENABLE_START)
#define SOCK_MARK2_METER_HW_INDEX_GET(MARK)                         (unsigned long long)((MARK & SOCK_MARK2_METER_HW_INDEX_ENABLE_BIT_MASK) >> SOCK_MARK2_METER_HW_INDEX_ENABLE_START)
#define SOCK_MARK2_METER_HW_INDEX_SET(MARK, V)                      (unsigned long long)((MARK & ~SOCK_MARK2_METER_HW_INDEX_ENABLE_BIT_MASK) | (V << SOCK_MARK2_METER_HW_INDEX_ENABLE_START))

/* Meter index enabled: mark[2:2] */
#define SOCK_MARK2_METER_INDEX_ENABLE_START                         2
#define SOCK_MARK2_METER_INDEX_ENABLE_END                           2
#define SOCK_MARK2_METER_INDEX_ENABLE_BIT_NUM                       (SOCK_MARK2_METER_INDEX_ENABLE_END-SOCK_MARK2_METER_INDEX_ENABLE_START+1)
#define SOCK_MARK2_METER_INDEX_ENABLE_BIT_MASK                      (unsigned long long)(((1<<SOCK_MARK2_METER_INDEX_ENABLE_BIT_NUM)-1) << SOCK_MARK2_METER_INDEX_ENABLE_START)
#define SOCK_MARK2_MARK_TO_METER_INDEX_ENABLE(MARK)                 (unsigned long long)((MARK & SOCK_MARK2_METER_INDEX_ENABLE_BIT_MASK) >> SOCK_MARK2_METER_INDEX_ENABLE_START)
#define SOCK_MARK2_METER_INDEX_ENABLE_TO_MARK(MARK)                 (unsigned long long)(MARK << SOCK_MARK2_METER_INDEX_ENABLE_START)
#define SOCK_MARK2_METER_INDEX_ENABLE_GET(MARK)                     (unsigned long long)((MARK & SOCK_MARK2_METER_INDEX_ENABLE_BIT_MASK) >> SOCK_MARK2_METER_INDEX_ENABLE_START)
#define SOCK_MARK2_METER_INDEX_ENABLE_SET(MARK, V)                  (unsigned long long)((MARK & ~SOCK_MARK2_METER_INDEX_ENABLE_BIT_MASK) | (V << SOCK_MARK2_METER_INDEX_ENABLE_START))

/* Meter index: mark[7:3] */
#define SOCK_MARK2_METER_INDEX_START                                3
#define SOCK_MARK2_METER_INDEX_END                                  7
#define SOCK_MARK2_METER_INDEX_BIT_NUM                              (SOCK_MARK2_METER_INDEX_END-SOCK_MARK2_METER_INDEX_START+1)
#define SOCK_MARK2_METER_INDEX_BIT_MASK                             (unsigned long long)(((1<<SOCK_MARK2_METER_INDEX_BIT_NUM)-1) << SOCK_MARK2_METER_INDEX_START)
#define SOCK_MARK2_MARK_TO_METER_INDEX(MARK)                        (unsigned long long)((MARK & SOCK_MARK2_METER_INDEX_BIT_MASK) >> SOCK_MARK2_METER_INDEX_START)
#define SOCK_MARK2_METER_INDEX_TO_MARK(MARK)                        (unsigned long long)(MARK << SOCK_MARK2_METER_INDEX_START)
#define SOCK_MARK2_METER_INDEX_GET(MARK) 							(unsigned long long)((MARK & SOCK_MARK2_METER_INDEX_BIT_MASK) >> SOCK_MARK2_METER_INDEX_START)
#define SOCK_MARK2_METER_INDEX_SET(MARK, V) 						(unsigned long long)((MARK & ~SOCK_MARK2_METER_INDEX_BIT_MASK) | (V << SOCK_MARK2_METER_INDEX_START))


/* MIB counter index enable: mark2[8:8] */
#define SOCK_MARK2_MIB_INDEX_ENABLE_START                           8
#define SOCK_MARK2_MIB_INDEX_ENABLE_END                             8
#define SOCK_MARK2_MIB_INDEX_ENABLE_BIT_NUM                         (SOCK_MARK2_MIB_INDEX_ENABLE_END-SOCK_MARK2_MIB_INDEX_ENABLE_START+1)
#define SOCK_MARK2_MIB_INDEX_ENABLE_BIT_MASK                        (unsigned long long)(((1<<SOCK_MARK2_MIB_INDEX_ENABLE_BIT_NUM)-1) << SOCK_MARK2_MIB_INDEX_ENABLE_START)
#define SOCK_MARK2_MARK_TO_MIB_INDEX_ENABLE(MARK)                   (unsigned long long)((MARK & SOCK_MARK2_MIB_INDEX_ENABLE_BIT_MASK) >> SOCK_MARK2_MIB_INDEX_ENABLE_START)
#define SOCK_MARK2_MIB_INDEX_ENABLE_TO_MARK(MARK)                   (unsigned long long)(MARK << SOCK_MARK2_MIB_INDEX_ENABLE_START)
#define SOCK_MARK2_MIB_INDEX_ENABLE_GET(MARK)                       (unsigned long long)((MARK & SOCK_MARK2_MIB_INDEX_ENABLE_BIT_MASK) >> SOCK_MARK2_MIB_INDEX_ENABLE_START)
#define SOCK_MARK2_MIB_INDEX_ENABLE_SET(MARK, V)                    (unsigned long long)((MARK & ~SOCK_MARK2_MIB_INDEX_ENABLE_BIT_MASK) | (V << SOCK_MARK2_MIB_INDEX_ENABLE_START))

/* [Mark2] mib index: mark[13:9] */
#define SOCK_MARK2_MIB_INDEX_START                                  9
#define SOCK_MARK2_MIB_INDEX_END                                    13
#define SOCK_MARK2_MIB_INDEX_BIT_NUM                                (SOCK_MARK2_MIB_INDEX_END-SOCK_MARK2_MIB_INDEX_START+1)
#define SOCK_MARK2_MIB_INDEX_BIT_MASK                               (unsigned long long)(((1<<SOCK_MARK2_MIB_INDEX_BIT_NUM)-1) << SOCK_MARK2_MIB_INDEX_START)
#define SOCK_MARK2_MARK_TO_MIB_INDEX(MARK)                          (unsigned long long)((MARK & SOCK_MARK2_MIB_INDEX_BIT_MASK) >> SOCK_MARK2_MIB_INDEX_START)
#define SOCK_MARK2_MIB_INDEX_TO_MARK(MARK)                          (unsigned long long)(MARK << SOCK_MARK2_MIB_INDEX_START)
#define SOCK_MARK2_MIB_INDEX_GET(MARK) 								(unsigned long long)((MARK & SOCK_MARK2_MIB_INDEX_BIT_MASK) >> SOCK_MARK2_MIB_INDEX_START)
#define SOCK_MARK2_MIB_INDEX_SET(MARK, V) 							(unsigned long long)((MARK & ~SOCK_MARK2_MIB_INDEX_BIT_MASK) | (V << SOCK_MARK2_MIB_INDEX_START))

/* [Mark2] stream ID enable bit [14:14] */
#define SOCK_MARK2_STREAMID_ENABLE_START                            14
#define SOCK_MARK2_STREAMID_ENABLE_END                              14
#define SOCK_MARK2_STREAMID_ENABLE_NUM                              (SOCK_MARK2_STREAMID_ENABLE_END-SOCK_MARK2_STREAMID_ENABLE_START+1)
#define SOCK_MARK2_STREAMID_ENABLE_MASK                             (unsigned long long)(((1<<SOCK_MARK2_STREAMID_ENABLE_NUM)-1) << SOCK_MARK2_STREAMID_ENABLE_START)
#define SOCK_MARK2_STREAMID_ENABLE_GET(MARK)                        (unsigned long long)((MARK & SOCK_MARK2_STREAMID_ENABLE_MASK) >> SOCK_MARK2_STREAMID_ENABLE_START)
#define SOCK_MARK2_STREAMID_ENABLE_SET(MARK, V)                     (unsigned long long)((MARK & ~SOCK_MARK2_STREAMID_ENABLE_MASK) | (V << SOCK_MARK2_STREAMID_ENABLE_START))


/* [Mark2] stream ID bit [21:15] */
#define SOCK_MARK2_STREAMID_START                                   15
#define SOCK_MARK2_STREAMID_END                                     21
#define SOCK_MARK2_STREAMID_NUM                                     (SOCK_MARK2_STREAMID_END-SOCK_MARK2_STREAMID_START+1)
#define SOCK_MARK2_STREAMID_MASK                                    (unsigned long long)(((1<<SOCK_MARK2_STREAMID_NUM)-1) << SOCK_MARK2_STREAMID_START)
#define SOCK_MARK2_STREAMID_GET(MARK)                               (unsigned long long)((MARK & SOCK_MARK2_STREAMID_MASK) >> SOCK_MARK2_STREAMID_START)
#define SOCK_MARK2_STREAMID_SET(MARK, V)                            (unsigned long long)((MARK & ~SOCK_MARK2_STREAMID_MASK) | (V << SOCK_MARK2_STREAMID_START))


/* Match MC/BC filter VEIPIF rule: mark2[60:60] */
#define SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_START                      60
#define SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_END                        60
#define SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_NUM                        (SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_END-SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_START+1)
#define SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_MASK                       (unsigned long long)((((unsigned long long)1<<SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_NUM)-1) << SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_START)
#define SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_GET(MARK)                  (unsigned long long)(((unsigned long long)MARK & SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_MASK) >> SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_START)
#define SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_SET(MARK, V)               (unsigned long long)(((unsigned long long)MARK & ~SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_MASK) | ((unsigned long long)V << SOCK_MARK2_BCMC_RULE_VEIPIF_MATCH_START))

/* [Mark2] Match MC/BC filter rule: mark2[61:61] */
#define SOCK_MARK2_BCMC_RULE_MATCH_START                             61
#define SOCK_MARK2_BCMC_RULE_MATCH_END                               61
#define SOCK_MARK2_BCMC_RULE_MATCH_NUM                               (SOCK_MARK2_BCMC_RULE_MATCH_END-SOCK_MARK2_BCMC_RULE_MATCH_START+1)
#define SOCK_MARK2_BCMC_RULE_MATCH_MASK                              (unsigned long long)((((unsigned long long)1<<SOCK_MARK2_BCMC_RULE_MATCH_NUM)-1) << SOCK_MARK2_BCMC_RULE_MATCH_START)
#define SOCK_MARK2_BCMC_RULE_MATCH_GET(MARK)                         (unsigned long long)(((unsigned long long)MARK & SOCK_MARK2_BCMC_RULE_MATCH_MASK) >> SOCK_MARK2_BCMC_RULE_MATCH_START)
#define SOCK_MARK2_BCMC_RULE_MATCH_SET(MARK, V)                      (unsigned long long)(((unsigned long long)MARK & ~SOCK_MARK2_BCMC_RULE_MATCH_MASK) | ((unsigned long long)V << SOCK_MARK2_BCMC_RULE_MATCH_START))

/* [Mark2] Local service accelerate bit [62:62] */
#define SOCK_MARK2_LOCAL_SERVICE_ACC_START                           62
#define SOCK_MARK2_LOCAL_SERVICE_ACC_END                             62
#define SOCK_MARK2_LOCAL_SERVICE_ACC_NUM                             (SOCK_MARK2_LOCAL_SERVICE_ACC_END-SOCK_MARK2_LOCAL_SERVICE_ACC_START+1)
#define SOCK_MARK2_LOCAL_SERVICE_ACC_MASK                            (unsigned long long)(((1<<SOCK_MARK2_LOCAL_SERVICE_ACC_NUM)-1) << SOCK_MARK2_LOCAL_SERVICE_ACC_START)
#define SOCK_MARK2_LOCAL_SERVICE_ACC_GET(MARK)                       (unsigned long long)((MARK & SOCK_MARK2_LOCAL_SERVICE_ACC_MASK) >> SOCK_MARK2_LOCAL_SERVICE_ACC_START)
#define SOCK_MARK2_LOCAL_SERVICE_ACC_SET(MARK, V)                    (unsigned long long)(MARK |= (V << SOCK_MARK2_LOCAL_SERVICE_ACC_START))

/* Skip FC Func: mark2[63:63] */
#define SOCK_MARK2_SKIP_FC_FUNC_START                                63
#define SOCK_MARK2_SKIP_FC_FUNC_END                                  63
#define SOCK_MARK2_SKIP_FC_FUNC_NUM                                  (SOCK_MARK2_SKIP_FC_FUNC_END-SOCK_MARK2_SKIP_FC_FUNC_START+1)
#define SOCK_MARK2_SKIP_FC_FUNC_MASK                                 (unsigned long long)(((1<<SOCK_MARK2_SKIP_FC_FUNC_NUM)-1) << SOCK_MARK2_SKIP_FC_FUNC_START)
#define SOCK_MARK2_SKIP_FC_FUNC_GET(MARK)                            (unsigned long long)((MARK & SOCK_MARK2_SKIP_FC_FUNC_MASK) >> SOCK_MARK2_SKIP_FC_FUNC_START)
#define SOCK_MARK2_SKIP_FC_FUNC_SET(MARK, V)                         (unsigned long long)((MARK & ~SOCK_MARK2_SKIP_FC_FUNC_MASK) | (V << SOCK_MARK2_SKIP_FC_FUNC_START))

#endif //end CONFIG_RTK_SKB_MARK2

#endif // INCLUDE_SOCKMARKDEFINE_H

