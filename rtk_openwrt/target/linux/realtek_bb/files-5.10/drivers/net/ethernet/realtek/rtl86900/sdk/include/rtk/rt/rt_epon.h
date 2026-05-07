/*
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 * *
 * $Revision: $
 * $Date: $
 *
 * Purpose : Definition of EPON MAC APIs
 *           If the system uses OAM APP provided by RTK, all MPCP procedule will handle by RTK OAM APP.
 *           User can bypass all MPCP register procedule, in this case users just need to care about PON queue relative API. 
 *           
 * Feature : The file includes the following modules and sub-modules
 *           Module Initialize 
 *              (1)EPON module initialize   
 *           EPON MPCP Layer
 *              (1)MPCP Register API
 *              (2)MPCP event Notify
 *              (3)MPCP Register packet Rx Noifty
 *              (4)EPON MAC churning decrytion API
 *              (5)EPON Mib counter retrieve API 
 *              (6)EPOM MPCP Threshold report API 
 *              (5)EPON Tx/Rx FEC API 
 *           OAM Tx/Rx  
 *              (1)OAM Tx/Rx API
 *              (2)OAM Dying gasp Tx API
 *           PON queue configuration
 */

#ifndef __RT_EPON_H__
#define __RT_EPON_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <hal/chipdef/chip.h>
#include <rtk/switch.h>

/*
 * Symbol Definition
 */
#define RT_EPON_KEY_SIZE 3
#define RT_EPON_MAX_QUEUE_PER_LLID 10


/*
 * Data Declaration
 */


typedef struct rt_epon_llid_entry_s
{  /*MPCP LLID information structure*/
    uint8          llidIdx;         /*indicate LLID index*/
    uint16         llid;            /*LLID value for this LLID entry*/
    rt_enable_t    valid;           /*set function - 0:disable ths LLID entry 1:enable this LLID entry
                                      get function - indicate valid status for this LLID entry
                                    */ 
    uint8          reportTimer;     /*specify the report timer for this LLID entry*/ 
    rt_enable_t    isReportTimeout; /*read only: indicate this LLID entry already report timeout*/
    rt_mac_t       mac;             /*MAC address for this LLID entry*/
}rt_epon_llid_entry_t;


typedef struct rt_epon_regReq_s
{   /*MPCP register status control structure*/
    uint8           llidIdx;       /*specify LLID index*/ 
    rt_mac_t        mac;           /*MAC address for this requested LLID index*/
    uint8           pendGrantNum;  /*specify Pending grants in RWGISTER_REQ mpcp packet*/
    rt_enable_t     doRequest;     /*0:EPON stop trigger mpcp register request
                                     1:EPON start trigger mpcp register request*/
}rt_epon_regReq_t;


typedef struct rt_epon_churningKeyEntry_s
{   /*churnning key structure*/
    uint8   llidIdx;
    uint8   keyIdx;   /*key index, it would be 0 or 1*/
    /*RT_EPON_KEY_SIZE for CTC is 3*/
    /*1G  EPON only use churningKey*/
    /*10G EPON use churningKey, churningKey1 and churningKey2*/
    uint8   churningKey[RT_EPON_KEY_SIZE];        
    uint8   churningKey1[RT_EPON_KEY_SIZE];
    uint8   churningKey2[RT_EPON_KEY_SIZE];
}rt_epon_churningKeyEntry_t;


typedef struct rt_epon_llidCounter_s
{   /*per LLID counter structure*/
    uint32  queueTxFrames[RT_EPON_MAX_QUEUE_PER_LLID]; /*per Queue Tx Frame counter*/
    uint32  mpcpTxReport;                              /*Tx Report Frame Counter for this LLID index*/
    uint32  mpcpRxGate;                                /*Rx Gate Frame Counter for this LLID index*/
    uint32  onuLlidNotBcst;                            /*Rx Frame Counter LLID is belong to this LLID index and mode bit=0*/
}rt_epon_llidCounter_t;


typedef struct rt_epon_counter_s
{
    rt_epon_llidCounter_t  llidIdxCnt; /*per LLID relative counter*/
    uint8   llidIdx; /*input: indicate LLID relative counter is get from which LLID index*/
    /*below counters are whole EPON MAC blobal counter*/
    uint32  mpcpRxDiscGate;       /*record discovery gate counter*/
    /*FEC relative counters*/
    uint32  fecCorrectedBlocks;   /*RFC4837 dot3EponFecCorrectedBlocks*/ 
    uint32  fecUncorrectedBlocks; /*RFC4837 dot3EponFecUncorrectableBlocks*/
    uint32  fecCodingVio;         /*RFC4837 dot3EponFecPCSCodingViolation*/
    uint32  notBcstBitLlid7fff;   /*Rx Frame Counter with LLID is 0x7FFF and mode bit=0*/
    uint32  notBcstBitNotOnuLlid; /*Rx Frame Counter with LLID is not match LLID value in LLID table and mode bit=0*/
    uint32  bcstBitPlusOnuLLid;   /*Rx Frame Counter with LLID is match LLID value in LLID table and mode bit=1*/
    uint32  bcstNotOnuLLid;       /*Rx Frame Counter with LLID is not match LLID value in LLID table and mode bit=1*/
    uint32  crc8Err;              /*Rx Frame Counter with preamble CRC8 error*/
    uint32  mpcpTxRegRequest;     /*Tx MPCP REGISTER_REQ Frame Counter*/
    uint32  mpcpTxRegAck;         /*Tx MPCP REGISTER_ACK Frame Counter*/
}rt_epon_counter_t;

typedef enum rt_epon_llid_scheduler_mode_e{
    RT_EPON_LLID_QUEUE_SCHEDULER_SP = 0, /* Strict Priority */
    RT_EPON_LLID_QUEUE_SCHEDULER_WRR = 1, /* Weighted Round Robin */
} rt_epon_llid_scheduler_mode_t;

typedef struct rt_epon_queueCfg_s{
    rt_epon_llid_scheduler_mode_t       scheduleType; /*specify scdedule type of this Queue*/
    uint32                              cir;     /*Commited rate.  The scheduler will service queues which egress rate is under CIR first.*/
    uint32                              pir;     /*The PIR rate setting is use to limit the max egress rate for a queue.*/ 
    uint32                              weight;  /*weight will specify the WRR weight*/
}rt_epon_queueCfg_t;



typedef enum rt_epon_register_pkt_flag_e{
    /*this flag specify the flag filed in mpcp register packet*/
    RT_EPON_REGISTER_FLAG_RESERVED = 0, 
    RT_EPON_REGISTER_FLAG_REREGISTER = 1, 
    RT_EPON_REGISTER_FLAG_DEREGISTER = 2, 
    RT_EPON_REGISTER_FLAG_ACK = 3, 
    RT_EPON_REGISTER_FLAG_NACK = 4 
} rt_epon_register_pkt_flag_t;

typedef struct rt_epon_report_threshold_e
{   /*use to confiurate EPON mpcp threshold report format and threshold value
      user must make sure th3>th2>th1*/
    uint8          levelNum;  /*how many queue set will enable in report packet*/
    uint16         th1;       /*threshold value for queue set 0*/
    uint16         th2;       /*threshold value for queue set 1*/
    uint16         th3;       /*threshold value for queue set 2*/
}rt_epon_report_threshold_t;



typedef struct rt_epon_mpcp_info_e
{
    uint32 laser_on_time; 
        /* Indicates the final laser on time after MPCP
        registration process is completed; initially
        laser on time is configured by system startup
        configuration; ONU will update
        laser on time if laser on time in MPCP registration
        is more than the initial value. {0x01 ~ 0xff}*/
    uint32 laser_off_time; /* Indicates the final laser off time after MPCP
        registration process is completed; initially
        laser off time is configured by system startup
        configuration; ONU will update laser off time if laser
        off time in MPCP registration is more than the initial
        value. {0x01 ~ 0xff}*/
    uint32 sync_time; /* Indicates the synchronization time from MPCP
    discovery date and MPCP registration frame. Note that
    this field is only valid if ONU has been in registered
    state. {0x0000 ~ 0xffff} */
    uint32 onu_discovery_info; 
        /* Indicates the 10G MPCP discovery information in
        registration request.
        Bit[0] : ONU 1G upstream capability.
        Bit[1] : ONU 10G upstream capability.
        Bit[4] : ONU 1G registration attempt.
        Bit[5] : ONU 10G registration attempt. */
    uint32 olt_discovery_info; 
        /* Indicates the 10G MPCP discovery information in
        discovery gate. this field is read only for SW
        Bit[0] : OLT 1G upstream capability.
        Bit[1] : OLT 10G upstream capability.
        Bit[4] : OLT 1G window opening state.
        Bit[5] : OLT 10G window opening state.*/
} rt_epon_mpcp_info_t;


typedef struct rt_epon_register_pkt_info_s{
    /*use to specify mpcp information in received REGISTER packet*/
    uint16     assignPort;       /*Assigned port filed in REGISTER packet*/
    uint8      flag;             /*Flags filed in REGISTER packet*/
    uint16     syncTime;         /*Sync Time filed in REGISTER packet*/
    uint8      echoPendingGrant; /*Echoed pendding grants filed in REGISTER packet*/
}rt_epon_register_pkt_info_t;


typedef enum rt_epon_info_event_type_e
{/*mpcp register status change event type*/
    RT_EPON_INFO_MPCP_REG_SUCCESS,       /*mpcp register success event*/
    RT_EPON_INFO_MPCP_REG_FAIL,          /*mpcp register fail event*/
    RT_EPON_INFO_MPCP_GATE_TIMEOUT,      /*mpcp gate timeout event*/
    RT_EPON_INFO_LASER_OFF,              /*lost of signal event->laser off*/
    RT_EPON_INFO_LASER_RECOVER,          /*lost of signal event->laser recover*/
    RT_EPON_INFO_MPCP_TIMEDRIFT,         /*mpcp local timedrift event detected*/
    RT_EPON_INFO_MPCP_LLID_ENTRY_DISABLE,/*LLID entry state change to Disable state*/ 
    RT_EPON_INFO_MPCP_RX_REG_PKT,        /*mpcp REGISTERPacket Rx notify event
                                           not support in apollo family*/
    RT_EPON_INFO_END
}rt_epon_info_event_type_t;



typedef struct rt_epon_info_notify_e
{
    uint8                       llidIdx;  /*specfiy which LLID index trigger this event*/
    rt_epon_info_event_type_t   event;    /*specfiy event type*/
    uint32                      info;     /*event=RT_EPON_INFO_MPCP_RX_REG_PKT -> this field is specify flag filed in Register packet*/
}rt_epon_info_notify_t;


/*OAM Packet receive callback function prototype.*/
typedef uint32 (*rt_epon_oam_rx_callback)(uint32 msgLen,uint8 *pMsg, uint8 llidIdx);

/*MPCP Register Packet receive callback function prototype.*/
typedef uint32 (*rt_epon_mpcp_rx_callback)(rt_epon_register_pkt_info_t *info, uint8 llidIdx);

/*MPCP Register status change notify callback function prototype.*/
typedef uint32 (*rt_epon_info_notify_cb)(rt_epon_info_notify_t info);



/*
 * Function Declaration
 */


/* Function Name:
 *      rt_epon_init
 * Description:
 *      Initialize EPON MAC.
 * Input:
 *       None
 * Output:
 *       None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routine is used to initialize and bring up EPON MAC. 
 */
extern int32 rt_epon_init(void);

/* Function Name:
 *      rt_epon_llid_entry_get
 * Description:
 *      Get the MPCP LLID entry registration information.
 * Input:
 *      llidIdx - LLID table index
 * Output:
 *      pLlidEntry - LLID entry 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      System software could use this API to get EPON registration status.
 */
extern int32 rt_epon_llid_entry_get(rt_epon_llid_entry_t *pLlidEntry);  

/* Function Name:
 *      rt_epon_llid_entry_set
 * Description:
 *      Set the MPCP LLID entry registration information.
 * Input:
 *      llidIdx - LLID table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      System software could use this API to set EPON registration status.
 *      System software can force deregister this LLID by set pLlidEntry->valid=DISABLED 
 */
extern int32 rt_epon_llid_entry_set(rt_epon_llid_entry_t *pLlidEntry);

/* Function Name:
 *      rt_epon_registerReq_get
 * Description:
 *      Get register request relative parameter.
 * Input:
 *      pRegEntry - register request relative parament
 * Output:
 *       None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routine is used to get register request relative parameter.
 */
extern int32 rt_epon_registerReq_get(rt_epon_regReq_t *pRegEntry);

/* Function Name:
 *      rt_epon_registerReq_set
 * Description:
 *      Set register request relative parameter
 * Input:
 *       pRegEntry - register request relative parament 
 * Output:
 *       None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      System software should invoke this API every time ONU wants to start the EPON registration process. 
 *      To deregister certain LLID, this API should be invoked with parameter pRegEntry->doRequest = DISABLED.
 */
extern int32 rt_epon_registerReq_set(rt_epon_regReq_t *pRegEntry);

/* Function Name:
 *      rt_epon_churningKey_get
 * Description:
 *      Get churning key entry
 * Input:
 *       None
 * Output:
 *       pEntry - churning key relative parameter 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      These API routine are used to set or get the churning decryption active key index of LLID.
 */
extern int32 rt_epon_churningKey_get(rt_epon_churningKeyEntry_t *pEntry);

/* Function Name:
 *      rt_epon_churningKey_set
 * Description:
 *      Set churning key entry
 * Input:
 *       pEntry - churning key relative parameter 
 * Output:
 *       None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      These API routine are used to set or get the churning decryption active key index of LLID.
 */
extern int32 rt_epon_churningKey_set(rt_epon_churningKeyEntry_t *pEntry);



/* Function Name:
 *      rt_epon_churningStatus_get
 * Description:
 *      Get EPON tripple churning status
 * Input:
 *	    none
 * Output:
 *      pEnabled - pointer of status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_NULL_POINTER - NULL input parameter 
 * Note:
 *      Get churning key enable status. This API just keep OLT configuration.
 *      because EPON MAC do not need to keep churning state in ASIC
 *      EPON mac enable churning per packet by preamble indicator.   
 */
extern int32 rt_epon_churningStatus_get(rt_enable_t *pEnable);


/* Function Name:
 *      rt_epon_usFecState_get
 * Description:
 *      Get upstream FEC state for EPON mac
 * Input:
 *       None
 * Output:
 *       *pState - upstream FEC state 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routines is used to get the FEC state for upstream.
 */
extern int32 rt_epon_usFecState_get(rt_enable_t *pState);


/* Function Name:
 *      rt_epon_usFecState_set
 * Description:
 *      Set upstream FEC state for EPON mac
 * Input:
 *       state - upstream FEC state
 * Output:
 *       None 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routines is used to get the FEC state for upstream.
 */
extern int32 rt_epon_usFecState_set(rt_enable_t state);


/* Function Name:
 *      rt_epon_dsFecState_get
 * Description:
 *      Get downstream FEC state for EPON mac
 * Input:
 *       None
 * Output:
 *       *pState - down-stream FEC state 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routines is used to get the FEC state for downstream.
 */
extern int32 rt_epon_dsFecState_get(rt_enable_t *pState);


/* Function Name:
 *      rt_epon_dsFecState_set
 * Description:
 *      Set downstream FEC state for EPON mac
 * Input:
 *       state - down-stream FEC state
 * Output:
 *       None 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routines is used to get the FEC state for downstream.
 */
extern int32 rt_epon_dsFecState_set(rt_enable_t state);

/* Function Name:
 *      rt_epon_mibCounter_get
 * Description:
 *      Get the statistics for EPON port.
 * Input:
 *       None
 * Output:
 *       pCounter - EPON mib counter 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routine is used to retrieve statistics data for EPON mac.
 */
extern int32 rt_epon_mibCounter_get(rt_epon_counter_t *pCounter);


/* Function Name:
 *      rt_epon_mibCounter_reset
 * Description:
 *      Clear the statistics for EPON mac.
 * Input:
 *       None
 * Output:
 *       None 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routine is used to claer statistics data for EPON mac.
 */
extern int32 rt_epon_mibCounter_reset(void);

/* Function Name:
 *      rt_epon_losState_get
 * Description:
 *      Get laser lose of signal state.
 * Input:
 *      pState - LOS state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routine is used to get lose of signal state from laser input.
 */
extern int32 rt_epon_losState_get(rt_enable_t *pState);


/* Function Name:
 *      rt_epon_lofState_get
 * Description:
 *      Get laser lose of frame state.
 * Input:
 *      pState - LOF state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routine is used to get lose of frame state.
 */
extern int32 rt_epon_lofState_get(rt_enable_t *pState);




/* Function Name:
 *      rt_epon_mpcp_gate_timer_set
 * Description:
 *      Set mpcp gate expire timer and deregister state.
 * Input:
 *      gate_timer  -    gate expire timer
 *      deregistration -  if gate expired EPON MAC local deregister or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API is used to config gate expired timer and gate expired action
 *      deregistration == ENABLED  --> onu will trigger deregister when detect gate expired
 *      deregistration == DISABLED --> onu won't trigger deregister when detect gate expired
 */
extern int32 rt_epon_mpcp_gate_timer_set(uint32 gateTimer,rt_enable_t deregistration);



/* Function Name:
 *      rt_epon_mpcp_gate_timer_get
 * Description:
 *      Get mpcp gate expire timer and deregister state.
 * Input:
 *      *pGate_timer   -   gate expire timer
 *      *pDeregistration -  gate expired deregister or not
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API is used to get gate expired timer and gate expired action
 */
extern int32 rt_epon_mpcp_gate_timer_get(uint32 *pGateTimer,rt_enable_t *pDeregistration);


/* Function Name:
 *      rt_epon_ponQueue_set
 * Description:
 *      Configure scheduling type/weights of a given queue.
 * Input:
 *      llid    - logical llid index
 *      queueId  - logical llid queue id
 *      pQueuecfg  - pointer to the queue configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If given queue is not exist, API will add this queue to this LLID
 *      Configure or retrieve scheduling weights of a given queue(LLID+Queue id).
 *      Each LLID support 8 queues.   
 *      For modes which use RT_EPON_LLID_QUEUE_SCHEDULER_WRR it means WRR schedule mode,
 *       weight will specify the WRR weight for this queue.
 *      For modes which use RT_EPON_LLID_QUEUE_SCHEDULER_SP it means strict schedule mode,
 *       higher queue id means higher priority.  
 *      This API can configure pon queue egress rate: pir/cir
 *      cir: Commited rate for this queue. 
 *           The scheduler will service queues which egress rate is under CIR first.
 *      pir: The PIR rate setting is use to limit the max egress rate for a queue. 
 *           Once the egress rate reached the PIR rate, the scheduler will stop service this queue.    
 *      The unit of granularity is 8Kbps.
 */
extern int32 rt_epon_ponQueue_set(uint32 llid, uint32 queueId, rt_epon_queueCfg_t *pQueuecfg);

/* Function Name:
 *      rt_epon_ponQueue_get
 * Description:
 *      Retrieve scheduling type/weights of a given queue.
 * Input:
 *      llid    - logical llid index
 *      queueId  - logical llid queue id
 * Output:
 *      pQueuecfg - pointer to the queue configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Retrieve scheduling type/weights and queue egress rate of a given queue.    
 *      The unit of granularity is 8Kbps.
 */
extern int32 rt_epon_ponQueue_get(uint32 llid, uint32 queueId, rt_epon_queueCfg_t *pQueuecfg);

/* Function Name:
 *      rt_epon_ponQueue_del
 * Description:
 *      Delete the pon queue from given LLID
 * Input:
 *      llid     - logical llid index
 *      queueId  - logical llid queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API is used to delete queue from LLID  
 */
extern int32 rt_epon_ponQueue_del(uint32 llid, uint32 queueId);

/* Function Name:
 *      rt_epon_egrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate for EPON port.
 * Input:
 *      None
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      This API is used to get egress bandwidth control rate for EPON port.
 *      The unit of granularity is 1Kbps.
 */
extern int32 rt_epon_egrBandwidthCtrlRate_get(uint32 *pRate);

/* Function Name:
 *      rt_epon_egrBandwidthCtrlRate_set
 * Description:
 *      Set the egress bandwidth control rate for EPON port.
 * Input:
 *      rate - egress bandwidth control rate
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      This API is used to set egress bandwidth control rate for EPON port.
 *      The unit of granularity is 1Kbps.
 */
extern int32 rt_epon_egrBandwidthCtrlRate_set(uint32 rate);

/* Function Name:
 *      rt_epon_oam_tx
 * Description:
 *      Send OAM Packet to OLT from given LLID channel.
 * Input:
 *      msgLen - length of OAM Packet
 *      pMsg - content of OAM Packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API is used to send OAM message to OLT. 
 */
extern int32 rt_epon_oam_tx(uint32 msgLen,uint8 *pMsg,uint8 llidIdx);

/* Function Name:
 *      rt_epon_oam_rx_callback_register
 * Description:
 *      Register callback functions to handle the received OAM packets.
 * Input:
 *      oamRx - call back function of OAM Packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      These API is used to register callback functions to handle received OAM packets.
 */
extern int32 rt_epon_oam_rx_callback_register(rt_epon_oam_rx_callback oamRx);

/* Function Name:
 *      rt_epon_dyinggasp_set
 * Description:
 *      Save pre-built dying gasp OAM informational frame in driver.
 * Input:
 *      msgLen - length of dying gasp OAM Packet
 *      pMsg - content o dying gaspf OAM Packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      This API routine is used to store a pre-built dying gasp EPON informational OAM frame in driver. 
 *      The stored frame will be sent to OLT before ONU loses power. The OAM frame should be 64 bytes in length.
 */
extern int32 rt_epon_dyinggasp_set(uint32 msgLen,uint8 *pMsg);

/* Function Name:
 *      rt_epon_mpcp_info_get
 * Description:
 *      Get epon mpcp information
 * Input:
 * Output:
 *      info - mpcp relative information 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *
 *   This API routine is used to get generic ONU MPCP information and can be invoked at any time. 
 *   However some of fields like laser on laser_off sync_time and olt_discinfo will be valid if ONU is not in registered
 *   state. Note that field onu_discinfo and olt_discinfo are only appropriate for 10G ONU. 
 */
extern int32 rt_epon_mpcp_info_get(rt_epon_mpcp_info_t *info);




/* Function Name:
 *      rt_epon_mpcp_queue_threshold_set
 * Description:
 *      Set epon threshold report
 * Input:
 *      pThresholdRpt - threshole report setting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 * Note:
 *      These API routines are used to set or get queue threshold for the specified LLID instance. 
 *      In general, the threshold for the higher queue id should be always greater than that of the lower queue id.
 *      The the report level 1 for normal report type, max value is 4.
 */
extern int32
rt_epon_mpcp_queue_threshold_set(rt_epon_report_threshold_t *pThresholdRpt);

/* Function Name:
 *      rt_epon_mpcp_queue_threshold_get
 * Description:
 *      Get epon threshold report setting
 * Input:
 *      pThresholdRpt - threshole report setting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 * Note:
 */
extern int32
rt_epon_mpcp_queue_threshold_get(rt_epon_report_threshold_t *pThresholdRpt);


/* Function Name:
 *      rt_epon_mpcpRxregisterInfoCb_register
 * Description:
 *      Register callback functions to handle mpcp register packet
 * Input:
 *      mpcpRx - call back function of mpcp register packet info update
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      These API is used to register callback functions to handle mpcp register packet
 */
extern int32 rt_epon_mpcpRxregisterInfoCb_register(rt_epon_mpcp_rx_callback mpcpRx);


/* Function Name:
 *      rt_epon_info_notify_callback_register
 * Description:
 *      Send OAM Packet.
 * Input:
 *      callback - Register an event receiving callback for mpcp register status info change update
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      These API is used to register callback for mpcp register status info change update.
 *      Update info please check rt_epon_info_event_type_t 
 */
extern int32 rt_epon_info_notify_callback_register(rt_epon_info_notify_cb callback);


/* Function Name:
 *      rt_epon_ponLoopback_set
 * Description:
 *      Set the loopback for the PON port.
 * Input:
 *      enable - set loopback status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 * Note:
 *      Enable loopback by this API, packet ingress from EPON port will be looped back to OLT.
 */
extern int32 rt_epon_ponLoopback_set(uint32 llid,rt_enable_t enable);



#endif